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
#include <QtCore/QStack>

class QString;
template <typename T1, typename T2> class QPair;

namespace QMake
{

class Parser;
class ProjectAST;
class AST;

class BuildASTVisitor : public DefaultVisitor
{
public:
    BuildASTVisitor(Parser* parser, ProjectAST* project);
    virtual ~BuildASTVisitor();
    virtual void visitArgumentList( ArgumentListAst *node );
    virtual void visitFunctionArguments( FunctionArgumentsAst *node );
    virtual void visitOrOperator( OrOperatorAst *node );
    virtual void visitItem( ItemAst *node );
    virtual void visitScope( ScopeAst *node );
    virtual void visitOp( OpAst *node );
    virtual void visitProject( ProjectAst *node );
    virtual void visitScopeBody( ScopeBodyAst *node );
    virtual void visitStatement( StatementAst *node );
    virtual void visitValue( ValueAst *node );
    virtual void visitValueList( ValueListAst *node );
    virtual void visitVariableAssignment( VariableAssignmentAst *node );

private:
    QString getTokenString(qint64 idx);
    QPair<qint64,qint64> getTokenLineAndColumn( qint64 idx );

    template <typename T> T* stackTop();
    template <typename T> T* stackPop();

    QStack<AST*> aststack;
    Parser* m_parser;
};

}

#endif

