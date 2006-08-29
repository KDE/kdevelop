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
  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  DefinitionBuilderBase::visitFunctionDeclaration(node);

  popSpecifiers();
}

void DefinitionBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  DefinitionBuilderBase::visitSimpleDeclaration(node);

  popSpecifiers();
}

void DefinitionBuilder::visitDeclarator (DeclaratorAST* node)
{
  if (node->parameter_declaration_clause) {
    openDefinition(node->id, node, true);

    /*if (!node->type_specifier) {
      // TODO detect identifiers not equal to classname
      if (currentDefinition()->identifier().toString().startsWith('~'))
        static_cast<ClassFunctionDefinition*>(currentDefinition())->setDestructor(true);
      else
        static_cast<ClassFunctionDefinition*>(currentDefinition())->setConstructor(true);
    }*/

    applyFunctionSpecifiers();

  } else {
    openDefinition(node->id, node);
  }

  applyStorageSpecifiers();

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
  if (isFunction)
    definition = new ClassFunctionDefinition(range);
  else if (scope == Definition::ClassScope)
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
  ClassMemberDefinition::StorageSpecifiers specs = 0;

  if (storage_specifiers) {
    const ListNode<std::size_t> *it = storage_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_friend:
          specs |= ClassMemberDefinition::FriendSpecifier;
          break;
        case Token_auto:
          specs |= ClassMemberDefinition::AutoSpecifier;
          break;
        case Token_register:
          specs |= ClassMemberDefinition::RegisterSpecifier;
          break;
        case Token_static:
          specs |= ClassMemberDefinition::StaticSpecifier;
          break;
        case Token_extern:
          specs |= ClassMemberDefinition::ExternSpecifier;
          break;
        case Token_mutable:
          specs |= ClassMemberDefinition::MutableSpecifier;
          break;
      }

      it = it->next;
    } while (it != end);
  }

  m_storageSpecifiers.push(specs);
}

void DefinitionBuilder::parseFunctionSpecifiers(const ListNode<std::size_t>* function_specifiers)
{
  ClassFunctionDefinition::FunctionSpecifiers specs = 0;

  if (function_specifiers) {
    const ListNode<std::size_t> *it = function_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_inline:
          specs |= ClassFunctionDefinition::InlineSpecifier;
          break;
        case Token_virtual:
          specs |= ClassFunctionDefinition::VirtualSpecifier;
          break;
        case Token_explicit:
          specs |= ClassFunctionDefinition::ExplicitSpecifier;
          break;
      }

      it = it->next;
    } while (it != end);
  }

  m_functionSpecifiers.push(specs);
}

void DefinitionBuilder::popSpecifiers()
{
  m_functionSpecifiers.pop();
  m_storageSpecifiers.pop();
}

void DefinitionBuilder::applyStorageSpecifiers()
{
  if (!m_storageSpecifiers.isEmpty())
    if (ClassMemberDefinition* member = dynamic_cast<ClassMemberDefinition*>(currentDefinition()))
      member->setStorageSpecifiers(m_storageSpecifiers.top());
}

void DefinitionBuilder::applyFunctionSpecifiers()
{
  if (!m_functionSpecifiers.isEmpty()) {
    Q_ASSERT(dynamic_cast<ClassFunctionDefinition*>(currentDefinition()));
    ClassFunctionDefinition* function = static_cast<ClassFunctionDefinition*>(currentDefinition());
    function->setFunctionSpecifiers(m_functionSpecifiers.top());
  }
}
