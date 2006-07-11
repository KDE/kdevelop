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
#include <ktexteditor/document.h>

#include "lexer.h"
#include "duchain.h"
#include "ducontext.h"
#include "typesystem.h"
#include "editorintegrator.h"
#include "name_compiler.h"
#include "definition.h"

using namespace KTextEditor;

DUBuilder::DUBuilder (TokenStream *token_stream):
  _M_token_stream (token_stream), m_editor(new EditorIntegrator(token_stream)), m_nameCompiler(new NameCompiler(token_stream)),
  in_namespace(false), in_class(false), in_template_declaration(false),
  in_typedef(false), in_function_definition(false), in_parameter_declaration(false),
  m_types(new TypeEnvironment)
{
  flags = 0;
}

DUBuilder::~DUBuilder ()
{
  delete m_editor;
  delete m_nameCompiler;
}

DUContext* DUBuilder::build(const KUrl& url, AST *node)
{
  m_editor->setCurrentUrl(url);

  DUContext* topLevelContext = DUChain::self()->chainForDocument(url);

  if (topLevelContext) {
    // FIXME for now, just clear the chain... later, need to implement incremental parsing
    topLevelContext->deleteChildContextsRecursively();
    topLevelContext->deleteLocalDefinitions();

  } else {
    // FIXME the top range will probably get deleted without the editor integrator knowing...?
    topLevelContext = new DUContext(m_editor->topRange(EditorIntegrator::DefinitionUseChain));

    DUChain::self()->addDocumentChain(url, topLevelContext);
  }

  m_currentContext = topLevelContext;

  visit (node);

  return topLevelContext;
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
  kDebug() << "Visit function declaration" << endl;

  DUContext* previousContext = m_currentContext;

  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);

  closeContext(node, previousContext);

  kDebug() << "End visit function declaration" << endl;
}

void DUBuilder::closeContext(AST* node, DUContext* parent)
{
  // Find the end position of this function definition (just inside the bracket)
  DocumentCursor endPosition = m_editor->findPosition(node->end_token, EditorIntegrator::FrontEdge);

  // Set the correct end point of all of the contexts finishing here
  foreach (DUContext* context, parent->childContexts())
    context->textRange().end() = endPosition;

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
  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  DefaultVisitor::visitParameterDeclaration (node);

  m_currentDefinition = oldDefinition;
}

void DUBuilder::visitCompoundStatement (CompoundStatementAST * node)
{
  kDebug() << "Visit compound statement" << endl;

  DUContext* previousContext = m_currentContext;

  DefaultVisitor::visitCompoundStatement (node);

  if (previousContext != m_currentContext)
    closeContext(node, previousContext);

  kDebug() << "End visit compound statement" << endl;
}

void DUBuilder::visitSimpleDeclaration (SimpleDeclarationAST *node)
{
  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  DefaultVisitor::visitSimpleDeclaration (node);

  m_currentDefinition = oldDefinition;
}

void DUBuilder::visitName (NameAST *node)
{
  m_nameCompiler->run(node);
  m_currentIdentifier = m_nameCompiler->name();

  //Range* use = m_editor->createRange(node);

  DefaultVisitor::visitName(node);
}

void DUBuilder::visitDeclarator (DeclaratorAST* node)
{
  DefaultVisitor::visitDeclarator(node);

  if (m_currentDefinition)
    m_currentDefinition->setIdentifier(m_currentIdentifier);
}

Definition* DUBuilder::newDeclaration(Range* range)
{
  Definition::Scope scope = Definition::GlobalScope;
  if (in_function_definition)
    scope = Definition::LocalScope;
  else if (in_class)
    scope = Definition::ClassScope;
  else if (in_namespace)
    scope = Definition::NamespaceScope;

  //kDebug() << "Visit declaration: " << identifier << " range " << *range << endl;
  Definition* definition = new Definition(range, scope);
  m_currentContext->addDefinition(definition);

  return definition;
}

// kate: indent-width 2;
