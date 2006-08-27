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
#include "classfunctiondefinition.h"
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

void DefinitionBuilder::visitFunctionDeclaration(FunctionDefinitionAST* node)
{
  bool functionOpened = false;

  if (node && node->init_declarator && node->init_declarator->declarator && node->init_declarator->declarator->id) {
    functionOpened = true;
    openDefinition(node->init_declarator->declarator->id, node, true);

    parseStorageSpecifiers(node->storage_specifiers);
    parseFunctionSpecifiers(node->function_specifiers);
  }

  DefinitionBuilderBase::visitFunctionDeclaration(node);

  if (functionOpened)
    closeDefinition();
}

void DefinitionBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  bool functionOpened = false;

  /*if (node && node->init_declarator && node->init_declarator->declarator && node->init_declarator->declarator->id) {
    functionOpened = true;
    openDefinition(node->init_declarator->declarator->id, node, true);

    parseStorageSpecifiers(node->storage_specifiers);
    parseFunctionSpecifiers(node->function_specifiers);
  }*/

  DefinitionBuilderBase::visitSimpleDeclaration(node);

  if (functionOpened)
    closeDefinition();
}

void DefinitionBuilder::visitDeclarator (DeclaratorAST* node)
{
  if (node->parameter_declaration_clause) {
    switch (currentContext()->type()) {
      case DUContext::Global:
      case DUContext::Namespace:
      case DUContext::Class:
      case DUContext::Function:
          DefinitionBuilderBase::visitDeclarator(node);
          return;

      default:
          openDefinition(node->id, node);
          break;
    }

  } else {
    openDefinition(node->id, node);
  }

  DefinitionBuilderBase::visitDeclarator(node);

  closeDefinition();
}

Definition* DefinitionBuilder::openDefinition(NameAST* name, AST* rangeNode, bool isFunction)
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

  Definition* definition;
  if (scope == Definition::ClassScope)
    if (isFunction)
      definition = new ClassFunctionDefinition(range);
    else
      definition = new ClassMemberDefinition(range);
  else
    definition = new Definition(range, scope);

  currentContext()->addDefinition(definition);

  if (name) {
    QualifiedIdentifier id = identifierForName(name);

    // FIXME this can happen if we're defining a staticly declared variable
    //Q_ASSERT(m_nameCompiler->identifier().count() == 1);
    Q_ASSERT(!id.isEmpty());
    definition->setIdentifier(id.first());
  }

  if (currentContext()->type() == DUContext::Class)
    static_cast<ClassMemberDefinition*>(definition)->setAccessPolicy(currentAccessPolicy());

  m_definitionStack.push(definition);

  return definition;
}

void DefinitionBuilder::closeDefinition()
{
  if (AbstractType::Ptr type = lastType())
    currentDefinition()->setType(type);

  m_definitionStack.pop();
}

void DefinitionBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  openDefinition(node->name, node);

  int kind = m_editor->parseSession()->token_stream->kind(node->class_key);
  if (kind == Token_struct || kind == Token_union)
    m_accessPolicyStack.push(Cpp::Public);
  else
    m_accessPolicyStack.push(Cpp::Private);

  DefinitionBuilderBase::visitClassSpecifier(node);

  closeDefinition();

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

void DefinitionBuilder::parseStorageSpecifiers(const ListNode<std::size_t>* storage_specifiers)
{
  if (!storage_specifiers)
    return;

  if (ClassMemberDefinition* member = dynamic_cast<ClassMemberDefinition*>(currentDefinition())) {
    const ListNode<std::size_t> *it = storage_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_friend:
          member->setFriend(true);
          break;
        case Token_auto:
          member->setAuto(true);
          break;
        case Token_register:
          member->setRegister(true);
          break;
        case Token_static:
          member->setStatic(true);
          break;
        case Token_extern:
          member->setExtern(true);
          break;
        case Token_mutable:
          member->setMutable(true);
          break;
      }

      it = it->next;
    } while (it != end);
  }
}

void DefinitionBuilder::parseFunctionSpecifiers(const ListNode<std::size_t>* function_specifiers)
{
  if (!function_specifiers)
    return;

  if (ClassFunctionDefinition* function = dynamic_cast<ClassFunctionDefinition*>(currentDefinition())) {
    const ListNode<std::size_t> *it = function_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_inline:
          function->setInline(true);
          break;
        case Token_virtual:
          function->setVirtual(true);
          break;
        case Token_explicit:
          function->setExplicit(true);
          break;
      }

      it = it->next;
    } while (it != end);
  }
}
