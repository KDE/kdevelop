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

#include "typebuilder.h"

#include <ktexteditor/smartrange.h>

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "ducontext.h"

TypeBuilder::TypeBuilder(ParseSession* session)
  : TypeBuilderBase(session)
  , m_currentType(0)
{
}

TypeBuilder::TypeBuilder(CppEditorIntegrator * editor)
  : TypeBuilderBase(editor)
  , m_currentType(0)
{
}

void TypeBuilder::buildTypes(AST *node)
{
  supportBuild(node);
}

void TypeBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  TypeBuilderBase::visitClassSpecifier(node);
}

void TypeBuilder::visitEnumSpecifier(EnumSpecifierAST *node)
{
  TypeBuilderBase::visitEnumSpecifier(node);
}

void TypeBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  TypeBuilderBase::visitElaboratedTypeSpecifier(node);
}

void TypeBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  TypeBuilderBase::visitSimpleTypeSpecifier(node);
}

/*QStringList TypeBuilder::cvString() const
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
}*/

// kate: space-indent on; indent-width 2; replace-tabs on;
