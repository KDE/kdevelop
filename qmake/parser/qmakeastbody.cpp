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
#include <kdebug.h>

namespace QMake
{
    ScopeBodyAST::ScopeBodyAST( AST* parent )
        : AST( parent )
    {}

    ScopeBodyAST::~ScopeBodyAST()
    {
        QList<StatementAST*>::const_iterator it;
        for ( it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            delete( *it );
        }
        m_statements.clear();
    }

    void ScopeBodyAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        buf += m_begin;
        QList<StatementAST*>::const_iterator it;
        for ( it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            if (( *it ) )
                ( *it )->writeToString( buf );
        }
        buf += m_end;
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

    void ScopeBodyAST::setStatements( QList<StatementAST*> newlist )
    {
        m_statements = newlist;
    }

    void ScopeBodyAST::removeStatement( int i )
    {
        m_statements.removeAt( i );
    }

    QString ScopeBodyAST::begin() const
    {
        return m_begin;
    }

    QString ScopeBodyAST::end() const
    {
        return m_end;
    }

    void ScopeBodyAST::setBegin( const QString& begin )
    {
        m_begin = begin;
    }

    void ScopeBodyAST::setEnd( const QString& end )
    {
        m_end = end;
    }
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
