/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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

void TypeBuilder::openType(AbstractType* type, AST* node, NameAST* id)
{
  if (FunctionType* function = currentType<FunctionType>()) {
    function->addArgument(type);

  } else if (StructureType* structure = currentType<StructureType>()) {
    return structure->addElement(type);

  } else if (PointerType* pointer = currentType<PointerType>()) {
    pointer->setBaseType(type);

  } else if (ReferenceType* reference = currentType<ReferenceType>()) {
    reference->setBaseType(type);

  } else if (ArrayType* array = currentType<ArrayType>()) {
    array->setElementType(type);

  } else {
    TypeInstance* instance = new TypeInstance(m_editor->createCursor(node->start_token));
    instance->setType(type);
    if (id) {
      QualifiedIdentifier qid = identifierForName(id);
      Q_ASSERT(qid.count() == 1);
      instance->setIdentifier(qid.first());
    }

    currentContext()->addType(instance);
  }

  node->type = type;

  m_typeStack.append(type);
}

void TypeBuilder::closeType()
{
  m_typeStack.pop();
}

void TypeBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  openType(new CppClassType(currentContext()), node, node->name);

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
  openType(new CppEnumerationType(), node);

  TypeBuilderBase::visitEnumSpecifier(node);

  closeType();
}

void TypeBuilder::visitEnumerator(EnumeratorAST* node)
{
  bool ok = false;
  if (CppEnumerationType* parent = currentType<CppEnumerationType>()) {
    CppEnumeratorType* enumerator = new CppEnumeratorType(parent);
    openType(enumerator, node);
    ok = true;
  }

  TypeBuilderBase::visitEnumerator(node);

  if (ok)
    closeType();
}

void TypeBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *node)
{
  TypeBuilderBase::visitElaboratedTypeSpecifier(node);
}

void TypeBuilder::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *node)
{
  if (CppClassType* classType = currentType<CppClassType>())
    openType(new CppSpecificClassMemberType<CppIntegralType>(classType), node);
  else
    openType(new CppIntegralType(), node);

  TypeBuilderBase::visitSimpleTypeSpecifier(node);

  closeType();
}

void TypeBuilder::visitTypedef(TypedefAST* node)
{
  openType(new CppTypeAliasType(currentContext()), node);

  TypeBuilderBase::visitTypedef(node);

  closeType();
}

void TypeBuilder::visitFunctionDefinition(FunctionDefinitionAST* node)
{
  if (CppClassType* classType = currentType<CppClassType>())
    openType(new CppClassFunctionType(classType), node);
  else
    openType(new FunctionType(), node);

  TypeBuilderBase::visitFunctionDefinition(node);

  closeType();
}

void TypeBuilder::visitTypeSpecifierAST(TypeSpecifierAST* node)
{
  if (node->cv) {
    if (CppTypeInfo* typeInfo = currentType<CppTypeInfo>()) {
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
  openType(new PointerType(), node);

  if (CppClassType* classType = currentType<CppClassType>())
    openType(new CppSpecificClassMemberType<CppIntegralType>(classType), node);
  else
    openType(new CppIntegralType(), node);

  TypeBuilderBase::visitPtrOperator(node);

  closeType();
  closeType();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
