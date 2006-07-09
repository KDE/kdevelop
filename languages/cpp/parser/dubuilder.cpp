/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>
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

#include "dubuilder.h"

#include <ktexteditor/smartrange.h>

#include "lexer.h"
#include "duchain.h"
#include "ducontext.h"
#include "typesystem.h"
#include "editorintegrator.h"
#include "name_compiler.h"
#include "definition.h"

DUBuilder::DUBuilder (TokenStream *token_stream, DUChain* chain):
  _M_token_stream (token_stream), m_editor(new EditorIntegrator(token_stream)), m_nameCompiler(new NameCompiler(token_stream)),
  in_namespace(false), in_class(false), in_template_declaration(false),
  in_typedef(false), in_function_definition(false), in_parameter_declaration(false),
  m_chain(chain), m_types(new TypeEnvironment)
{
}

DUBuilder::~DUBuilder ()
{
  delete m_editor;
  delete m_nameCompiler;
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
  DUContext* previousContext = m_currentContext;

  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);

  closeContext(node, previousContext);
}

void DUBuilder::closeContext(AST* node, DUContext* parent)
{
  // Find the end position of this function definition (just inside the bracket)
  KTextEditor::Cursor endPosition = m_editor->findPosition(node->end_token, EditorIntegrator::FrontEdge);

  // Set the correct end point of all of the contexts finishing here
  foreach (DUContext* context, parent->childContexts())
    context->textRange().end().setPosition(endPosition);

  // Go back to the context prior to this function definition
  m_currentContext = parent;
}

void DUBuilder::visitParameterDeclarationClause (ParameterDeclarationClauseAST * node)
{
  // TODO can you put other declarations inside a parameter declaration list??
  bool was = inParameterDeclaration (node);
  DefaultVisitor::visitParameterDeclarationClause (node);
  inParameterDeclaration (was);
}

void DUBuilder::visitParameterDeclaration (ParameterDeclarationAST * node)
{
  newDeclaration(node->type_specifier);

  DefaultVisitor::visitParameterDeclaration (node);
}

void DUBuilder::visitCompoundStatement (CompoundStatementAST * node)
{
  DUContext* previousContext = m_currentContext;

  DefaultVisitor::visitCompoundStatement (node);

  if (previousContext != m_currentContext)
    closeContext(node, previousContext);
}

void DUBuilder::visitSimpleDeclaration (SimpleDeclarationAST *node)
{
  newDeclaration(node->type_specifier);

  DefaultVisitor::visitSimpleDeclaration (node);
}

void DUBuilder::visitName (NameAST *node)
{
  m_nameCompiler->run(node);

  // Find definition
  DUContext* definitionDUContext = m_currentContext->definitionContext(m_nameCompiler->name());
  Definition* definition = definitionDUContext->findLocalDefinition(m_nameCompiler->name());

  KTextEditor::SmartRange* use = m_editor->createRange(node);
  definition->addUse(use);
}

DUContext * DUBuilder::newDeclaration( TypeSpecifierAST* type )
{
  // This cast may well be an incorrect assumption...
  m_nameCompiler->run(static_cast<ElaboratedTypeSpecifierAST*>(type)->name);

  QString identifier = m_nameCompiler->name();

  // FIXME here we need to interface with the type system properly...
  AbstractType* abstractType = 0;//m_types->findIntegral(integralType);

  Definition::Scope scope = Definition::GlobalScope;
  if (in_function_definition)
    scope = Definition::LocalScope;
  else if (in_class)
    scope = Definition::ClassScope;
  else if (in_namespace)
    scope = Definition::NamespaceScope;

  m_currentContext->addDefinition(new Definition(abstractType, identifier, scope));

  return m_currentContext;
}

// kate: indent-width 2;
