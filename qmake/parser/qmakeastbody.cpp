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
ScopeBodyAST::ScopeBodyAST( AST* parent )
        : AST( parent )
{}

ScopeBodyAST::~ScopeBodyAST()
{
    qDeleteAll( m_statements );
    m_statements.clear();
}

void ScopeBodyAST::insertStatement( int i, StatementAST* a )
{
    m_statements.insert( i, a );
}

void ScopeBodyAST::addStatement( StatementAST* a )
{
    m_statements.append( a );
}

QList<StatementAST*> ScopeBodyAST::statements() const
{
    return m_statements;
}

void ScopeBodyAST::removeStatement( int i )
{
    m_statements.removeAt( i );
}

}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
