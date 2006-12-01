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

    QString ProjectAST::filename() const
    {
        return m_filename;
    }

    void ProjectAST::writeToString( QString& buf ) const
    {
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            ( *it )->writeToString( buf );
        }
    }

    void ProjectAST::addStatement( StatementAST* a )
    {
        m_statements.append( a );
    }

    QList<StatementAST*> ProjectAST::statements() const
    {
        return m_statements;
    }

    void ProjectAST::removeStatement( StatementAST* a )
    {
        m_statements.removeAll( a );
    }

    AssignmentAST::AssignmentAST( const QString& variable, OpAST* op, const QStringList& values, AST* parent )
            : StatementAST( parent ), m_variable( variable ), m_op( op ), m_values( values )
    {}

    AssignmentAST::~AssignmentAST()
    {
        delete m_op;
        m_op = 0;
    }

    void AssignmentAST::addValue( const QString& value )
    {
        m_values.append( value );
    }

    void AssignmentAST::removeValue( const QString& value )
    {
        m_values.removeAll( value );
    }


    QString AssignmentAST::variable() const
    {
        return m_variable;
    }

    void AssignmentAST::setVariable( const QString& variable )
    {
        m_variable = variable;
    }

    OpAST* AssignmentAST::op() const
    {
        return m_op;
    }

    void AssignmentAST::setOp( OpAST* op )
    {
        m_op = op;
    }

    void AssignmentAST::writeToString( QString& buf ) const
    {
        buf += m_variable;
        m_op->writeToString( buf );
        buf += m_values.join( "" );
    }

    OpAST::OpAST( const QString& lws , const QString& op, const QString& rws, AST* parent )
            : AST( parent ), m_op( op ), m_lWs( lws ), m_rWs( rws )
    {

    }

    OpAST::~OpAST()
    {}

    QString OpAST::rightWhitespace() const
    {
        return m_rWs;
    }

    QString OpAST::leftWhitespace() const
    {
        return m_lWs;
    }

    QString OpAST::op() const
    {
        return m_op;
    }

    void OpAST::setRightWhitespace( const QString& rws )
    {
        m_rWs = rws;
    }

    void OpAST::setLeftWhitespace( const QString& lws )
    {
        m_lWs = lws;
    }

    void OpAST::setOp( const QString& op )
    {
        m_op = op;
    }

    void OpAST::writeToString( QString& buf ) const
    {
        buf += m_lWs;
        buf += m_op;
        buf += m_rWs;
    }


    CommentAST::CommentAST( const QString& comment, AST* parent )
            : StatementAST( parent ), m_comment( comment )
    {
    }

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
        if( !m_comment.startsWith("#") )
            buf += "#";
        buf += m_comment;
    }


    WhitespaceAST::WhitespaceAST( const QString& ws, AST* parent )
            : StatementAST( parent ), m_ws( ws )
    {
    }

    QString WhitespaceAST::whitespace() const
    {
        return m_ws;
    }

    void WhitespaceAST::setWhitespace( const QString& ws )
    {
        m_ws = ws;
    }

    void WhitespaceAST::writeToString( QString& buf ) const
    {
        buf += m_ws;
    }

}


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
