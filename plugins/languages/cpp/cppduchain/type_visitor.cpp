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
#include <language/duchain/duchainlock.h>



#include <QtCore/QString>

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())


TypeASTVisitor::TypeASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, bool debug) : m_session(session), m_visitor(visitor), m_context(context), m_source(source), m_debug(debug)
{
  m_position = m_context->range().end;
}

void TypeASTVisitor::run(TypeSpecifierAST *node)
{
  m_typeId.clear();
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

TypePtr< KDevelop::AbstractType > TypeASTVisitor::type() const {
  return m_type;
}

QList<KDevelop::DeclarationPointer> TypeASTVisitor::declarations() const
{
  return m_declarations;
}

void TypeASTVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  ///@todo Merge this with ExpressionVisitor::visitSimpleTypeSpecifier
  Cpp::FindDeclaration find( m_context, m_source, DUContext::NoSearchFlags, m_context->range().end );
  find.openQualifiedIdentifier(false);
  
  if (const ListNode<std::size_t> *it = node->integrals)
    {
      uint type = IntegralType::TypeNone;
      uint modifiers = AbstractType::NoModifiers;

      const ListNode<std::size_t> *it = node->integrals->toFront();
      const ListNode<std::size_t> *end = it;
      do {
        int kind = m_session->token_stream->kind(it->element);
        switch (kind) {
          case Token_char:
            type = IntegralType::TypeChar;
            break;
          case Token_wchar_t:
            type = IntegralType::TypeWchar_t;
            break;
          case Token_bool:
            type = IntegralType::TypeBoolean;
            break;
          case Token_short:
            modifiers |= AbstractType::ShortModifier;
            break;
          case Token_int:
            type = IntegralType::TypeInt;
            break;
          case Token_long:
            if (modifiers & AbstractType::LongModifier)
              modifiers |= AbstractType::LongLongModifier;
            else
              modifiers |= AbstractType::LongModifier;
            break;
          case Token_signed:
            modifiers |= AbstractType::SignedModifier;
            break;
          case Token_unsigned:
            modifiers |= AbstractType::UnsignedModifier;
            break;
          case Token_float:
            type = IntegralType::TypeFloat;
            break;
          case Token_double:
            type = IntegralType::TypeDouble;
            break;
          case Token_void:
            type = IntegralType::TypeVoid;
            break;
        }

        it = it->next;
      } while (it != end);

      if(type == IntegralType::TypeNone)
        type = IntegralType::TypeInt; //Happens, example: "unsigned short"

      KDevelop::IntegralType::Ptr integral ( new KDevelop::IntegralType(type) );
      integral->setModifiers(modifiers);
      
      m_type = integral.cast<AbstractType>();
      
      m_typeId = TypeIdentifier(integral->toString());
    }
  else if (node->type_of)
    {
      // ### implement me
      m_typeId.push(Identifier("typeof<...>"));
    }

  {
    LOCKDUCHAIN;
    find.closeQualifiedIdentifier();
    m_declarations = find.lastDeclarations();
    if(!m_declarations.isEmpty())
      m_type = m_declarations[0]->abstractType();
  }
  
  visit(node->name);
}

void TypeASTVisitor::visitName(NameAST *node)
{
  NameASTVisitor name_cc(m_session, m_visitor, m_context, m_source, m_position, KDevelop::DUContext::NoSearchFlags, m_debug);
  name_cc.run(node);
  m_typeId = name_cc.identifier();
  m_declarations = name_cc.declarations();
  if(!m_declarations.isEmpty())
    m_type = m_declarations[0]->abstractType();
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
  return m_typeId;
}

