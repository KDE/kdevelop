/*
 *    This file is part of KDevelop
 *
 *    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
 *    Copyright 2015 Milian Wolff <mail@milianw.de
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef BUILDER_H
#define BUILDER_H

#include <duchain/clangduchainexport.h>

#include "clanghelpers.h"

namespace Builder {

/**
 * Visit the AST in @p tu and build declarations for cursors belonging to @p file.
 * 
 * @param update Set to true when an existing DUChain cache is getting updated.
 */
KDEVCLANGDUCHAIN_EXPORT void visit(CXTranslationUnit tu, CXFile file,
                                   const IncludeFileContexts& includes, const bool update);

}

#endif //BUILDER_H
