/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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
#include <typesystem.h>
#include <declaration.h>
#include "cppduchainbuilderexport.h"

class CppClassType;
class CppFunctionType;

namespace KDevelop {
  class QualifiedIdentifier;
  class ForwardDeclaration;
}

typedef ContextBuilder TypeBuilderBase;

/**
 * Create types from an AST tree.
 *
 * \note This builder overrides visitDeclarator, in order to support
 * array types; parent classes will not have
 * their visitDeclarator function called.
 */
class KDEVCPPDUCHAINBUILDER_EXPORT TypeBuilder: public TypeBuilderBase
{
public:
  TypeBuilder(ParseSession* session);
  TypeBuilder(CppEditorIntegrator* editor);

  /**
   * Build types by iterating the given \a node.
   * @param context the context to use. Must be set when the given node has no context. When it has one attached, this parameter is not needed. However it will always be preferred over the node's context.
   */
  virtual void supportBuild(AST *node, KDevelop::DUContext* context = 0);

  const QList< KDevelop::AbstractType::Ptr >& topTypes() const;

protected:
  ///Returns either the current context, or the last importend parent-context(needed to find template-argument function return-values)
  KDevelop::DUContext* searchContext() ;
  
  KDevelop::AbstractType::Ptr lastType() const;

  void setLastType(KDevelop::AbstractType::Ptr ptr);
  
  // Called at the beginning of processing a class-specifier, right after the type for the class was created. The type can be gotten through currentAbstractType().
  virtual void classTypeOpened(KDevelop::AbstractType::Ptr) {};
  
  // Created visitors
  virtual void visitArrayExpression(ExpressionAST*);
  
  // Regular visitors
  virtual void visitClassSpecifier(ClassSpecifierAST*);
  virtual void visitBaseSpecifier(BaseSpecifierAST*);
  virtual void visitEnumSpecifier(EnumSpecifierAST*);
  virtual void visitEnumerator(EnumeratorAST*);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST*);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST*);
  virtual void visitTypedef(TypedefAST*);
  virtual void visitFunctionDeclaration(FunctionDefinitionAST*);
  virtual void visitPtrOperator(PtrOperatorAST*);
  virtual void visitDeclarator(DeclaratorAST*);
  virtual void visitParameterDeclaration(ParameterDeclarationAST*);
  virtual void visitTemplateParameter(TemplateParameterAST *);

  virtual void addBaseType( CppClassType::BaseClassInstance base );

  private:
  template <class T>
  void openType(KSharedPtr<T> type, AST* node)
  { openAbstractType(KDevelop::AbstractType::Ptr::staticCast(type), node); }

  void openDelayedType(const KDevelop::QualifiedIdentifier& identifier, AST* node, DelayedType::Kind kind);
  
  void openAbstractType(KDevelop::AbstractType::Ptr type, AST* node);
  void closeType();

  CppClassType* openClass(int kind);
  CppFunctionType* openFunction(DeclaratorAST *node);

  KDevelop::Declaration::CVSpecs parseConstVolatile(const ListNode<std::size_t>* cv);

  bool hasCurrentType() { return !m_typeStack.isEmpty(); }

  // You must not use this in creating another type definition, as it may not be the registered type.
  inline KDevelop::AbstractType::Ptr currentAbstractType() { return m_typeStack.top(); }

  // You must not use this in creating another type definition, as it may not be the registered type.
  template <class T>
  KSharedPtr<T> currentType() { return KSharedPtr<T>::dynamicCast(m_typeStack.top()); }

  QStack<KDevelop::AbstractType::Ptr> m_typeStack;

  KDevelop::AbstractType::Ptr m_lastType;

  QList<KDevelop::AbstractType::Ptr> m_topTypes;
};

#endif // TYPEBUILDER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
