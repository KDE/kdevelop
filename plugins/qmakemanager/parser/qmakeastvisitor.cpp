/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakeastvisitor.h"
#include "ast.h"

namespace QMake {
ASTVisitor::~ASTVisitor()
{
}

void ASTVisitor::visitNode(AST* node)
{
    using Type = AST::Type;
    switch (node ? node->type : Type::Invalid) {
    case Type::Project:
        visitProject(static_cast<ProjectAST*>(node));
        return;
    case Type::ScopeBody:
        visitScopeBody(static_cast<ScopeBodyAST*>(node));
        return;
    case Type::Assignment:
        visitAssignment(static_cast<AssignmentAST*>(node));
        return;
    case Type::FunctionCall:
        visitFunctionCall(static_cast<FunctionCallAST*>(node));
        return;
    case Type::SimpleScope:
        visitSimpleScope(static_cast<SimpleScopeAST*>(node));
        return;
    case Type::Or:
        visitOr(static_cast<OrAST*>(node));
        return;
    case Type::Value:
        visitValue(static_cast<ValueAST*>(node));
        return;
    case Type::Invalid:
        break;
    }
}

void ASTVisitor::visitProject(ProjectAST*)
{
}

void ASTVisitor::visitAssignment(AssignmentAST*)
{
}

void ASTVisitor::visitValue(ValueAST*)
{
}

void ASTVisitor::visitFunctionCall(FunctionCallAST*)
{
}

void ASTVisitor::visitScopeBody(ScopeBodyAST*)
{
}

void ASTVisitor::visitOr(OrAST*)
{
}

void ASTVisitor::visitSimpleScope(SimpleScopeAST*)
{
}
}
