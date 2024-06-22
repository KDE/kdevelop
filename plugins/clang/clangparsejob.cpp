/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

#include "clangsettings/clangsettingsmanager.h"
#include "duchain/clanghelpers.h"
#include "duchain/clangpch.h"
#include "duchain/duchainutils.h"
#include "duchain/parsesession.h"
#include "duchain/clangindex.h"
#include "duchain/clangparsingenvironmentfile.h"
#include "util/clangdebug.h"
#include "util/clangtypes.h"
#include "util/clangutils.h"

#include "clangsupport.h"
#include "duchain/documentfinderhelpers.h"

#include <KTextEditor/Document>

#include <QFile>
#include <QFileInfo>
#include <QReadLocker>
#include <QStringList>

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
    const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    Path::List paths;
    paths.reserve(lines.length());
    for (const auto& line : lines) {
        paths << Path(line);
    }
    return paths;
}

/**
 * File should contain the header to precompile and use while parsing
 * @returns the first path in the file
 */
Path userDefinedPchIncludeForFile(const QString& sourcefile)
{
    const QString pchIncludeFilename = QStringLiteral(".kdev_pch_include");
    const auto paths = readPathListFile(findConfigFile(sourcefile, pchIncludeFilename));
    return paths.isEmpty() ? Path() : paths.first();
}

ProjectFileItem* findProjectFileItem(const IndexedString& url, bool* hasBuildSystemInfo)
{
    ProjectFileItem* file = nullptr;

    *hasBuildSystemInfo = false;
    const auto& projects = ICore::self()->projectController()->projects();
    for (auto project : projects) {
        const auto files = project->filesForPath(url);
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
            *hasBuildSystemInfo = bsm->hasBuildInfo(file);
        }
    }
    return file;
}

[[maybe_unused]] ClangParsingEnvironmentFile* parsingEnvironmentFile(const TopDUContext* context)
{
    return dynamic_cast<ClangParsingEnvironmentFile*>(context->parsingEnvironmentFile().data());
}

DocumentChangeTracker* trackerForUrl(const IndexedString& url)
{
    return ICore::self()->languageController()->backgroundParser()->trackerForUrl(url);
}

}

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
    : ParseJob(url, languageSupport)
    , m_options(ParseSessionData::NoOption)
{
    const auto tuUrl = clang()->index()->translationUnitForUrl(url);
    bool hasBuildSystemInfo;
    if (auto file = findProjectFileItem(tuUrl, &hasBuildSystemInfo)) {
        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includes(file));
        m_environment.addFrameworkDirectories(IDefinesAndIncludesManager::manager()->frameworkDirectories(file));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->defines(file));
        m_environment.setParserSettings(ClangSettingsManager::self()->parserSettings(file));
        if (hasBuildSystemInfo) {
            // Assume the builder invokes the compiler in the build directory.
            m_environment.setWorkingDirectory(file->project()->buildSystemManager()->buildDirectory(file));
        }
    } else {
        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includes(tuUrl.str()));
        m_environment.addFrameworkDirectories(IDefinesAndIncludesManager::manager()->frameworkDirectories(tuUrl.str()));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->defines(tuUrl.str()));
        m_environment.setParserSettings(ClangSettingsManager::self()->parserSettings(tuUrl.str()));
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
    for (auto project : projects) {
        projectPaths.append(project->path());
        if (auto* bsm = project->buildSystemManager()) {
            projectPaths.append(bsm->buildDirectory(project->projectItem()));
        }
    }
    m_environment.setProjectPaths(projectPaths);

    m_unsavedFiles = ClangUtils::unsavedFiles();

    const auto documents = ICore::self()->documentController()->openDocuments();
    for (auto* document : documents) {
        auto textDocument = document->textDocument();
        if ( !textDocument ) {
            continue;
        }
        const IndexedString indexedUrl(textDocument->url());
        if (indexedUrl == tuUrl) {
            m_tuDocumentIsUnsaved = true;
        }
        m_unsavedRevisions.insert(indexedUrl, ModificationRevision::revisionForFile(indexedUrl));
    }

    if (auto tracker = trackerForUrl(url)) {
        tracker->reset();
        m_options |= ParseSessionData::OpenedInEditor;
    } else if (tuUrl != url && trackerForUrl(tuUrl)) {
        m_options |= ParseSessionData::OpenedInEditor;
    }
}

ClangSupport* ClangParseJob::clang() const
{
    return static_cast<ClangSupport*>(languageSupport());
}

