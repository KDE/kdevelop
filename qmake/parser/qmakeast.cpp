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

#include "qmakeast.h"

namespace QMake
{
AST::AST( AST* parent )
        : m_parent( parent )
{}

AST::~AST( )
{}

AST* AST::parent() const
{
    return m_parent;
}

void AST::setParent( AST* parent )
{
    m_parent = parent;
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

StatementAST::StatementAST( AST* parent )
        : AST( parent )
{}

QString StatementAST::identifier() const
{
    return m_identifier;
}

void StatementAST::setIdentifier( const QString& id )
{
    m_identifier = id;
}

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
