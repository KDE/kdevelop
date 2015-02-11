/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef DEFAULT_VISITOR_H
#define DEFAULT_VISITOR_H

#include "visitor.h"
#include <cppparserexport.h>

class KDEVCPPPARSER_EXPORT DefaultVisitor: public Visitor
{
public:
  DefaultVisitor() {}

protected:
  virtual void visitAccessSpecifier(AccessSpecifierAST *) override;
  virtual void visitAsmDefinition(AsmDefinitionAST *) override;
  virtual void visitBaseClause(BaseClauseAST *) override;
  virtual void visitBaseSpecifier(BaseSpecifierAST *) override;
  virtual void visitBinaryExpression(BinaryExpressionAST *) override;
  virtual void visitBracedInitList(BracedInitListAST *) override;
  virtual void visitCastExpression(CastExpressionAST *) override;
  virtual void visitClassMemberAccess(ClassMemberAccessAST *) override;
  virtual void visitClassSpecifier(ClassSpecifierAST *) override;
  virtual void visitCompoundStatement(CompoundStatementAST *) override;
  virtual void visitCondition(ConditionAST *) override;
  virtual void visitConditionalExpression(ConditionalExpressionAST *) override;
  virtual void visitCppCastExpression(CppCastExpressionAST *) override;
  virtual void visitCtorInitializer(CtorInitializerAST *) override;
  virtual void visitDeclarationStatement(DeclarationStatementAST *) override;
  virtual void visitDeclarator(DeclaratorAST *) override;
  virtual void visitDeleteExpression(DeleteExpressionAST *) override;
  virtual void visitDoStatement(DoStatementAST *) override;
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *) override;
  virtual void visitEnumSpecifier(EnumSpecifierAST *) override;
  virtual void visitEnumerator(EnumeratorAST *) override;
  virtual void visitExceptionSpecification(ExceptionSpecificationAST *) override;
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *) override;
  virtual void visitExpressionStatement(ExpressionStatementAST *) override;
  virtual void visitForStatement(ForStatementAST *) override;
  virtual void visitForRangeDeclaration(ForRangeDeclarationAst* ) override;
  virtual void visitFunctionCall(FunctionCallAST *) override;
  virtual void visitFunctionDefinition(FunctionDefinitionAST *) override;
  virtual void visitIfStatement(IfStatementAST *) override;
  virtual void visitIncrDecrExpression(IncrDecrExpressionAST *) override;
  virtual void visitInitDeclarator(InitDeclaratorAST *) override;
  virtual void visitInitializer(InitializerAST *) override;
  virtual void visitInitializerClause(InitializerClauseAST *) override;
  virtual void visitInitializerList(InitializerListAST *) override;
  virtual void visitJumpStatement(JumpStatementAST *) override;
  virtual void visitLabeledStatement(LabeledStatementAST *) override;
  virtual void visitLinkageBody(LinkageBodyAST *) override;
  virtual void visitLinkageSpecification(LinkageSpecificationAST *) override;
  virtual void visitMemInitializer(MemInitializerAST *) override;
  virtual void visitName(NameAST *) override;
  virtual void visitNamespace(NamespaceAST *) override;
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST *) override;
  virtual void visitNewDeclarator(NewDeclaratorAST *) override;
  virtual void visitNewExpression(NewExpressionAST *) override;
  virtual void visitNewInitializer(NewInitializerAST *) override;
  virtual void visitNewTypeId(NewTypeIdAST *) override;
  virtual void visitOperator(OperatorAST *) override;
  virtual void visitOperatorFunctionId(OperatorFunctionIdAST *) override;
  virtual void visitParameterDeclaration(ParameterDeclarationAST *) override;
  virtual void visitParameterDeclarationClause(ParameterDeclarationClauseAST *) override;
  virtual void visitPostfixExpression(PostfixExpressionAST *) override;
  virtual void visitPrimaryExpression(PrimaryExpressionAST *) override;
  virtual void visitPtrOperator(PtrOperatorAST *) override;
  virtual void visitPtrToMember(PtrToMemberAST *) override;
  virtual void visitReturnStatement(ReturnStatementAST *) override;
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *) override;
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *) override;
  virtual void visitSizeofExpression(SizeofExpressionAST *) override;
  virtual void visitStringLiteral(StringLiteralAST *) override;
  virtual void visitSubscriptExpression(SubscriptExpressionAST *) override;
  virtual void visitSwitchStatement(SwitchStatementAST *) override;
  virtual void visitTemplateArgument(TemplateArgumentAST *) override;
  virtual void visitTemplateDeclaration(TemplateDeclarationAST *) override;
  virtual void visitTemplateParameter(TemplateParameterAST *) override;
  virtual void visitThrowExpression(ThrowExpressionAST *) override;
  virtual void visitTranslationUnit(TranslationUnitAST *) override;
  virtual void visitTryBlockStatement(TryBlockStatementAST *) override;
  virtual void visitCatchStatement(CatchStatementAST *) override;
  virtual void visitTypeId(TypeIdAST *) override;
  virtual void visitTypeIdentification(TypeIdentificationAST *) override;
  virtual void visitTypeParameter(TypeParameterAST *) override;
  virtual void visitTypedef(TypedefAST *) override;
  virtual void visitUnaryExpression(UnaryExpressionAST *) override;
  virtual void visitUnqualifiedName(UnqualifiedNameAST *) override;
  virtual void visitUsing(UsingAST *) override;
  virtual void visitUsingDirective(UsingDirectiveAST *) override;
  virtual void visitWhileStatement(WhileStatementAST *) override;
  virtual void visitWinDeclSpec(WinDeclSpecAST *) override;
  virtual void visitSignalSlotExpression(SignalSlotExpressionAST *) override;
  virtual void visitQPropertyDeclaration(QPropertyDeclarationAST *) override;
  virtual void visitTypeIDOperator(TypeIDOperatorAST *) override;
  virtual void visitStaticAssert(StaticAssertAST *) override;
  virtual void visitTrailingReturnType(TrailingReturnTypeAST *) override;
  virtual void visitLambdaExpression(LambdaExpressionAST* ) override;
  virtual void visitLambdaCapture(LambdaCaptureAST* ) override;
  virtual void visitLambdaDeclarator(LambdaDeclaratorAST* ) override;
  virtual void visitAliasDeclaration(AliasDeclarationAST* ) override;
};

#endif // VISITOR_H

