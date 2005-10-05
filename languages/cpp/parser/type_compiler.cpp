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

#include "type_compiler.h"
#include "name_compiler.h"
#include "lexer.h"
#include "symbol.h"
#include "tokens.h"

#include <QtCore/QString>

TypeCompiler::TypeCompiler(TokenStream *token_stream)
  : _M_token_stream(token_stream)
{
}

void TypeCompiler::run(TypeSpecifierAST *node)
{
  _M_type.clear();
  _M_cv.clear();

  visit(node);

  if (node && node->cv)
    {
      const ListNode<std::size_t> *it = node->cv->toFront();
      const ListNode<std::size_t> *end = it;
      do
        {
          int kind = _M_token_stream->kind(it->element);
          if (! _M_cv.contains(kind))
            _M_cv.append(kind);

          it = it->next;
        }
      while (it != end);
    }
}

void TypeCompiler::visitClassSpecifier(ClassSpecifierAST *node)
{
  visit(node->name);
}

void TypeCompiler::visitEnumSpecifier(EnumSpecifierAST *node)
{
  visit(node->name);
}

void TypeCompiler::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  visit(node->name);
}

void TypeCompiler::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  if (const ListNode<std::size_t> *it = node->integrals)
    {
      it = it->toFront();
      const ListNode<std::size_t> *end = it;
      do
        {
          std::size_t token = it->element;
          _M_type += token_name(_M_token_stream->kind(token));
          it = it->next;
        }
      while (it != end);
    }
  else if (node->type_of)
    {
      // ### implement me
      _M_type += QLatin1String("typeof<...>");
    }

  visit(node->name);
}

void TypeCompiler::visitName(NameAST *node)
{
  NameCompiler name_cc(_M_token_stream);
  name_cc.run(node);
  _M_type = name_cc.qualifiedName();
}

QStringList TypeCompiler::cvString() const
{
  QStringList lst;

  foreach (int q, cv())
    {
      if (q == Token_const)
        lst.append(QLatin1String("const"));
      else if (q == Token_volatile)
        lst.append(QLatin1String("volatile"));
    }

  return lst;
}

bool TypeCompiler::isConstant() const
{
    return _M_cv.contains(Token_const);
}

bool TypeCompiler::isVolatile() const
{
    return _M_cv.contains(Token_volatile);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
