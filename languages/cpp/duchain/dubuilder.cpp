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

#include <QMutexLocker>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

#include "duchain.h"
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "definition.h"

using namespace KTextEditor;

DUBuilder::DUBuilder (ParseSession* session)
  : m_editor(new CppEditorIntegrator(session))
  , m_nameCompiler(new NameCompiler(session))
  , in_namespace(false)
  , in_class(false)
  , in_template_declaration(false)
  , in_typedef(false)
  , in_function_definition(false)
  , in_parameter_declaration(false)
  , m_secondParentContext(0)
{
}

DUBuilder::~DUBuilder ()
{
  delete m_editor;
  delete m_nameCompiler;
}

DUContext* DUBuilder::build(const KUrl& url, AST *node, DefinitionOrUse definition)
{
  m_compilingDefinitions = definition == CompileDefinitions;

  m_editor->setCurrentUrl(url);

  DUContext* topLevelContext = DUChain::self()->chainForDocument(url);

  if (topLevelContext) {
    m_contextStack.push(topLevelContext);

    Q_ASSERT(topLevelContext->textRangePtr());

    if (m_compilingDefinitions) {
      // FIXME for now, just clear the chain... later, need to implement incremental parsing
      topLevelContext->deleteChildContextsRecursively();
      topLevelContext->deleteLocalDefinitions();

      Q_ASSERT(topLevelContext->textRangePtr());

      // FIXME remove once conversion works
      if (!topLevelContext->smartRange() && m_editor->smart())
        topLevelContext->setTextRange(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain));
    }

  } else {
    Q_ASSERT(m_compilingDefinitions);

    // FIXME the top range will probably get deleted without the editor integrator knowing...?
    topLevelContext = openContextInternal(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain), DUContext::Global);

    DUChain::self()->addDocumentChain(url, topLevelContext);
  }

  m_editor->setCurrentRange(currentContext()->textRangePtr());

  visit (node);

  closeContext();

  Q_ASSERT(m_contextStack.isEmpty());

  // FIXME Hrm, didn't get used..??
  if (m_secondParentContext) {
    kWarning() << k_funcinfo << "Previous parameter declaration context didn't get used??" << endl;
    m_secondParentContext = 0;
  }

  return topLevelContext;
}

void DUBuilder::visitNamespace (NamespaceAST *node)
{
  QualifiedIdentifier identifier;
  if (m_compilingDefinitions) {
    identifier = currentContext()->scopeIdentifier();
    if (node->namespace_name)
      identifier << QualifiedIdentifier(m_editor->tokenToString(node->namespace_name));
    else
      identifier << Identifier::unique(0);
  }

  DUContext* nsCtx = openContext(node, DUContext::Namespace);

  if (m_compilingDefinitions)
    nsCtx->setLocalScopeIdentifier(identifier);

  bool was = inNamespace (true);
  DefaultVisitor::visitNamespace (node);
  inNamespace (was);

  closeContext();
}

void DUBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  openContext(node, DUContext::Class);

  bool was = inClass (true);
  DefaultVisitor::visitClassSpecifier (node);
  inClass (was);

  closeContext(node->name);
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
  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);

  // Didn't get claimed if it was set
  m_secondParentContext = 0;
}

