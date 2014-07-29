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

#ifndef CLANGHELPERS_H
#define CLANGHELPERS_H

#include <clang-c/Index.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/topducontext.h>

#include "duchainexport.h"

class ParseSession;

struct Import
{
    CXFile file;
    KDevelop::CursorInRevision location;
};

using Imports = QMultiHash<CXFile, Import>;
using IncludeFileContexts = QHash<CXFile, KDevelop::ReferencedTopDUContext>;

namespace ClangHelpers {

KDevelop::DeclarationPointer findDeclaration(CXCursor cursor, const IncludeFileContexts& includes);
KDevelop::DeclarationPointer findDeclaration(CXType type, const IncludeFileContexts& includes);

/**
 * @returns all the Imports for each file in the @param tu
 */
KDEVCLANGDUCHAIN_EXPORT Imports tuImports(CXTranslationUnit tu);

/**
 * Recursively builds a duchain with the specified @param features for the
 * @param file and each of its @param imports using the TU from @param session.
 * The resulting contexts are placed in @param includedFiles.
 * @returns the context created for @param file
 */
KDEVCLANGDUCHAIN_EXPORT KDevelop::ReferencedTopDUContext buildDUChain(
    CXFile file, const Imports& imports, const ParseSession& session,
    KDevelop::TopDUContext::Features features, IncludeFileContexts& includedFiles);

/**
 * @return List of possible header extensions used for definition/declaration fallback switching
 */
QStringList headerExtensions();

/**
 * @return List of possible source extensions used for definition/declaration fallback switching
 */
QStringList sourceExtensions();

/**
 * @return True if the given file @p path has the extension of a C++ source file, not of a header.
 */
bool isSource(const QString& path);

}

#endif //CLANGHELPERS_H
