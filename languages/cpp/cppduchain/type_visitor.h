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
#include <language/duchain/identifier.h>
#include <cppduchainexport.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

#include <language/duchain/ducontext.h>
#include <language/duchain/duchainpointer.h>


class ParseSession;

namespace Cpp {
class ExpressionVisitor;
}

/** DUChain must not be locked when using this function */
class KDEVCPPDUCHAIN_EXPORT TypeASTVisitor: protected DefaultVisitor
{
public:
  TypeASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, const KDevelop::DUContext* localVisibilityContext, bool debug = false);

  KDevelop::QualifiedIdentifier identifier() const;
  inline QStringList qualifiedName() const { if(m_stopSearch) return QStringList(); return m_typeId.toStringList(); }
  inline QList<int> cv() const { if(m_stopSearch) return QList<int>(); return _M_cv; }

  bool isConstant() const;
  bool isVolatile() const;

  QStringList cvString() const;

  void run(TypeIdAST *node);
  
  void run(TypeSpecifierAST *node);

  KDevelop::AbstractType::Ptr type() const;
  
  QList<KDevelop::DeclarationPointer> declarations() const;
  
  void setSearchFlags(KDevelop::DUContext::SearchFlags flags) {
    m_flags = flags;
  }
  
  bool stoppedSearch() const {
    return m_stopSearch;
  }
  
protected:
  virtual void visitClassSpecifier(ClassSpecifierAST *node) override;
  virtual void visitEnumSpecifier(EnumSpecifierAST *node) override;
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node) override;
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node) override;

  virtual void visitName(NameAST *node) override;

private:
  ParseSession* m_session;
  Cpp::ExpressionVisitor* m_visitor;
  const KDevelop::DUContext* m_context;
  const KDevelop::DUContext* m_localContext;
  KDevelop::CursorInRevision m_position;
  QList<KDevelop::DeclarationPointer> m_declarations;
  const KDevelop::TopDUContext* m_source;
  KDevelop::QualifiedIdentifier m_typeId;
  KDevelop::AbstractType::Ptr m_type;
  KDevelop::DUContext::SearchFlags m_flags;
  QList<int> _M_cv;
  bool m_debug;
  bool m_stopSearch;
};

#endif // TYPE_VISITOR_H

