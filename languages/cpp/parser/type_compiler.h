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

#ifndef TYPE_COMPILER_H
#define TYPE_COMPILER_H

#include "default_visitor.h"
#include <language/duchain/identifier.h>
#include <cppparserexport.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

class ParseSession;

class KDEVCPPPARSER_EXPORT TypeCompiler: protected DefaultVisitor
{
public:
  TypeCompiler(ParseSession* session);

  KDevelop::QualifiedIdentifier identifier() const;
  inline QStringList qualifiedName() const { return _M_type.toStringList(); }
  inline QList<int> cv() const { return _M_cv; }

  bool isConstant() const;
  bool isVolatile() const;

  QStringList cvString() const;

  void run(TypeSpecifierAST *node);

protected:
  virtual void visitClassSpecifier(ClassSpecifierAST *node) override;
  virtual void visitEnumSpecifier(EnumSpecifierAST *node) override;
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node) override;
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node) override;

  virtual void visitName(NameAST *node) override;

private:
  ParseSession* m_session;
  KDevelop::QualifiedIdentifier _M_type;
  QList<int> _M_cv;
};

#endif // TYPE_COMPILER_H

