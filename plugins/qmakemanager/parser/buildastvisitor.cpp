/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "buildastvisitor.h"

#include "qmakeast.h"
#include "qmakeparser.h"
#include "ast.h"

#include <debug.h>

namespace QMake {

void setIdentifierForStatement(StatementAST* stmt, ValueAST* val)
{
    if (auto* orop = dynamic_cast<OrAST*>(stmt)) {
        setIdentifierForStatement(orop->scopes.at(0), val);
    } else if (auto* assign = dynamic_cast<AssignmentAST*>(stmt)) {
        assign->identifier = val;
    } else if (auto* funcall = dynamic_cast<FunctionCallAST*>(stmt)) {
        funcall->identifier = val;
    } else if (auto* simple = dynamic_cast<SimpleScopeAST*>(stmt)) {
        simple->identifier = val;
    }
}

BuildASTVisitor::BuildASTVisitor(Parser* parser, ProjectAST* project)
    : m_parser(parser)
{
    aststack.push(project);
}

BuildASTVisitor::~BuildASTVisitor()
{
    aststack.clear();
    m_parser = nullptr;
}

void BuildASTVisitor::visitArgumentList(ArgumentListAst* node)
{
    // Nothing to be done here as we just need to iterate through the items
    DefaultVisitor::visitArgumentList(node);
}

void BuildASTVisitor::visitFunctionArguments(FunctionArgumentsAst* node)
{
    // Nothing to be done here as we just need to iterate through the items
    DefaultVisitor::visitFunctionArguments(node);
}

void BuildASTVisitor::visitOrOperator(OrOperatorAst* node)
{
    // Nothing to be done here as we just need to iterate through the items
    DefaultVisitor::visitOrOperator(node);
}

void BuildASTVisitor::visitItem(ItemAst* node)
{
    if (node->functionArguments) {
        auto* call = createAst<FunctionCallAST>(node, aststack.top());
        auto* val = createAst<ValueAST>(node, call);
        val->value = getTokenString(node->id);
        setPositionForToken(node->id, val);
        call->identifier = val;
        auto* orast = stackTop<OrAST>();
        orast->scopes.append(call);
        aststack.push(call);
        DefaultVisitor::visitItem(node);
        aststack.pop();
    } else {
        auto* simple = createAst<SimpleScopeAST>(node, aststack.top());
        auto* val = createAst<ValueAST>(node, simple);
        val->value = getTokenString(node->id);
        setPositionForToken(node->id, val);
        simple->identifier = val;
        auto* orast = stackTop<OrAST>();
        orast->scopes.append(simple);
        DefaultVisitor::visitItem(node);
    }
}

void BuildASTVisitor::visitScope(ScopeAst* node)
{
    if (node->orOperator) {
        auto* orast = createAst<OrAST>(node, aststack.top());
        // qCDebug(KDEV_QMAKE) << "created orast:" << orast;
        if (node->functionArguments) {
            auto* ast = createAst<FunctionCallAST>(node, orast);
            aststack.push(ast);
            // qCDebug(KDEV_QMAKE) << "creating function call as first or-op" << ast;
            visitNode(node->functionArguments);
            // qCDebug(KDEV_QMAKE) << "function call done";
            aststack.pop();
            orast->scopes.append(ast);
        } else {
            auto* simple = createAst<SimpleScopeAST>(node, orast);
            // qCDebug(KDEV_QMAKE) << "creating simple scope as first or-op";
            orast->scopes.append(simple);
        }
        aststack.push(orast);
        visitNode(node->orOperator);
    } else {
        if (node->functionArguments) {
            auto* call = createAst<FunctionCallAST>(node, aststack.top());
            aststack.push(call);
            visitNode(node->functionArguments);
        } else {
            auto* simple = createAst<SimpleScopeAST>(node, aststack.top());
            aststack.push(simple);
        }
    }
    if (node->scopeBody) {
        auto* scopebody = createAst<ScopeBodyAST>(node, aststack.top());
        auto* scope = stackTop<ScopeAST>();
        scope->body = scopebody;
        aststack.push(scopebody);
        visitNode(node->scopeBody);
        aststack.pop();
    }
}

void BuildASTVisitor::visitOp(OpAst* node)
{
    auto* assign = stackTop<AssignmentAST>();
    auto* val = createAst<ValueAST>(node, assign);
    val->value = getTokenString(node->optoken);
    setPositionForToken(node->optoken, val);
    assign->op = val;
    DefaultVisitor::visitOp(node);
}

void BuildASTVisitor::visitProject(ProjectAst* node)
{
    DefaultVisitor::visitProject(node);
}

void BuildASTVisitor::visitScopeBody(ScopeBodyAst* node)
{
    DefaultVisitor::visitScopeBody(node);
}

void BuildASTVisitor::visitStatement(StatementAst* node)
{
    DefaultVisitor::visitStatement(node);
    if (!node->isNewline) {
        auto* stmt = stackPop<StatementAST>();
        // qCDebug(KDEV_QMAKE) << "got statement ast, setting value" << stmt;
        auto* val = createAst<ValueAST>(node, stmt);
        // qCDebug(KDEV_QMAKE) << "created value ast:" << val;
        val->value = getTokenString(node->id);
        // qCDebug(KDEV_QMAKE) << "set value" << val << val->value;
        setPositionForToken(node->id, val);
        if (node->isExclam) {
            // qCDebug(KDEV_QMAKE) << "found exclam";
            val->value = QLatin1Char('!') + val->value;
        }
        setIdentifierForStatement(stmt, val);

        auto* scope = stackTop<ScopeBodyAST>();
        //         qCDebug(KDEV_QMAKE) << "scope:" << scope;
        scope->statements.append(stmt);
    }
}

void BuildASTVisitor::visitValue(ValueAst* node)
{
    auto* assign = dynamic_cast<AssignmentAST*>(aststack.top());
    if (assign) {
        auto* value = createAst<ValueAST>(node, assign);
        value->value = getTokenString(node->value);
        assign->values.append(value);
    } else {
        auto* call = stackTop<FunctionCallAST>();
        auto* value = createAst<ValueAST>(node, call);
        value->value = getTokenString(node->value);
        setPositionForToken(node->value, value);
        call->args.append(value);
    }
    DefaultVisitor::visitValue(node);
}

void BuildASTVisitor::visitValueList(ValueListAst* node)
{
    DefaultVisitor::visitValueList(node);
}

void BuildASTVisitor::visitVariableAssignment(VariableAssignmentAst* node)
{
    auto* assign = createAst<AssignmentAST>(node, aststack.top());
    aststack.push(assign);
    DefaultVisitor::visitVariableAssignment(node);
}

template <typename T>
T* BuildASTVisitor::createAst(AstNode* node, AST* parent)
{
    if (!node) {
        return nullptr;
    }
    auto ast = new T(parent);
    setPositionForAst(node, ast);
    return ast;
}

template <typename T>
T* BuildASTVisitor::stackTop()
{
    if (aststack.isEmpty()) {
        qCDebug(KDEV_QMAKE);
        qCCritical(KDEV_QMAKE) << "ERROR: AST stack is empty, this should never happen";
        exit(255);
    }
    T* ast = dynamic_cast<T*>(aststack.top());
    if (!ast) {
        qCDebug(KDEV_QMAKE);
        qCCritical(KDEV_QMAKE) << "ERROR: AST stack is screwed, doing a hard exit" << aststack.top()->type;
        exit(255);
    }
    return ast;
}

template <typename T>
T* BuildASTVisitor::stackPop()
{
    if (aststack.isEmpty()) {
        qCDebug(KDEV_QMAKE);
        qCCritical(KDEV_QMAKE) << "ERROR: AST stack is empty, this should never happen";
        exit(255);
    }
    AST* tmp = aststack.pop();
    T* ast = dynamic_cast<T*>(tmp);
    if (!ast) {
        qCDebug(KDEV_QMAKE);
        qCCritical(KDEV_QMAKE) << "ERROR: AST stack is screwed, doing a hard exit" << tmp->type;
        exit(255);
    }
    return ast;
}

QString BuildASTVisitor::getTokenString(qint64 idx)
{
    QMake::Parser::Token token = m_parser->tokenStream->at(idx);
    return m_parser->tokenText(token.begin, token.end).replace(QLatin1Char('\n'), QLatin1String("\\n"));
}

void BuildASTVisitor::setPositionForAst(AstNode* node, AST* ast)
{
    qint64 line, col;
    m_parser->tokenStream->startPosition(node->startToken, &line, &col);
    ast->startLine = line;
    ast->startColumn = col;
    QMake::Parser::Token tok = m_parser->tokenStream->at(node->startToken);
    ast->start = tok.begin;
    m_parser->tokenStream->endPosition(node->endToken, &line, &col);
    ast->endLine = line;
    ast->endColumn = col;
    tok = m_parser->tokenStream->at(node->endToken);
    ast->end = tok.end;
}

void BuildASTVisitor::setPositionForToken(qint64 idx, ValueAST* ast)
{
    qint64 line, col;
    QMake::Parser::Token token = m_parser->tokenStream->at(idx);
    m_parser->tokenStream->startPosition(idx, &line, &col);
    ast->startLine = line;
    ast->startColumn = col;
    ast->start = token.begin;
    ast->end = token.end;
    m_parser->tokenStream->endPosition(idx, &line, &col);
    ast->endLine = line;
    ast->endColumn = col;
}
}
