/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2012 Milian Wolff <mail@milianw.de>                         *
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

#include "debugvisitor.h"

#include "parsesession.h"

#include <qmljs/parser/qmljsast_p.h>

using namespace KDevelop;

QString DebugVisitor::stringForAstKind(int kind)
{
    switch (kind) {
        case QmlJS::AST::Node::Kind_Undefined: return "Undefined";
        case QmlJS::AST::Node::Kind_ArgumentList: return "ArgumentList";
        case QmlJS::AST::Node::Kind_ArrayLiteral: return "ArrayLiteral";
        case QmlJS::AST::Node::Kind_ArrayMemberExpression: return "ArrayMemberExpression";
        case QmlJS::AST::Node::Kind_BinaryExpression: return "BinaryExpression";
        case QmlJS::AST::Node::Kind_Block: return "Block";
        case QmlJS::AST::Node::Kind_BreakStatement: return "BreakStatement";
        case QmlJS::AST::Node::Kind_CallExpression: return "CallExpression";
        case QmlJS::AST::Node::Kind_CaseBlock: return "CaseBlock";
        case QmlJS::AST::Node::Kind_CaseClause: return "CaseClause";
        case QmlJS::AST::Node::Kind_CaseClauses: return "CaseClauses";
        case QmlJS::AST::Node::Kind_Catch: return "Catch";
        case QmlJS::AST::Node::Kind_ConditionalExpression: return "ConditionalExpression";
        case QmlJS::AST::Node::Kind_ContinueStatement: return "ContinueStatement";
        case QmlJS::AST::Node::Kind_DebuggerStatement: return "DebuggerStatement";
        case QmlJS::AST::Node::Kind_DefaultClause: return "DefaultClause";
        case QmlJS::AST::Node::Kind_DeleteExpression: return "DeleteExpression";
        case QmlJS::AST::Node::Kind_DoWhileStatement: return "DoWhileStatement";
        case QmlJS::AST::Node::Kind_ElementList: return "ElementList";
        case QmlJS::AST::Node::Kind_Elision: return "Elision";
        case QmlJS::AST::Node::Kind_EmptyStatement: return "EmptyStatement";
        case QmlJS::AST::Node::Kind_Expression: return "Expression";
        case QmlJS::AST::Node::Kind_ExpressionStatement: return "ExpressionStatement";
        case QmlJS::AST::Node::Kind_FalseLiteral: return "FalseLiteral";
        case QmlJS::AST::Node::Kind_FieldMemberExpression: return "FieldMemberExpression";
        case QmlJS::AST::Node::Kind_Finally: return "Finally";
        case QmlJS::AST::Node::Kind_ForEachStatement: return "ForEachStatement";
        case QmlJS::AST::Node::Kind_ForStatement: return "ForStatement";
        case QmlJS::AST::Node::Kind_FormalParameterList: return "FormalParameterList";
        case QmlJS::AST::Node::Kind_FunctionBody: return "FunctionBody";
        case QmlJS::AST::Node::Kind_FunctionDeclaration: return "FunctionDeclaration";
        case QmlJS::AST::Node::Kind_FunctionExpression: return "FunctionExpression";
        case QmlJS::AST::Node::Kind_FunctionSourceElement: return "FunctionSourceElement";
        case QmlJS::AST::Node::Kind_IdentifierExpression: return "IdentifierExpression";
        case QmlJS::AST::Node::Kind_IdentifierPropertyName: return "IdentifierPropertyName";
        case QmlJS::AST::Node::Kind_IfStatement: return "IfStatement";
        case QmlJS::AST::Node::Kind_LabelledStatement: return "LabelledStatement";
        case QmlJS::AST::Node::Kind_LocalForEachStatement: return "LocalForEachStatement";
        case QmlJS::AST::Node::Kind_LocalForStatement: return "LocalForStatement";
        case QmlJS::AST::Node::Kind_NewExpression: return "NewExpression";
        case QmlJS::AST::Node::Kind_NewMemberExpression: return "NewMemberExpression";
        case QmlJS::AST::Node::Kind_NotExpression: return "NotExpression";
        case QmlJS::AST::Node::Kind_NullExpression: return "NullExpression";
        case QmlJS::AST::Node::Kind_NumericLiteral: return "NumericLiteral";
        case QmlJS::AST::Node::Kind_NumericLiteralPropertyName: return "NumericLiteralPropertyName";
        case QmlJS::AST::Node::Kind_ObjectLiteral: return "ObjectLiteral";
        case QmlJS::AST::Node::Kind_PostDecrementExpression: return "PostDecrementExpression";
        case QmlJS::AST::Node::Kind_PostIncrementExpression: return "PostIncrementExpression";
        case QmlJS::AST::Node::Kind_PreDecrementExpression: return "PreDecrementExpression";
        case QmlJS::AST::Node::Kind_PreIncrementExpression: return "PreIncrementExpression";
        case QmlJS::AST::Node::Kind_Program: return "Program";
        case QmlJS::AST::Node::Kind_PropertyName: return "PropertyName";
        case QmlJS::AST::Node::Kind_PropertyNameAndValueList: return "PropertyNameAndValueList";
        case QmlJS::AST::Node::Kind_RegExpLiteral: return "RegExpLiteral";
        case QmlJS::AST::Node::Kind_ReturnStatement: return "ReturnStatement";
        case QmlJS::AST::Node::Kind_SourceElement: return "SourceElement";
        case QmlJS::AST::Node::Kind_SourceElements: return "SourceElements";
        case QmlJS::AST::Node::Kind_StatementList: return "StatementList";
        case QmlJS::AST::Node::Kind_StatementSourceElement: return "StatementSourceElement";
        case QmlJS::AST::Node::Kind_StringLiteral: return "StringLiteral";
        case QmlJS::AST::Node::Kind_StringLiteralPropertyName: return "StringLiteralPropertyName";
        case QmlJS::AST::Node::Kind_SwitchStatement: return "SwitchStatement";
        case QmlJS::AST::Node::Kind_ThisExpression: return "ThisExpression";
        case QmlJS::AST::Node::Kind_ThrowStatement: return "ThrowStatement";
        case QmlJS::AST::Node::Kind_TildeExpression: return "TildeExpression";
        case QmlJS::AST::Node::Kind_TrueLiteral: return "TrueLiteral";
        case QmlJS::AST::Node::Kind_TryStatement: return "TryStatement";
        case QmlJS::AST::Node::Kind_TypeOfExpression: return "TypeOfExpression";
        case QmlJS::AST::Node::Kind_UnaryMinusExpression: return "UnaryMinusExpression";
        case QmlJS::AST::Node::Kind_UnaryPlusExpression: return "UnaryPlusExpression";
        case QmlJS::AST::Node::Kind_VariableDeclaration: return "VariableDeclaration";
        case QmlJS::AST::Node::Kind_VariableDeclarationList: return "VariableDeclarationList";
        case QmlJS::AST::Node::Kind_VariableStatement: return "VariableStatement";
        case QmlJS::AST::Node::Kind_VoidExpression: return "VoidExpression";
        case QmlJS::AST::Node::Kind_WhileStatement: return "WhileStatement";
        case QmlJS::AST::Node::Kind_WithStatement: return "WithStatement";
        case QmlJS::AST::Node::Kind_NestedExpression: return "NestedExpression";
        case QmlJS::AST::Node::Kind_UiArrayBinding: return "UiArrayBinding";
        case QmlJS::AST::Node::Kind_UiImport: return "UiImport";
        case QmlJS::AST::Node::Kind_UiImportList: return "UiImportList";
        case QmlJS::AST::Node::Kind_UiObjectBinding: return "UiObjectBinding";
        case QmlJS::AST::Node::Kind_UiObjectDefinition: return "UiObjectDefinition";
        case QmlJS::AST::Node::Kind_UiObjectInitializer: return "UiObjectInitializer";
        case QmlJS::AST::Node::Kind_UiObjectMemberList: return "UiObjectMemberList";
        case QmlJS::AST::Node::Kind_UiArrayMemberList: return "UiArrayMemberList";
        case QmlJS::AST::Node::Kind_UiProgram: return "UiProgram";
        case QmlJS::AST::Node::Kind_UiParameterList: return "UiParameterList";
        case QmlJS::AST::Node::Kind_UiPublicMember: return "UiPublicMember";
        case QmlJS::AST::Node::Kind_UiQualifiedId: return "UiQualifiedId";
        case QmlJS::AST::Node::Kind_UiScriptBinding: return "UiScriptBinding";
        case QmlJS::AST::Node::Kind_UiSourceElement: return "UiSourceElement";
    }
    return "<unknown node kind>";
}

