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
#include "lexer.h"

DUBuilder::DUBuilder (TokenStream *token_stream):
  _M_token_stream (token_stream),
  in_namespace(false), in_class(false), in_template_declaration(false),
  in_typedef(false), in_function_definition(false)
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

void DUBuilder::visitTypedef (TypedefAST *node)
{
  bool was = inTypedef (node);
  DefaultVisitor::visitTypedef (node);
  inTypedef (was);
}

void DUBuilder::visitFunctionDefinition (FunctionDefinitionAST *node)
{
  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);
}

void DUBuilder::visitSimpleDeclaration (SimpleDeclarationAST *node)
{
  // ### implement me
  DefaultVisitor::visitSimpleDeclaration (node);
}

void DUBuilder::visitName (NameAST *node)
{
  // ### implement me
}

// kate: indent-width 2;