void ClangParseJob::run(ThreadWeaver::JobPointer /*self*/, ThreadWeaver::Thread* /*thread*/)
{
    QReadLocker parseLock(languageSupport()->parseLock());

    if (abortRequested()) {
        return;
    }

    {
        const auto tuUrlStr = m_environment.translationUnitUrl().str();
        if (!m_tuDocumentIsUnsaved && !QFile::exists(tuUrlStr)) {
            // maybe we requested a parse job some time ago but now the file
            // does not exist anymore. return early then
            clang()->index()->unpinTranslationUnitForUrl(document());
            return;
        }

        m_environment.addIncludes(IDefinesAndIncludesManager::manager()->includesInBackground(tuUrlStr));
        m_environment.addFrameworkDirectories(IDefinesAndIncludesManager::manager()->frameworkDirectoriesInBackground(tuUrlStr));
        m_environment.addDefines(IDefinesAndIncludesManager::manager()->definesInBackground(tuUrlStr));
        m_environment.addParserArguments(IDefinesAndIncludesManager::manager()->parserArgumentsInBackground(tuUrlStr));
        m_environment.setPchInclude(userDefinedPchIncludeForFile(tuUrlStr));
    }

    if (abortRequested()) {
        return;
    }

    // NOTE: we must have all declarations, contexts and uses available for files that are opened in the editor
    //       it is very hard to check this for all included files of this TU, and previously lead to problems
    //       when we tried to skip function bodies as an optimization for files that where not open in the editor.
    //       now, we always build everything, which is correct but a tad bit slower. we can try to optimize later.
    setMinimumFeatures(minimumFeatures() | TopDUContext::AllDeclarationsContextsAndUses);

    if (minimumFeatures() & AttachASTWithoutUpdating) {
        // The context doesn't need to be updated, but has no AST attached (restored from disk),
        // so attach AST to it, without updating DUChain
        ParseSession session(createSessionData());

        DUChainWriteLocker lock;
        auto ctx = DUChainUtils::standardContextForUrl(document().toUrl());
        if (!ctx) {
            clangDebug() << "Lost context while attaching AST";
            return;
        }
        ctx->setAst(IAstContainer::Ptr(session.data()));

        if (minimumFeatures() & UpdateHighlighting) {
            lock.unlock();
            languageSupport()->codeHighlighting()->highlightDUChain(ctx);
        }
        return;
    }

    {
        UrlParseLock urlLock(document());
        if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
            return;
        }
    }

    ParseSession session(ClangIntegration::DUChainUtils::findParseSessionData(document(), m_environment.translationUnitUrl()));
    if (abortRequested()) {
        return;
    }

    if (!session.data() || !session.reparse(m_unsavedFiles, m_environment)) {
        session.setData(createSessionData());
    }

    if (!session.unit()) {
        // failed to parse file, unpin and don't try again
        clang()->index()->unpinTranslationUnitForUrl(document());
        return;
    }

    if (!clang_getFile(session.unit(), document().byteArray().constData())) {
        // this parse job's document does not exist in the pinned translation unit
        // so we need to unpin and re-add this document
        // Ideally we'd reset m_environment and session, but this is much simpler
        // and shouldn't be a common case
        clang()->index()->unpinTranslationUnitForUrl(document());
        if (!(minimumFeatures() & Rescheduled)) {
            auto features = static_cast<TopDUContext::Features>(minimumFeatures() | Rescheduled);
            ICore::self()->languageController()->backgroundParser()->addDocument(document(), features, priority());
        }
        return;
    }

    Imports imports = ClangHelpers::tuImports(session.unit());
    IncludeFileContexts includedFiles;
    if (auto pch = clang()->index()->pch(m_environment)) {
        auto pchFile = pch->mapFile(session.unit());
        includedFiles = pch->mapIncludes(session.unit());
        includedFiles.insert(pchFile, pch->context());
        auto tuFile = clang_getFile(session.unit(), m_environment.translationUnitUrl().byteArray().constData());
        imports.insert(tuFile, { pchFile, CursorInRevision(0, 0) } );
    }

    if (abortRequested()) {
        return;
    }

    auto context = ClangHelpers::buildDUChain(session.mainFile(), imports, session, minimumFeatures(), includedFiles,
                                              m_unsavedRevisions, document(), clang()->index(),
                                              [this] { return abortRequested(); });
    setDuChain(context);

    if (abortRequested()) {
        return;
    }

    if (context) {
        if (minimumFeatures() & TopDUContext::AST) {
            DUChainWriteLocker lock;
            context->setAst(IAstContainer::Ptr(session.data()));
        }
#ifdef QT_DEBUG
        DUChainReadLocker lock;
        auto file = parsingEnvironmentFile(context);
        Q_ASSERT(file);
        // verify that features and environment where properly set in ClangHelpers::buildDUChain
        Q_ASSERT(file->featuresSatisfied(minimumFeatures() & ~TopDUContext::ForceUpdateRecursive));
        if (trackerForUrl(context->url())) {
            Q_ASSERT(file->featuresSatisfied(TopDUContext::AllDeclarationsContextsAndUses));
        }
#endif
    }

    for (const auto& context : std::as_const(includedFiles)) {
        if (!context) {
            continue;
        }
        if (trackerForUrl(context->url())) {
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
    return ParseSessionData::Ptr(new ParseSessionData(m_unsavedFiles, clang()->index(), m_environment, m_options));
}

const ParsingEnvironment* ClangParseJob::environment() const
{
    return &m_environment;
}

#include "moc_clangparsejob.cpp"
