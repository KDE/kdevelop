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

#ifndef USEBUILDER_H
#define USEBUILDER_H

#include "contextbuilder.h"
#include "cppduchainexport.h"
#include <language/duchain/builders/abstractusebuilder.h>

class ParseSession;

class UseBuilderBase : public KDevelop::AbstractUseBuilder<AST, NameAST, ContextBuilder> {
  public:
  UseBuilderBase(ParseSession* session) : KDevelop::AbstractUseBuilder<AST, NameAST, ContextBuilder>(session) {
  }
};

/**
 * A class which iterates the AST to extract uses of definitions.
 */
class KDEVCPPDUCHAIN_EXPORT  UseBuilder: public UseBuilderBase
{
public:
  UseBuilder(ParseSession* session);
  virtual ~UseBuilder();

  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   */
  void buildUses(AST *node);

  QList<QExplicitlySharedDataPointer<KDevelop::Problem> > problems() const;

  void addProblem(QExplicitlySharedDataPointer<KDevelop::Problem> problem);
  
  using UseBuilderBase::newUse;

protected:
  virtual void visitPrimaryExpression (PrimaryExpressionAST*);
  virtual void visitMemInitializer(MemInitializerAST *);

  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) ;
  virtual void visitExpressionStatement(ExpressionStatementAST *) ;

  virtual void visitBinaryExpression(BinaryExpressionAST *) ;
  virtual void visitCastExpression(CastExpressionAST *) ;
  virtual void visitConditionalExpression(ConditionalExpressionAST *) ;
  virtual void visitCppCastExpression(CppCastExpressionAST *) ;
  //virtual void visitDeleteExpression(DeleteExpressionAST *) ;
  //virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) ;
  virtual void visitNewExpression(NewExpressionAST *) ;
  virtual void visitPostfixExpression(PostfixExpressionAST *) ;
  virtual void visitSizeofExpression(SizeofExpressionAST *) ;
  virtual void visitSubscriptExpression(SubscriptExpressionAST *) ;
  //virtual void visitThrowExpression(ThrowExpressionAST *) ;
  virtual void visitUnaryExpression(UnaryExpressionAST *) ;
  virtual void visitBaseSpecifier(BaseSpecifierAST *);

//   virtual void visitBaseSpecifier(BaseSpecifierAST* node);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *);
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST* );  
  virtual void visitCondition(ConditionAST *node);
  virtual void visitDeclarator(DeclaratorAST* node);
  virtual void visitUsing(UsingAST *);
  void visitTypeId(TypeIdAST* type_id);
  virtual void visitClassSpecifier(ClassSpecifierAST* node);
  virtual void visitUsingDirective(UsingDirectiveAST* node);
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node);
  virtual void visitTypeIDOperator(TypeIDOperatorAST* node);
  virtual void visitQPropertyDeclaration(QPropertyDeclarationAST* );

private:
  void buildUsesForName(NameAST* name);

  void visitExpression(AST* node);

  inline int& nextUseIndex() { return m_nextUseStack.top(); }
  inline QVector<int>& skippedUses() { return m_skippedUses.top(); }
  QStack<int> m_nextUseStack;
  QStack<QVector<int> > m_skippedUses;
  QStack<DUContext*> m_contexts;

  QList< QExplicitlySharedDataPointer< KDevelop::Problem > > m_problems;
};

#endif // USEBUILDER_H

