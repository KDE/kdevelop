/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "dumptree.h"
#include "lexer.h"
#include "parsesession.h"
#include "debug.h"

#include <QtCore/QString>

#include "rpp/chartools.h"

char const * const names[] = {
  0,
  "AccessSpecifier",
  "AsmDefinition",
  "BaseClause",
  "BaseSpecifier",
  "BinaryExpression",
  "CastExpression",
  "ClassMemberAccess",
  "ClassSpecifier",
  "CompoundStatement",
  "Condition",
  "ConditionalExpression",
  "CppCastExpression",
  "CtorInitializer",
  "DeclarationStatement",
  "Declarator",
  "DeleteExpression",
  "DoStatement",
  "ElaboratedTypeSpecifier",
  "EnumSpecifier",
  "Enumerator",
  "ExceptionSpecification",
  "ExpressionOrDeclarationStatement",
  "ExpressionStatement",
  "ForStatement",
  "FunctionCall",
  "FunctionDefinition",
  "IfStatement",
  "IncrDecrExpression",
  "InitDeclarator",
  "Initializer",
  "InitializerClause",
  "LabeledStatement",
  "LinkageBody",
  "LinkageSpecification",
  "MemInitializer",
  "Name",
  "Namespace",
  "NamespaceAliasDefinition",
  "NewDeclarator",
  "NewExpression",
  "NewInitializer",
  "NewTypeId",
  "Operator",
  "OperatorFunctionId",
  "ParameterDeclaration",
  "ParameterDeclarationClause",
  "PostfixExpression",
  "PrimaryExpression",
  "PtrOperator",
  "PtrToMember",
  "ReturnStatement",
  "SimpleDeclaration",
  "SimpleTypeSpecifier",
  "SizeofExpression",
  "StringLiteral",
  "SubscriptExpression",
  "SwitchStatement",
  "TemplateArgument",
  "TemplateDeclaration",
  "TemplateParameter",
  "ThrowExpression",
  "TranslationUnit",
  "TryBlockStatement",
  "CatchStatement",
  "TypeId",
  "TypeIdentification",
  "TypeParameter",
  "Typedef",
  "UnaryExpression",
  "UnqualifiedName",
  "Using",
  "UsingDirective",
  "WhileStatement",
  "WinDeclSpec",
  "Comment",
  "JumpStatement",
  "SignalSlotExpression",
  "QProperty",
  "ForRangeDeclaration",
  "TypeIDOperator",
  "StaticAssert",
  "TrailingReturnType",
  "LambdaExpression",
  "LambdaCapture",
  "LambdaDeclarator",
  "InitializerList",
  "BracedInitList",
  "AliasDeclaration"
};

DumpTree::DumpTree()
  : m_tokenStream(0), indent(0), m_forceOutput(false)
{
  Q_ASSERT(sizeof(names) / sizeof(char const * const) == AST::NODE_KIND_COUNT);
}

void DumpTree::dump( AST * node, class TokenStream * tokenStream, bool forceOutput )
{
  m_tokenStream = tokenStream;
  m_forceOutput = forceOutput;
  visit(node);
  m_tokenStream = 0;
}

void DumpTree::visit(AST *node)
{
  if (!node) {
    return;
  }

  QString nodeText;
  if( m_tokenStream ) {
    ///TODO: reuse parsesession->stringForNode
    for( uint a = node->start_token; a < node->end_token; a++ ) {
      nodeText += m_tokenStream->symbolString(a) + ' ';
    }
  }

  qCDebug(CPPPARSER) << QString(indent * 2, ' ').toLatin1().constData() << names[node->kind]
               <<  "[" << node->start_token << "," << node->end_token << "]" << nodeText;

  ++indent;
  DefaultVisitor::visit(node);
  --indent;

  qCDebug(CPPPARSER) << QString(indent * 2, ' ').toLatin1().constData() << names[node->kind];
}

DumpTree::~ DumpTree( )
{
}

