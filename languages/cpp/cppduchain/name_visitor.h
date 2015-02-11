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
#include <language/duchain/identifier.h>
#include <cppduchainexport.h>
#include "cppducontext.h"
#include <language/duchain/ducontext.h>

class ParseSession;
class OperatorAST;

namespace KDevelop {
  class CursorInRevision;
}

namespace Cpp {
  class ExpressionVisitor;
}

///This searches for the name while walking its AST.
class KDEVCPPDUCHAIN_EXPORT NameASTVisitor: protected DefaultVisitor
{
public:
  ///@param context Context in which to look up the name
  ///@param source The source top-context from where the parsing was triggerd
  ///@param localVisibilityContext Local context from where to look up template parameters
  NameASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, const KDevelop::DUContext* localVisibilityContext, const KDevelop::CursorInRevision& position, KDevelop::DUContext::SearchFlags localSearchFlags = KDevelop::DUContext::NoSearchFlags, bool debug = false);

  void run(NameAST *node, bool skipLastNamePart = false);
  ///@param skipThisName if this is true, only the template-parameters of the given node are processed
  void run(UnqualifiedNameAST *node, bool skipThisName = false);

  QString name() const { if(m_stopSearch) return QString(); return _M_name.toString(); }
  QStringList qualifiedName() const { if(m_stopSearch) return QStringList(); return _M_name.toStringList(); }

  const KDevelop::QualifiedIdentifier& identifier() const;

  /**
   * When the name contains one type-specifier, this should contain that specifier after the run.
   * Especially this is the type-specifier of a conversion-operator like "operator int()"
   * */
  TypeSpecifierAST* lastTypeSpecifier() const;

  ///Retrieve the declarations found for the name
  QList<KDevelop::DeclarationPointer> declarations() const;

  bool stoppedSearch() const {
    return m_stopSearch;
  }
  
  ///Whether at least one part of the scope could be resolved
  DeclarationPointer foundSomething() const;
  
  ///This can be used from outside to only process the type of a template-argument.
  ///This NameASTVisitor will be in an invalid state after this is called, so don't continue using it!
  Cpp::ExpressionEvaluationResult processTemplateArgument(TemplateArgumentAST *node);
protected:
  virtual void visitUnqualifiedName(UnqualifiedNameAST *node) override;
  void visitTemplateArgument(TemplateArgumentAST *node) override;
  
private:
  ParseSession* m_session;
  Cpp::ExpressionVisitor* m_visitor;
  const KDevelop::DUContext* m_context;
  const KDevelop::TopDUContext* m_source;
  const KDevelop::DUContext* m_localContext;
  TypeSpecifierAST* m_typeSpecifier;
  KDevelop::Identifier m_currentIdentifier;
  KDevelop::QualifiedIdentifier _M_name;
  Cpp::FindDeclaration m_find;
  bool m_debug;
  UnqualifiedNameAST* m_finalName;
  KDevelop::DUContext::SearchFlags m_flags;
  bool m_stopSearch;
  DeclarationPointer m_foundSomething;
};

QString decode(ParseSession* session, AST* ast, bool without_spaces = false);

#endif // NAME_VISITOR_H

