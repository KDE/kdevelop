/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

class TokenStream;

class TypeCompiler: protected DefaultVisitor
{
public:
  TypeCompiler(TokenStream *token_stream);

  inline QStringList qualifiedName() const { return _M_type; }
  inline QList<int> cv() const { return _M_cv; }

  bool isConstant() const;
  bool isVolatile() const;

  QStringList cvString() const;

  void run(TypeSpecifierAST *node);

protected:
  virtual void visitClassSpecifier(ClassSpecifierAST *node);
  virtual void visitEnumSpecifier(EnumSpecifierAST *node);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node);

  virtual void visitName(NameAST *node);

private:
  TokenStream *_M_token_stream;
  QStringList _M_type;
  QList<int> _M_cv;
};

#endif // TYPE_COMPILER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
