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
#include "duchain/clangtypes.h"
#include "duchain/tuduchain.h"
#include "duchain/parsesession.h"

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
        m_includes = IDefinesAndIncludesManager::manager()->includes(file);
        m_defines = IDefinesAndIncludesManager::manager()->defines(file);
    } else {
        m_includes = IDefinesAndIncludesManager::manager()->includes(url.str());
        m_defines = IDefinesAndIncludesManager::manager()->defines(url.str());
    }
}

ClangSupport* ClangParseJob::clang() const
{
    return static_cast<ClangSupport*>(languageSupport());
}

void ClangParseJob::run()
{
    QReadLocker parseLock(languageSupport()->language()->parseLock());

    if (abortRequested()) {
        return;
    }

    auto pchInclude = userDefinedPchIncludeForFile(document().str());
    auto pch = clang()->index()->pch(pchInclude, m_includes, m_defines);

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
                sessionData = ParseSessionData::Ptr::dynamicCast(context->ast());
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
        sessionData = createSessionData(pchInclude);
        needsUpdate = false;
    }

    ParseSession session(sessionData);

    if (abortRequested()) {
        return;
    }

    if (needsUpdate && !session.reparse(contents().contents)) {
        session.setData(createSessionData(pchInclude));
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

    auto context = ClangHelpers::buildDUChain(session.file(), imports, session, minimumFeatures(), includedFiles);
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
            context->setAst(KSharedPtr<IAstContainer>::staticCast(session.data()));
        }
        context->setFeatures(minimumFeatures());
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
    }

    // release the data here, so we don't lock it while highlighting
    session.setData({});

    highlightDUChain();
}

ParseSessionData::Ptr ClangParseJob::createSessionData(const Path& pchInclude)
{
    const bool skipFunctionBodies = (minimumFeatures() <= TopDUContext::VisibleDeclarationsAndContexts);
    return ParseSessionData::Ptr(new ParseSessionData(document(), contents().contents, clang()->index(), m_includes, pchInclude, m_defines,
                                 (skipFunctionBodies ? ParseSessionData::SkipFunctionBodies : ParseSessionData::NoOption)));
}
