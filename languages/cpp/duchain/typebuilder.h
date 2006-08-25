/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef TYPEBUILDER_H
#define TYPEBUILDER_H

#include "contextbuilder.h"

typedef ContextBuilder TypeBuilderBase;

class TypeBuilder: public TypeBuilderBase
{
public:
  TypeBuilder(ParseSession* session);
  TypeBuilder(CppEditorIntegrator* editor);

  /**
   * Build types by iterating the given \a node.
   */
  void buildTypes(AST *node);

protected:
  virtual void visitClassSpecifier(ClassSpecifierAST *node);
  virtual void visitEnumSpecifier(EnumSpecifierAST *node);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node);

private:
  AbstractType* m_currentType;
};

#endif // TYPEBUILDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
