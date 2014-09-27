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

#include "changevisitor.h"

const ChangeVisitor::visitor_fun_ptr ChangeVisitor::_S_table[AST::NODE_KIND_COUNT] = {
  0,
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitAccessSpecifier),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitAsmDefinition),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitBaseClause),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitBaseSpecifier),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitBinaryExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitCastExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitClassMemberAccess),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitClassSpecifier),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitCompoundStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitCondition),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitConditionalExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitCppCastExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitCtorInitializer),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitDeclarationStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitDeclarator),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitDeleteExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitDoStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitElaboratedTypeSpecifier),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitEnumSpecifier),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitEnumerator),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitExceptionSpecification),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitExpressionOrDeclarationStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitExpressionStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitForStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitFunctionCall),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitFunctionDefinition),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitIfStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitIncrDecrExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitInitDeclarator),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitInitializer),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitInitializerClause),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitLabeledStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitLinkageBody),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitLinkageSpecification),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitMemInitializer),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitName),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitNamespace),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitNamespaceAliasDefinition),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitNewDeclarator),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitNewExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitNewInitializer),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitNewTypeId),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitOperator),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitOperatorFunctionId),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitParameterDeclaration),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitParameterDeclarationClause),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitPostfixExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitPrimaryExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitPtrOperator),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitPtrToMember),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitReturnStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitSimpleDeclaration),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitSimpleTypeSpecifier),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitSizeofExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitStringLiteral),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitSubscriptExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitSwitchStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTemplateArgument),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTemplateDeclaration),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTemplateParameter),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitThrowExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTranslationUnit),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTryBlockStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitCatchStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTypeId),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTypeIdentification),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTypeParameter),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitTypedef),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitUnaryExpression),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitUnqualifiedName),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitUsing),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitUsingDirective),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitWhileStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitWinDeclSpec),
  0,
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitJumpStatement),
  reinterpret_cast<ChangeVisitor::visitor_fun_ptr>(&ChangeVisitor::visitSignalSlotExpression)
};

ChangeVisitor::ChangeVisitor()
{
}

ChangeVisitor::~ChangeVisitor()
{
}

void ChangeVisitor::visit(Change* change, AST *node)
{
  if (node)
    (this->*_S_table[node->kind])(change, node);
}

