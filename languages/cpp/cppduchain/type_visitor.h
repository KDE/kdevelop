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

#ifndef TYPE_VISITOR_H
#define TYPE_VISITOR_H

#include "default_visitor.h"
#include <identifier.h>
#include <cppduchainexport.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

#include <duchain/ducontext.h>
#include <duchain/duchainpointer.h>


class ParseSession;

namespace Cpp {
class ExpressionVisitor;
}

/** DUChain must not be locked when using this function */
class KDEVCPPDUCHAIN_EXPORT TypeASTVisitor: protected DefaultVisitor
{
public:
  TypeASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::DUContext::ImportTrace& trace);

  KDevelop::QualifiedIdentifier identifier() const;
  inline QStringList qualifiedName() const { return _M_type.toStringList(); }
  inline QList<int> cv() const { return _M_cv; }

  bool isConstant() const;
  bool isVolatile() const;

  QStringList cvString() const;

  void run(TypeSpecifierAST *node);

  QList<KDevelop::DeclarationPointer> declarations() const;
  
protected:
  virtual void visitClassSpecifier(ClassSpecifierAST *node);
  virtual void visitEnumSpecifier(EnumSpecifierAST *node);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node);

  virtual void visitName(NameAST *node);

private:
  ParseSession* m_session;
  Cpp::ExpressionVisitor* m_visitor;
  const KDevelop::DUContext* m_context;
  KTextEditor::Cursor m_position;
  QList<KDevelop::DeclarationPointer> m_declarations;
  KDevelop::DUContext::ImportTrace m_trace;
  KDevelop::QualifiedIdentifier _M_type;
  QList<int> _M_cv;
};

#endif // TYPE_VISITOR_H

