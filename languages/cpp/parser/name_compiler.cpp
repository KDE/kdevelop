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

uint parseConstVolatile(ParseSession* session, const ListNode<uint> *cv)
{
  uint ret = AbstractType::NoModifiers;

  if (cv) {
    const ListNode<uint> *it = cv->toFront();
    const ListNode<uint> *end = it;
    do {
      int kind = session->token_stream->kind(it->element);
      if (kind == Token_const)
        ret |= AbstractType::ConstModifier;
      else if (kind == Token_volatile)
        ret |= AbstractType::VolatileModifier;

      it = it->next;
    } while (it != end);
  }

  return ret;
}

IndexedTypeIdentifier typeIdentifierFromTemplateArgument(ParseSession* session, TemplateArgumentAST *node)
{
  IndexedTypeIdentifier id;
  if(node->expression) {
    id = IndexedTypeIdentifier(session->stringForNode(node), true);
  }else if(node->type_id) {
    //Parse the pointer operators
    TypeCompiler tc(session);
    tc.run(node->type_id->type_specifier);
    id = IndexedTypeIdentifier(tc.identifier());
    //node->type_id->type_specifier->cv
    
    if(node->type_id->type_specifier) {
      uint cv = parseConstVolatile(session, node->type_id->type_specifier->cv);
      id.setIsConstant(cv & AbstractType::ConstModifier);
      id.setIsVolatile(cv & AbstractType::VolatileModifier);
    }
    
    if(node->type_id->declarator && node->type_id->declarator->ptr_ops)
    {
      const ListNode<PtrOperatorAST*> *it = node->type_id->declarator->ptr_ops->toFront();
      const ListNode<PtrOperatorAST*> *end = it; ///@todo check ordering, eventually walk the list in reversed order
      do
        {
          if(it->element && it->element->op) { ///@todo What about ptr-to-member?
            int kind = session->token_stream->kind(it->element->op);
            if(kind == '&') {
              //We're handling a 'reference'
              id.setIsReference(true);
            } else if(kind == Token_and) {
              //We're handling an rvalue-reference
              id.setIsReference(true);
              id.setIsRValue(true);
            } else {
              //We're handling a real pointer
              id.setPointerDepth(id.pointerDepth()+1);

              if(it->element->cv) {
                id.setIsConstPointer(id.pointerDepth()-1, parseConstVolatile(session, it->element->cv) & AbstractType::ConstModifier);
              }
            }
          }
          it = it->next;
        }
      while (it != end);
    } else if (node->type_id->declarator && node->type_id->declarator->array_dimensions) {
      ///FIXME: find a way to put this information into the identifier
      ///       e.g.: id.setArrayDepth(id.arrayDepth() + 1) ?
      const ListNode< ExpressionAST* >* it = node->type_id->declarator->array_dimensions->toFront();
      const ListNode< ExpressionAST* >* end = node->type_id->declarator->array_dimensions-> toBack();
      do {
        QualifiedIdentifier qid = id.identifier();
        Identifier last = qid.last();
        qid.pop();
        last.setIdentifier(last.toString() + "[]");
        qid.push(last);
        id.setIdentifier(IndexedQualifiedIdentifier( qid ));
        it = it->next;
      } while (it != end);
    }
  }
  return id;
}

NameCompiler::NameCompiler(ParseSession* session)
  : m_session(session)
  , m_typeSpecifier(nullptr)
  , _M_name(&m_localName)
{
}

void NameCompiler::internal_run(AST *node)
{
  _M_name->clear();
  visit(node);
}

void NameCompiler::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  IndexedString tmp_name;

  if (node->id)
    tmp_name = m_session->token_stream->symbol(node->id);

  if (node->ellipsis)
    tmp_name = IndexedString("...");
  
  if (node->tilde)
    tmp_name = IndexedString('~' + tmp_name.byteArray());

  if (OperatorFunctionIdAST *op_id = node->operator_id)
    {
      static QString operatorString("operator");
      QString tmp = operatorString;

      if (op_id->op && op_id->op->op)
        tmp +=  m_session->stringForNode(op_id->op, true);
      else
        tmp += QLatin1String("{...cast...}");

      tmp_name = IndexedString(tmp);
      m_typeSpecifier = op_id->type_specifier;
    }

  m_currentIdentifier = Identifier(tmp_name);

  if (node->template_arguments)
    {
      visitNodes(this, node->template_arguments);
    }
  else if (node->end_token == node->start_token + 3 && node->id == node->start_token
      && m_session->token_stream->kind(node->id+1) == '<')
    {
      ///@todo Represent this nicer in the AST
      ///It's probably a type-specifier with instantiation of the default-parameter, like "Bla<>".
      m_currentIdentifier.appendTemplateIdentifier( IndexedTypeIdentifier() );
    }

  _M_name->push(m_currentIdentifier);
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
  return *_M_name;
}

void NameCompiler::run(NameAST *node, QualifiedIdentifier* target)
{
  if(target)
    _M_name = target;
  else
    _M_name = &m_localName;
    
  m_typeSpecifier = 0; internal_run(node); if(node && node->global) _M_name->setExplicitlyGlobal( node->global );
}
