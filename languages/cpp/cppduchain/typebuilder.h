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
#include <language/duchain/builders/abstracttypebuilder.h>
#include <language/duchain/declaration.h>
#include "cppduchainexport.h"

namespace KDevelop {
  class FunctionType;
}

class CppClassType;

/**
 * Create types from an AST tree.
 */
class KDEVCPPDUCHAIN_EXPORT TypeBuilder: public ContextBuilder
{
public:
  TypeBuilder(ParseSession* session);

  static uint parseConstVolatile(ParseSession* session, const ListNode<uint>* cv);
  
protected:
  ///Returns either the current context, or the last importend parent-context(needed to find template-argument function return-values)
  KDevelop::DUContext* searchContext() const;

  // Created visitors
  virtual void visitArrayExpression(ExpressionAST*);

  AbstractType::Ptr typeForCurrentDeclaration();
  
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
  virtual void visitPtrToMember(PtrToMemberAST*);
  virtual void visitUsing(UsingAST *);
  virtual void visitParameterDeclaration(ParameterDeclarationAST*);
  virtual void visitTemplateParameter(TemplateParameterAST *);
  virtual void createTypeForDeclarator(DeclaratorAST *node);
  virtual void closeTypeForDeclarator(DeclaratorAST *node);
  virtual void createTypeForInitializer(InitializerAST *node);
  virtual void createTypeForCondition(ConditionAST *node);
  virtual void visitTrailingReturnType(TrailingReturnTypeAST *node);

  /// Returns whether a type was opened
  /// The implementation is quite different from the generic code, so leave this implementation here for now.
  /// @param modifiers If this is nonzero, the returned type will have those modifiers assigned(In AbstractType::modifiers())
  bool openTypeFromName(NameAST* name, uint modifiers = 0, bool needClass = false);

  /// Notify that a class type was opened.
  ///
  /// This is called at the beginning of processing a class-specifier, right after the type for the class was created.
  /// The type can be retrieved through currentAbstractType().
  virtual void classTypeOpened(KDevelop::AbstractType::Ptr) {
  }
  
  inline KDevelop::AbstractType::Ptr lastType() const
  {
    return m_lastType;
  }
  
  inline void setLastType(KDevelop::AbstractType::Ptr ptr)
  {
    m_lastType = ptr;
  }
  
  inline void clearLastType()
  {
    m_lastType = 0;
  }

  /// Sets the given \a type as the current type.
  template <class T2>
  void openType(TypePtr<T2> type)
  { openAbstractType(KDevelop::AbstractType::Ptr::staticCast(type)); }
  
  /// Sets the given \a type as the current type.
  void openAbstractType(KDevelop::AbstractType::Ptr type)
  {
    m_typeStack.append(type);
  }
  
  /// Closes the current type, making it the last type.
  inline void closeType()
  {
    m_lastType = currentAbstractType();
    m_typeStack.pop();
  }

  inline KDevelop::AbstractType::Ptr currentAbstractType() {
    return m_typeStack.empty() ? KDevelop::AbstractType::Ptr() : m_typeStack.top();
  }
  
  template <class T2>
  TypePtr<T2> currentType() {
    return TypePtr<T2>::dynamicCast(currentAbstractType());
  }
  
  bool openTypeFromName(NameAST* name, bool needClass)
  {
    return openTypeFromName(identifierForNode(name), name, needClass);
  }
  
  /**
  * Search for a type with the identifier given by \a name.
  *
  * \param id the identifier of the type for which to search.
  * \param typeNode the AST node representing the type to open.
  * \param needClass if true, only class types will be searched, if false all named types will be searched.
  *
  * \returns whether a type was found (and thus opened).
  */
  bool openTypeFromName(QualifiedIdentifier id, AST* typeNode, bool needClass);
    
  /// Returns whether the last type was referenced as part of an instance
  /// (in contrast to a type-declaration like a class-declaration or a typedef)
  bool lastTypeWasInstance() const;
  void setLastTypeWasInstance(bool wasInstance);

  /// Returns true when last type was an auto-type
  bool lastTypeWasAuto() const;
  void setLastTypeWasAuto(bool wasAuto);

  // Use PushValue<bool> to manipulate this.
  bool m_inTypedef;
  
private:

  //Correctly applies the current modifiers to the given type
  void applyModifiers(AbstractType::Ptr type, uint modifiers);

  void openDelayedType(const KDevelop::IndexedTypeIdentifier& identifier, AST* node, DelayedType::Kind kind);

  KDevelop::FunctionType* openFunction(DeclaratorAST *node);

  void createIntegralTypeForExpression(ExpressionAST *expression);
  AbstractType::Ptr prepareTypeForExpression(AbstractType::Ptr type, quint64 modifiers);

  int m_currentEnumeratorValue;
  bool m_lastTypeWasInstance;
  QStack<KDevelop::AbstractType::Ptr> m_typeStack;
  KDevelop::AbstractType::Ptr m_lastType;
  bool m_lastTypeWasAuto;
};

///Helper-function that extracts the text from start_token until end_token
QString stringFromSessionTokens( ParseSession* session, int start_token, int end_token );

#endif // TYPEBUILDER_H

