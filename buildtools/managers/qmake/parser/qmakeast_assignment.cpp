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
        AssignmentAST::AssignmentAST( AST* parent )
        : StatementAST( parent )
    {}

    AssignmentAST::~AssignmentAST()
    {}

    void AssignmentAST::addValue( const QString& value )
    {
        m_values.append( value );
    }

    void AssignmentAST::removeValue( const QString& value )
    {
        m_values.removeAll( value );
    }

    QStringList AssignmentAST::values() const
    {
        return m_values;
    }

    void AssignmentAST::setValues( const QStringList& newlist )
    {
        m_values = newlist;
    }

    QString AssignmentAST::variable() const
    {
        return m_variable;
    }

    void AssignmentAST::setVariable( const QString& variable )
    {
        m_variable = variable;
    }

    QString AssignmentAST::op() const
    {
        return m_op;
    }

    void AssignmentAST::setOp( const QString& op )
    {
        m_op = op;
    }

    void AssignmentAST::setLineEnding( const QString& end )
    {
        m_lineend = end;
    }

    void AssignmentAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        buf += m_variable;
        buf += m_op;
        buf += m_values.join( "" );
        if( m_lineend.isEmpty() )
#ifdef Q_WS_WIN
            buf += "\r\n";
#else
            buf += "\n";
#endif
        else
            buf += m_lineend;
    }
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
