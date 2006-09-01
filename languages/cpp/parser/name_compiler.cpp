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

#include "name_compiler.h"
#include "type_compiler.h"
#include "lexer.h"
#include "symbol.h"
#include "parsesession.h"

#include <QtCore/qdebug.h>

NameCompiler::NameCompiler(ParseSession* session)
  : m_session(session)
{
}

QString NameCompiler::decode_operator(std::size_t index) const
{
  const Token &tk = m_session->token_stream->token(index);
  return tk.symbol();
}

void NameCompiler::internal_run(AST *node)
{
  _M_name.clear();
  visit(node);
}

void NameCompiler::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  QString tmp_name;

  if (node->tilde)
    tmp_name += QLatin1String("~");

  if (node->id)
    tmp_name += m_session->token_stream->token(node->id).symbol();

  if (OperatorFunctionIdAST *op_id = node->operator_id)
    {
#if defined(__GNUC__)
#warning "NameCompiler::visitUnqualifiedName() -- implement me"
#endif

      tmp_name += QLatin1String("operator");

      if (op_id->op && op_id->op->op)
        tmp_name +=  decode_operator(op_id->op->op);
      else
        tmp_name += QLatin1String("<...cast...>");
    }

  m_currentIdentifier = Identifier(tmp_name);

  if (node->template_arguments)
    {
      visitNodes(this, node->template_arguments);
    }

  _M_name.push(m_currentIdentifier);
}

void NameCompiler::visitTemplateArgument(TemplateArgumentAST *node)
{
  if (node->type_id && node->type_id->type_specifier)
    {
      TypeCompiler type_cc(m_session);
      type_cc.run(node->type_id->type_specifier);
      m_currentIdentifier.appendTemplateIdentifier(QualifiedIdentifier(type_cc.identifier()));
    }
}

// kate: space-indent on; indent-width 2; replace-tabs on;

const QualifiedIdentifier& NameCompiler::identifier() const
{
  return _M_name;
}
