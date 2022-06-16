/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakedebugvisitor.h"

#include "qmakeparser.h"
#include "qmakeast.h"
#include "kdev-pg-token-stream.h"
#include <debug.h>

namespace QMake {

DebugVisitor::DebugVisitor(QMake::Parser* parser)
    : m_out(stderr)
    , m_parser(parser)
    , indent(0)
{
}

QString DebugVisitor::getTokenInfo(qint64 idx)
{
    qint64 line, col;
    QMake::Parser::Token token = m_parser->tokenStream->at(idx);
    m_parser->tokenStream->startPosition(idx, &line, &col);
    return QStringLiteral("%1,%2,%3").arg(line).arg(col).arg(m_parser->tokenText(token.begin, token.end).replace(QLatin1Char('\n'), QLatin1String("\\n")));
}

QString DebugVisitor::getIndent()
{
    return QString().fill(QLatin1Char(' '), indent * 4);
}

void DebugVisitor::visitArgumentList(ArgumentListAst* node)
{
    m_out << getIndent() << "BEGIN(arg_list)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    DefaultVisitor::visitArgumentList(node);
    indent--;
    m_out << getIndent() << "END(arg_list)(" << getTokenInfo(node->startToken) << ")\n";
}

void DebugVisitor::visitOrOperator(OrOperatorAst* node)
{
    m_out << getIndent() << "BEGIN(or_op)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    DefaultVisitor::visitOrOperator(node);
    indent--;
    m_out << getIndent() << "END(or_op)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitScope(ScopeAst* node)
{
    m_out << getIndent() << "BEGIN(scope)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    visitNode(node->functionArguments);
    visitNode(node->orOperator);
    visitNode(node->ifElse);
    indent--;
    m_out << getIndent() << "END(scope)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitIfElse(IfElseAst* node)
{
    m_out << getIndent() << "BEGIN(scope)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    visitNode(node->ifBody);
    if (node->elseBody) {
        m_out << getIndent() << "ELSE:\n";
        visitNode(node->elseBody);
    }
    indent--;
    m_out << getIndent() << "END(scope)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitFunctionArguments(FunctionArgumentsAst* node)
{
    m_out << getIndent() << "BEGIN(function_args)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    DefaultVisitor::visitFunctionArguments(node);
    indent--;
    m_out << getIndent() << "END(function_args)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitOp(OpAst* node)
{
    m_out << getIndent() << "BEGIN(op)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    m_out << getIndent() << "optoken =" << getTokenInfo(node->optoken) << '\n';
    DefaultVisitor::visitOp(node);
    indent--;
    m_out << getIndent() << "END(op)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitProject(ProjectAst* node)
{
    m_out << getIndent() << "BEGIN(project)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    DefaultVisitor::visitProject(node);
    indent--;
    m_out << getIndent() << "END(project)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitScopeBody(ScopeBodyAst* node)
{
    m_out << getIndent() << "BEGIN(scope_body)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    DefaultVisitor::visitScopeBody(node);
    indent--;
    m_out << getIndent() << "END(scope_body)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitStatement(StatementAst* node)
{
    m_out << getIndent() << "BEGIN(stmt)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    m_out << getIndent() << "isExclam=" << node->isExclam << '\n';
    if (!node->isNewline) {
        m_out << getIndent() << "id=" << getTokenInfo(node->id) << '\n';
    }
    DefaultVisitor::visitStatement(node);
    indent--;
    m_out << getIndent() << "END(stmt)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitValue(ValueAst* node)
{
    m_out << getIndent() << "BEGIN(value)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    m_out << getIndent() << "value=" << getTokenInfo(node->value) << '\n';
    DefaultVisitor::visitValue(node);
    indent--;
    m_out << getIndent() << "END(value)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitValueList(ValueListAst* node)
{
    m_out << getIndent() << "BEGIN(value_list)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    DefaultVisitor::visitValueList(node);
    indent--;
    m_out << getIndent() << "END(value_list)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitVariableAssignment(VariableAssignmentAst* node)
{
    m_out << getIndent() << "BEGIN(variable_assignment)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    DefaultVisitor::visitVariableAssignment(node);
    indent--;
    m_out << getIndent() << "END(variable_assignment)(" << getTokenInfo(node->endToken) << ")\n";
}

void DebugVisitor::visitItem(ItemAst* node)
{

    m_out << getIndent() << "BEGIN(item)(" << getTokenInfo(node->startToken) << ")\n";
    indent++;
    m_out << getIndent() << "id=" << getTokenInfo(node->id) << '\n';
    DefaultVisitor::visitItem(node);
    indent--;
    m_out << getIndent() << "END(item)(" << getTokenInfo(node->endToken) << ")\n";
}
}
