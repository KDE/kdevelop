/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ast.h"

namespace QMake {

AST::AST(AST* parent, AST::Type type)
    : type(type)
    , startLine(-1)
    , endLine(-1)
    , startColumn(-1)
    , endColumn(-1)
    , start(-1)
    , end(-1)
    , parent(parent)
{
}

AST::~AST()
{
}

ValueAST::ValueAST(AST* parent)
    : AST(parent, AST::Value)
{
}

StatementAST::StatementAST(AST* parent, AST::Type type)
    : AST(parent, type)
{
}

StatementAST::~StatementAST()
{
}

AssignmentAST::AssignmentAST(AST* parent)
    : StatementAST(parent, AST::Assignment)
    , identifier(nullptr)
    , op(nullptr)
{
}

AssignmentAST::~AssignmentAST()
{
    delete identifier;
    identifier = nullptr;
    qDeleteAll(values);
    values.clear();
    delete op;
}

ScopeBodyAST::ScopeBodyAST(AST* parent, AST::Type type)
    : AST(parent, type)
{
}

ScopeBodyAST::~ScopeBodyAST()
{
    qDeleteAll(ifStatements);
    ifStatements.clear();
    qDeleteAll(elseStatements);
    elseStatements.clear();
}

FunctionCallAST::FunctionCallAST(AST* parent)
    : ScopeAST(parent, AST::FunctionCall)
    , identifier(nullptr)
{
}

FunctionCallAST::~FunctionCallAST()
{
    delete identifier;
    identifier = nullptr;
    qDeleteAll(args);
    args.clear();
}

OrAST::OrAST(AST* parent)
    : ScopeAST(parent, AST::Or)
{
}

OrAST::~OrAST()
{
    qDeleteAll(scopes);
    scopes.clear();
}

ProjectAST::ProjectAST()
    : AST(nullptr, AST::Project)
{
}

ProjectAST::~ProjectAST()
{
    qDeleteAll(statements);
    statements.clear();
}

ScopeAST::ScopeAST(AST* parent, AST::Type type)
    : StatementAST(parent, type)
    , body(nullptr)
{
}

ScopeAST::~ScopeAST()
{
    delete body;
    body = nullptr;
}

SimpleScopeAST::SimpleScopeAST(AST* parent)
    : ScopeAST(parent, AST::SimpleScope)
    , identifier(nullptr)
{
}

SimpleScopeAST::~SimpleScopeAST()
{
    delete identifier;
    identifier = nullptr;
}
}
