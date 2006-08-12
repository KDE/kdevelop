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
// kate: indent-width 2;

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
  function_just_defined(false), m_types(new TypeEnvironment), m_currentDefinition(0)
{
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

  QualifiedIdentifier identifier = QualifiedIdentifier::merge(m_identifierStack);
  if (node->namespace_name)
    identifier << QualifiedIdentifier(_M_token_stream->symbol(node->namespace_name)->as_string());
  else
    identifier << Identifier::unique(reinterpret_cast<long>(_M_token_stream));

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

  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  m_currentContext = new DUContext(m_editor->createRange(node), m_currentContext);
  m_currentContext->setType(DUContext::Class);

  bool was = inClass (true);
  DefaultVisitor::visitClassSpecifier (node);
  inClass (was);

  closeContext(node, previousContext, identifierStackDepth);

  setIdentifier(identifierStackDepth);

  m_currentDefinition = oldDefinition;
}

void DUBuilder::visitBaseSpecifier(BaseSpecifierAST* node)
{
  int identifierStackDepth = m_identifierStack.count();

  DefaultVisitor::visitBaseSpecifier(node);

  ignoreIdentifier(identifierStackDepth);
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
  function_just_defined = true;

  /*Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  DUContext* previousContext = m_currentContext;
  m_currentContext = new DUContext(m_editor->createRange(node), m_currentContext);
  m_currentContext->setType(DUContext::Function);

  int stackCount = m_identifierStack.count();*/

  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);

  closeContext(node, previousContext);

  /*setIdentifier(stackCount);

  // TODO once type system is established, check to see if there was a forward
  // declaration and if so, merge this definition with that definition.

  m_currentDefinition = oldDefinition;*/
}

void DUBuilder::closeContext(AST* node, DUContext* parent, int identifierStackDepth)
{
  // Find the end position of this function definition (just inside the bracket)
  DocumentCursor endPosition = m_editor->findPosition(node->end_token, EditorIntegrator::FrontEdge);

  // Set the correct end point of the current context finishing here
  if (m_currentContext->textRange().end() != endPosition)
      m_currentContext->textRange().end() = endPosition;

  // Set context identifier
  if (identifierStackDepth != -1 && identifierStackDepth < m_identifierStack.count())
    m_currentContext->setLocalScopeIdentifier(m_identifierStack.top());

  // Go back to the context prior to this function definition
  m_currentContext = parent;
}

void DUBuilder::visitParameterDeclarationClause (ParameterDeclarationClauseAST * node)
{
  DUContext* previousContext = m_currentContext;

  m_currentContext = new DUContext(m_editor->createRange(m_currentDefinition->textRange()), m_currentContext);
  m_currentContext->setType(DUContext::Function);

  bool was = inParameterDeclaration (node);
  DefaultVisitor::visitParameterDeclarationClause (node);
  inParameterDeclaration (was);

  // Don't close context here, it's closed in the function definition
  if (!function_just_defined)
    m_currentContext = previousContext;

  function_just_defined = false;
}

void DUBuilder::visitParameterDeclaration (ParameterDeclarationAST * node)
{
  /*Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  int stackCount = m_identifierStack.count();*/

  DefaultVisitor::visitParameterDeclaration (node);

  /*setIdentifier(stackCount);

  m_currentDefinition = oldDefinition;*/
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
  /*Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  int stackCount = m_identifierStack.count();*/

  DefaultVisitor::visitSimpleDeclaration (node);

  /*setIdentifier(stackCount);

  m_currentDefinition = oldDefinition;*/
}

void DUBuilder::visitInitDeclarator(InitDeclaratorAST* node)
{
  /*Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  int stackCount = m_identifierStack.count();*/

  DefaultVisitor::visitInitDeclarator(node);

  /*setIdentifier(stackCount);

  m_currentDefinition = oldDefinition;*/
}

void DUBuilder::visitDeclarator (DeclaratorAST* node)
{
  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range);

  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitDeclarator(node);

  setIdentifier(stackCount);

  m_currentDefinition = oldDefinition;
}

void DUBuilder::setIdentifier(int stackCount)
{
  Q_ASSERT(m_identifierStack.count() >= stackCount);

  int index = m_identifierStack.count();
  while (index > stackCount + 1) {
    kWarning() << k_funcinfo << "Unrecognised identifier present at " << m_currentDefinition->textRange() << endl;
    m_identifierStack.pop();
    --index;
  }

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

void DUBuilder::ignoreIdentifier(int stackCount)
{
  Q_ASSERT(m_identifierStack.count() >= stackCount);
  Q_ASSERT(m_identifierStack.count() <= stackCount + 2);

  if (m_identifierStack.count() == stackCount + 1)
    m_identifierStack.pop();
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

  //else kWarning() << k_funcinfo << "Could not find definition for identifier " << id << " at " << *use << endl;
}

void DUBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitSimpleTypeSpecifier(node);

  ignoreIdentifier(stackCount);
}

void DUBuilder::visitName (NameAST *node)
{
  m_nameCompiler->run(node);

  m_identifierStack.push(m_nameCompiler->identifier());

  // Note: we don't want to visit the name node, the name compiler does that for us
  //DefaultVisitor::visitName(node);
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

void DUBuilder::visitClassMemberAccess(ClassMemberAccessAST * node)
{
  // FIXME need type system
  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitClassMemberAccess(node);

  ignoreIdentifier(stackCount);
}

void DUBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitElaboratedTypeSpecifier(node);

  ignoreIdentifier(stackCount);
}

void DUBuilder::visitEnumSpecifier(EnumSpecifierAST* node)
{
  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitEnumSpecifier(node);

  ignoreIdentifier(stackCount);
}

void DUBuilder::visitTypeParameter(TypeParameterAST* node)
{
  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitTypeParameter(node);

  ignoreIdentifier(stackCount);
}

void DUBuilder::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
  // TODO store the alias

  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitNamespaceAliasDefinition(node);

  ignoreIdentifier(stackCount);
}

void DUBuilder::visitTypeIdentification(TypeIdentificationAST* node)
{
  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitTypeIdentification(node);

  ignoreIdentifier(stackCount);
}

void DUBuilder::visitUsing(UsingAST* node)
{
  // TODO store the using

  int stackCount = m_identifierStack.count();

  DefaultVisitor::visitUsing(node);

  ignoreIdentifier(stackCount);
}
