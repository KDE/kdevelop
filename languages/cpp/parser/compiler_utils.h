/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef COMPILER_UTILS_H
#define COMPILER_UTILS_H

#include <utility>

#include "codemodel.h"

class QString;
class QStringList;
struct TypeSpecifierAST;
struct DeclaratorAST;
class TokenStream;

namespace CompilerUtils
{

TypeInfo typeDescription(TypeSpecifierAST *type_specifier, DeclaratorAST *declarator, TokenStream *token_stream);

} // namespace CompilerUtils

#endif // COMPILER_UTILS_H

// kate: space-indent on; indent-width 2; replace-tabs on;
