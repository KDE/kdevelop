/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakeastvisitor.h"
#include "ast.h"

namespace QMake {

ASTVisitor::parser_fun_t ASTVisitor::_S_parser_table[] = {
    reinterpret_cast<parser_fun_t>(&ASTVisitor::visitProject),
    reinterpret_cast<parser_fun_t>(&ASTVisitor::visitScopeBody),
    reinterpret_cast<parser_fun_t>(&ASTVisitor::visitAssignment),
    reinterpret_cast<parser_fun_t>(&ASTVisitor::visitFunctionCall),
    reinterpret_cast<parser_fun_t>(&ASTVisitor::visitSimpleScope),
    reinterpret_cast<parser_fun_t>(&ASTVisitor::visitOr),
    reinterpret_cast<parser_fun_t>(&ASTVisitor::visitValue),
}; // _S_parser_table[]

ASTVisitor::~ASTVisitor()
{
}

void ASTVisitor::visitNode(AST* node)
{
    if (node) {
        (this->*_S_parser_table[node->type])(node);
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
