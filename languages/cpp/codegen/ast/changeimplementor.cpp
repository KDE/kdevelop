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

#include "changeimplementor.h"
#include "astchange.h"
#include "tokens.h"
#include <parsesession.h>
#include <lexer.h>

void ChangeImplementor::replaceToken(std::size_t token, int newToken)
{
  const Token& t = m_session->token_stream->token( token );
  rpp::Anchor a = m_session->positionAt( token );
  KDevelop::RangeInRevision tokenRange(a, t.size);

  m_changes.addChange(KDevelop::DocumentChangePointer(new KDevelop::DocumentChange(m_url, tokenRange.castToSimpleRange(), t.symbolString(), token_text( newToken ))));
}

void ChangeImplementor::visitAccessSpecifier(AccessSpecifierChange* change, AccessSpecifierAST* node)
{
  ChangeVisitor::visitAccessSpecifier(change, node);
}

void ChangeImplementor::visitAsmDefinition(AsmDefinitionChange* change, AsmDefinitionAST* node)
{
  ChangeVisitor::visitAsmDefinition(change, node);
}

void ChangeImplementor::visitBaseClause(BaseClauseChange* change, BaseClauseAST* node)
{
  ChangeVisitor::visitBaseClause(change, node);
}

void ChangeImplementor::visitBaseSpecifier(BaseSpecifierChange* change, BaseSpecifierAST* node)
{
  replaceToken(node->access_specifier, change->accessSpecifier());
  replaceToken(node->virt, change->isVirtual() ? Token_virtual : Token_none);

  ChangeVisitor::visitBaseSpecifier(change, node);
}

void ChangeImplementor::visitBinaryExpression(BinaryExpressionChange* change, BinaryExpressionAST* node)
{
  ChangeVisitor::visitBinaryExpression(change, node);
}

void ChangeImplementor::visitCastExpression(CastExpressionChange* change, CastExpressionAST* node)
{
  ChangeVisitor::visitCastExpression(change, node);
}

void ChangeImplementor::visitClassMemberAccess(ClassMemberAccessChange* change, ClassMemberAccessAST* node)
{
  ChangeVisitor::visitClassMemberAccess(change, node);
}

void ChangeImplementor::visitClassSpecifier(ClassSpecifierChange* change, ClassSpecifierAST* node)
{
  ChangeVisitor::visitClassSpecifier(change, node);
}

void ChangeImplementor::visitCompoundStatement(CompoundStatementChange* change, CompoundStatementAST* node)
{
  ChangeVisitor::visitCompoundStatement(change, node);
}

void ChangeImplementor::visitCondition(ConditionChange* change, ConditionAST* node)
{
  ChangeVisitor::visitCondition(change, node);
}

void ChangeImplementor::visitConditionalExpression(ConditionalExpressionChange* change, ConditionalExpressionAST* node)
{
  ChangeVisitor::visitConditionalExpression(change, node);
}

void ChangeImplementor::visitCppCastExpression(CppCastExpressionChange* change, CppCastExpressionAST* node)
{
  ChangeVisitor::visitCppCastExpression(change, node);
}

void ChangeImplementor::visitCtorInitializer(CtorInitializerChange* change, CtorInitializerAST* node)
{
  ChangeVisitor::visitCtorInitializer(change, node);
}

void ChangeImplementor::visitDeclarationStatement(DeclarationStatementChange* change, DeclarationStatementAST* node)
{
  ChangeVisitor::visitDeclarationStatement(change, node);
}

void ChangeImplementor::visitDeclarator(DeclaratorChange* change, DeclaratorAST* node)
{
  ChangeVisitor::visitDeclarator(change, node);
}

void ChangeImplementor::visitDeleteExpression(DeleteExpressionChange* change, DeleteExpressionAST* node)
{
  ChangeVisitor::visitDeleteExpression(change, node);
}

void ChangeImplementor::visitDoStatement(DoStatementChange* change, DoStatementAST* node)
{
  ChangeVisitor::visitDoStatement(change, node);
}

void ChangeImplementor::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierChange* change, ElaboratedTypeSpecifierAST* node)
{
  ChangeVisitor::visitElaboratedTypeSpecifier(change, node);
}

void ChangeImplementor::visitEnumSpecifier(EnumSpecifierChange* change, EnumSpecifierAST* node)
{
  ChangeVisitor::visitEnumSpecifier(change, node);
}

void ChangeImplementor::visitEnumerator(EnumeratorChange* change, EnumeratorAST* node)
{
  ChangeVisitor::visitEnumerator(change, node);
}

void ChangeImplementor::visitExceptionSpecification(ExceptionSpecificationChange* change, ExceptionSpecificationAST* node)
{
  ChangeVisitor::visitExceptionSpecification(change, node);
}

void ChangeImplementor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementChange* change, ExpressionOrDeclarationStatementAST* node)
{
  ChangeVisitor::visitExpressionOrDeclarationStatement(change, node);
}

