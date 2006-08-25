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

#include "definitionbuilder.h"

#include <ktexteditor/smartrange.h>

#include "duchain.h"
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "definition.h"
#include "definitionuse.h"
#include "topducontext.h"
#include "dumpchain.h"

using namespace KTextEditor;

ContextBuilder::ContextBuilder (ParseSession* session)
  : m_editor(new CppEditorIntegrator(session))
  , m_nameCompiler(new NameCompiler(session))
  , m_ownsEditorIntegrator(true)
  , m_compilingContexts(false)
{
}

ContextBuilder::ContextBuilder (CppEditorIntegrator* editor)
  : m_editor(editor)
  , m_nameCompiler(new NameCompiler(editor->parseSession()))
  , m_ownsEditorIntegrator(false)
{
}

ContextBuilder::~ContextBuilder ()
{
  if (m_ownsEditorIntegrator)
    delete m_editor;

  delete m_nameCompiler;
}

TopDUContext* ContextBuilder::buildContexts(const KUrl& url, AST *node, QList<DUContext*>* includes)
{
  m_compilingContexts = true;

  m_editor->setCurrentUrl(url);

  TopDUContext* topLevelContext = DUChain::self()->chainForDocument(url);

  if (topLevelContext) {
    Q_ASSERT(topLevelContext->textRangePtr());

    if (m_compilingContexts) {
      // FIXME for now, just clear the chain... later, need to implement incremental parsing
      topLevelContext->deleteChildContextsRecursively();
      topLevelContext->deleteLocalDefinitions();
      topLevelContext->deleteOrphanUses();

      Q_ASSERT(topLevelContext->textRangePtr());

      // FIXME remove once conversion works
      if (!topLevelContext->smartRange() && m_editor->smart())
        topLevelContext->setTextRange(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain));
    }

  } else {
    Q_ASSERT(m_compilingContexts);

    Range* range = m_editor->topRange(CppEditorIntegrator::DefinitionUseChain);
    topLevelContext = new TopDUContext(range);
    topLevelContext->setType(DUContext::Global);

    DUChain::self()->addDocumentChain(url, topLevelContext);
  }

  node->ducontext = topLevelContext;

  if (includes) {
    foreach (DUContext* parent, topLevelContext->importedParentContexts())
      if (includes->contains(parent))
        includes->removeAll(parent);
      else
        topLevelContext->removeImportedParentContext(parent);

    foreach (DUContext* included, *includes)
      topLevelContext->addImportedParentContext(included);
  }

  supportBuild(node);

  m_compilingContexts = false;

  // FIXME Hrm, didn't get used..??
  if (!m_importedParentContexts.isEmpty()) {
    kWarning() << k_funcinfo << url << " Previous parameter declaration context didn't get used??" << endl;
    DumpChain dump;
    dump.dump(topLevelContext);
    m_importedParentContexts.clear();
  }

  return topLevelContext;
}

void ContextBuilder::supportBuild(AST *node)
{
  Q_ASSERT(node->ducontext);
  m_contextStack.push(node->ducontext);

  m_editor->setCurrentUrl(node->ducontext->url());

  m_editor->setCurrentRange(currentContext()->textRangePtr());

  visit (node);

  closeContext();

  Q_ASSERT(m_contextStack.isEmpty());
}

void ContextBuilder::visitNamespace (NamespaceAST *node)
{
  QualifiedIdentifier identifier;
  if (m_compilingContexts) {
    identifier = currentContext()->scopeIdentifier();
    if (node->namespace_name)
      identifier << QualifiedIdentifier(m_editor->tokenToString(node->namespace_name));
    else
      identifier << Identifier::unique(0);
  }

  DUContext* nsCtx = openContext(node, DUContext::Namespace);

  if (m_compilingContexts)
    nsCtx->setLocalScopeIdentifier(identifier);

  DefaultVisitor::visitNamespace (node);

  closeContext();
}

void ContextBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  openContext(node, DUContext::Class, node->name);

  DefaultVisitor::visitClassSpecifier (node);

  closeContext(node->name);
}

void ContextBuilder::visitTypedef (TypedefAST *node)
{
  DefaultVisitor::visitTypedef (node);

  // Didn't get claimed if it was still set
  m_importedParentContexts.clear();
}

void ContextBuilder::visitFunctionDefinition (FunctionDefinitionAST *node)
{
  if (node && node->init_declarator && node->init_declarator->declarator && node->init_declarator->declarator->id) {
    QualifiedIdentifier functionName = identifierForName(node->init_declarator->declarator->id);
    if (functionName.count() >= 2) {
      // This is a class function
      functionName.pop();

      if (DUContext* classContext = currentContext()->findContext(DUContext::Class, functionName))
        m_importedParentContexts.append(classContext);
    }
  }

  visit(node->type_specifier);
  visit(node->init_declarator);

  if (node->constructor_initializers && node->function_body) {
    openContext(node->constructor_initializers, node->function_body, DUContext::Other);
    addImportedContexts();
  }
  // Otherwise, the context is created in the function body visit

  visit(node->constructor_initializers);
  visit(node->function_body);

  if (node->constructor_initializers)
    closeContext();

  visit(node->win_decl_specifiers);

  // If still defined, not needed
  m_importedParentContexts.clear();
}

