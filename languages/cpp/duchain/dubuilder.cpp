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
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "definition.h"

using namespace KTextEditor;

DUBuilder::DUBuilder (TokenStream *token_stream):
  _M_token_stream (token_stream), m_editor(new CppEditorIntegrator(token_stream)), m_nameCompiler(new NameCompiler(token_stream)),
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

    // FIXME remove once conversion works
    if (!topLevelContext->smartRange() && m_editor->smart())
      topLevelContext->setTextRange(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain));

  } else {
    // FIXME the top range will probably get deleted without the editor integrator knowing...?
    topLevelContext = new DUContext(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain));

    DUChain::self()->addDocumentChain(url, topLevelContext);
  }

  m_currentContext = topLevelContext;

  visit (node);

  return topLevelContext;
}

void DUBuilder::visitNamespace (NamespaceAST *node)
{
  DUContext* previousContext = m_currentContext;

  QualifiedIdentifier identifier = previousContext->scopeIdentifier();
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

  Definition* oldDefinition = m_currentDefinition;

  Range* contextRange = m_editor->createRange(node);

  Range* range = m_editor->createRange(node->name);
  m_currentDefinition = newDeclaration(range, node->name);
  m_editor->exitCurrentRange();

  Q_ASSERT(m_editor->currentRange() == contextRange);

  m_currentContext = new DUContext(contextRange, m_currentContext);
  m_currentContext->setType(DUContext::Class);

  bool was = inClass (true);
  DefaultVisitor::visitClassSpecifier (node);
  inClass (was);

  closeContext(node, previousContext, node->name);

  m_currentDefinition = oldDefinition;
}

void DUBuilder::visitBaseSpecifier(BaseSpecifierAST* node)
{
  DefaultVisitor::visitBaseSpecifier(node);
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

  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);

  closeContext(node, previousContext);
}

void DUBuilder::closeContext(AST* node, DUContext* parent, NameAST* name)
{
  Q_ASSERT(m_currentContext->parentContexts().contains(parent));
  if (m_currentContext->smartRange() && parent->smartRange())
    Q_ASSERT(m_currentContext->smartRange()->parentRange() == parent->smartRange());

  // Find the end position of this function definition (just inside the bracket)
  KDevDocumentCursor endPosition = m_editor->findPosition(node->end_token, CppEditorIntegrator::FrontEdge);

  // Set the correct end point of the current context finishing here
  if (m_currentContext->textRange().end() != endPosition)
      m_currentContext->textRange().end() = endPosition;

  // Set context identifier
  if (name) {
    m_nameCompiler->run(name);
    m_currentContext->setLocalScopeIdentifier(m_nameCompiler->identifier());
  }

  // Go back to the context prior to this function definition
  m_currentContext = parent;

  // Go back to the previous range
  m_editor->exitCurrentRange();
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
  DefaultVisitor::visitParameterDeclaration (node);
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
  DefaultVisitor::visitSimpleDeclaration (node);
}

void DUBuilder::visitInitDeclarator(InitDeclaratorAST* node)
{
  DefaultVisitor::visitInitDeclarator(node);
}

void DUBuilder::visitDeclarator (DeclaratorAST* node)
{
  Definition* oldDefinition = m_currentDefinition;

  Range* range = m_editor->createRange(node);
  m_currentDefinition = newDeclaration(range, node->id);
  m_editor->exitCurrentRange();

  DefaultVisitor::visitDeclarator(node);

  m_currentDefinition = oldDefinition;
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
  Range* current = m_editor->currentRange();
  Range* use = m_editor->createRange(name);
  m_editor->exitCurrentRange();
  Q_ASSERT(m_editor->currentRange() == current);

  if (use->isSmartRange())
    Q_ASSERT(use->toSmartRange()->parentRange() == m_currentContext->smartRange());

  m_nameCompiler->run(name);

  Definition* definition = m_currentContext->findDefinition(m_nameCompiler->identifier(), KDevDocumentCursor(use, KDevDocumentCursor::Start));
  if (definition)
    definition->addUse(use);

  //else kWarning() << k_funcinfo << "Could not find definition for identifier " << id << " at " << *use << endl;
}

void DUBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  DefaultVisitor::visitSimpleTypeSpecifier(node);
}

void DUBuilder::visitName (NameAST *)
{
  //m_nameCompiler->run(node);
  //m_identifierStack.push(m_nameCompiler->identifier());

  // Note: we don't want to visit the name node, the name compiler does that for us
  //DefaultVisitor::visitName(node);
}

Definition* DUBuilder::newDeclaration(Range* range, NameAST* name)
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

  if (name) {
    m_nameCompiler->run(name);

    // FIXME this can happen if we're defining a staticly declared variable
    //Q_ASSERT(m_nameCompiler->identifier().count() == 1);
    Q_ASSERT(!m_nameCompiler->identifier().isEmpty());
    definition->setIdentifier(m_nameCompiler->identifier().first());
  }

  return definition;
}

void DUBuilder::visitUsingDirective(UsingDirectiveAST * node)
{
  DefaultVisitor::visitUsingDirective(node);

  m_nameCompiler->run(node->name);

  m_currentContext->addUsingNamespace(m_editor->createCursor(node->end_token, CppEditorIntegrator::FrontEdge), m_nameCompiler->identifier());
}

void DUBuilder::visitClassMemberAccess(ClassMemberAccessAST * node)
{
  DefaultVisitor::visitClassMemberAccess(node);
}

void DUBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  DefaultVisitor::visitElaboratedTypeSpecifier(node);
}

void DUBuilder::visitEnumSpecifier(EnumSpecifierAST* node)
{
  DefaultVisitor::visitEnumSpecifier(node);
}

void DUBuilder::visitTypeParameter(TypeParameterAST* node)
{
  DefaultVisitor::visitTypeParameter(node);
}

void DUBuilder::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
  // TODO store the alias
  DefaultVisitor::visitNamespaceAliasDefinition(node);
}

void DUBuilder::visitTypeIdentification(TypeIdentificationAST* node)
{
  DefaultVisitor::visitTypeIdentification(node);
}

void DUBuilder::visitUsing(UsingAST* node)
{
  // TODO store the using
  DefaultVisitor::visitUsing(node);
}
