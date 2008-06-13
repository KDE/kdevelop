/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "type_visitor.h"
#include "name_visitor.h"
#include "lexer.h"
#include "symbol.h"
#include "tokens.h"
#include "parsesession.h"
#include "cppduchain.h"
#include "expressionvisitor.h"
#include <duchain/duchainlock.h>



#include <QtCore/QString>

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())


TypeASTVisitor::TypeASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, bool debug) : m_session(session), m_visitor(visitor), m_context(context), m_source(source), m_debug(debug)
{
  m_position = m_context->range().end;
}

void TypeASTVisitor::run(TypeSpecifierAST *node)
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
          int kind = m_session->token_stream->kind(it->element);
          if (! _M_cv.contains(kind))
            _M_cv.append(kind);

          it = it->next;
        }
      while (it != end);
    }
}

void TypeASTVisitor::visitClassSpecifier(ClassSpecifierAST *node)
{
  visit(node->name);
}

void TypeASTVisitor::visitEnumSpecifier(EnumSpecifierAST *node)
{
  visit(node->name);
}

void TypeASTVisitor::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  visit(node->name);
}

QList<KDevelop::DeclarationPointer> TypeASTVisitor::declarations() const
{
  return m_declarations;
}

void TypeASTVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  Cpp::FindDeclaration find( m_context, m_source, DUContext::NoSearchFlags, m_context->range().end );
  find.openQualifiedIdentifier(false);
  
  if (const ListNode<std::size_t> *it = node->integrals)
    {
      it = it->toFront();
      const ListNode<std::size_t> *end = it;
      do
        {
          std::size_t token = it->element;
          // FIXME
          Identifier id(token_name(m_session->token_stream->kind(token)));
          find.openIdentifier(id);

          {
            LOCKDUCHAIN;
            find.closeIdentifier(it == end);
            _M_type.push(id);
          }
          
          if( !find.lastDeclarations().isEmpty() ) {
            bool had = false;
            foreach(DeclarationPointer decl, find.lastDeclarations()) {
              if(decl && !decl->isForwardDeclaration()) {
                //Prefer registering non forward-declaration uses
                m_visitor->newUse( node, token, token+1, decl );
                had = true;
                break;
              }
            }
            
            if(!had) //only forward-declarations, register to any.
              m_visitor->newUse( node, token, token+1, find.lastDeclarations()[0] );
          } else if( m_debug )
            kDebug( 9007 ) << "failed to find " << id << " as part of " << decode( m_session, node) << ", searched in " << find.describeLastContext();

          it = it->next;
        }
      while (it != end);
    }
  else if (node->type_of)
    {
      // ### implement me
      _M_type.push(Identifier("typeof<...>"));
    }

  {
    LOCKDUCHAIN;
    find.closeQualifiedIdentifier();
    m_declarations = find.lastDeclarations();
  }
  
  visit(node->name);
}

void TypeASTVisitor::visitName(NameAST *node)
{
  NameASTVisitor name_cc(m_session, m_visitor, m_context, m_source, m_position, KDevelop::DUContext::NoSearchFlags, m_debug);
  name_cc.run(node);
  _M_type = name_cc.identifier();
  m_declarations = name_cc.declarations();
}

QStringList TypeASTVisitor::cvString() const
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

bool TypeASTVisitor::isConstant() const
{
    return _M_cv.contains(Token_const);
}

bool TypeASTVisitor::isVolatile() const
{
    return _M_cv.contains(Token_volatile);
}

QualifiedIdentifier TypeASTVisitor::identifier() const
{
  return _M_type;
}

