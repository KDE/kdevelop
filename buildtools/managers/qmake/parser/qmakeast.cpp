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
#include <kdebug.h>

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

    QString AST::whitespace() const
    {
        return m_ws;
    }

    void AST::setWhitespace( const QString& ws )
    {
        m_ws = ws;
    }

    NewlineAST::NewlineAST( AST* parent )
        : StatementAST( parent )
    {
    }

    void NewlineAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
    }

    CommentAST::CommentAST( AST* parent )
        : StatementAST( parent )
    {}

    QString CommentAST::comment() const
    {
        return m_comment;
    }

    void CommentAST::setComment( const QString& comment )
    {
        m_comment = comment;
    }

    void CommentAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        if ( !m_comment.startsWith( "#" ) )
            buf += "#";
        buf += m_comment;
    }

    StatementAST::StatementAST( AST* parent )
        : AST( parent )
    {
    }
}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