DUContext* DUBuilder::openContext(AST* rangeNode, DUContext::ContextType type)
{
  if (m_compilingDefinitions) {
    Range* range = m_editor->createRange(rangeNode);
    DUContext* ret = openContextInternal(range, type);
    rangeNode->ducontext = ret;
    return ret;

  } else {
    m_contextStack.push(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* DUBuilder::openContext(AST* fromRange, AST* toRange, DUContext::ContextType type)
{
  if (m_compilingDefinitions) {
    Range* range = m_editor->createRange(fromRange, toRange);
    DUContext* ret = openContextInternal(range, type);
    fromRange->ducontext = ret;
    return ret;

  } else {
    m_contextStack.push(fromRange->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* DUBuilder::openContextInternal(Range* range, DUContext::ContextType type)
{
  Q_ASSERT(m_compilingDefinitions);

  DUContext* ret = new DUContext(range, m_contextStack.isEmpty() ? 0 : currentContext());
  ret->setType(type);

  m_contextStack.push(ret);

  return ret;
}

void DUBuilder::closeContext(NameAST* name, AST* node)
{
  if (m_compilingDefinitions) {
    if (node) {
      // Find the end position of this function definition (just inside the bracket)
      KDevDocumentCursor endPosition = m_editor->findPosition(node->end_token, CppEditorIntegrator::FrontEdge);

      // Set the correct end point of the current context finishing here
      if (currentContext()->textRange().end() != endPosition)
        currentContext()->textRange().end() = endPosition;
    }

    // Set context identifier
    if (name) {
      m_nameCompiler->run(name);
      currentContext()->setLocalScopeIdentifier(m_nameCompiler->identifier());
    }
  }

  // Go back to the context prior to this function definition
  m_contextStack.pop();

  // Go back to the previous range
  m_editor->exitCurrentRange();
}

void DUBuilder::visitParameterDeclarationClause (ParameterDeclarationClauseAST * node)
{
  m_secondParentContext = openContext(node, DUContext::Function);

  bool was = inParameterDeclaration (node);
  DefaultVisitor::visitParameterDeclarationClause (node);
  inParameterDeclaration (was);

  closeContext();
}

void DUBuilder::visitParameterDeclaration (ParameterDeclarationAST * node)
{
  DefaultVisitor::visitParameterDeclaration (node);
}

void DUBuilder::visitCompoundStatement (CompoundStatementAST * node)
{
  openContext(node, DUContext::Other);

  reparentSecondContext();

  DefaultVisitor::visitCompoundStatement (node);

  closeContext();
}

void DUBuilder::visitSimpleDeclaration (SimpleDeclarationAST *node)
{
  DefaultVisitor::visitSimpleDeclaration (node);

  // Didn't get claimed if it was still set
  m_secondParentContext = 0;

  // We need to detect function declarations
  /*visit(node->type_specifier);

  if (node->init_declarators) {
    const ListNode<InitDeclaratorAST*> *it = node->init_declarators->toFront(), *end = it;

    do {
      if (it->element && it->element->declarator && it->element->declarator->parameter_declaration_clause) {
        // This is a forward definition for a function, create a context
        openContext(it->element->declarator->parameter_declaration_clause, DUContext::Function);
        visit(it->element);
        closeContext();
      } else {
        visit(it->element);
      }
      it = it->next;
    } while (it != end);
  }

  visit(node->win_decl_specifiers);*/
}

void DUBuilder::visitInitDeclarator(InitDeclaratorAST* node)
{
  DefaultVisitor::visitInitDeclarator(node);
}

void DUBuilder::visitDeclarator (DeclaratorAST* node)
{
  // Don't create a definition for a function
  if (node->parameter_declaration_clause) {
    switch (currentContext()->type()) {
      case DUContext::Global:
      case DUContext::Namespace:
      case DUContext::Class:
      case DUContext::Function:
          break;

      case DUContext::Other:
          newDeclaration(node->id, node);
          break;
    }

  } else {
    newDeclaration(node->id, node);
  }

  DefaultVisitor::visitDeclarator(node);
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
  if (m_compilingDefinitions)
    return;

  Range* current = m_editor->currentRange();
  Range* use = m_editor->createRange(name);
  m_editor->exitCurrentRange();
  Q_ASSERT(m_editor->currentRange() == current);

  if (use->isSmartRange())
    if (use->toSmartRange()->parentRange() != currentContext()->smartRange())
      kWarning() << k_funcinfo << "Use " << *use << " parent " << *use->toSmartRange()->parentRange() << " " << use->toSmartRange()->parentRange() << " != current context " << *currentContext()->smartRange() << " " << currentContext()->smartRange() << " id " << currentContext()->scopeIdentifier() << endl;

  m_nameCompiler->run(name);

  if (m_secondParentContext)
    if (Definition* definition = m_secondParentContext->findDefinition(m_nameCompiler->identifier(), KDevDocumentCursor(use, KDevDocumentCursor::Start))) {
      definition->addUse(use);
      return;
    }

  if (Definition* definition = currentContext()->findDefinition(m_nameCompiler->identifier(), KDevDocumentCursor(use, KDevDocumentCursor::Start)))
    definition->addUse(use);
  else
    currentContext()->addOrphanUse(use);
    //kWarning() << k_funcinfo << "Could not find definition for identifier " << id << " at " << *use << endl;
}

void DUBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  DefaultVisitor::visitSimpleTypeSpecifier(node);
}

void DUBuilder::visitName (NameAST *)
{
  // Note: we don't want to visit the name node, the name compiler does that for us (only when we need it)
}

Definition* DUBuilder::newDeclaration(NameAST* name, AST* rangeNode)
{
  if (!m_compilingDefinitions)
    return 0;

  Definition::Scope scope = Definition::GlobalScope;
  if (in_function_definition)
    scope = Definition::LocalScope;
  else if (in_class)
    scope = Definition::ClassScope;
  else if (in_namespace)
    scope = Definition::NamespaceScope;

  //kDebug() << "Visit declaration: " << identifier << " range " << *range << endl;

  Range* prior = m_editor->currentRange();
  Range* range = m_editor->createRange(name ? static_cast<AST*>(name) : rangeNode);
  m_editor->exitCurrentRange();
  Q_ASSERT(m_editor->currentRange() == prior);

  Definition* definition = new Definition(range, scope);
  currentContext()->addDefinition(definition);

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

  if (m_compilingDefinitions) {
    m_nameCompiler->run(node->name);

    currentContext()->addUsingNamespace(m_editor->createCursor(node->end_token, CppEditorIntegrator::FrontEdge), m_nameCompiler->identifier());
  }
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

void DUBuilder::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
  switch (currentContext()->type()) {
    case DUContext::Global:
    case DUContext::Namespace:
    case DUContext::Class:
        visit(node->declaration);
        break;

    case DUContext::Function:
    case DUContext::Other:
        visit(node->expression);
        break;
  }
}

void DUBuilder::visitForStatement(ForStatementAST *node)
{
  // Not setting the member var because it gets nuked in visitSimpleDeclaration
  AST* first = node->init_statement;
  if (!first)
    first = node->condition;
  if (!first)
    first = node->expression;
  if (!first)
    return;

  AST* second = node->expression;
  if (!second)
    second = node->condition;
  if (!second)
    second = node->init_statement;

  DUContext* secondParentContext = openContext(first, second, DUContext::Other);

  visit(node->init_statement);
  visit(node->condition);
  visit(node->expression);

  closeContext();

  const bool contextNeeded = createContextIfNeeded(node->statement, secondParentContext);

  visit(node->statement);

  if (contextNeeded)
    closeContext();

  // Didn't get claimed if it was still set
  m_secondParentContext = 0;
}

void DUBuilder::reparentSecondContext()
{
  if (m_compilingDefinitions && m_secondParentContext) {
    if (m_secondParentContext->parentContexts().count())
      m_secondParentContext->parentContexts().first()->takeChildContext(m_secondParentContext);
    m_secondParentContext->addChildContext(currentContext());
    m_secondParentContext = 0;
  }
}

void DUBuilder::visitIfStatement(IfStatementAST* node)
{
  // Not setting the member var because it gets nuked in visitSimpleDeclaration
  DUContext* secondParentContext = openContext(node->condition, DUContext::Other);

  visit(node->condition);

  closeContext();

  if (node->statement) {
    const bool contextNeeded = createContextIfNeeded(node->statement, secondParentContext);

    visit(node->statement);

    if (contextNeeded)
      closeContext();
  }

  if (node->else_statement) {
    const bool contextNeeded = createContextIfNeeded(node->else_statement, secondParentContext);

    visit(node->else_statement);

    if (contextNeeded)
      closeContext();
  }
}

bool DUBuilder::createContextIfNeeded(AST* node, DUContext* secondParentContext)
{
  m_secondParentContext = secondParentContext;

  const bool contextNeeded = !ast_cast<CompoundStatementAST*>(node);
  if (contextNeeded) {
    openContext(node, DUContext::Other);
    reparentSecondContext();
  }
  return contextNeeded;
}