DUContext* ContextBuilder::openContext(AST* rangeNode, DUContext::ContextType type, NameAST* identifier)
{
  if (m_compilingContexts) {
    Range* range = m_editor->createRange(rangeNode);
    DUContext* ret = openContextInternal(range, type, identifier);
    rangeNode->ducontext = ret;
    return ret;

  } else {
    m_contextStack.push(rangeNode->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContext(AST* fromRange, AST* toRange, DUContext::ContextType type, NameAST* identifier)
{
  if (m_compilingContexts) {
    Range* range = m_editor->createRange(fromRange, toRange);
    DUContext* ret = openContextInternal(range, type, identifier);
    fromRange->ducontext = ret;
    return ret;

  } else {
    m_contextStack.push(fromRange->ducontext);
    m_editor->setCurrentRange(currentContext()->textRangePtr());
    return currentContext();
  }
}

DUContext* ContextBuilder::openContextInternal(Range* range, DUContext::ContextType type, NameAST* identifier)
{
  Q_ASSERT(m_compilingContexts);

  DUContext* ret = new DUContext(range, m_contextStack.isEmpty() ? 0 : currentContext());
  ret->setType(type);

  if (identifier)
    ret->setLocalScopeIdentifier(identifierForName(identifier));

  m_contextStack.push(ret);

  return ret;
}

void ContextBuilder::closeContext(NameAST* name, AST* node)
{
  if (m_compilingContexts) {
    if (node) {
      // Find the end position of this function definition (just inside the bracket)
      Cursor endPosition = m_editor->findPosition(node->end_token, CppEditorIntegrator::FrontEdge);

      // Set the correct end point of the current context finishing here
      if (currentContext()->textRange().end() != endPosition)
        currentContext()->textRange().end() = endPosition;
    }

    // Set context identifier
    if (name)
      currentContext()->setLocalScopeIdentifier(identifierForName(name));
  }

  // Go back to the context prior to this function definition
  m_contextStack.pop();

  // Go back to the previous range
  m_editor->exitCurrentRange();
}

void ContextBuilder::visitParameterDeclarationClause (ParameterDeclarationClauseAST * node)
{
  m_importedParentContexts.append(openContext(node, DUContext::Function));

  DefaultVisitor::visitParameterDeclarationClause (node);

  closeContext();
}

void ContextBuilder::visitCompoundStatement (CompoundStatementAST * node)
{
  openContext(node, DUContext::Other);

  addImportedContexts();

  DefaultVisitor::visitCompoundStatement (node);

  closeContext();
}

void ContextBuilder::visitSimpleDeclaration (SimpleDeclarationAST *node)
{
  DefaultVisitor::visitSimpleDeclaration (node);

  // Didn't get claimed if it was still set
  m_importedParentContexts.clear();
}

void ContextBuilder::visitName (NameAST *)
{
  // Note: we don't want to visit the name node, the name compiler does that for us (only when we need it)
}

void ContextBuilder::visitUsingDirective(UsingDirectiveAST * node)
{
  DefaultVisitor::visitUsingDirective(node);

  if (m_compilingContexts && node->name)
    currentContext()->addUsingNamespace(m_editor->createCursor(node->end_token, CppEditorIntegrator::FrontEdge), identifierForName(node->name));
}

void ContextBuilder::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
  // TODO store the alias
  DefaultVisitor::visitNamespaceAliasDefinition(node);
}

void ContextBuilder::visitUsing(UsingAST* node)
{
  // TODO store the using
  DefaultVisitor::visitUsing(node);
}

void ContextBuilder::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
  // FIXME ... needs type support
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

void ContextBuilder::visitForStatement(ForStatementAST *node)
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
  m_importedParentContexts.clear();
}

void ContextBuilder::addImportedContexts()
{
  if (m_compilingContexts && !m_importedParentContexts.isEmpty()) {
    /*foreach (DUContext* imported, m_importedParentContexts)
      if (imported->parentContext() && imported->url() == currentContext()->url())
        imported->parentContext()->takeChildContext(imported);*/

    foreach (DUContext* imported, m_importedParentContexts)
      currentContext()->addImportedParentContext(imported);

    m_importedParentContexts.clear();
  }
}

void ContextBuilder::visitIfStatement(IfStatementAST* node)
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

bool ContextBuilder::createContextIfNeeded(AST* node, DUContext* importedParentContext)
{
  return createContextIfNeeded(node, QList<DUContext*>() << importedParentContext);
}

bool ContextBuilder::createContextIfNeeded(AST* node, const QList<DUContext*>& importedParentContexts)
{
  m_importedParentContexts = importedParentContexts;

  const bool contextNeeded = !ast_cast<CompoundStatementAST*>(node);
  if (contextNeeded) {
    openContext(node, DUContext::Other);
    addImportedContexts();
  }
  return contextNeeded;
}

const QualifiedIdentifier& ContextBuilder::identifierForName(NameAST* id) const
{
  Q_ASSERT(id);
  m_nameCompiler->run(id);
  return m_nameCompiler->identifier();
}
