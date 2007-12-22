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

#include "ast.h"

namespace QMake
{
AST::AST( AST* parent )
        : m_line(-1), m_column(-1), m_parent( parent )
{}

AST::~AST( )
{}

AST* AST::parent() const
{
    return m_parent;
}

int AST::line() const
{
    return m_line;
}

void AST::setLine( int line )
{
    m_line = line;
}

int AST::column() const
{
    return m_column;
}

void AST::setColumn( int col )
{
    m_column = col;
}

ValueAST::ValueAST( AST* parent )
        : AST( parent )
{}

void ValueAST::setValue( const QString& value )
{
    m_value = value;
}

QString ValueAST::value() const
{
    return m_value;
}


AST::Type ValueAST::type() const
{
    return AST::Value;
}

StatementAST::StatementAST( AST* parent )
        : AST( parent ), m_identifier(0)
{}

StatementAST::~StatementAST( )
{
    delete m_identifier;
    m_identifier = 0;
}


ValueAST* StatementAST::identifier() const
{
    return m_identifier;
}

void StatementAST::setIdentifier( ValueAST* id )
{
    m_identifier = id;
}

int StatementAST::line() const
{
    return m_identifier->line();
}

int StatementAST::column() const
{
    return m_identifier->column();
}

}

