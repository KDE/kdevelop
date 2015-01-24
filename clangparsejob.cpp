/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "clangparsejob.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <language/interfaces/icodehighlighting.h>

#include <language/backgroundparser/urlparselock.h>
#include <language/backgroundparser/backgroundparser.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>

#include <custom-definesandincludes/idefinesandincludesmanager.h>

#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "duchain/clanghelpers.h"
#include "duchain/clangpch.h"
#include "duchain/tuduchain.h"
#include "duchain/parsesession.h"
#include "duchain/clangindex.h"
#include "duchain/clangparsingenvironmentfile.h"
#include "util/clangdebug.h"
#include "util/clangtypes.h"

#include "clangsupport.h"
#include "documentfinderhelpers.h"

#include <QFile>
#include <QStringList>
#include <QFileInfo>
#include <QReadLocker>
#include <QProcess>
#include <memory>

using namespace KDevelop;

namespace {

QString findConfigFile(const QString& forFile, const QString& configFileName)
{
    QDir dir = QFileInfo(forFile).dir();
    while (dir.exists()) {
        const QFileInfo customIncludePaths(dir, configFileName);
        if (customIncludePaths.exists()) {
            return customIncludePaths.absoluteFilePath();
        }

        if (!dir.cdUp()) {
            break;
        }
    }

    return {};
}

Path::List readPathListFile(const QString& filepath)
{
    if (filepath.isEmpty()) {
        return {};
    }

    QFile f(filepath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString text = QString::fromLocal8Bit(f.readAll());
    const QStringList lines = text.split('\n', QString::SkipEmptyParts);
    Path::List paths(lines.length());
    std::transform(lines.begin(), lines.end(), paths.begin(), [] (const QString& line) { return Path(line); });
    return paths;
}

/**
 * File should contain the header to precompile and use while parsing
 * @returns the first path in the file
 */
Path userDefinedPchIncludeForFile(const QString& sourcefile)
{
    static const QString pchIncludeFilename = QStringLiteral(".kdev_pch_include");
    const auto paths = readPathListFile(findConfigFile(sourcefile, pchIncludeFilename));
    return paths.isEmpty() ? Path() : paths.first();
}

ProjectFileItem* findProjectFileItem(const IndexedString& url, bool* hasBuildSystemInfo)
{
    ProjectFileItem* file = nullptr;

    *hasBuildSystemInfo = false;
    for (auto project: ICore::self()->projectController()->projects()) {
        auto files = project->filesForPath(url);
        if (files.isEmpty()) {
            continue;
        }

        file = files.last();

        // A file might be defined in different targets.
        // Prefer file items defined inside a target with non-empty includes.
        for (auto f: files) {
            if (!dynamic_cast<ProjectTargetItem*>(f->parent())) {
                continue;
            }
            file = f;
            if (!IDefinesAndIncludesManager::manager()->includes(f, IDefinesAndIncludesManager::ProjectSpecific).isEmpty()) {
                break;
            }
        }
    }
    if (file && file->project()) {
        if (auto bsm = file->project()->buildSystemManager()) {
            *hasBuildSystemInfo = bsm->hasIncludesOrDefines(file);
        }
    }
    return file;
}

QString languageStandard(ProjectFileItem* item)
{
    static const QString defaultLanguageStandard = QStringLiteral("c++11");
    struct PathStandard {
        QString path;
        QString standard;
    };
    QList<PathStandard> paths;

    auto cfg = item->project()->projectConfiguration();
    auto grp = cfg->group("CustomDefinesAndIncludes");
    for (const QString& grpName : grp.groupList()) {
        if (grpName.startsWith("ProjectPath")) {
            KConfigGroup pathgrp = grp.group(grpName);
            PathStandard entry;
            entry.path = pathgrp.readEntry("Path", "");

            auto cgrp = pathgrp.group("Compiler");
            entry.standard = cgrp.readEntry("Standard", QString());
            if (entry.standard.isEmpty()) {
                continue;
            }
            paths.append(entry);
        }
    }

    if (paths.isEmpty()) {
        return defaultLanguageStandard;
    }

    const Path itemPath = item->path();
    const Path rootDirectory = item->project()->path();

    Path closestPath;
    QString standard;

    for (const auto& entry : paths) {
        Path targetDirectory = rootDirectory;

        targetDirectory.addPath(entry.path);

        if (targetDirectory == itemPath) {
            return entry.standard;;
        }

        if (targetDirectory.isParentOf(itemPath)) {
            if (closestPath.isEmpty() || targetDirectory.segments().size() > closestPath.segments().size()) {
                closestPath = targetDirectory;
                standard = entry.standard;
            }
        }
    }

    return standard.isEmpty() ? defaultLanguageStandard : standard;
}

ClangParsingEnvironmentFile* parsingEnvironmentFile(const TopDUContext* context)
{
    return dynamic_cast<ClangParsingEnvironmentFile*>(context->parsingEnvironmentFile().data());
}

bool hasTracker(const IndexedString& url)
{
    return ICore::self()->languageController()->backgroundParser()->trackerForUrl(url);
}

ParseSessionData::Ptr findParseSession(const IndexedString &file)
{
    DUChainReadLocker lock;
    const auto& context = DUChainUtils::standardContextForUrl(file.toUrl());
    if (context) {
        return ParseSessionData::Ptr(dynamic_cast<ParseSessionData*>(context->ast().data()));
    }
    return {};
}

}

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
    : ParseJob(url, languageSupport)
{
    const auto tuUrl = clang()->index()->translationUnitForUrl(url);
    bool hasBuildSystemInfo;
    if (auto file = findProjectFileItem(tuUrl, &hasBuildSystemInfo)) {
        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includes(file));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->defines(file));
        m_environment.setLanguageStandard(languageStandard(file));
    } else {
        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includes(tuUrl.str()));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->defines(tuUrl.str()));
        m_environment.setLanguageStandard(QStringLiteral("c++11"));
    }
    const bool isSource = ClangHelpers::isSource(tuUrl.str());
    m_environment.setQuality(
        isSource ? (hasBuildSystemInfo ? ClangParsingEnvironment::BuildSystem : ClangParsingEnvironment::Source)
        : ClangParsingEnvironment::Unknown
    );
    m_environment.setTranslationUnitUrl(tuUrl);

    Path::List projectPaths;
    const auto& projects = ICore::self()->projectController()->projects();
    projectPaths.reserve(projects.size());
    foreach (auto project, projects) {
        projectPaths.append(project->path());
    }
    m_environment.setProjectPaths(projectPaths);

    foreach(auto document, ICore::self()->documentController()->openDocuments()) {
        auto textDocument = document->textDocument();
        if (!textDocument || !textDocument->isModified() || !textDocument->url().isLocalFile()
            || !DocumentFinderHelpers::mimeTypesList().contains(textDocument->mimeType()))
        {
            continue;
        }
        m_unsavedFiles << UnsavedFile(textDocument->url().toLocalFile(), textDocument->textLines(textDocument->documentRange()));
        const IndexedString indexedUrl(textDocument->url());
        m_unsavedRevisions.insert(indexedUrl, ModificationRevision::revisionForFile(indexedUrl));
    }
}

