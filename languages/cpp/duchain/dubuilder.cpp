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

#include "lexer.h"
#include "duchain.h"
#include "typesystem.h"
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "definition.h"

using namespace KTextEditor;

DUBuilder::DUBuilder (TokenStream *token_stream):
  _M_token_stream (token_stream), m_editor(new CppEditorIntegrator(token_stream)), m_nameCompiler(new NameCompiler(token_stream)),
  in_namespace(false), in_class(false), in_template_declaration(false),
  in_typedef(false), in_function_definition(false), in_parameter_declaration(false),
  m_types(new TypeEnvironment)
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
    m_contextStack.push(topLevelContext);

    Q_ASSERT(topLevelContext->textRangePtr());
    // FIXME for now, just clear the chain... later, need to implement incremental parsing
    topLevelContext->deleteChildContextsRecursively();
    topLevelContext->deleteLocalDefinitions();

    Q_ASSERT(topLevelContext->textRangePtr());

    // FIXME remove once conversion works
    if (!topLevelContext->smartRange() && m_editor->smart())
      topLevelContext->setTextRange(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain));

  } else {
    // FIXME the top range will probably get deleted without the editor integrator knowing...?
    topLevelContext = openContext(m_editor->topRange(CppEditorIntegrator::DefinitionUseChain), DUContext::Global);

    DUChain::self()->addDocumentChain(url, topLevelContext);
  }

  m_editor->setCurrentRange(currentContext()->textRangePtr());

  visit (node);

  closeContext();

  Q_ASSERT(m_contextStack.isEmpty());

  return topLevelContext;
}

void DUBuilder::visitNamespace (NamespaceAST *node)
{
  QualifiedIdentifier identifier = currentContext()->scopeIdentifier();
  if (node->namespace_name)
    identifier << QualifiedIdentifier(_M_token_stream->symbol(node->namespace_name)->as_string());
  else
    identifier << Identifier::unique(reinterpret_cast<long>(_M_token_stream));

  openContext(node, DUContext::Namespace)->setLocalScopeIdentifier(identifier);

  bool was = inNamespace (true);
  DefaultVisitor::visitNamespace (node);
  inNamespace (was);

  closeContext();
}

void DUBuilder::visitClassSpecifier (ClassSpecifierAST *node)
{
  openContext(node, DUContext::Class);

  //newDeclaration(node->name, previousContext);

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
  openContext(node, DUContext::Function);

  bool was = inFunctionDefinition (node);
  DefaultVisitor::visitFunctionDefinition (node);
  inFunctionDefinition (was);

  closeContext(0, node);
}

DUContext* DUBuilder::openContext(AST* rangeNode, DUContext::ContextType type)
{
  Range* range = m_editor->createRange(rangeNode);
  return openContext(range, type);
}

DUContext* DUBuilder::openContext(Range* range, DUContext::ContextType type)
{
  DUContext* ret = new DUContext(range, m_contextStack.isEmpty() ? 0 : currentContext());
  ret->setType(type);

  m_contextStack.push(ret);

  return ret;
}

void DUBuilder::closeContext(NameAST* name, AST* node)
{
  /* FIXME hrm... not sure why this doesn't pass
  if (currentContext()->smartRange() && parent->smartRange())
    Q_ASSERT(currentContext()->smartRange()->parentRange() == parent->smartRange());*/

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

  // Go back to the context prior to this function definition
  m_contextStack.pop();

  // Go back to the previous range
  m_editor->exitCurrentRange();
}

void DUBuilder::visitParameterDeclarationClause (ParameterDeclarationClauseAST * node)
{
  //openContext(node, DUContext::Function);

  bool was = inParameterDeclaration (node);
  DefaultVisitor::visitParameterDeclarationClause (node);
  inParameterDeclaration (was);
}

void DUBuilder::visitParameterDeclaration (ParameterDeclarationAST * node)
{
  DefaultVisitor::visitParameterDeclaration (node);
}

void DUBuilder::visitCompoundStatement (CompoundStatementAST * node)
{
  openContext(node, DUContext::Other);

  DefaultVisitor::visitCompoundStatement (node);

  closeContext();
}

void DUBuilder::visitSimpleDeclaration (SimpleDeclarationAST *node)
{
  // We need to detect function declarations
  // Replaces DefaultVisitor::visitSimpleDeclaration (node);

  visit(node->type_specifier);

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

  visit(node->win_decl_specifiers);
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
          break;

      case DUContext::Function:
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
  Range* current = m_editor->currentRange();
  Range* use = m_editor->createRange(name);
  m_editor->exitCurrentRange();
  Q_ASSERT(m_editor->currentRange() == current);

  if (use->isSmartRange())
    if (use->toSmartRange()->parentRange() != currentContext()->smartRange())
      kWarning() << k_funcinfo << "Use " << *use << " parent " << *use->toSmartRange()->parentRange() << " " << use->toSmartRange()->parentRange() << " != current context " << *currentContext()->smartRange() << " " << currentContext()->smartRange() << " id " << currentContext()->scopeIdentifier() << endl;

  m_nameCompiler->run(name);

  Definition* definition = currentContext()->findDefinition(m_nameCompiler->identifier(), KDevDocumentCursor(use, KDevDocumentCursor::Start));
  if (definition)
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

  m_nameCompiler->run(node->name);

  currentContext()->addUsingNamespace(m_editor->createCursor(node->end_token, CppEditorIntegrator::FrontEdge), m_nameCompiler->identifier());
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
