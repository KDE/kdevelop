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

typedef ContextBuilder LangugageSpecificTypeBuilderBase;

#include <language/duchain/abstracttypebuilder.h>
#include <typesystem.h>
#include <declaration.h>
#include "cppduchainexport.h"

class CppClassType;
class CppFunctionType;

typedef KDevelop::AbstractTypeBuilder<AST, NameAST> TypeBuilderBase;

/**
 * Create types from an AST tree.
 *
 * \note This builder overrides visitDeclarator, in order to support
 * array types; parent classes will not have
 * their visitDeclarator function called.
 */
class KDEVCPPDUCHAIN_EXPORT TypeBuilder: public TypeBuilderBase
{
public:
  TypeBuilder();

protected:
  virtual KDevelop::ITypeRepository* typeRepository() const;
  ///Returns either the current context, or the last importend parent-context(needed to find template-argument function return-values)
  virtual KDevelop::DUContext* searchContext() const;
  
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
  virtual void visitParameterDeclaration(ParameterDeclarationAST*);
  virtual void visitTemplateParameter(TemplateParameterAST *);
  virtual void createTypeForDeclarator(DeclaratorAST *node);
  virtual void closeTypeForDeclarator(DeclaratorAST *node);

  virtual void addBaseType( CppClassType::BaseClassInstance base );

  bool m_declarationHasInitDeclarators; //Is set when processing the type-specifiers within SimpleDeclarationASTs, to change the behavior for elaborated type-specifiers.

  ///Returns whether a type was opened
  /// The implementation is quite different from the generic code, so leave this implementation here for now.
  bool openTypeFromName(NameAST* name, bool needClass = false);

  bool lastTypeWasInstance() const;
  
  private:
  void openDelayedType(const KDevelop::QualifiedIdentifier& identifier, AST* node, DelayedType::Kind kind);
  

  CppClassType* openClass(int kind);
  CppFunctionType* openFunction(DeclaratorAST *node);

  KDevelop::Declaration::CVSpecs parseConstVolatile(const ListNode<std::size_t>* cv);

  int m_currentEnumeratorValue;

  bool m_lastTypeWasInstance;
};

///Helper-function that extracts the text from start_token until end_token
QString stringFromSessionTokens( ParseSession* session, int start_token, int end_token );

#endif // TYPEBUILDER_H

