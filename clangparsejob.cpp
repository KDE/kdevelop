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
#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include "duchain/parsesession.h"
#include "duchain/buildduchainvisitor.h"
#include "duchain/clangtypes.h"

#include "debug.h"
#include "clanglanguagesupport.h"

#include <QReadLocker>
#include <QProcess>
#include <memory>

using namespace KDevelop;

namespace {
// TODO: investigate why this is required to find e.g. stddef.h
KUrl::List defaultIncludes()
{
    static KUrl::List includePaths;

    if (!includePaths.isEmpty()) {
        return includePaths;
    }

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    // The following command will spit out a bnuch of information we don't care
    // about before spitting out the include paths.  The parts we care about
    // look like this:
    // #include "..." search starts here:
    // #include <...> search starts here:
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/i486-linux-gnu
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/backward
    //  /usr/local/include
    //  /usr/lib/gcc/i486-linux-gnu/4.1.2/include
    //  /usr/include
    // End of search list.
    proc.start("clang++", {"-std=c++11", "-xc++", "-E", "-v", "/dev/null"});
    if (!proc.waitForStarted(1000) || !proc.waitForFinished(1000)) {
        return {};
    }

    // We'll use the following constants to know what we're currently parsing.
    enum Status {
        Initial,
        FirstSearch,
        Includes,
        Finished
    };
    Status mode = Initial;

    foreach(const QString& line, QString::fromLocal8Bit(proc.readAllStandardOutput()).split('\n')) {
        switch (mode) {
            case Initial:
                if (line.indexOf("#include \"...\"") != -1) {
                    mode = FirstSearch;
                }
                break;
            case FirstSearch:
                if (line.indexOf("#include <...>") != -1) {
                    mode = Includes;
                    break;
                }
            case Includes:
                //if (!line.indexOf(QDir::separator()) == -1 && line != "." ) {
                //Detect the include-paths by the first space that is prepended. Reason: The list may contain relative paths like "."
                if (!line.startsWith(" ") ) {
                    // We've reached the end of the list.
                    mode = Finished;
                } else {
                    // This is an include path, add it to the list.
                    includePaths << QDir::cleanPath(line.trimmed());
                }
                break;
            default:
                break;
        }
        if (mode == Finished) {
            break;
        }
    }

    return includePaths;
}

struct Import
{
    CXFile file;
    CursorInRevision location;
};

using Imports = QMultiHash<CXFile, Import>;

struct InclusionClientData
{
    QMultiMap<unsigned, CXFile> includesByDepth;
    Imports imports;
};

void visitInclusions(CXFile file, CXSourceLocation* stack, unsigned stackDepth, CXClientData d)
{
    auto data = static_cast<InclusionClientData*>(d);
    data->includesByDepth.insert(stackDepth, file);

    if (stackDepth) {
        CXFile parentFile;
        uint line, column;
        clang_getFileLocation(stack[0], &parentFile, &line, &column, nullptr);
        data->imports.insert(parentFile, {file, CursorInRevision(line, column)});
    }
}

ReferencedTopDUContext createTopContext(const IndexedString& path)
{
    ParsingEnvironmentFile *file = new ParsingEnvironmentFile(path);
    file->setLanguage(ParseSession::languageString());
    ReferencedTopDUContext context = new TopDUContext(path, RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
    DUChain::self()->addDocumentChain(context);
    return context;
}

void buildDUChain(CXTranslationUnit unit, CXFile file, IncludeFileContexts* includedFiles, const Imports& imports)
{
    if (includedFiles->contains(file)) {
        return;
    }

    // prevent recursion
    includedFiles->insert(file, {});

    // ensure DUChain for imports are build properly
    foreach(const auto& import, imports.values(file)) {
        buildDUChain(unit, import.file, includedFiles, imports);
    }

    const IndexedString path(QDir::cleanPath(QString::fromUtf8(ClangString(clang_getFileName(file)))));

    UrlParseLock urlLock(path);
    ReferencedTopDUContext context;
    {
        DUChainWriteLocker lock;
        bool created = false;
        context = DUChain::self()->chainForDocument(path);
        if (!context) {
            context = createTopContext(path);
            created = true;
        }
        includedFiles->insert(file, context);
        if (!created && !context->parsingEnvironmentFile()->needsUpdate()) {
            return;
        }
    }

    {
        DUChainWriteLocker lock;
        foreach(const auto& import, imports.values(file)) {
            Q_ASSERT(includedFiles->contains(import.file));
            auto ctx = includedFiles->value(import.file);
            if (!ctx) {
                // happens for cyclic imports
                continue;
            }
            context->addImportedParentContext(ctx, import.location);
        }
    }

    BuildDUChainVisitor visitor;
    visitor.visit(unit, file, *includedFiles);
}

}

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{
    auto item = ICore::self()->projectController()->projectModel()->itemForUrl(url);
    if (item && item->project()->buildSystemManager()) {
        auto bsm = item->project()->buildSystemManager();
        m_includes = bsm->includeDirectories(item);
        m_defines = bsm->defines(item);
    }

    m_includes += defaultIncludes();
}

ClangLanguageSupport* ClangParseJob::clang() const
{
    return static_cast<ClangLanguageSupport*>(languageSupport());
}

void ClangParseJob::run()
{
    QReadLocker parseLock(languageSupport()->language()->parseLock());

    KSharedPtr<ParseSession> session;
    {
        UrlParseLock urlLock(document());
        if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
            return;
        }
        {
            DUChainWriteLocker lock;
            const auto& context = DUChainUtils::standardContextForUrl(document().toUrl());
            if (context) {
                session = KSharedPtr<ParseSession>::dynamicCast(context->ast());
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

    if (!session || !session->reparse(contents().contents)) {
        session = new ParseSession(document(), contents().contents, clang()->index(), m_includes, m_defines);
    } else {
        Q_ASSERT(session->url() == document());
        Q_ASSERT(session->unit());
    }

    if (abortRequested() || !session->unit()) {
        return;
    }

    InclusionClientData includes;
    clang_getInclusions(session->unit(), &::visitInclusions, &includes);

    if (abortRequested()) {
        return;
    }

    IncludeFileContexts includedFiles;
    auto it = includes.includesByDepth.constEnd();
    auto end = includes.includesByDepth.constBegin();

    while ( it != end ) {
        --it;
        if (abortRequested()) {
            return;
        }
        buildDUChain(session->unit(), *it, &includedFiles, includes.imports);
    }

    if (abortRequested()) {
        return;
    }

    Q_ASSERT(includedFiles.contains(session->file()));
    auto context = includedFiles[session->file()];
    setDuChain(context);

    {
        DUChainWriteLocker lock;
        if (hasTracker()) {
            // cache the parse session and the contained translation unit for this chain
            // this then allows us to quickly reparse the document if it is changed by
            // the user
            // otherwise no editor component is open for this document and we can dispose
            // the TU to save memory
            context->setAst(KSharedPtr<IAstContainer>::staticCast(session));
        }
        context->setProblems(session->problems());
        context->setFeatures(minimumFeatures());
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
    }

    highlightDUChain();
}
