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
    ProjectAST::ProjectAST( AST* parent )
            : AST( parent ), m_lineEnding(Unix)
    {}

    ProjectAST::~ProjectAST()
    {
        QList<StatementAST*>::const_iterator it;
        for ( it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            delete( *it );
        }
        m_statements.clear();
    }

    void ProjectAST::setFilename( const QString& filename )
    {
        m_filename = filename;
    }

    QString ProjectAST::filename() const
    {
        return m_filename;
    }

    ProjectAST::LineEnding ProjectAST::lineEnding()
    {
        return m_lineEnding;
    }

    void ProjectAST::setLineEnding( ProjectAST::LineEnding l )
    {
        m_lineEnding = l;
    }

    void ProjectAST::writeToString( QString& buf ) const
    {
        QList<StatementAST*>::const_iterator it;
        for ( it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            if (( *it ) )
                ( *it )->writeToString( buf );
        }
    }

    void ProjectAST::insertStatement( int i, StatementAST* a )
    {
        m_statements.insert( i, a );
    }

    void ProjectAST::addStatement( StatementAST* a )
    {
        m_statements.append( a );
    }

    QList<StatementAST*> ProjectAST::statements() const
    {
        return m_statements;
    }

    void ProjectAST::removeStatement( int i )
    {
        m_statements.removeAt( i );
    }
}
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
