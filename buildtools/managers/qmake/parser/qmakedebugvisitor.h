/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef QMAKEDEBUGVISITOR_H
#define QMAKEDEBUGVISITOR_H

#include "qmake_default_visitor.h"

namespace QMake
{

class parser;

class DebugVisitor:  public default_visitor
{

public:
    DebugVisitor(QMake::parser* parser);
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
    QString getTokenInfo(std::size_t idx);
    QString getIndent();
private:
    QMake::parser* m_parser;
    int indent;
};

} // end of namespace QMake

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
