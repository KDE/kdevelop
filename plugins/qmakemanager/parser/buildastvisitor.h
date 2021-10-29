/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BUILDASTVISITOR_H
#define BUILDASTVISITOR_H

#include "qmakedefaultvisitor.h"
#include <util/stack.h>

class QString;

namespace QMake
{

class Parser;
class ProjectAST;
class AST;
class ValueAST;

class BuildASTVisitor : public DefaultVisitor
{
public:
    BuildASTVisitor(Parser* parser, ProjectAST* project);
    ~BuildASTVisitor() override;
    void visitArgumentList( ArgumentListAst *node ) override;
    void visitFunctionArguments( FunctionArgumentsAst *node ) override;
    void visitOrOperator( OrOperatorAst *node ) override;
    void visitItem( ItemAst *node ) override;
    void visitScope( ScopeAst *node ) override;
    void visitOp( OpAst *node ) override;
    void visitProject( ProjectAst *node ) override;
    void visitScopeBody( ScopeBodyAst *node ) override;
    void visitStatement( StatementAst *node ) override;
    void visitValue( ValueAst *node ) override;
    void visitValueList( ValueListAst *node ) override;
    void visitVariableAssignment( VariableAssignmentAst *node ) override;

private:
    QString getTokenString(qint64 idx);
    void setPositionForToken( qint64 idx, ValueAST* ast );
    void setPositionForAst( AstNode* node, AST* ast );

    template <typename T> T* stackTop();
    template <typename T> T* stackPop();
    template <typename T> T* createAst( AstNode* node, AST* ast );

    KDevelop::Stack<AST*> aststack;
    Parser* m_parser;
};

}

#endif

