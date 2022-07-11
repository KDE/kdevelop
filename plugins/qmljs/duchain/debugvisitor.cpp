/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "debugvisitor.h"

#include "parsesession.h"

#include <qmljs/parser/qmljsast_p.h>

using namespace KDevelop;
namespace
{
QString stringForAstKind(QmlJS::AST::Node::Kind kind)
{
    switch (kind) {
    case QmlJS::AST::Node::Kind_ArgumentList:
        return QStringLiteral("ArgumentList");
    case QmlJS::AST::Node::Kind_ArrayMemberExpression:
        return QStringLiteral("ArrayMemberExpression");
    case QmlJS::AST::Node::Kind_ArrayPattern:
        return QStringLiteral("ArrayPattern");
    case QmlJS::AST::Node::Kind_BinaryExpression:
        return QStringLiteral("BinaryExpression");
    case QmlJS::AST::Node::Kind_Block:
        return QStringLiteral("Block");
    case QmlJS::AST::Node::Kind_BreakStatement:
        return QStringLiteral("BreakStatement");
    case QmlJS::AST::Node::Kind_CallExpression:
        return QStringLiteral("CallExpression");
    case QmlJS::AST::Node::Kind_CaseBlock:
        return QStringLiteral("CaseBlock");
    case QmlJS::AST::Node::Kind_CaseClause:
        return QStringLiteral("CaseClause");
    case QmlJS::AST::Node::Kind_CaseClauses:
        return QStringLiteral("CaseClauses");
    case QmlJS::AST::Node::Kind_Catch:
        return QStringLiteral("Catch");
    case QmlJS::AST::Node::Kind_ClassDeclaration:
        return QStringLiteral("ClassDeclaration");
    case QmlJS::AST::Node::Kind_ClassElementList:
        return QStringLiteral("ClassElementList");
    case QmlJS::AST::Node::Kind_ClassExpression:
        return QStringLiteral("ClassExpression");
    case QmlJS::AST::Node::Kind_ComputedPropertyName:
        return QStringLiteral("ComputedPropertyName");
    case QmlJS::AST::Node::Kind_ConditionalExpression:
        return QStringLiteral("ConditionalExpression");
    case QmlJS::AST::Node::Kind_ContinueStatement:
        return QStringLiteral("ContinueStatement");
    case QmlJS::AST::Node::Kind_DebuggerStatement:
        return QStringLiteral("DebuggerStatement");
    case QmlJS::AST::Node::Kind_DefaultClause:
        return QStringLiteral("DefaultClause");
    case QmlJS::AST::Node::Kind_DeleteExpression:
        return QStringLiteral("DeleteExpression");
    case QmlJS::AST::Node::Kind_DoWhileStatement:
        return QStringLiteral("DoWhileStatement");
    case QmlJS::AST::Node::Kind_ElementList:
        return QStringLiteral("ElementList");
    case QmlJS::AST::Node::Kind_Elision:
        return QStringLiteral("Elision");
    case QmlJS::AST::Node::Kind_EmptyStatement:
        return QStringLiteral("EmptyStatement");
    case QmlJS::AST::Node::Kind_ExportClause:
        return QStringLiteral("ExportClause");
    case QmlJS::AST::Node::Kind_ExportDeclaration:
        return QStringLiteral("ExportDeclaration");
    case QmlJS::AST::Node::Kind_ExportSpecifier:
        return QStringLiteral("ExportSpecifier");
    case QmlJS::AST::Node::Kind_ExportsList:
        return QStringLiteral("ExportsList");
    case QmlJS::AST::Node::Kind_Expression:
        return QStringLiteral("Expression");
    case QmlJS::AST::Node::Kind_ExpressionStatement:
        return QStringLiteral("ExpressionStatement");
    case QmlJS::AST::Node::Kind_FalseLiteral:
        return QStringLiteral("FalseLiteral");
    case QmlJS::AST::Node::Kind_FieldMemberExpression:
        return QStringLiteral("FieldMemberExpression");
    case QmlJS::AST::Node::Kind_Finally:
        return QStringLiteral("Finally");
    case QmlJS::AST::Node::Kind_ForEachStatement:
        return QStringLiteral("ForEachStatement");
    case QmlJS::AST::Node::Kind_ForStatement:
        return QStringLiteral("ForStatement");
    case QmlJS::AST::Node::Kind_FormalParameterList:
        return QStringLiteral("FormalParameterList");
    case QmlJS::AST::Node::Kind_FromClause:
        return QStringLiteral("FromClause");
    case QmlJS::AST::Node::Kind_FunctionBody:
        return QStringLiteral("FunctionBody");
    case QmlJS::AST::Node::Kind_FunctionDeclaration:
        return QStringLiteral("FunctionDeclaration");
    case QmlJS::AST::Node::Kind_FunctionExpression:
        return QStringLiteral("FunctionExpression");
    case QmlJS::AST::Node::Kind_IdentifierExpression:
        return QStringLiteral("IdentifierExpression");
    case QmlJS::AST::Node::Kind_IdentifierPropertyName:
        return QStringLiteral("IdentifierPropertyName");
    case QmlJS::AST::Node::Kind_IfStatement:
        return QStringLiteral("IfStatement");
    case QmlJS::AST::Node::Kind_ImportClause:
        return QStringLiteral("ImportClause");
    case QmlJS::AST::Node::Kind_ImportDeclaration:
        return QStringLiteral("ImportDeclaration");
    case QmlJS::AST::Node::Kind_ImportSpecifier:
        return QStringLiteral("ImportSpecifier");
    case QmlJS::AST::Node::Kind_ImportsList:
        return QStringLiteral("ImportsList");
    case QmlJS::AST::Node::Kind_LabelledStatement:
        return QStringLiteral("LabelledStatement");
    case QmlJS::AST::Node::Kind_Module:
        return QStringLiteral("Module");
    case QmlJS::AST::Node::Kind_NameSpaceImport:
        return QStringLiteral("NameSpaceImport");
    case QmlJS::AST::Node::Kind_NamedImports:
        return QStringLiteral("NamedImports");
    case QmlJS::AST::Node::Kind_NestedExpression:
        return QStringLiteral("NestedExpression");
    case QmlJS::AST::Node::Kind_NewExpression:
        return QStringLiteral("NewExpression");
    case QmlJS::AST::Node::Kind_NewMemberExpression:
        return QStringLiteral("NewMemberExpression");
    case QmlJS::AST::Node::Kind_NotExpression:
        return QStringLiteral("NotExpression");
    case QmlJS::AST::Node::Kind_NullExpression:
        return QStringLiteral("NullExpression");
    case QmlJS::AST::Node::Kind_NumericLiteral:
        return QStringLiteral("NumericLiteral");
    case QmlJS::AST::Node::Kind_NumericLiteralPropertyName:
        return QStringLiteral("NumericLiteralPropertyName");
    case QmlJS::AST::Node::Kind_ObjectPattern:
        return QStringLiteral("ObjectPattern");
    case QmlJS::AST::Node::Kind_PatternElement:
        return QStringLiteral("PatternElement");
    case QmlJS::AST::Node::Kind_PatternElementList:
        return QStringLiteral("PatternElementList");
    case QmlJS::AST::Node::Kind_PatternProperty:
        return QStringLiteral("PatternProperty");
    case QmlJS::AST::Node::Kind_PatternPropertyList:
        return QStringLiteral("PatternPropertyList");
    case QmlJS::AST::Node::Kind_PostDecrementExpression:
        return QStringLiteral("PostDecrementExpression");
    case QmlJS::AST::Node::Kind_PostIncrementExpression:
        return QStringLiteral("PostIncrementExpression");
    case QmlJS::AST::Node::Kind_PreDecrementExpression:
        return QStringLiteral("PreDecrementExpression");
    case QmlJS::AST::Node::Kind_PreIncrementExpression:
        return QStringLiteral("PreIncrementExpression");
    case QmlJS::AST::Node::Kind_Program:
        return QStringLiteral("Program");
    case QmlJS::AST::Node::Kind_PropertyDefinitionList:
        return QStringLiteral("PropertyDefinitionList");
    case QmlJS::AST::Node::Kind_PropertyGetterSetter:
        return QStringLiteral("PropertyGetterSetter");
    case QmlJS::AST::Node::Kind_PropertyName:
        return QStringLiteral("PropertyName");
    case QmlJS::AST::Node::Kind_PropertyNameAndValue:
        return QStringLiteral("PropertyNameAndValue");
    case QmlJS::AST::Node::Kind_RegExpLiteral:
        return QStringLiteral("RegExpLiteral");
    case QmlJS::AST::Node::Kind_ReturnStatement:
        return QStringLiteral("ReturnStatement");
    case QmlJS::AST::Node::Kind_StatementList:
        return QStringLiteral("StatementList");
    case QmlJS::AST::Node::Kind_StringLiteral:
        return QStringLiteral("StringLiteral");
    case QmlJS::AST::Node::Kind_StringLiteralPropertyName:
        return QStringLiteral("StringLiteralPropertyName");
    case QmlJS::AST::Node::Kind_SuperLiteral:
        return QStringLiteral("SuperLiteral");
    case QmlJS::AST::Node::Kind_SwitchStatement:
        return QStringLiteral("SwitchStatement");
    case QmlJS::AST::Node::Kind_TaggedTemplate:
        return QStringLiteral("TaggedTemplate");
    case QmlJS::AST::Node::Kind_TemplateLiteral:
        return QStringLiteral("TemplateLiteral");
    case QmlJS::AST::Node::Kind_ThisExpression:
        return QStringLiteral("ThisExpression");
    case QmlJS::AST::Node::Kind_ThrowStatement:
        return QStringLiteral("ThrowStatement");
    case QmlJS::AST::Node::Kind_TildeExpression:
        return QStringLiteral("TildeExpression");
    case QmlJS::AST::Node::Kind_TrueLiteral:
        return QStringLiteral("TrueLiteral");
    case QmlJS::AST::Node::Kind_TryStatement:
        return QStringLiteral("TryStatement");
    case QmlJS::AST::Node::Kind_Type:
        return QStringLiteral("Type");
    case QmlJS::AST::Node::Kind_TypeAnnotation:
        return QStringLiteral("TypeAnnotation");
    case QmlJS::AST::Node::Kind_TypeArgumentList:
        return QStringLiteral("TypeArgumentList");
    case QmlJS::AST::Node::Kind_TypeExpression:
        return QStringLiteral("TypeExpression");
    case QmlJS::AST::Node::Kind_TypeOfExpression:
        return QStringLiteral("TypeOfExpression");
    case QmlJS::AST::Node::Kind_UiAnnotation:
        return QStringLiteral("UiAnnotation");
    case QmlJS::AST::Node::Kind_UiAnnotationList:
        return QStringLiteral("UiAnnotationList");
    case QmlJS::AST::Node::Kind_UiArrayBinding:
        return QStringLiteral("UiArrayBinding");
    case QmlJS::AST::Node::Kind_UiArrayMemberList:
        return QStringLiteral("UiArrayMemberList");
    case QmlJS::AST::Node::Kind_UiEnumDeclaration:
        return QStringLiteral("UiEnumDeclaration");
    case QmlJS::AST::Node::Kind_UiEnumMemberList:
        return QStringLiteral("UiEnumMemberList");
    case QmlJS::AST::Node::Kind_UiHeaderItemList:
        return QStringLiteral("UiHeaderItemList");
    case QmlJS::AST::Node::Kind_UiImport:
        return QStringLiteral("UiImport");
    case QmlJS::AST::Node::Kind_UiInlineComponent:
        return QStringLiteral("UiInlineComponent");
    case QmlJS::AST::Node::Kind_UiObjectBinding:
        return QStringLiteral("UiObjectBinding");
    case QmlJS::AST::Node::Kind_UiObjectDefinition:
        return QStringLiteral("UiObjectDefinition");
    case QmlJS::AST::Node::Kind_UiObjectInitializer:
        return QStringLiteral("UiObjectInitializer");
    case QmlJS::AST::Node::Kind_UiObjectMemberList:
        return QStringLiteral("UiObjectMemberList");
    case QmlJS::AST::Node::Kind_UiParameterList:
        return QStringLiteral("UiParameterList");
    case QmlJS::AST::Node::Kind_UiPragma:
        return QStringLiteral("UiPragma");
    case QmlJS::AST::Node::Kind_UiProgram:
        return QStringLiteral("UiProgram");
    case QmlJS::AST::Node::Kind_UiPropertyAttributes:
        return QStringLiteral("UiPropertyAttributes");
    case QmlJS::AST::Node::Kind_UiPublicMember:
        return QStringLiteral("UiPublicMember");
    case QmlJS::AST::Node::Kind_UiQualifiedId:
        return QStringLiteral("UiQualifiedId");
    case QmlJS::AST::Node::Kind_UiRequired:
        return QStringLiteral("UiRequired");
    case QmlJS::AST::Node::Kind_UiScriptBinding:
        return QStringLiteral("UiScriptBinding");
    case QmlJS::AST::Node::Kind_UiSourceElement:
        return QStringLiteral("UiSourceElement");
    case QmlJS::AST::Node::Kind_UiVersionSpecifier:
        return QStringLiteral("UiVersionSpecifier");
    case QmlJS::AST::Node::Kind_UnaryMinusExpression:
        return QStringLiteral("UnaryMinusExpression");
    case QmlJS::AST::Node::Kind_UnaryPlusExpression:
        return QStringLiteral("UnaryPlusExpression");
    case QmlJS::AST::Node::Kind_Undefined:
        return QStringLiteral("Undefined");
    case QmlJS::AST::Node::Kind_VariableDeclaration:
        return QStringLiteral("VariableDeclaration");
    case QmlJS::AST::Node::Kind_VariableDeclarationList:
        return QStringLiteral("VariableDeclarationList");
    case QmlJS::AST::Node::Kind_VariableStatement:
        return QStringLiteral("VariableStatement");
    case QmlJS::AST::Node::Kind_VoidExpression:
        return QStringLiteral("VoidExpression");
    case QmlJS::AST::Node::Kind_WhileStatement:
        return QStringLiteral("WhileStatement");
    case QmlJS::AST::Node::Kind_WithStatement:
        return QStringLiteral("WithStatement");
    case QmlJS::AST::Node::Kind_YieldExpression:
        return QStringLiteral("YieldExpression");
    }
    return QStringLiteral("<unknown node kind>");
}
}

DebugVisitor::DebugVisitor(const ParseSession* session)
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

void DebugVisitor::throwRecursionDepthError()
{
    static QTextStream qout(stdout);
    qout << "RECURSION DEPTH ERROR\n";
}

QString DebugVisitor::indent() const
{
    return QString().fill(QLatin1Char(' '), m_depth * 2);
}

void DebugVisitor::printNode(QmlJS::AST::Node* node, Position position)
{
    const auto start = node->firstSourceLocation();
    const auto end = node->lastSourceLocation();
    const auto location = position == Start ? start : end;

    static QTextStream qout(stdout);

    qout << indent() << stringForAstKind(static_cast<QmlJS::AST::Node::Kind>(node->kind)) << " [(" << start.startLine
         << ", " << start.startColumn << "), "
         << "(" << end.startLine << ", " << (end.startColumn + end.length) << ")]"
         << " \"" << m_session->symbolAt(location) << "\"\n";
}