void ChangeImplementor::visitExpressionStatement(ExpressionStatementChange* change, ExpressionStatementAST* node)
{
  ChangeVisitor::visitExpressionStatement(change, node);
}

void ChangeImplementor::visitForStatement(ForStatementChange* change, ForStatementAST* node)
{
  ChangeVisitor::visitForStatement(change, node);
}

void ChangeImplementor::visitFunctionCall(FunctionCallChange* change, FunctionCallAST* node)
{
  ChangeVisitor::visitFunctionCall(change, node);
}

void ChangeImplementor::visitFunctionDefinition(FunctionDefinitionChange* change, FunctionDefinitionAST* node)
{
  ChangeVisitor::visitFunctionDefinition(change, node);
}

void ChangeImplementor::visitIfStatement(IfStatementChange* change, IfStatementAST* node)
{
  ChangeVisitor::visitIfStatement(change, node);
}

void ChangeImplementor::visitIncrDecrExpression(IncrDecrExpressionChange* change, IncrDecrExpressionAST* node)
{
  ChangeVisitor::visitIncrDecrExpression(change, node);
}

void ChangeImplementor::visitInitDeclarator(InitDeclaratorChange* change, InitDeclaratorAST* node)
{
  ChangeVisitor::visitInitDeclarator(change, node);
}

void ChangeImplementor::visitInitializer(InitializerChange* change, InitializerAST* node)
{
  ChangeVisitor::visitInitializer(change, node);
}

void ChangeImplementor::visitInitializerClause(InitializerClauseChange* change, InitializerClauseAST* node)
{
  ChangeVisitor::visitInitializerClause(change, node);
}

void ChangeImplementor::visitJumpStatement(JumpStatementChange* change, JumpStatementAST* node)
{
  ChangeVisitor::visitJumpStatement(change, node);
}

void ChangeImplementor::visitLabeledStatement(LabeledStatementChange* change, LabeledStatementAST* node)
{
  ChangeVisitor::visitLabeledStatement(change, node);
}

void ChangeImplementor::visitLinkageBody(LinkageBodyChange* change, LinkageBodyAST* node)
{
  ChangeVisitor::visitLinkageBody(change, node);
}

void ChangeImplementor::visitLinkageSpecification(LinkageSpecificationChange* change, LinkageSpecificationAST* node)
{
  ChangeVisitor::visitLinkageSpecification(change, node);
}

void ChangeImplementor::visitMemInitializer(MemInitializerChange* change, MemInitializerAST* node)
{
  ChangeVisitor::visitMemInitializer(change, node);
}

void ChangeImplementor::visitName(NameChange* change, NameAST* node)
{
  ChangeVisitor::visitName(change, node);
}

void ChangeImplementor::visitNamespace(NamespaceChange* change, NamespaceAST* node)
{
  ChangeVisitor::visitNamespace(change, node);
}

void ChangeImplementor::visitNamespaceAliasDefinition(NamespaceAliasDefinitionChange* change, NamespaceAliasDefinitionAST* node)
{
  ChangeVisitor::visitNamespaceAliasDefinition(change, node);
}

void ChangeImplementor::visitNewDeclarator(NewDeclaratorChange* change, NewDeclaratorAST* node)
{
  ChangeVisitor::visitNewDeclarator(change, node);
}

void ChangeImplementor::visitNewExpression(NewExpressionChange* change, NewExpressionAST* node)
{
  ChangeVisitor::visitNewExpression(change, node);
}

void ChangeImplementor::visitNewInitializer(NewInitializerChange* change, NewInitializerAST* node)
{
  ChangeVisitor::visitNewInitializer(change, node);
}

void ChangeImplementor::visitNewTypeId(NewTypeIdChange* change, NewTypeIdAST* node)
{
  ChangeVisitor::visitNewTypeId(change, node);
}

void ChangeImplementor::visitOperator(OperatorChange* change, OperatorAST* node)
{
  ChangeVisitor::visitOperator(change, node);
}

void ChangeImplementor::visitOperatorFunctionId(OperatorFunctionIdChange* change, OperatorFunctionIdAST* node)
{
  ChangeVisitor::visitOperatorFunctionId(change, node);
}

void ChangeImplementor::visitParameterDeclaration(ParameterDeclarationChange* change, ParameterDeclarationAST* node)
{
  ChangeVisitor::visitParameterDeclaration(change, node);
}

void ChangeImplementor::visitParameterDeclarationClause(ParameterDeclarationClauseChange* change, ParameterDeclarationClauseAST* node)
{
  ChangeVisitor::visitParameterDeclarationClause(change, node);
}

void ChangeImplementor::visitPostfixExpression(PostfixExpressionChange* change, PostfixExpressionAST* node)
{
  ChangeVisitor::visitPostfixExpression(change, node);
}

void ChangeImplementor::visitPrimaryExpression(PrimaryExpressionChange* change, PrimaryExpressionAST* node)
{
  ChangeVisitor::visitPrimaryExpression(change, node);
}

