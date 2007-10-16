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

#ifndef NAME_COMPILER_H
#define NAME_COMPILER_H

#include "default_visitor.h"
#include <identifier.h>
#include <cppparserexport.h>

class ParseSession;
class OperatorAST;

class KDEVCPPPARSER_EXPORT NameCompiler: protected DefaultVisitor
{
public:
  NameCompiler(ParseSession* session);

  void run(NameAST *node);
  void run(UnqualifiedNameAST *node) { m_typeSpecifier = 0; internal_run(node); }

  QString name() const { return _M_name.toString(); }
  QStringList qualifiedName() const { return _M_name.toStringList(); }

  const KDevelop::QualifiedIdentifier& identifier() const;

  /**
   * When the name contains one type-specifier, this should contain that specifier after the run.
   * Especially this is the type-specifier of a conversion-operator like "operator int()"
   * */
  TypeSpecifierAST* lastTypeSpecifier() const;

protected:
  virtual void visitUnqualifiedName(UnqualifiedNameAST *node);
  virtual void visitTemplateArgument(TemplateArgumentAST *node);

  void internal_run(AST *node);
  QString decode_operator(OperatorAST* ast) const;

private:
  ParseSession* m_session;
  TypeSpecifierAST* m_typeSpecifier;
  KDevelop::QualifiedIdentifier m_base;
  KDevelop::Identifier m_currentIdentifier;
  KDevelop::QualifiedIdentifier _M_name;
};

#endif // NAME_COMPILER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