ClangSupport* ClangParseJob::clang() const
{
    return static_cast<ClangSupport*>(languageSupport());
}

void ClangParseJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread */*thread*/)
{
    QReadLocker parseLock(languageSupport()->parseLock());

    if (abortRequested()) {
        return;
    }

    {
        const auto tuUrlStr = m_environment.translationUnitUrl().str();
        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includesInBackground(tuUrlStr));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->definesInBackground(tuUrlStr));
        m_environment.setPchInclude(userDefinedPchIncludeForFile(tuUrlStr));
    }

    if (abortRequested()) {
        return;
    }

    {
        UrlParseLock urlLock(document());
        if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
            return;
        }
    }

    ParseSession session(findParseSession(document()));
    if (abortRequested()) {
        return;
    }

    if (!session.data() && document() != m_environment.translationUnitUrl()) {
        // no cached data found for the current file, but maybe
        // we are lucky and can grab it from the TU context
        // this happens e.g. when originally a .cpp file is open and then one
        // of its included files is opened in the editor.
        session.setData(findParseSession(m_environment.translationUnitUrl()));
        if (abortRequested()) {
            return;
        }
    }

    if (!session.data() || !session.reparse(m_unsavedFiles, m_environment)) {
        session.setData(createSessionData());
    }

    if (!session.unit()) {
        // failed to parse file, unpin and don't try again
        clang()->index()->unpinTranslationUnitForUrl(document());
        return;
    }

    if (!clang_getFile(session.unit(), document().byteArray())) {
        // this parse job's document does not exist in the pinned translation unit
        // so we need to unpin and re-add this document
        // Ideally we'd reset m_environment and session, but this is much simpler
        // and shouldn't be a common case
        clang()->index()->unpinTranslationUnitForUrl(document());
        ICore::self()->languageController()->backgroundParser()->addDocument(document(), minimumFeatures(), priority());
        return;
    }

    Imports imports = ClangHelpers::tuImports(session.unit());
    IncludeFileContexts includedFiles;
    if (auto pch = clang()->index()->pch(m_environment)) {
        auto pchFile = pch->mapFile(session.unit());
        includedFiles = pch->mapIncludes(session.unit());
        includedFiles.insert(pchFile, pch->context());
        auto tuFile = clang_getFile(session.unit(), m_environment.translationUnitUrl().byteArray());
        imports.insert(tuFile, { pchFile, CursorInRevision(0, 0) } );
    }

    if (abortRequested()) {
        return;
    }

    auto context = ClangHelpers::buildDUChain(session.mainFile(), imports, session,
                                              minimumFeatures(), includedFiles, clang()->index());
    setDuChain(context);

    if (abortRequested()) {
        return;
    }

    {
        if (minimumFeatures() & TopDUContext::AST) {
            DUChainWriteLocker lock;
            context->setAst(IAstContainer::Ptr(session.data()));
        }
#ifdef QT_DEBUG
        DUChainReadLocker lock;
        auto file = parsingEnvironmentFile(context);
        Q_ASSERT(file);
        // verify that features and environment where properly set in ClangHelpers::buildDUChain
        Q_ASSERT(file->featuresSatisfied(TopDUContext::Features(minimumFeatures() & ~TopDUContext::ForceUpdateRecursive)));
#endif
    }

    foreach(const auto& context, includedFiles) {
        if (!context) {
            continue;
        }
        {
            // prefer the editor modification revision, instead of the on-disk revision
            auto it = m_unsavedRevisions.find(context->url());
            if (it != m_unsavedRevisions.end()) {
                DUChainWriteLocker lock;
                auto file = parsingEnvironmentFile(context);
                Q_ASSERT(file);
                file->setModificationRevision(it.value());
            }
        }
        if (::hasTracker(context->url())) {
            if (clang()->index()->translationUnitForUrl(context->url()) == m_environment.translationUnitUrl()) {
                // cache the parse session and the contained translation unit for this chain
                // this then allows us to quickly reparse the document if it is changed by
                // the user
                // otherwise no editor component is open for this document and we can dispose
                // the TU to save memory
                // share the session data with all contexts that are pinned to this TU
                DUChainWriteLocker lock;
                context->setAst(IAstContainer::Ptr(session.data()));
            }
            languageSupport()->codeHighlighting()->highlightDUChain(context);
        }
    }
}

ParseSessionData::Ptr ClangParseJob::createSessionData() const
{
    const bool skipFunctionBodies = (minimumFeatures() <= TopDUContext::VisibleDeclarationsAndContexts);
    return ParseSessionData::Ptr(new ParseSessionData(m_unsavedFiles, clang()->index(), m_environment,
                                 (skipFunctionBodies ? ParseSessionData::SkipFunctionBodies : ParseSessionData::NoOption)));
}

const ParsingEnvironment* ClangParseJob::environment() const
{
    return &m_environment;
}
