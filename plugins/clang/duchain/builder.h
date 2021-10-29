/*
    SPDX-FileCopyrightText: 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BUILDER_H
#define BUILDER_H

#include "clangprivateexport.h"

#include "clanghelpers.h"

namespace Builder {

/**
 * Visit the AST in @p tu and build declarations for cursors belonging to @p file.
 * 
 * @param update Set to true when an existing DUChain cache is getting updated.
 */
KDEVCLANGPRIVATE_EXPORT void visit(CXTranslationUnit tu, CXFile file,
                                   const IncludeFileContexts& includes, const bool update);

}

#endif //BUILDER_H
