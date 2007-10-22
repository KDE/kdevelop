/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "qmakeast.h"

namespace QMake
{
AssignmentAST::AssignmentAST( AST* parent )
        : StatementAST( parent )
{}

AssignmentAST::~AssignmentAST()
{
    qDeleteAll( m_values );
    m_values.clear();
    delete m_op;
}

void AssignmentAST::insertValue( int i, ValueAST* value )
{
  m_values.insert(i, value);
}

void AssignmentAST::addValue( ValueAST* value )
{
    m_values.append( value );
}

void AssignmentAST::removeValue( int i )
{
    m_values.removeAt( i );
}

QList<ValueAST*> AssignmentAST::values() const
{
    return m_values;
}

ValueAST* AssignmentAST::variable() const
{
    return identifier();
}

void AssignmentAST::setVariable( ValueAST* variable )
{
    setIdentifier(variable);
}

ValueAST* AssignmentAST::op() const
{
    return m_op;
}

void AssignmentAST::setOp( ValueAST* op )
{
    m_op = op;
}

AST::Type AssignmentAST::type() const
{
    return AST::Assignment;
}

}

