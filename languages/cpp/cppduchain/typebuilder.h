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

typedef KDevelop::AbstractTypeBuilder<AST, NameAST, ContextBuilder> TypeBuilderBase;

/**
 * Create types from an AST tree.
 */
class KDEVCPPDUCHAIN_EXPORT TypeBuilder: public TypeBuilderBase
{
public:
  TypeBuilder();

  static uint parseConstVolatile(ParseSession* session, const ListNode<uint>* cv);
  
protected:
  ///Returns either the current context, or the last importend parent-context(needed to find template-argument function return-values)
  virtual KDevelop::DUContext* searchContext() const;

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
  virtual void visitUsing(UsingAST *);
  virtual void visitParameterDeclaration(ParameterDeclarationAST*);
  virtual void visitTemplateParameter(TemplateParameterAST *);
  virtual void createTypeForDeclarator(DeclaratorAST *node);
  virtual void closeTypeForDeclarator(DeclaratorAST *node);
  virtual void createTypeForInitializer(InitializerAST *node);
  virtual void closeTypeForInitializer(InitializerAST *node);

  ///Returns whether a type was opened
  /// The implementation is quite different from the generic code, so leave this implementation here for now.
  ///@param modifiers If this is nonzero, the returned type will have those modifiers assigned(In AbstractType::modifiers())
  bool openTypeFromName(NameAST* name, uint modifiers = 0, bool needClass = false);

  bool lastTypeWasInstance() const;

  //Correctly applies the current modifiers to the given type
  void applyModifiers(AbstractType::Ptr type, uint modifiers);
  
  bool m_inTypedef;
  
  private:
  void openDelayedType(const KDevelop::IndexedTypeIdentifier& identifier, AST* node, DelayedType::Kind kind);


  KDevelop::FunctionType* openFunction(DeclaratorAST *node);

  int m_currentEnumeratorValue;

  bool m_lastTypeWasInstance;
};

///Helper-function that extracts the text from start_token until end_token
QString stringFromSessionTokens( ParseSession* session, int start_token, int end_token );

#endif // TYPEBUILDER_H

