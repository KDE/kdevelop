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
  m_types(new TypeEnvironment), m_currentDefinition(0)
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

  //Q_ASSERT(m_identifierStack.isEmpty());
  if (!m_identifierStack.isEmpty())
    kWarning() << k_funcinfo << "Unused identifiers: " << m_identifierStack.toList() << endl;

  return topLevelContext;
}

void DUBuilder::visitNamespace (NamespaceAST *node)
{
  DUContext* previousContext = m_currentContext;

  // Namespaces have their own context, but because their definitions
  // do not go out of scope (just require qualification or using statements
  // to become accessible again), the context itself will assign any definitions to
  // the next non-namespace parent context (todo: this is always the top context, correct?)
  QualifiedIdentifier identifier = QualifiedIdentifier::merge(m_identifierStack);
  if (node->namespace_name)
    identifier << QualifiedIdentifier(_M_token_stream->symbol(node->namespace_name)->as_string());
  else
    identifier << Identifier::unique(reinterpret_cast<int>(_M_token_stream));

  m_currentContext = new DUContext(m_editor->createRange(node), m_currentContext);
  m_currentContext->setType(DUContext::Namespace);
  m_currentContext->setLocalScopeIdentifier(identifier);

  bool was = inNamespace (true);
  DefaultVisitor::visitNamespace (node);
  inNamespace (was);

  closeContext(node, previousContext);
}

void DUBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  DUContext* previousContext = m_currentContext;
  int identifierStackDepth = m_identifierStack.count();

  m_currentContext = new DUContext(m_editor->createRange(node), m_currentContext);
  m_currentContext->setType(DUContext::Class);

  bool was = inClass (true);
  DefaultVisitor::visitClassSpecifier (node);
  inClass (was);

  closeContext(node, previousContext, identifierStackDepth);
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
  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  DUContext* previousContext = m_currentContext;
  m_currentContext = new DUContext(m_editor->createRange(node), m_currentContext);
  m_currentContext->setType(DUContext::Function);

  int stackCount = m_identifierStack.count();

  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);

  closeContext(node, previousContext);

  setIdentifier(stackCount);

  // TODO once type system is established, check to see if there was a forward
  // declaration and if so, merge this definition with that definition.

  m_currentDefinition = oldDefinition;
}

void DUBuilder::closeContext(AST* node, DUContext* parent, int identifierStackDepth)
{
  Q_UNUSED(node);
  // Find the end position of this function definition (just inside the bracket)
  /*DocumentCursor endPosition = m_editor->findPosition(node->end_token, EditorIntegrator::FrontEdge);

  // Set the correct end point of all of the contexts finishing here
  foreach (DUContext* context, parent->childContexts())
    context->textRange().end() = endPosition;*/

  // Set context identifier
  if (identifierStackDepth != -1 && identifierStackDepth < m_identifierStack.count())
    m_currentContext->setLocalScopeIdentifier(m_identifierStack.top());

  // Go back to the context prior to this function definition
  m_currentContext = parent;
}

void DUBuilder::visitParameterDeclarationClause (ParameterDeclarationClauseAST * node)
{
  DUContext* previousContext = m_currentContext;

  if (m_currentDefinition->textRange() != m_currentContext->textRange()) {
    // This is within a forward declaration, but we need a new context so as to properly scope
    // definitions within parameters.  TODO: do we even want to know about these definitions?
    m_currentContext = new DUContext(m_editor->createRange(m_currentDefinition->textRange()), m_currentContext);
    m_currentContext->setType(DUContext::Function);
  }

  bool was = inParameterDeclaration (node);
  DefaultVisitor::visitParameterDeclarationClause (node);
  inParameterDeclaration (was);

  m_currentContext = previousContext;
}

void DUBuilder::visitParameterDeclaration (ParameterDeclarationAST * node)
{
  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitParameterDeclaration (node);

  setIdentifier(stackCount);

  m_currentDefinition = oldDefinition;
}

void DUBuilder::visitCompoundStatement (CompoundStatementAST * node)
{
  DUContext* previousContext = m_currentContext;
  m_currentContext = new DUContext(m_editor->createRange(node), m_currentContext);

  DefaultVisitor::visitCompoundStatement (node);

  closeContext(node, previousContext);
}

void DUBuilder::visitSimpleDeclaration (SimpleDeclarationAST *node)
{
  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitSimpleDeclaration (node);

  setIdentifier(stackCount);

  m_currentDefinition = oldDefinition;
}

void DUBuilder::setIdentifier(int stackCount)
{
  Q_ASSERT(m_identifierStack.count() >= stackCount);
  Q_ASSERT(m_identifierStack.count() <= stackCount + 1);

  if (m_identifierStack.count() == stackCount + 1)
    if (m_currentDefinition) {
      // FIXME this can happen if we're defining a staticly declared variable
      //Q_ASSERT(m_identifierStack.top().count() == 1);
      m_currentDefinition->setIdentifier(m_identifierStack.pop().first());

    } else {
      // Unused identifier...?
      m_identifierStack.pop();
    }
}

void DUBuilder::visitPrimaryExpression (PrimaryExpressionAST* node)
{
  DefaultVisitor::visitPrimaryExpression(node);

  if (node->name)
    newUse(node->name);
}

void DUBuilder::visitMemInitializer(MemInitializerAST * node)
{
  DefaultVisitor::visitMemInitializer(node);

  if (node->initializer_id)
    newUse(node->initializer_id);
}

void DUBuilder::newUse(NameAST* name)
{
  Range* use = m_editor->createRange(name);

  QualifiedIdentifier id = m_identifierStack.pop();
  Definition* definition = m_currentContext->findDefinition(id, DocumentCursor(use, DocumentCursor::Start));
  if (definition)
    definition->addUse(use);
  else
    kWarning() << k_funcinfo << "Could not find definition for identifier " << id << " at " << *use << endl;
}

void DUBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  DefaultVisitor::visitSimpleTypeSpecifier(node);

  // Pop off unneeded name...
  if (node->name)
    m_identifierStack.pop();
}

void DUBuilder::visitName (NameAST *node)
{
  m_nameCompiler->run(node);

  m_identifierStack.push(m_nameCompiler->identifier());

  DefaultVisitor::visitName(node);
}

void DUBuilder::visitDeclarator (DeclaratorAST* node)
{
  DefaultVisitor::visitDeclarator(node);
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

void DUBuilder::visitUsingDirective(UsingDirectiveAST * node)
{
  DefaultVisitor::visitUsingDirective(node);

  m_currentContext->addUsingNamespace(m_editor->createCursor(node->end_token, EditorIntegrator::FrontEdge), m_identifierStack.pop());
}

// kate: indent-width 2;
