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

#include "typebuilder.h"

#include <ktexteditor/smartrange.h>

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "ducontext.h"
#include "cpptypes.h"
#include "parsesession.h"
#include "tokens.h"
#include "definition.h"

TypeBuilder::TypeBuilder(ParseSession* session)
  : TypeBuilderBase(session)
{
}

TypeBuilder::TypeBuilder(CppEditorIntegrator * editor)
  : TypeBuilderBase(editor)
{
}

void TypeBuilder::buildTypes(AST *node)
{
  supportBuild(node);

  Q_ASSERT(m_typeStack.isEmpty());
}

void TypeBuilder::openAbstractType(AbstractType::Ptr type, AST* node, NameAST* id)
{
  if (FunctionType::Ptr function = currentType<FunctionType>()) {
    if (!function->returnType())
      function->setReturnType(type);
    else
      function->addArgument(type);

  } else if (StructureType::Ptr structure = currentType<StructureType>()) {
    structure->addElement(type);

  } else if (PointerType::Ptr pointer = currentType<PointerType>()) {
    pointer->setBaseType(type);

  } else if (ReferenceType::Ptr reference = currentType<ReferenceType>()) {
    reference->setBaseType(type);

  } else if (ArrayType::Ptr array = currentType<ArrayType>()) {
    array->setElementType(type);
  }

  m_typeStack.append(type);
}

void TypeBuilder::closeType()
{
  m_typeStack.pop();
}

void TypeBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  CppClassType::Ptr classType(new CppClassType());
  openType(classType, node, node->name);

  int kind = m_editor->parseSession()->token_stream->kind(node->class_key);
  if (kind == Token_struct)
    classType->setClassType(CppClassType::Struct);
  else if (kind == Token_union)
    classType->setClassType(CppClassType::Union);

  TypeBuilderBase::visitClassSpecifier(node);

  closeType();
}

void TypeBuilder::visitBaseSpecifier(BaseSpecifierAST *node)
{
  if (node->name) {
    QualifiedIdentifier baseClassIdentifier = identifierForName(node->name);
  }

  TypeBuilderBase::visitBaseSpecifier(node);
}

void TypeBuilder::visitEnumSpecifier(EnumSpecifierAST *node)
{
  openType(CppEnumerationType::Ptr(new CppEnumerationType()), node);

  TypeBuilderBase::visitEnumSpecifier(node);

  closeType();
}

void TypeBuilder::visitEnumerator(EnumeratorAST* node)
{
  bool ok = false;
  if (CppEnumerationType::Ptr parent = currentType<CppEnumerationType>()) {
    CppEnumeratorType::Ptr enumerator(new CppEnumeratorType());
    openType(enumerator, node);
    ok = true;
  }

  TypeBuilderBase::visitEnumerator(node);

  if (ok)
    closeType();
}

void TypeBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  AbstractType::Ptr type;

  if (node->name) {
    QualifiedIdentifier id = identifierForName(node->name);
    KDevDocumentCursor pos(m_editor->currentUrl(), m_editor->findPosition(node->start_token, KDevEditorIntegrator::FrontEdge));

    Definition * def = currentContext()->findDefinition(id, pos);

    if (def && def->type()) {
      int kind = m_editor->parseSession()->token_stream->kind(node->type);
      switch (kind) {
        case Token_class:
        case Token_struct:
        case Token_union:
          if (CppClassType::Ptr::dynamicCast(def->type()))
            type = def->type();
          break;
        case Token_enum:
          if (CppEnumeratorType::Ptr::dynamicCast(def->type()))
            type = def->type();
          break;
        case Token_typename:
          type = def->type();
          break;
      }

      if (type)
        openType(type, node, node->name);
    }
  }

  TypeBuilderBase::visitElaboratedTypeSpecifier(node);

  if (type)
    closeType();
}

void TypeBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  if (node->integrals) {
    if (CppClassType::Ptr classType = currentType<CppClassType>())
      openType(CppSpecificClassMemberType<CppIntegralType>::Ptr(new CppSpecificClassMemberType<CppIntegralType>(classType)), node, node->name);
    else
      openType(CppIntegralType::Ptr(new CppIntegralType()), node, node->name);

    const ListNode<std::size_t> *it = node->integrals->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_char:
        case Token_wchar_t:
        case Token_bool:
        case Token_short:
        case Token_int:
        case Token_long:
        case Token_signed:
        case Token_unsigned:
        case Token_float:
        case Token_double:
        case Token_void:
          break;
      }

      it = it->next;
    } while (it != end);
  }

  TypeBuilderBase::visitSimpleTypeSpecifier(node);

  if (node->integrals)
    closeType();
}

void TypeBuilder::visitTypedef(TypedefAST* node)
{
  openType(CppTypeAliasType::Ptr(new CppTypeAliasType()), node);

  TypeBuilderBase::visitTypedef(node);

  closeType();
}

void TypeBuilder::visitFunctionDefinition(FunctionDefinitionAST* node)
{
  if (CppClassType::Ptr classType = currentType<CppClassType>()) {
    openType(CppClassFunctionType::Ptr(new CppClassFunctionType(classType)), node);

  } else {
    openType(FunctionType::Ptr(new FunctionType()), node);
  }

  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  TypeBuilderBase::visitFunctionDefinition(node);

  closeType();
}

void TypeBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  if (CppClassType::Ptr classType = currentType<CppClassType>())
    openType(CppClassFunctionType::Ptr(new CppClassFunctionType(classType)), node);
  else
    openType(FunctionType::Ptr(new FunctionType()), node);

  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  TypeBuilderBase::visitSimpleDeclaration(node);

  closeType();
}

void TypeBuilder::visitTypeSpecifierAST(TypeSpecifierAST* node)
{
  if (node->cv) {
    if (CppTypeInfo* typeInfo = dynamic_cast<CppTypeInfo*>(currentAbstractType().data())) {
      const ListNode<std::size_t> *it = node->cv->toFront();
      const ListNode<std::size_t> *end = it;
      do {
        int kind = m_editor->parseSession()->token_stream->kind(it->element);
        if (kind == Token_const)
          typeInfo->setConstant(true);
        else if (kind == Token_volatile)
          typeInfo->setVolatile(true);

        it = it->next;
      } while (it != end);
    }
  }
}

void TypeBuilder::visitPtrOperator(PtrOperatorAST* node)
{
  openType(PointerType::Ptr(new PointerType()), node);

  if (CppClassType::Ptr classType = currentType<CppClassType>())
    openType(CppSpecificClassMemberType<CppIntegralType>::Ptr(new CppSpecificClassMemberType<CppIntegralType>(classType)), node);
  else
    openType(CppIntegralType::Ptr(new CppIntegralType()), node);

  TypeBuilderBase::visitPtrOperator(node);

  closeType();
  closeType();
}

void TypeBuilder::parseStorageSpecifiers(const ListNode<std::size_t>* storage_specifiers)
{
  if (CppClassMemberType* memberType = dynamic_cast<CppClassMemberType*>(currentAbstractType().data())) {
    const ListNode<std::size_t> *it = storage_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_friend:
          memberType->setFriend(true);
          break;
        case Token_auto:
          memberType->setAuto(true);
          break;
        case Token_register:
          memberType->setRegister(true);
          break;
        case Token_static:
          memberType->setStatic(true);
          break;
        case Token_extern:
          memberType->setExtern(true);
          break;
        case Token_mutable:
          memberType->setMutable(true);
          break;
      }

      it = it->next;
    } while (it != end);
  }
}

void TypeBuilder::parseFunctionSpecifiers(const ListNode<std::size_t>* function_specifiers)
{
  if (CppClassFunctionType::Ptr functionType = currentType<CppClassFunctionType>()) {
    const ListNode<std::size_t> *it = function_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_inline:
          functionType->setInline(true);
          break;
        case Token_virtual:
          functionType->setVirtual(true);
          break;
        case Token_explicit:
          functionType->setExplicit(true);
          break;
      }

      it = it->next;
    } while (it != end);
  }
}

// kate: space-indent on; indent-width 2; replace-tabs on;
