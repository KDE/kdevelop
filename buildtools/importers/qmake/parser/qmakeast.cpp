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
#include <QDebug>

namespace QMake
{
    AST::AST( const QString ws, AST* parent )
            : m_parent( parent ), m_ws( ws )
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

    ProjectAST::ProjectAST( AST* parent )
            : AST( "", parent )
    {}

    ProjectAST::~ProjectAST()
    {
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
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

    AssignmentAST::AssignmentAST( const QString& variable, const QString& op,
                                  const QStringList& values, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_variable( variable ), m_op( op )
            , m_values( values )
    {}

    AssignmentAST::~AssignmentAST()
    {
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

    QString AssignmentAST::op() const
    {
        return m_op;
    }

    void AssignmentAST::setOp( const QString& op )
    {
        m_op = op;
    }

    void AssignmentAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        buf += m_variable;
        buf += m_op;
        buf += m_values.join( "" );
    }

    CommentAST::CommentAST( const QString& comment, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_comment( comment )
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
        buf += whitespace();
        if ( !m_comment.startsWith( "#" ) )
            buf += "#";
        buf += m_comment;
    }

    FunctionAST::FunctionAST( FunctionCallAST* call, const QString& begin, QList<StatementAST*> stmts,
                              const QString& end, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_call( call ), m_statements( stmts ), m_begin( begin ), m_end( end )
    {
    }

    FunctionAST::FunctionAST( FunctionCallAST* call, const QString& begin, StatementAST* stmt,
                              const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_call( call ), m_begin( begin ), m_end( "" )
    {
        m_statements.append( stmt );
    }


    FunctionAST::FunctionAST( FunctionCallAST* call, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_call( call ), m_begin( "" ), m_end( "" )
    {
    }

    FunctionAST::~FunctionAST()
    {
        delete m_call;
        m_call = 0;
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            delete( *it );
        }
        m_statements.clear();
    }

    void FunctionAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        m_call->writeToString( buf );
        buf += m_begin;
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            ( *it )->writeToString( buf );
        }
        buf += m_end;
    }

    OrAST::OrAST( FunctionCallAST* lcall, FunctionCallAST* rcall, const QString& begin,
                  QList<StatementAST*> stmts, const QString& end, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_lCall( lcall ), m_rCall( rcall ),
            m_statements( stmts ), m_begin( begin ), m_end( end )
    {
    }

    OrAST::~OrAST()
    {
        delete m_lCall;
        m_lCall = 0;
        delete m_rCall;
        m_rCall = 0;
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            delete( *it );
        }
        m_statements.clear();
    }

    void OrAST::writeToString( QString& buf ) const
    {
        m_lCall->writeToString( buf );
        buf += whitespace();
        buf += "|";
        m_rCall->writeToString( buf );

        buf += m_begin;
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            ( *it )->writeToString( buf );
        }
        buf += m_end;

    }

    FunctionArgAST::FunctionArgAST( const QString& ws, AST* parent )
            : AST( ws, parent )
    {}

    FunctionArgAST::~FunctionArgAST()
    {}

    void FunctionArgAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
    }

    FunctionCallAST::FunctionCallAST( const QString& functionname, const QString& begin,
                                      QList<FunctionArgAST*> args, const QString& end, const QString& ws, AST* parent )
            : FunctionArgAST( ws, parent ), m_args( args ), m_asFunctionArg( false ),
            m_functionName( functionname ), m_begin( begin ), m_end( end )
    {}


    FunctionCallAST::~FunctionCallAST()
    {
        for ( QList<FunctionArgAST*>::const_iterator it = m_args.begin(); it != m_args.end(); ++it )
        {
            delete( *it );
        }
        m_args.clear();
    }

    void FunctionCallAST::setAsFunctionArg( bool useAsArg )
    {
        m_asFunctionArg = useAsArg;
    }

    bool FunctionCallAST::asFunctionArg() const
    {
        return m_asFunctionArg;
    }

    void FunctionCallAST::writeToString( QString& buf ) const
    {
        FunctionArgAST::writeToString( buf );
        if ( m_asFunctionArg )
            buf += "$$";
        buf += m_functionName;
        buf += m_begin;
        for ( QList<FunctionArgAST*>::const_iterator it = m_args.begin(); it != m_args.end(); )
        {
            ( *it )->writeToString( buf );
            if ( ++it != m_args.end() )
                buf += ",";
        }
        buf += m_end;
    }

    SimpleFunctionArgAST::SimpleFunctionArgAST( const QString& value, const QString& ws, AST* parent )
            : FunctionArgAST( ws, parent ), m_value( value )
    {
    }

    SimpleFunctionArgAST::~SimpleFunctionArgAST()
    {}

    void SimpleFunctionArgAST::writeToString( QString& buf ) const
    {
        FunctionArgAST::writeToString( buf );
        buf += m_value;
    }

    ScopeAST::ScopeAST( const QString& scopename, const QString& begin, QList<StatementAST*> stmts,
                        const QString& end, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_scopeName( scopename ), m_statements( stmts ),
            m_begin( begin ), m_end( end )
    {
    }

    ScopeAST::~ScopeAST()
    {
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            delete( *it );
        }
        m_statements.clear();
    }

    void ScopeAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        buf += m_scopeName;
        buf += m_begin;
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            ( *it )->writeToString( buf );
        }
        buf += m_end;
    }

}


// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
