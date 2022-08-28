/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakeastdefaultvisitor.h"
#include "ast.h"

namespace QMake {

ASTDefaultVisitor::~ASTDefaultVisitor()
{
}

void ASTDefaultVisitor::visitProject(ProjectAST* node)
{
    for (StatementAST* stmt : std::as_const(node->statements)) {
        visitNode(stmt);
    }
}

void ASTDefaultVisitor::visitAssignment(AssignmentAST* node)
{
    visitNode(node->identifier);
    visitNode(node->op);
    for (ValueAST* value : std::as_const(node->values)) {
        visitNode(value);
    }
}

void ASTDefaultVisitor::visitValue(ValueAST*)
{
}

void ASTDefaultVisitor::visitFunctionCall(FunctionCallAST* node)
{
    visitNode(node->identifier);
    for (ValueAST* value : std::as_const(node->args)) {
        visitNode(value);
    }
    visitNode(node->body);
}

void ASTDefaultVisitor::visitScopeBody(ScopeBodyAST* node)
{
    for (StatementAST* stmt : std::as_const(node->ifStatements)) {
        visitNode(stmt);
    }
    for (StatementAST* stmt : std::as_const(node->elseStatements)) {
        visitNode(stmt);
    }
}

void ASTDefaultVisitor::visitOr(OrAST* node)
{
    for (ScopeAST* scope : std::as_const(node->scopes)) {
        visitNode(scope);
    }
    visitNode(node->body);
}

void ASTDefaultVisitor::visitSimpleScope(SimpleScopeAST* node)
{
    visitNode(node->identifier);
    visitNode(node->body);
}
}
