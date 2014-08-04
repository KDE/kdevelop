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
        imports->insert(parentFile, {file, CursorInRevision(line-1, column-1)});
    }
}

ReferencedTopDUContext createTopContext(const IndexedString& path)
{
    ParsingEnvironmentFile* file = new ParsingEnvironmentFile(path);
    file->setLanguage(ParseSession::languageString());
    ReferencedTopDUContext context = new ClangTopDUContext(path, RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
    DUChain::self()->addDocumentChain(context);
    return context;
}

}

Imports ClangHelpers::tuImports(CXTranslationUnit tu)
{
    Imports imports;
    clang_getInclusions(tu, &::visitInclusions, &imports);
    return imports;
}

ReferencedTopDUContext ClangHelpers::buildDUChain(CXFile file, const Imports& imports, const ParseSession& session,
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
        const auto problems = session.problemsForFile(file);

        DUChainWriteLocker lock;
        context = DUChain::self()->chainForDocument(path);
        if (!context) {
            context = ::createTopContext(path);
        } else {
            update = true;
        }

        context->setFeatures(features);
        context->setProblems(problems);

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

    TUDUChain tuduchain(session.unit(), file, includedFiles, update);

    return context;
}

DeclarationPointer ClangHelpers::findDeclaration(CXCursor cursor, const IncludeFileContexts& includes)
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

DeclarationPointer ClangHelpers::findDeclaration(CXType type, const IncludeFileContexts& includes)
{
    CXCursor cursor = clang_getTypeDeclaration(type);
    return findDeclaration(cursor, includes);
}

RangeInRevision ClangHelpers::cursorSpellingNameRange(CXCursor cursor, const Identifier& id)
{
    auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRangeInRevision();
    auto kind = clang_getCursorKind(cursor);
    // TODO: Upstream issue: Check why clang reports invalid ranges for destructors and methods like 'operator='
    // Current issues:
    // - CXCursor_Destructor: Only returns the range of '~'
    // - CXCursor_CXXMethod: For operator overloads, only returns the range of 'operator'
    if (kind == CXCursor_Destructor || kind == CXCursor_CXXMethod) {
        range.end.column = range.start.column + id.toString().length();
    }
    return range;
}

QStringList ClangHelpers::headerExtensions()
{
    static const QStringList headerExtensions = {"h", "H", "hh", "hxx", "hpp", "tlh", "h++"};
    return headerExtensions;
}

QStringList ClangHelpers::sourceExtensions()
{
    static const QStringList sourceExtensions = {"c", "cc", "cpp", "c++", "cxx", "C", "m", "mm", "M", "inl", "_impl.h"};
    return sourceExtensions;
}

bool ClangHelpers::isSource(const QString& path)
{
    foreach(const QString& ext, sourceExtensions()) {
        if (path.endsWith(ext)) {
            return true;
        }
    }
    return false;
}
