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

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/backgroundparser/urlparselock.h>

#include "builder.h"
#include "parsesession.h"
#include "clangparsingenvironmentfile.h"
#include "clangindex.h"
#include "clangducontext.h"

#include "util/clangtypes.h"

#include <algorithm>

using namespace KDevelop;

namespace {

CXChildVisitResult visitCursor(CXCursor cursor, CXCursor, CXClientData data)
{
    if (cursor.kind != CXCursor_InclusionDirective) {
        return CXChildVisit_Continue;
    }

    auto imports = static_cast<Imports*>(data);
    CXFile file = clang_getIncludedFile(cursor);
    if(!file){
        return CXChildVisit_Continue;
    }

    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile parentFile;
    uint line, column;
    clang_getFileLocation(location, &parentFile, &line, &column, nullptr);

    foreach (const auto& import, imports->values(parentFile)) {
        // clang_getInclusions doesn't include the same import twice, so we shouldn't do it too.
        if (import.file == file) {
            return CXChildVisit_Continue;
        }
    }

    imports->insert(parentFile, {file, CursorInRevision(line-1, column-1)});

    return CXChildVisit_Recurse;
}

ReferencedTopDUContext createTopContext(const IndexedString& path, const ClangParsingEnvironment& environment)
{
    ClangParsingEnvironmentFile* file = new ClangParsingEnvironmentFile(path, environment);
    ReferencedTopDUContext context = new ClangTopDUContext(path, RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
    DUChain::self()->addDocumentChain(context);
    context->updateImportsCache();
    return context;
}

}

Imports ClangHelpers::tuImports(CXTranslationUnit tu)
{
    Imports imports;

    // Intentionally don't use clang_getInclusions here, as it skips already visited inclusions
    // which makes TestDUChain::testNestedImports fail
    CXCursor tuCursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(tuCursor, &visitCursor, &imports);

    return imports;
}

bool importLocationLessThan(const Import& lhs, const Import& rhs)
{
    return lhs.location.line < rhs.location.line;
}

ReferencedTopDUContext ClangHelpers::buildDUChain(CXFile file, const Imports& imports, const ParseSession& session,
                                                  TopDUContext::Features features, IncludeFileContexts& includedFiles,
                                                  ClangIndex* index, const std::function<bool()>& abortFunction)
{
    if (includedFiles.contains(file)) {
        return {};
    }

    if (abortFunction && abortFunction()) {
        return {};
    }

    // prevent recursion
    includedFiles.insert(file, {});

    // ensure DUChain for imports are built properly, and in correct order
    QList<Import> sortedImports = imports.values(file);
    std::sort(sortedImports.begin(), sortedImports.end(), importLocationLessThan);

    foreach(const auto& import, sortedImports) {
        buildDUChain(import.file, imports, session, features, includedFiles, index, abortFunction);
    }

    const IndexedString path(QDir(ClangString(clang_getFileName(file)).toString()).canonicalPath());
    if (path.isEmpty()) {
        // may happen when the file gets removed before the job is run
        return {};
    }

    const auto& environment = session.environment();

    bool update = false;
    UrlParseLock urlLock(path);
    ReferencedTopDUContext context;
    {
        DUChainWriteLocker lock;
        context = DUChain::self()->chainForDocument(path, &environment);
        if (!context) {
            context = ::createTopContext(path, environment);
        } else {
            update = true;
        }

        includedFiles.insert(file, context);
        if (update) {
            auto envFile = ClangParsingEnvironmentFile::Ptr(dynamic_cast<ClangParsingEnvironmentFile*>(context->parsingEnvironmentFile().data()));
            Q_ASSERT(envFile);
            if (!envFile)
                return context;

            /* NOTE: When we are here, then either the translation unit or one of its headers was changed.
             *       Thus we must always update the translation unit to propagate the change(s).
             *       See also: https://bugs.kde.org/show_bug.cgi?id=356327
             *       This assumes that headers are independent, we may need to improve that in the future
             *       and also update header files more often when other files included therein got updated.
             */
            if (path != environment.translationUnitUrl() && !envFile->needsUpdate(&environment) && envFile->featuresSatisfied(features)) {
                return context;
            } else {
                //TODO: don't attempt to update if this environment is worse quality than the outdated one
                if (index && envFile->environmentQuality() < environment.quality()) {
                    index->pinTranslationUnitForUrl(environment.translationUnitUrl(), path);
                }
                envFile->setEnvironment(environment);
                envFile->setModificationRevision(ModificationRevision::revisionForFile(context->url()));
            }

            context->clearImportedParentContexts();
        }
        context->setFeatures(features);

        foreach(const auto& import, sortedImports) {
            auto ctx = includedFiles.value(import.file);
            if (!ctx) {
                // happens for cyclic imports
                continue;
            }
            context->addImportedParentContext(ctx, import.location);
        }
        context->updateImportsCache();
    }

    const auto problems = session.problemsForFile(file);
    {
        DUChainWriteLocker lock;
        context->setProblems(problems);
    }

    Builder::visit(session.unit(), file, includedFiles, update);

    DUChain::self()->emitUpdateReady(path, context);

    return context;
}

DeclarationPointer ClangHelpers::findDeclaration(CXSourceLocation location, QualifiedIdentifier id, const ReferencedTopDUContext& top)
{
    if (!top) {
        // may happen for cyclic includes
        return {};
    }

    auto cursor = CursorInRevision(ClangLocation(location));
    DUChainReadLocker lock;

    QList<Declaration*> decls;
    if (!id.isEmpty())
    {
        decls = top->findDeclarations(id);
        foreach (Declaration* decl, decls)
        {
            if (decl->range().contains(cursor) ||
                (decl->range().isEmpty() && decl->range().start == cursor))
            {
                return DeclarationPointer(decl);
            }
        }
    }

    // there was no match based on the IDs, try the classical
    // range based search (very slow)

    Q_ASSERT(top);
    if (DUContext *local = top->findContextAt(cursor)) {
        if (local->owner() && local->owner()->range().contains(cursor)) {
           return DeclarationPointer(local->owner());
        }
        return DeclarationPointer(local->findDeclarationAt(cursor));
    }
    return {};
}

DeclarationPointer ClangHelpers::findDeclaration(CXCursor cursor, const IncludeFileContexts& includes)
{
    auto location = clang_getCursorLocation(cursor);
    CXFile file = nullptr;
    clang_getFileLocation(location, &file, nullptr, nullptr, nullptr);
    if (!file) {
        return {};
    }

    // build a qualified identifier by following the chain of semantic parents
    QList<Identifier> ids;
    CXCursor currentCursor = cursor;
    while (currentCursor.kind != CXCursor_TranslationUnit &&
           currentCursor.kind != CXCursor_InvalidFile)
    {
        ids << Identifier(ClangString(clang_getCursorSpelling(currentCursor)).toString());
        currentCursor = clang_getCursorSemanticParent(currentCursor);
    }
    QualifiedIdentifier qid;
    for (int i = ids.size()-1; i >= 0; --i)
    {
        qid.push(ids[i]);
    }

    return findDeclaration(location, qid, includes.value(file));
}

DeclarationPointer ClangHelpers::findDeclaration(CXType type, const IncludeFileContexts& includes)
{
    CXCursor cursor = clang_getTypeDeclaration(type);
    return findDeclaration(cursor, includes);
}

DeclarationPointer ClangHelpers::findForwardDeclaration(CXType type, DUContext* context, CXCursor cursor)
{
    if(type.kind != CXType_Record && type.kind != CXType_ObjCInterface && type.kind != CXType_ObjCClass){
        return {};
    }

    auto qualifiedIdentifier = QualifiedIdentifier(ClangString(clang_getTypeSpelling(type)).toString());

    DUChainReadLocker lock;
    auto decls = context->findDeclarations(qualifiedIdentifier,
        CursorInRevision(ClangLocation(clang_getCursorLocation(cursor)))
    );

    foreach (auto decl, decls) {
        if (decl->isForwardDeclaration()) {
            return DeclarationPointer(decl);
        }
    }
    return {};
}

RangeInRevision ClangHelpers::cursorSpellingNameRange(CXCursor cursor, const Identifier& id)
{
    auto range = ClangRange(clang_Cursor_getSpellingNameRange(cursor, 0, 0)).toRangeInRevision();
#if CINDEX_VERSION_MINOR < 29
    auto kind = clang_getCursorKind(cursor);
    // Clang used to report invalid ranges for destructors and methods like 'operator='
    if (kind == CXCursor_Destructor || kind == CXCursor_CXXMethod) {
        range.end.column = range.start.column + id.toString().length();
    }
#endif
    Q_UNUSED(id);
    return range;
}

QStringList ClangHelpers::headerExtensions()
{
    static const QStringList headerExtensions = {
            QStringLiteral("h"),
            QStringLiteral("H"),
            QStringLiteral("hh"),
            QStringLiteral("hxx"),
            QStringLiteral("hpp"),
            QStringLiteral("tlh"),
            QStringLiteral("h++"),
    };
    return headerExtensions;
}

QStringList ClangHelpers::sourceExtensions()
{
    static const QStringList sourceExtensions = {
        QStringLiteral("c"),
        QStringLiteral("cc"),
        QStringLiteral("cpp"),
        QStringLiteral("c++"),
        QStringLiteral("cxx"),
        QStringLiteral("C"),
        QStringLiteral("m"),
        QStringLiteral("mm"),
        QStringLiteral("M"),
        QStringLiteral("inl"),
        QStringLiteral("_impl.h"),
    };
    return sourceExtensions;
}

bool ClangHelpers::isSource(const QString& path)
{
    const auto& extensions = sourceExtensions();
    return std::any_of(extensions.constBegin(), extensions.constEnd(),
                       [&](const QString& ext) { return path.endsWith(ext); });
}

bool ClangHelpers::isHeader(const QString& path)
{
    const auto& extensions = headerExtensions();
    return std::any_of(extensions.constBegin(), extensions.constEnd(),
                       [&](const QString& ext) { return path.endsWith(ext); });
}
