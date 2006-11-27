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

    void AST::setDepth( unsigned int depth )
    {
        m_depth = depth;
    }

    unsigned int AST::depth() const
    {
        return m_depth;
    }

    ProjectAST::ProjectAST( AST* parent )
            : AST( parent )
    {}

    ProjectAST::~ProjectAST()
    {}

    void ProjectAST::setFilename( const QString& filename )
    {
        m_filename = filename;
    }

    void ProjectAST::writeToString( QString& buf ) const
    {
        for ( QList<AST*>::const_iterator it = childs().begin(); it != childs().end() ; ++it ) {
            ( *it )->writeToString( buf );
        }
    }

    void ProjectAST::addChild( AST* a )
    {
        m_childs.append( a );
        a->setParent( this );
    }

    QList<AST*> ProjectAST::childs() const
    {
        return m_childs;
    }

    QString ProjectAST::filename() const
    {
        return m_filename;
    }

    CommentAST::CommentAST( const QString& comment, AST* parent )
            : AST( parent ), m_comment( comment )
    {}

    CommentAST::~CommentAST()
    {}

    QString CommentAST::comment( ) const
    {
        return m_comment;
    }

    void CommentAST::setComment( const QString& comment )
    {
        m_comment = comment;
    }

    void CommentAST::writeToString( QString& buf ) const
    {
        if ( m_comment.startsWith( "#" ) )
            buf += m_comment;
        else
            buf += "#" + m_comment;
    }

    NewlineAST::NewlineAST( AST* parent )
            : AST( parent )
{}

    NewlineAST::~NewlineAST()
    {}

    void NewlineAST::writeToString( QString& buf ) const
    {
        buf += "\n";
    }

}


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
