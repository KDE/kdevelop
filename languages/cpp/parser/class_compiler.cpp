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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "class_compiler.h"
#include "lexer.h"

ClassCompiler::ClassCompiler(TokenStream *token_stream)
  : _M_token_stream(token_stream),
     name_cc(token_stream),
     type_cc(token_stream)
{
}

ClassCompiler::~ClassCompiler()
{
}

void ClassCompiler::run(ClassSpecifierAST *node)
{
  name_cc.run(node->name);
  _M_name = name_cc.name();
  _M_base_classes.clear();

  visit(node);
}

void ClassCompiler::visitClassSpecifier(ClassSpecifierAST *node)
{
  visit(node->base_clause);
}

void ClassCompiler::visitBaseSpecifier(BaseSpecifierAST *node)
{
  name_cc.run(node->name);
  QString name = name_cc.name();

  if (! name.isEmpty())
    _M_base_classes.append(name);
}


// kate: space-indent on; indent-width 2; replace-tabs on;
