/* This file is part of KDevelop
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

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "definition.h"
#include "tokens.h"
#include "parsesession.h"

using namespace KTextEditor;

DefinitionBuilder::DefinitionBuilder (ParseSession* session)
  : DefinitionBuilderBase(session)
{
}

DefinitionBuilder::DefinitionBuilder (CppEditorIntegrator* editor)
  : DefinitionBuilderBase(editor)
{
}

TopDUContext* DefinitionBuilder::buildDefinitions(const KUrl& url, AST *node, QList<DUContext*>* includes)
{
  TopDUContext* top = buildContexts(url, node, includes);

  Q_ASSERT(m_accessPolicyStack.isEmpty());

  return top;
}

void DefinitionBuilder::visitDeclarator (DeclaratorAST* node)
{
  // Don't create a definition for a function
  /*if (node->parameter_declaration_clause) {
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
    openDefinition(node->id, node);
  }*/

  // TODO: don't create another definition if this is just a definition of a previously declared function or variable
  openDefinition(node->id, node);

  parseConstVolatile(node->fun_cv);

  DefinitionBuilderBase::visitDeclarator(node);

  closeDefinition();
}

void DefinitionBuilder::visitPtrOperator(PtrOperatorAST* node)
{
  parseConstVolatile(node->cv);

  DefinitionBuilderBase::visitPtrOperator(node);
}

Definition* DefinitionBuilder::openDefinition(NameAST* name, AST* rangeNode)
{
  Definition::Scope scope = Definition::GlobalScope;
  switch (currentContext()->type()) {
    case DUContext::Namespace:
      scope = Definition::NamespaceScope;
      break;
    case DUContext::Class:
      scope = Definition::ClassScope;
      break;
    case DUContext::Function:
      scope = Definition::LocalScope;
      break;
    default:
      break;
  }

  Range* prior = m_editor->currentRange();
  Range* range = m_editor->createRange(name ? static_cast<AST*>(name) : rangeNode);
  m_editor->exitCurrentRange();
  Q_ASSERT(m_editor->currentRange() == prior);

  Definition* definition = new Definition(range, scope);
  currentContext()->addDefinition(definition);

  if (name) {
    QualifiedIdentifier id = identifierForName(name);

    // FIXME this can happen if we're defining a staticly declared variable
    //Q_ASSERT(m_nameCompiler->identifier().count() == 1);
    Q_ASSERT(!id.isEmpty());
    definition->setIdentifier(id.first());
  }

  if (currentContext()->type() == DUContext::Class)
    definition->setAccessPolicy(currentAccessPolicy());

  m_definitionStack.push(definition);

  return definition;
}

void DefinitionBuilder::closeDefinition()
{
  m_definitionStack.pop();
}

void DefinitionBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  int kind = m_editor->parseSession()->token_stream->kind(node->class_key);
  if (kind == Token_struct || kind == Token_union)
    m_accessPolicyStack.push(Cpp::Public);
  else
    m_accessPolicyStack.push(Cpp::Private);

  DefinitionBuilderBase::visitClassSpecifier(node);

  m_accessPolicyStack.pop();
}

void DefinitionBuilder::visitAccessSpecifier(AccessSpecifierAST* node)
{
  if (node->specs) {
    const ListNode<std::size_t> *it = node->specs->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_signals:
        case Token_slots:
        case Token_k_dcop:
        case Token_k_dcop_signals:
          break;
        case Token_public:
          setAccessPolicy(Cpp::Public);
          break;
        case Token_protected:
          setAccessPolicy(Cpp::Protected);
          break;
        case Token_private:
          setAccessPolicy(Cpp::Private);
          break;
      }

      it = it->next;
    } while (it != end);
  }

  DefinitionBuilderBase::visitAccessSpecifier(node);
}

void DefinitionBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  parseConstVolatile(node->cv);

  DefinitionBuilderBase::visitElaboratedTypeSpecifier(node);
}

void DefinitionBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
  parseConstVolatile(node->cv);

  DefinitionBuilderBase::visitSimpleTypeSpecifier(node);
}

void DefinitionBuilder::parseConstVolatile(const ListNode<std::size_t> *cv)
{
  if (cv && currentDefinition()) {
    const ListNode<std::size_t> *it = cv->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      if (kind == Token_const)
        currentDefinition()->setConstant(true);
      else if (kind == Token_volatile)
        currentDefinition()->setVolatile(true);

      it = it->next;
    } while (it != end);
  }
}
