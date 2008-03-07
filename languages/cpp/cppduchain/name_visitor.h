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

#ifndef NAME_VISITOR_H
#define NAME_VISITOR_H

#include "default_visitor.h"
#include <identifier.h>
#include <cppduchainexport.h>
#include "cppducontext.h"
#include <ducontext.h>

class ParseSession;
class OperatorAST;

namespace KDevelop {
  class SimpleCursor;
}

namespace Cpp {
  class ExpressionVisitor;
}

///This searches for the name while walking its AST.
class KDEVCPPDUCHAIN_EXPORT NameASTVisitor: protected DefaultVisitor
{
public:
  NameASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::ImportTrace& trace, const KDevelop::SimpleCursor& position, KDevelop::DUContext::SearchFlags localSearchFlags = KDevelop::DUContext::NoSearchFlags, bool debug = false);

  void run(NameAST *node, bool skipLastNamePart = false);
  void run(UnqualifiedNameAST *node);

  QString name() const { return _M_name.toString(); }
  QStringList qualifiedName() const { return _M_name.toStringList(); }

  const KDevelop::QualifiedIdentifier& identifier() const;

  /**
   * When the name contains one type-specifier, this should contain that specifier after the run.
   * Especially this is the type-specifier of a conversion-operator like "operator int()"
   * */
  TypeSpecifierAST* lastTypeSpecifier() const;

  ///Retrieve the declarations found for the name
  QList<KDevelop::DeclarationPointer> declarations() const;
  
protected:
  virtual void visitUnqualifiedName(UnqualifiedNameAST *node);
  virtual void visitTemplateArgument(TemplateArgumentAST *node);

private:
  ParseSession* m_session;
  Cpp::ExpressionVisitor* m_visitor;
  const KDevelop::DUContext* m_context;
  KDevelop::ImportTrace m_trace;
  TypeSpecifierAST* m_typeSpecifier;
  KDevelop::Identifier m_currentIdentifier;
  KDevelop::QualifiedIdentifier _M_name;
  Cpp::FindDeclaration m_find;
  bool m_debug;
  UnqualifiedNameAST* m_finalName;
};

QString decode(ParseSession* session, AST* ast, bool without_spaces = false);

#endif // NAME_VISITOR_H

