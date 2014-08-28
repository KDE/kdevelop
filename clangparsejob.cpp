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

#include <language/backgroundparser/urlparselock.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <custom-definesandincludes/idefinesandincludesmanager.h>
#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include "duchain/clanghelpers.h"
#include "duchain/clangpch.h"
#include "duchain/tuduchain.h"
#include "duchain/parsesession.h"
#include "duchain/clangindex.h"
#include "duchain/clangparsingenvironmentfile.h"
#include "util/clangtypes.h"

#include "debug.h"
#include "clangsupport.h"

#include <QFile>
#include <QStringList>
#include <QFileInfo>
#include <QReadLocker>
#include <QProcess>
#include <memory>

using namespace KDevelop;

namespace {

static const QString pchIncludeFilename = QString::fromLatin1(".kdev_pch_include");

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
    const auto paths = readPathListFile(findConfigFile(sourcefile, pchIncludeFilename));
    return paths.isEmpty() ? Path() : paths.first();
}

ProjectFileItem* findProjectFileItem(const IndexedString& url)
{
    ProjectFileItem* file = nullptr;

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
    return file;
}

}

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{
    if (auto file = findProjectFileItem(url)) {
        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includes(file));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->defines(file));
        m_environment.setProjectKnown(true);
    } else {
        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includes(url.str()));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->defines(url.str()));
        m_environment.setProjectKnown(false);
    }

    Path::List projectPaths;
    const auto& projects = ICore::self()->projectController()->projects();
    projectPaths.reserve(projects.size());
    foreach (auto project, projects) {
        projectPaths.append(project->path());
    }
    m_environment.setProjectPaths(projectPaths);
}

ClangSupport* ClangParseJob::clang() const
{
    return static_cast<ClangSupport*>(languageSupport());
}

void ClangParseJob::run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread)
{
    QReadLocker parseLock(languageSupport()->language()->parseLock());

    if (abortRequested()) {
        return;
    }

    m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includesInBackground(document().str()));
    m_environment.addDefines(IDefinesAndIncludesManager::manager()->definesInBackground(document().str()));

    auto pchInclude = userDefinedPchIncludeForFile(document().str());
    m_environment.setPchInclude(pchInclude);
    auto pch = clang()->index()->pch(m_environment);

    if (abortRequested()) {
        return;
    }

    ParseSessionData::Ptr sessionData;
    {
        UrlParseLock urlLock(document());
        if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
            return;
        }
        {
            DUChainWriteLocker lock;
            const auto& context = DUChainUtils::standardContextForUrl(document().toUrl());
            if (context) {
                sessionData = ParseSessionData::Ptr(dynamic_cast<ParseSessionData*>(context->ast().data()));
            }
        }
    }

    if (abortRequested()) {
        return;
    }

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }

    if (abortRequested()) {
        return;
    }

    bool needsUpdate = true;
    if (!sessionData) {
        sessionData = createSessionData();
        needsUpdate = false;
    }

    ParseSession session(sessionData);

    if (abortRequested()) {
        return;
    }

    if (needsUpdate && !session.reparse(contents().contents, m_environment)) {
        session.setData(createSessionData());
    } else {
        Q_ASSERT(session.url() == document());
    }

    if (abortRequested() || !session.unit()) {
        return;
    }

    Imports imports = ClangHelpers::tuImports(session.unit());

    IncludeFileContexts includedFiles;
    if (pch) {
        auto pchFile = pch->mapFile(session.unit());
        includedFiles = pch->mapIncludes(session.unit());
        includedFiles.insert(pchFile, pch->context());
        imports.insert(session.file(), { pchFile, CursorInRevision(0, 0) } );
    }

    if (abortRequested()) {
        return;
    }

    auto context = ClangHelpers::buildDUChain(session.file(), imports, session, minimumFeatures(),
                                              includedFiles);
    setDuChain(context);

    if (abortRequested()) {
        return;
    }

    {
        DUChainWriteLocker lock;
        if (hasTracker() || minimumFeatures() & TopDUContext::AST) {
            // cache the parse session and the contained translation unit for this chain
            // this then allows us to quickly reparse the document if it is changed by
            // the user
            // otherwise no editor component is open for this document and we can dispose
            // the TU to save memory
            context->setAst(IAstContainer::Ptr(session.data()));
        }
        auto file = dynamic_cast<ClangParsingEnvironmentFile*>(context->parsingEnvironmentFile().data());
        Q_ASSERT(file);
        // verify that features and environment where properly set in ClangHelpers::buildDUChain
        Q_ASSERT(file->featuresSatisfied(TopDUContext::Features(minimumFeatures() & ~TopDUContext::ForceUpdateRecursive)));
        Q_ASSERT(file->environmentHash() == m_environment.hash());
        // prefer the editor modification revision, instead of the on-disk revision
        file->setModificationRevision(contents().modification);
    }

    // release the data here, so we don't lock it while highlighting
    session.setData({});

    highlightDUChain();
}

ParseSessionData::Ptr ClangParseJob::createSessionData() const
{
    const bool skipFunctionBodies = (minimumFeatures() <= TopDUContext::VisibleDeclarationsAndContexts);
    return ParseSessionData::Ptr(new ParseSessionData(document(), contents().contents, clang()->index(), m_environment,
                                 (skipFunctionBodies ? ParseSessionData::SkipFunctionBodies : ParseSessionData::NoOption)));
}

const ParsingEnvironment* ClangParseJob::environment() const
{
    return &m_environment;
}
