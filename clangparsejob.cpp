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
#include <language/interfaces/idefinesandincludesmanager.h>
#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include "duchain/clanghelpers.h"
#include "duchain/clangpch.h"
#include "duchain/clangtypes.h"
#include "duchain/tuduchain.h"

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

static const QString customIncludePathsFilename = QString::fromLatin1(".kdev_include_paths");
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

Path::List userDefinedIncludePathsForFile(const QString& sourcefile)
{
    return readPathListFile(findConfigFile(sourcefile, customIncludePathsFilename));
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

}

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{
    // get defines and include paths in foreground thread
    auto item = ICore::self()->projectController()->projectModel()->itemForPath(url);

    auto idm = KDevelop::IDefinesAndIncludesManager::manager();
    m_includes = idm->includes(item);
    m_defines = idm->defines(item);
}

ClangSupport* ClangParseJob::clang() const
{
    return static_cast<ClangSupport*>(languageSupport());
}

void ClangParseJob::run()
{
    QReadLocker parseLock(languageSupport()->language()->parseLock());

    // get user defined and default include paths in background thread
    m_includes += userDefinedIncludePathsForFile(document().str());

    if (abortRequested()) {
        return;
    }

    auto pchInclude = userDefinedPchIncludeForFile(document().str());
    auto pch = clang()->index()->pch(pchInclude, m_includes, m_defines);

    if (abortRequested()) {
        return;
    }

    {
        UrlParseLock urlLock(document());
        if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
            return;
        }
        {
            DUChainWriteLocker lock;
            const auto& context = DUChainUtils::standardContextForUrl(document().toUrl());
            if (context) {
                m_session = KSharedPtr<ParseSession>::dynamicCast(context->ast());
                // ensure that other threads don't access the TU while we parse it
                context->setAst({});
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

    if (!m_session || !m_session->reparse(contents().contents)) {
        const bool skipFunctionBodies = (minimumFeatures() <= TopDUContext::VisibleDeclarationsAndContexts);
        m_session = new ParseSession(document(), contents().contents, clang()->index(), m_includes, pchInclude, m_defines,
                                     (skipFunctionBodies ? ParseSession::SkipFunctionBodies : ParseSession::NoOption));
    } else {
        Q_ASSERT(m_session->url() == document());
        Q_ASSERT(m_session->unit());
    }

    if (abortRequested() || !m_session->unit()) {
        return;
    }

    Imports imports = ClangHelpers::tuImports(m_session->unit());

    IncludeFileContexts includedFiles;
    if (pch) {
        auto pchFile = pch->mapFile(m_session->unit());
        includedFiles = pch->mapIncludes(m_session->unit());
        includedFiles.insert(pchFile, pch->context());
        imports.insert(m_session->file(), { pchFile, CursorInRevision(0, 0) } );
    }

    if (abortRequested()) {
        return;
    }

    auto context = ClangHelpers::buildDUChain(m_session->file(), imports, m_session.data(), minimumFeatures(), includedFiles);
    setDuChain(context);

    if (abortRequested()) {
        return;
    }

    {
        DUChainWriteLocker lock;
        if (hasTracker()) {
            // cache the parse session and the contained translation unit for this chain
            // this then allows us to quickly reparse the document if it is changed by
            // the user
            // otherwise no editor component is open for this document and we can dispose
            // the TU to save memory
            context->setAst(KSharedPtr<IAstContainer>::staticCast(m_session));
        }
        context->setFeatures(minimumFeatures());
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
    }

    highlightDUChain();
}
