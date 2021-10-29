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
    foreach (StatementAST* stmt, node->statements) {
        visitNode(stmt);
    }
}

void ASTDefaultVisitor::visitAssignment(AssignmentAST* node)
{
    visitNode(node->identifier);
    visitNode(node->op);
    foreach (ValueAST* value, node->values) {
        visitNode(value);
    }
}

void ASTDefaultVisitor::visitValue(ValueAST*)
{
}

void ASTDefaultVisitor::visitFunctionCall(FunctionCallAST* node)
{
    visitNode(node->identifier);
    foreach (ValueAST* value, node->args) {
        visitNode(value);
    }
    visitNode(node->body);
}

void ASTDefaultVisitor::visitScopeBody(ScopeBodyAST* node)
{
    foreach (StatementAST* stmt, node->statements) {
        visitNode(stmt);
    }
}

void ASTDefaultVisitor::visitOr(OrAST* node)
{
    foreach (ScopeAST* scope, node->scopes) {
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
