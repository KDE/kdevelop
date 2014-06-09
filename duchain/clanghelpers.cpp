/*
 * Copyright 2014  Olivier de Gaalon <olivier.jg@gmail.com>
 * Copyright 2014  Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clanghelpers.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/backgroundparser/urlparselock.h>

#include "clangtypes.h"
#include "tuduchain.h"
#include "parsesession.h"


using namespace KDevelop;

namespace {

void visitInclusions(CXFile file, CXSourceLocation* stack, unsigned stackDepth, CXClientData d)
{
    if (stackDepth) {
        auto imports = static_cast<Imports*>(d);
        CXFile parentFile;
        uint line, column;
        clang_getFileLocation(stack[0], &parentFile, &line, &column, nullptr);
        imports->insert(parentFile, {file, CursorInRevision(line, column)});
    }
}

ReferencedTopDUContext createTopContext(const IndexedString& path)
{
    ParsingEnvironmentFile* file = new ParsingEnvironmentFile(path);
    file->setLanguage(ParseSession::languageString());
    ReferencedTopDUContext context = new TopDUContext(path, RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
    DUChain::self()->addDocumentChain(context);
    return context;
}

}

Imports tuImports(CXTranslationUnit tu)
{
    Imports imports;
    clang_getInclusions(tu, &::visitInclusions, &imports);
    return imports;
}

ReferencedTopDUContext buildDUChain(CXFile file, const Imports& imports, const ParseSession* session,
                                    TopDUContext::Features features, IncludeFileContexts& includedFiles)
{
    if (includedFiles.contains(file)) {
        return {};
    }

    // prevent recursion
    includedFiles.insert(file, {});

    // ensure DUChain for imports are build properly
    foreach(const auto& import, imports.values(file)) {
        buildDUChain(import.file, imports, session, features, includedFiles);
    }

    const IndexedString path(QDir::cleanPath(QString::fromUtf8(ClangString(clang_getFileName(file)))));

    bool update = false;
    UrlParseLock urlLock(path);
    ReferencedTopDUContext context;
    {
        DUChainWriteLocker lock;
        context = DUChain::self()->chainForDocument(path);
        if (!context) {
            context = ::createTopContext(path);
        } else {
            update = true;
        }

        context->setFeatures(features);
        context->setProblems(session->problemsForFile(file));

        includedFiles.insert(file, context);
        if (update) {
            if (!context->parsingEnvironmentFile()->needsUpdate()
                && context->parsingEnvironmentFile()->featuresSatisfied(features))
            {
                return context;
            }

            context->clearImportedParentContexts();
        }
        foreach(const auto& import, imports.values(file)) {
            Q_ASSERT(includedFiles.contains(import.file));
            auto ctx = includedFiles.value(import.file);
            if (!ctx) {
                // happens for cyclic imports
                continue;
            }
            context->addImportedParentContext(ctx, import.location);
        }
    }

    TUDUChain tuduchain(session->unit(), file, includedFiles, update);

    return context;
}

DeclarationPointer findDeclaration(CXCursor cursor, const IncludeFileContexts& includes)
{
    auto refLoc = clang_getCursorLocation(cursor);
    CXFile file = nullptr;
    clang_getFileLocation(refLoc, &file, nullptr, nullptr, nullptr);
    if (!file) {
        return {};
    }
    auto refCursor = CursorInRevision(ClangLocation(refLoc));

    const auto& top = includes.value(file);
    if (!top) {
        // may happen for cyclic includes
        return {};
    }

    DUChainReadLocker lock;
    Q_ASSERT(top);
    if (DUContext *local = top->findContextAt(refCursor)) {
        if (local->owner() && local->owner()->range().contains(refCursor)) {
           return DeclarationPointer(local->owner());
        }
        return DeclarationPointer(local->findDeclarationAt(refCursor));
    }
    return {};
}

DeclarationPointer findDeclaration(CXType type, const IncludeFileContexts& includes)
{
    CXCursor cursor = clang_getTypeDeclaration(type);
    return findDeclaration(cursor, includes);
}

QStringList headerExtensions()
{
    static const QStringList headerExtensions = QString("h,H,hh,hxx,hpp,tlh,h++").split(',');
    return headerExtensions;
}

QStringList sourceExtensions()
{
    static const QStringList sourceExtensions = QString("c,cc,cpp,c++,cxx,C,m,mm,M,inl,_impl.h").split(',');
    return sourceExtensions;
}
