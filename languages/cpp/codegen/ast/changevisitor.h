/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#ifndef CHANGEVISITOR_H
#define CHANGEVISITOR_H

#include "astchange.h"

class ChangeVisitor
{
public:
  ChangeVisitor();
  virtual ~ChangeVisitor();

  virtual void visit(Change* change, AST *node);

protected:
  virtual void visitAccessSpecifier(AccessSpecifierChange*, AccessSpecifierAST *) {}
  virtual void visitAsmDefinition(AsmDefinitionChange*, AsmDefinitionAST *) {}
  virtual void visitBaseClause(BaseClauseChange*, BaseClauseAST *) {}
  virtual void visitBaseSpecifier(BaseSpecifierChange*, BaseSpecifierAST *) {}
  virtual void visitBinaryExpression(BinaryExpressionChange*, BinaryExpressionAST *) {}
  virtual void visitCastExpression(CastExpressionChange*, CastExpressionAST *) {}
  virtual void visitClassMemberAccess(ClassMemberAccessChange*, ClassMemberAccessAST *) {}
  virtual void visitClassSpecifier(ClassSpecifierChange*, ClassSpecifierAST *) {}
  virtual void visitCompoundStatement(CompoundStatementChange*, CompoundStatementAST *) {}
  virtual void visitCondition(ConditionChange*, ConditionAST *) {}
  virtual void visitConditionalExpression(ConditionalExpressionChange*, ConditionalExpressionAST *) {}
  virtual void visitCppCastExpression(CppCastExpressionChange*, CppCastExpressionAST *) {}
  virtual void visitCtorInitializer(CtorInitializerChange*, CtorInitializerAST *) {}
  virtual void visitDeclarationStatement(DeclarationStatementChange*, DeclarationStatementAST *) {}
  virtual void visitDeclarator(DeclaratorChange*, DeclaratorAST *) {}
  virtual void visitDeleteExpression(DeleteExpressionChange*, DeleteExpressionAST *) {}
  virtual void visitDoStatement(DoStatementChange*, DoStatementAST *) {}
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierChange*, ElaboratedTypeSpecifierAST *) {}
  virtual void visitEnumSpecifier(EnumSpecifierChange*, EnumSpecifierAST *) {}
  virtual void visitEnumerator(EnumeratorChange*, EnumeratorAST *) {}
  virtual void visitExceptionSpecification(ExceptionSpecificationChange*, ExceptionSpecificationAST *) {}
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementChange*, ExpressionOrDeclarationStatementAST *) {}
  virtual void visitExpressionStatement(ExpressionStatementChange*, ExpressionStatementAST *) {}
  virtual void visitForStatement(ForStatementChange*, ForStatementAST *) {}
  virtual void visitFunctionCall(FunctionCallChange*, FunctionCallAST *) {}
  virtual void visitFunctionDefinition(FunctionDefinitionChange*, FunctionDefinitionAST *) {}
  virtual void visitIfStatement(IfStatementChange*, IfStatementAST *) {}
  virtual void visitIncrDecrExpression(IncrDecrExpressionChange*, IncrDecrExpressionAST *) {}
  virtual void visitInitDeclarator(InitDeclaratorChange*, InitDeclaratorAST *) {}
  virtual void visitInitializer(InitializerChange*, InitializerAST *) {}
  virtual void visitInitializerClause(InitializerClauseChange*, InitializerClauseAST *) {}
  virtual void visitJumpStatement(JumpStatementChange*, JumpStatementAST *) {}
  virtual void visitLabeledStatement(LabeledStatementChange*, LabeledStatementAST *) {}
  virtual void visitLinkageBody(LinkageBodyChange*, LinkageBodyAST *) {}
  virtual void visitLinkageSpecification(LinkageSpecificationChange*, LinkageSpecificationAST *) {}
  virtual void visitMemInitializer(MemInitializerChange*, MemInitializerAST *) {}
  virtual void visitName(NameChange*, NameAST *) {}
  virtual void visitNamespace(NamespaceChange*, NamespaceAST *) {}
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionChange*, NamespaceAliasDefinitionAST *) {}
  virtual void visitNewDeclarator(NewDeclaratorChange*, NewDeclaratorAST *) {}
  virtual void visitNewExpression(NewExpressionChange*, NewExpressionAST *) {}
  virtual void visitNewInitializer(NewInitializerChange*, NewInitializerAST *) {}
  virtual void visitNewTypeId(NewTypeIdChange*, NewTypeIdAST *) {}
  virtual void visitOperator(OperatorChange*, OperatorAST *) {}
  virtual void visitOperatorFunctionId(OperatorFunctionIdChange*, OperatorFunctionIdAST *) {}
  virtual void visitParameterDeclaration(ParameterDeclarationChange*, ParameterDeclarationAST *) {}
  virtual void visitParameterDeclarationClause(ParameterDeclarationClauseChange*, ParameterDeclarationClauseAST *) {}
  virtual void visitPostfixExpression(PostfixExpressionChange*, PostfixExpressionAST *) {}
  virtual void visitPrimaryExpression(PrimaryExpressionChange*, PrimaryExpressionAST *) {}
  virtual void visitPtrOperator(PtrOperatorChange*, PtrOperatorAST *) {}
  virtual void visitPtrToMember(PtrToMemberChange*, PtrToMemberAST *) {}
  virtual void visitReturnStatement(ReturnStatementChange*, ReturnStatementAST *) {}
  virtual void visitSimpleDeclaration(SimpleDeclarationChange*, SimpleDeclarationAST *) {}
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierChange*, SimpleTypeSpecifierAST *) {}
  virtual void visitSizeofExpression(SizeofExpressionChange*, SizeofExpressionAST *) {}
  virtual void visitStringLiteral(StringLiteralChange*, StringLiteralAST *) {}
  virtual void visitSubscriptExpression(SubscriptExpressionChange*, SubscriptExpressionAST *) {}
  virtual void visitSwitchStatement(SwitchStatementChange*, SwitchStatementAST *) {}
  virtual void visitTemplateArgument(TemplateArgumentChange*, TemplateArgumentAST *) {}
  virtual void visitTemplateDeclaration(TemplateDeclarationChange*, TemplateDeclarationAST *) {}
  virtual void visitTemplateParameter(TemplateParameterChange*, TemplateParameterAST *) {}
  virtual void visitThrowExpression(ThrowExpressionChange*, ThrowExpressionAST *) {}
  virtual void visitTranslationUnit(TranslationUnitChange*, TranslationUnitAST *) {}
  virtual void visitTryBlockStatement(TryBlockStatementChange*, TryBlockStatementAST *) {}
  virtual void visitCatchStatement(CatchStatementChange*, CatchStatementAST *) {}
  virtual void visitTypeId(TypeIdChange*, TypeIdAST *) {}
  virtual void visitTypeIdentification(TypeIdentificationChange*, TypeIdentificationAST *) {}
  virtual void visitTypeParameter(TypeParameterChange*, TypeParameterAST *) {}
  virtual void visitTypedef(TypedefChange*, TypedefAST *) {}
  virtual void visitUnaryExpression(UnaryExpressionChange*, UnaryExpressionAST *) {}
  virtual void visitUnqualifiedName(UnqualifiedNameChange*, UnqualifiedNameAST *) {}
  virtual void visitUsing(UsingChange*, UsingAST *) {}
  virtual void visitUsingDirective(UsingDirectiveChange*, UsingDirectiveAST *) {}
  virtual void visitWhileStatement(WhileStatementChange*, WhileStatementAST *) {}
  virtual void visitWinDeclSpec(WinDeclSpecChange*, WinDeclSpecAST *) {}
  virtual void visitSignalSlotExpression(SignalSlotExpressionChange*, SignalSlotExpressionAST *) {}

private:
  typedef void (ChangeVisitor::*visitor_fun_ptr)(Change*, AST *);
  static const visitor_fun_ptr _S_table[];
};

#endif // CHANGEVISITOR_H
