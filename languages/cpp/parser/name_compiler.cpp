/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

//krazy:excludeall=cpp

#include "name_compiler.h"
#include "type_compiler.h"
#include "lexer.h"
#include "symbol.h"
#include "parsesession.h"
#include "tokens.h"

#include <QtCore/qdebug.h>

using namespace KDevelop;

QString decode(ParseSession* session, AST* ast, bool without_spaces = false)
{
  QString ret;
  if( without_spaces ) {
    //Decode operator-names without spaces for now, since we rely on it in other places.
    ///@todo change this, here and in all the places that rely on it. Operators should then by written like "operator [ ]"(space between each token)
    for( size_t a = ast->start_token; a < ast->end_token; a++ ) {
      const Token &tk = session->token_stream->token(a);
      ret += tk.symbol();
    }
  } else {
    for( size_t a = ast->start_token; a < ast->end_token; a++ ) {
      const Token &tk = session->token_stream->token(a);
      ret += tk.symbol() + " ";
    }
  }
  return ret;
}

Declaration::CVSpecs parseConstVolatile(ParseSession* session, const ListNode<std::size_t> *cv)
{
  Declaration::CVSpecs ret = Declaration::CVNone;

  if (cv) {
    const ListNode<std::size_t> *it = cv->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = session->token_stream->kind(it->element);
      if (kind == Token_const)
        ret |= Declaration::Const;
      else if (kind == Token_volatile)
        ret |= Declaration::Volatile;

      it = it->next;
    } while (it != end);
  }

  return ret;
}

TypeIdentifier typeIdentifierFromTemplateArgument(ParseSession* session, TemplateArgumentAST *node)
{
  TypeIdentifier id;
  if(node->expression) {
    id = TypeIdentifier(decode(session, node));
    id.setIsExpression(true);
  }else{
    //Parse the pointer operators
    TypeCompiler tc(session);
    tc.run(node->type_id->type_specifier);
    id = TypeIdentifier(tc.identifier());
    
    if(node->type_id->declarator && node->type_id->declarator->ptr_ops)
    {
      const ListNode<PtrOperatorAST*> *it = node->type_id->declarator->ptr_ops->toFront();
      const ListNode<PtrOperatorAST*> *end = it; ///@todo check ordering, eventually walk the list in reversed order
      do
        {
          if(it->element) {
            if( session->token_stream->token(it->element->op).symbol().startsWith('&')) {
              //We're handling a 'reference'
              id.setIsReference(true);
              if(it->element->cv)
                id.setIsConstant(parseConstVolatile(session, it->element->cv) & Declaration::Const);
            } else {
              //We're handling a real pointer
              id.setPointerDepth(id.pointerDepth()+1);

              if(it->element->cv)
                id.setIsConstant(parseConstVolatile(session, it->element->cv) & Declaration::Const);
            }
          }
          it = it->next;
        }
      while (it != end);
    }
  }
  return id;
}

NameCompiler::NameCompiler(ParseSession* session)
  : m_session(session)
{
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
        tmp_name +=  decode(m_session, op_id->op, true);
      else
        tmp_name += QLatin1String("{...cast...}");

      m_typeSpecifier = op_id->type_specifier;
    }

  m_currentIdentifier = Identifier(m_session->unify(tmp_name));

  if (node->template_arguments)
    {
      visitNodes(this, node->template_arguments);
    }

  _M_name.push(m_currentIdentifier);
}

TypeSpecifierAST* NameCompiler::lastTypeSpecifier() const {
  return m_typeSpecifier;
}

void NameCompiler::visitTemplateArgument(TemplateArgumentAST *node)
{
  m_currentIdentifier.appendTemplateIdentifier( typeIdentifierFromTemplateArgument(m_session, node) );
}

const QualifiedIdentifier& NameCompiler::identifier() const
{
  return _M_name;
}

void NameCompiler::run(NameAST *node)
{
  m_typeSpecifier = 0; internal_run(node); _M_name.setExplicitlyGlobal( node->global );
}
