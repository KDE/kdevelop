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

#include "qmake_default_visitor.h"
#include <QtCore/QStack>

class QString;
template <typename T1, typename T2> class QPair;

namespace QMake
{

class parser;
class ProjectAST;
class AST;

class BuildASTVisitor : public default_visitor
{
public:
    BuildASTVisitor(parser* parser, ProjectAST* project);
    virtual ~BuildASTVisitor();
    virtual void visit_arg_list( arg_list_ast *node );
    virtual void visit_function_args( function_args_ast *node );
    virtual void visit_or_op( or_op_ast *node );
    virtual void visit_item( item_ast *node );
    virtual void visit_scope( scope_ast *node );
    virtual void visit_op( op_ast *node );
    virtual void visit_project( project_ast *node );
    virtual void visit_scope_body( scope_body_ast *node );
    virtual void visit_stmt( stmt_ast *node );
    virtual void visit_value( value_ast *node );
    virtual void visit_value_list( value_list_ast *node );
    virtual void visit_variable_assignment( variable_assignment_ast *node );

private:
    QString getTokenString(std::size_t idx);
    QPair<std::size_t,std::size_t> getTokenLineAndColumn( std::size_t idx );

    template <typename T> T* stackTop();
    template <typename T> T* stackPop();

    QStack<AST*> aststack;
    parser* m_parser;
};

}

#endif

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
