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
  virtual void visitPrimaryExpression (PrimaryExpressionAST*) override;
  virtual void visitMemInitializer(MemInitializerAST *) override;

  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) override ;
  virtual void visitExpressionStatement(ExpressionStatementAST *) override ;

  virtual void visitBinaryExpression(BinaryExpressionAST *) override ;
  virtual void visitCastExpression(CastExpressionAST *) override ;
  virtual void visitConditionalExpression(ConditionalExpressionAST *) override ;
  virtual void visitCppCastExpression(CppCastExpressionAST *) override ;
  //virtual void visitDeleteExpression(DeleteExpressionAST *) ;
  //virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) ;
  virtual void visitNewExpression(NewExpressionAST *) override ;
  virtual void visitPostfixExpression(PostfixExpressionAST *) override ;
  virtual void visitSizeofExpression(SizeofExpressionAST *) override ;
  virtual void visitSubscriptExpression(SubscriptExpressionAST *) override ;
  //virtual void visitThrowExpression(ThrowExpressionAST *) ;
  virtual void visitUnaryExpression(UnaryExpressionAST *) override ;
  virtual void visitBaseSpecifier(BaseSpecifierAST *) override;

//   virtual void visitBaseSpecifier(BaseSpecifierAST* node);
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *) override;
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node) override;
  virtual void visitSimpleDeclaration(SimpleDeclarationAST* ) override;  
  virtual void visitCondition(ConditionAST *node) override;
  virtual void visitDeclarator(DeclaratorAST* node) override;
  virtual void visitUsing(UsingAST *) override;
  void visitTypeId(TypeIdAST* type_id) override;
  virtual void visitClassSpecifier(ClassSpecifierAST* node) override;
  virtual void visitUsingDirective(UsingDirectiveAST* node) override;
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node) override;
  virtual void visitTypeIDOperator(TypeIDOperatorAST* node) override;
  virtual void visitQPropertyDeclaration(QPropertyDeclarationAST* ) override;

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

