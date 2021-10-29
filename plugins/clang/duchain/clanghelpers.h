/*
    SPDX-FileCopyrightText: 2014 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CLANGHELPERS_H
#define CLANGHELPERS_H

#include "clangprivateexport.h"

#include <language/duchain/duchainpointer.h>
#include <language/duchain/topducontext.h>

#include <clang-c/Index.h>

#include <functional>

class ParseSession;
class ClangIndex;

namespace KDevelop
{
class ModificationRevision;
}

struct Import
{
    CXFile file;
    KDevelop::CursorInRevision location;
};

Q_DECLARE_TYPEINFO(Import, Q_MOVABLE_TYPE);

using Imports = QMultiHash<CXFile, Import>;
using IncludeFileContexts = QHash<CXFile, KDevelop::ReferencedTopDUContext>;
using UnsavedRevisions = QHash<KDevelop::IndexedString, KDevelop::ModificationRevision>;

namespace ClangHelpers {

KDevelop::DeclarationPointer findDeclaration(CXSourceLocation cursor, const KDevelop::QualifiedIdentifier& id, const KDevelop::ReferencedTopDUContext& top);
KDevelop::DeclarationPointer findDeclaration(CXCursor cursor, const IncludeFileContexts& includes);
KDevelop::DeclarationPointer findDeclaration(CXType type, const IncludeFileContexts& includes);

/**
 * Try to look up the first reachable forward declaration for type @a type
 *
 * @param context The context where this search is happening
 * @param cursor The location from which we're searching
 */
KDevelop::DeclarationPointer findForwardDeclaration(CXType type, KDevelop::DUContext* context, CXCursor cursor);

/**
 * Wrapper for @ref clang_Cursor_getSpellingNameRange which sometimes reports invalid ranges
 */
KDevelop::RangeInRevision cursorSpellingNameRange(CXCursor cursor, const KDevelop::Identifier& id);

/**
 * @returns all the Imports for each file in the @a tu
 */
KDEVCLANGPRIVATE_EXPORT Imports tuImports(CXTranslationUnit tu);

/**
 * Recursively builds a duchain with the specified @a features for the
 * @a file and each of its @a imports using the TU from @a session.
 * The resulting contexts are placed in @a includedFiles.
 * @returns the context created for @a file
 */
KDEVCLANGPRIVATE_EXPORT KDevelop::ReferencedTopDUContext
buildDUChain(CXFile file, const Imports& imports, const ParseSession& session,
             KDevelop::TopDUContext::Features features, IncludeFileContexts& includedFiles,
             const UnsavedRevisions& unsavedRevisions, const KDevelop::IndexedString& parseDocument,
             ClangIndex* index = nullptr, const std::function<bool()>& abortFunction = {});

/**
 * @return List of possible header extensions used for definition/declaration fallback switching
 */
QStringList headerExtensions();

/**
 * @return List of possible source extensions used for definition/declaration fallback switching
 */
QStringList sourceExtensions();

/**
 * @return True if the given file @a path has the extension of a C++ source file
 */
KDEVCLANGPRIVATE_EXPORT bool isSource(const QString& path);

/**
 * @return True if the given file @a path has the extension of a C++ header file
 */
KDEVCLANGPRIVATE_EXPORT bool isHeader(const QString& path);

KDEVCLANGPRIVATE_EXPORT QString clangVersion();

/**
 * @return The path containing Clang built includes (e.g. stddef.h, stdarg.h, cpuid.h)
 * The returned path is the env. var KDEV_CLANG_BUILTIN_DIR when set otherwise the path
 * to the headers used when kdev-clang was built, possibly updated for  upgrades to
 * the library (e.g. 7.0.0 -> 7.0.1).
 * Returns an empty string if none of the checked locations contain the file cpuid.h .
 *
 * Also see: https://clang.llvm.org/docs/FAQ.html
 */
KDEVCLANGPRIVATE_EXPORT QString clangBuiltinIncludePath();

/**
 * @return True if the given @a path is a valid clang builtin directory.
 */
KDEVCLANGPRIVATE_EXPORT bool isValidClangBuiltingIncludePath(const QString& path);

}

#endif //CLANGHELPERS_H
