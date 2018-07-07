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