DebugVisitor::DebugVisitor(ParseSession* session)
: m_session(session)
, m_depth(0)
{
}

void DebugVisitor::startVisiting(QmlJS::AST::Node* node)
{
    QmlJS::AST::Node::accept(node, this);
}

bool DebugVisitor::preVisit(QmlJS::AST::Node* node)
{
    printNode(node, Start);
    ++m_depth;
    return true;
}

void DebugVisitor::postVisit(QmlJS::AST::Node* node)
{
    Q_ASSERT(m_depth);
    --m_depth;
    printNode(node, End);
}

QString DebugVisitor::indent() const
{
    return QString().fill(' ', m_depth * 2);
}

void DebugVisitor::printNode(QmlJS::AST::Node* node, Position position)
{
    const QmlJS::AST::SourceLocation start = node->firstSourceLocation();
    const QmlJS::AST::SourceLocation end = node->lastSourceLocation();
    const QmlJS::AST::SourceLocation location = position == Start ? start : end;

    static QTextStream qout(stdout);

    qout << indent() << stringForAstKind(node->kind)
         << " [(" << start.startLine << ", " << start.startColumn << "), "
         << "(" << end.startLine << ", " << (end.startColumn + end.length) << ")]"
         << " \"" << m_session->symbolAt(location) << "\"" << endl;
}

