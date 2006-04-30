/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>

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

#include "dubuilder.h"

DUBuilder::DUBuilder ():
  _M_flags (0)
{
}

DUBuilder::~DUBuilder ()
{
}

void DUBuilder::operator () (AST *node)
{
  visit (node);
}

void DUBuilder::visitNamespace (NamespaceAST *node)
{
  bool was = inNamespace (true);
  DefaultVisitor::visitNamespace (node);
  inNamespace (was);
}

void DUBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  bool was = inClass (true);
  DefaultVisitor::visitClassSpecifier (node);
  inClass (was);
}

void DUBuilder::visitTemplateDeclaration (TemplateDeclarationAST *node)
{
  bool was = inTemplateDeclaration (true);
  DefaultVisitor::visitTemplateDeclaration (node);
  inTemplateDeclaration (was);
}

// kate: indent-width 2;

