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

#include "qmakedefaultvisitor.h"
#include "qmakeparser.h"

namespace QMake
{

class parser;

class DebugVisitor:  public DefaultVisitor
{

public:
    DebugVisitor(QMake::Parser* parser);
    virtual void visitArgumentList( ArgumentListAst *node ) override;
    virtual void visitFunctionArguments( FunctionArgumentsAst *node ) override;
    virtual void visitOrOperator( OrOperatorAst *node ) override;
    virtual void visitItem( ItemAst *node ) override;
    virtual void visitScope( ScopeAst *node ) override;
    virtual void visitOp( OpAst *node ) override;
    virtual void visitProject( ProjectAst *node ) override;
    virtual void visitScopeBody( ScopeBodyAst *node ) override;
    virtual void visitStatement( StatementAst *node ) override;
    virtual void visitValue( ValueAst *node ) override;
    virtual void visitValueList( ValueListAst *node ) override;
    virtual void visitVariableAssignment( VariableAssignmentAst *node ) override;
    QString getTokenInfo(qint64 idx);
    QString getIndent();
private:
    QMake::Parser* m_parser;
    int indent;
};

} // end of namespace QMake

#endif

