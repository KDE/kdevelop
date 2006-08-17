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

#include "declarator_compiler.h"
#include "name_compiler.h"
#include "type_compiler.h"
#include "compiler_utils.h"
#include "lexer.h"
#include "parsesession.h"

DeclaratorCompiler::DeclaratorCompiler(ParseSession* session)
  : m_session(session)
{
}

void DeclaratorCompiler::run(DeclaratorAST *node)
{
  _M_id.clear();
  _M_parameters.clear();
  _M_array.clear();
  _M_function = false;
  _M_reference = false;
  _M_indirection = 0;

  if (node)
    {
      NameCompiler name_cc(m_session);
      name_cc.run(node->id);
      _M_id = name_cc.name();
      _M_function = (node->parameter_declaration_clause != 0);
      visitNodes(this, node->ptr_ops);
      visit(node->parameter_declaration_clause);

      if (const ListNode<ExpressionAST*> *it = node->array_dimensions)
        {
          it->toFront();
          const ListNode<ExpressionAST*> *end = it;

          do
            {
              _M_array += QLatin1String("[");

              if (ExpressionAST *expr = it->element)
                {
                  const Token &start_token = m_session->token_stream->token(expr->start_token);
                  const Token &end_token = m_session->token_stream->token(expr->end_token);

                  _M_array += QString::fromUtf8(&start_token.text[start_token.position],
                                                end_token.position - start_token.position).trimmed();
                }

              _M_array += QLatin1String("]");

              it = it->next;
            }
          while (it != end);
        }
    }
}

void DeclaratorCompiler::visitPtrOperator(PtrOperatorAST *node)
{
    std::size_t op =  m_session->token_stream->kind(node->op);
  switch (op)
    {
      case '&':
        _M_reference = true;
        break;
      case '*':
        ++_M_indirection;
        break;

      default:
        break;
    }

  if (node->mem_ptr)
    {
#if defined(__GNUC__)
#warning "ptr to mem -- not implemented"
#endif
    }
}

void DeclaratorCompiler::visitParameterDeclaration(ParameterDeclarationAST *node)
{
  Parameter p;

  TypeCompiler type_cc(m_session);
  DeclaratorCompiler decl_cc(m_session);

  decl_cc.run(node->declarator);

  p.name = decl_cc.id();
  p.type = CompilerUtils::typeDescription(node->type_specifier, node->declarator, m_session);
  p.defaultValue = (node->expression != 0);

  _M_parameters.append(p);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
