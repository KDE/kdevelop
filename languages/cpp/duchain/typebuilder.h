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

#ifndef TYPEBUILDER_H
#define TYPEBUILDER_H

#include "contextbuilder.h"
#include "typesystem.h"

typedef ContextBuilder TypeBuilderBase;

class TypeBuilder: public TypeBuilderBase
{
public:
  TypeBuilder(ParseSession* session);
  TypeBuilder(CppEditorIntegrator* editor);

  /**
   * Build types by iterating the given \a node.
   */
  void buildTypes(AST *node);

protected:
  AbstractType::Ptr lastType() const;

  virtual void visitClassSpecifier(ClassSpecifierAST*);
  virtual void visitBaseSpecifier(BaseSpecifierAST*);
  virtual void visitEnumSpecifier(EnumSpecifierAST*);
  virtual void visitEnumerator(EnumeratorAST*);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST*);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST*);
  virtual void visitTypedef(TypedefAST*);
  virtual void visitFunctionDefinition(FunctionDefinitionAST*);
  virtual void visitTypeSpecifierAST(TypeSpecifierAST*);
  virtual void visitPtrOperator(PtrOperatorAST*);

private:
  template <class T>
  void openType(KSharedPtr<T> type, AST* node)
  { openAbstractType(AbstractType::Ptr::staticCast(type), node); }

  void openAbstractType(AbstractType::Ptr type, AST* node);
  void closeType();

  void parseStorageSpecifiers(const ListNode<std::size_t>* storage_specifiers);
  void parseFunctionSpecifiers(const ListNode<std::size_t>* function_specifiers);

  inline AbstractType::Ptr currentAbstractType() { return m_typeStack.top(); }

  template <class T>
  KSharedPtr<T> currentType() { return KSharedPtr<T>::dynamicCast(m_typeStack.top()); }

  QStack<AbstractType::Ptr> m_typeStack;
};

#endif // TYPEBUILDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