void ChangeImplementor::visitPtrOperator(PtrOperatorChange* change, PtrOperatorAST* node)
{
  ChangeVisitor::visitPtrOperator(change, node);
}

void ChangeImplementor::visitPtrToMember(PtrToMemberChange* change, PtrToMemberAST* node)
{
  ChangeVisitor::visitPtrToMember(change, node);
}

void ChangeImplementor::visitReturnStatement(ReturnStatementChange* change, ReturnStatementAST* node)
{
  ChangeVisitor::visitReturnStatement(change, node);
}

void ChangeImplementor::visitSimpleDeclaration(SimpleDeclarationChange* change, SimpleDeclarationAST* node)
{
  ChangeVisitor::visitSimpleDeclaration(change, node);
}

void ChangeImplementor::visitSimpleTypeSpecifier(SimpleTypeSpecifierChange* change, SimpleTypeSpecifierAST* node)
{
  ChangeVisitor::visitSimpleTypeSpecifier(change, node);
}

void ChangeImplementor::visitSizeofExpression(SizeofExpressionChange* change, SizeofExpressionAST* node)
{
  ChangeVisitor::visitSizeofExpression(change, node);
}

void ChangeImplementor::visitStringLiteral(StringLiteralChange* change, StringLiteralAST* node)
{
  ChangeVisitor::visitStringLiteral(change, node);
}

void ChangeImplementor::visitSubscriptExpression(SubscriptExpressionChange* change, SubscriptExpressionAST* node)
{
  ChangeVisitor::visitSubscriptExpression(change, node);
}

void ChangeImplementor::visitSwitchStatement(SwitchStatementChange* change, SwitchStatementAST* node)
{
  ChangeVisitor::visitSwitchStatement(change, node);
}

void ChangeImplementor::visitTemplateArgument(TemplateArgumentChange* change, TemplateArgumentAST* node)
{
  ChangeVisitor::visitTemplateArgument(change, node);
}

void ChangeImplementor::visitTemplateDeclaration(TemplateDeclarationChange* change, TemplateDeclarationAST* node)
{
  ChangeVisitor::visitTemplateDeclaration(change, node);
}

void ChangeImplementor::visitTemplateParameter(TemplateParameterChange* change, TemplateParameterAST* node)
{
  ChangeVisitor::visitTemplateParameter(change, node);
}

void ChangeImplementor::visitThrowExpression(ThrowExpressionChange* change, ThrowExpressionAST* node)
{
  ChangeVisitor::visitThrowExpression(change, node);
}

void ChangeImplementor::visitTranslationUnit(TranslationUnitChange* change, TranslationUnitAST* node)
{
  ChangeVisitor::visitTranslationUnit(change, node);
}

void ChangeImplementor::visitTryBlockStatement(TryBlockStatementChange* change, TryBlockStatementAST* node)
{
  ChangeVisitor::visitTryBlockStatement(change, node);
}

void ChangeImplementor::visitCatchStatement(CatchStatementChange* change, CatchStatementAST* node)
{
  ChangeVisitor::visitCatchStatement(change, node);
}

void ChangeImplementor::visitTypeId(TypeIdChange* change, TypeIdAST* node)
{
  ChangeVisitor::visitTypeId(change, node);
}

void ChangeImplementor::visitTypeIdentification(TypeIdentificationChange* change, TypeIdentificationAST* node)
{
  ChangeVisitor::visitTypeIdentification(change, node);
}

void ChangeImplementor::visitTypeParameter(TypeParameterChange* change, TypeParameterAST* node)
{
  ChangeVisitor::visitTypeParameter(change, node);
}

void ChangeImplementor::visitTypedef(TypedefChange* change, TypedefAST* node)
{
  ChangeVisitor::visitTypedef(change, node);
}

void ChangeImplementor::visitUnaryExpression(UnaryExpressionChange* change, UnaryExpressionAST* node)
{
  ChangeVisitor::visitUnaryExpression(change, node);
}

void ChangeImplementor::visitUnqualifiedName(UnqualifiedNameChange* change, UnqualifiedNameAST* node)
{
  ChangeVisitor::visitUnqualifiedName(change, node);
}

void ChangeImplementor::visitUsing(UsingChange* change, UsingAST* node)
{
  ChangeVisitor::visitUsing(change, node);
}

void ChangeImplementor::visitUsingDirective(UsingDirectiveChange* change, UsingDirectiveAST* node)
{
  ChangeVisitor::visitUsingDirective(change, node);
}

void ChangeImplementor::visitWhileStatement(WhileStatementChange* change, WhileStatementAST* node)
{
  ChangeVisitor::visitWhileStatement(change, node);
}

void ChangeImplementor::visitWinDeclSpec(WinDeclSpecChange* change, WinDeclSpecAST* node)
{
  ChangeVisitor::visitWinDeclSpec(change, node);
}

void ChangeImplementor::visitSignalSlotExpression(SignalSlotExpressionChange* change, SignalSlotExpressionAST* node)
{
  ChangeVisitor::visitSignalSlotExpression(change, node);
}
