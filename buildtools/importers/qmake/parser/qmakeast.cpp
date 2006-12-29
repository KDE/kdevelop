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

    AssignmentAST::AssignmentAST( const QString& variable, const QString& op, const QStringList& values, const QString& comment, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_variable( variable ), m_op( op )
            , m_values( values ), m_comment( comment )
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
        buf += m_comment;
        buf += "\n";
    }

    CommentAST::CommentAST( const QString& comment, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_comment( comment )
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

    ScopeAST::ScopeAST( FunctionCallAST* call, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_call( call ), m_type( Function ), m_body( 0 )
    {
    }


    ScopeAST::ScopeAST( const QString& scopename, const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_call( 0 ), m_scopeName( scopename ), m_type( Simple ), m_body( 0 )
    {
    }


    ScopeAST::~ScopeAST()
    {
        delete m_call;
        m_call = 0;

    }

    ScopeBodyAST* ScopeAST::scopeBody() const
    {
        return m_body;
    }

    void ScopeAST::setScopeName( const QString& name )
    {
        m_scopeName = name;
        delete m_call;
        m_call = 0;
    }

    QString ScopeAST::scopeName( ) const
    {
        return m_scopeName;
    }

    void ScopeAST::setFunctionCall( FunctionCallAST* call )
    {
        m_call = call;
        m_scopeName = "";
    }

    FunctionCallAST* ScopeAST::functionCall() const
    {
        return m_call;
    }

    void ScopeAST::writeToString( QString& buf ) const
    {
        buf += whitespace();
        if ( m_type == ScopeAST::Function && m_call )
            m_call->writeToString( buf );
        else
            buf += m_scopeName;
        if ( m_body )
            m_body->writeToString( buf );

    }

    void ScopeAST::setScopeBody( ScopeBodyAST* body )
    {
        m_body = body;
    }

    ScopeBodyAST::ScopeBodyAST( const QString& begin, QList<StatementAST*> stmts,
                                const QString& end, AST* parent )
            : AST( "", parent ), m_statements( stmts ),
            m_begin( begin ), m_end( end )
    {}

    ScopeBodyAST::ScopeBodyAST( const QString& begin, StatementAST* stmt, AST* parent )
            : AST( "", parent ), m_begin( begin ), m_end( "" )
    {
        m_statements.append( stmt );
    }
    ScopeBodyAST::~ScopeBodyAST()
    {
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
        {
            delete( *it );
        }
        m_statements.clear();
    }

    void ScopeBodyAST::writeToString( QString& buf ) const
    {
        buf += m_begin;
        for ( QList<StatementAST*>::const_iterator it = m_statements.begin(); it != m_statements.end(); ++it )
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

    void ScopeBodyAST::removeStatement( int i )
    {
        m_statements.removeAt( i );
    }

    OrAST::OrAST( FunctionCallAST* lcall, const QString& orop, FunctionCallAST* rcall, ScopeBodyAST* body
                  , const QString& ws, AST* parent )
            : StatementAST( ws, parent ), m_lCall( lcall ), m_rCall( rcall ), m_body( body ), m_orop( orop )
    {}

    OrAST::~OrAST()
    {
        delete m_lCall;
        m_lCall = 0;
        delete m_rCall;
        m_rCall = 0;
        delete m_body;
        m_body = 0;
    }

    void OrAST::writeToString( QString& buf ) const
    {
        m_lCall->writeToString( buf );
        buf += whitespace();
        buf += m_orop;
        m_rCall->writeToString( buf );
        m_body->writeToString( buf );

    }


    FunctionCallAST* OrAST::leftCall() const
    {
        return m_lCall;
    }

    FunctionCallAST* OrAST::rightCall() const
    {
        return m_rCall;
    }

    ScopeBodyAST* OrAST::scopeBody() const
    {
        return m_body;
    }

    void OrAST::setLeftCall( FunctionCallAST* call )
    {
        m_lCall = call;
    }

    void OrAST::setRightCall( FunctionCallAST* call )
    {
        m_rCall = call;
    }

    void OrAST::setScopeBody( ScopeBodyAST* body )
    {
        m_body = body;
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
            : FunctionArgAST( ws, parent ), m_args( args ),
            m_functionName( functionname ), m_begin( begin ), m_end( end )
    {
    }


    FunctionCallAST::~FunctionCallAST()
    {
        for ( QList<FunctionArgAST*>::const_iterator it = m_args.begin(); it != m_args.end(); ++it )
        {
            delete( *it );
        }
        m_args.clear();
    }

    QString FunctionCallAST::value() const
    {
        QString val;
        writeToString( val );
        return val;
    }

    void FunctionCallAST::writeToString( QString& buf ) const
    {
        FunctionArgAST::writeToString( buf );
        buf += m_functionName;
        buf += m_begin;
        for ( QList<FunctionArgAST*>::const_iterator it = m_args.begin(); it != m_args.end(); )
        {
            if (( *it ) )
                ( *it )->writeToString( buf );
            if ( ++it != m_args.end() )
                buf += ",";
        }
        buf += m_end;
    }


    QList<FunctionArgAST*> FunctionCallAST::arguments() const
    {
        return m_args;
    }

    void FunctionCallAST::insertArgument( int i, FunctionArgAST* arg )
    {
        m_args.insert( i, arg );
    }

    void FunctionCallAST::removeArgument( int i )
    {
        m_args.removeAt( i );
    }

    SimpleFunctionArgAST::SimpleFunctionArgAST( const QString& value, const QString& ws, AST* parent )
            : FunctionArgAST( ws, parent ), m_value( value )
    {}

    SimpleFunctionArgAST::~SimpleFunctionArgAST()
    {}

    QString SimpleFunctionArgAST::value() const
    {
        return m_value;
    }

    void SimpleFunctionArgAST::setValue( const QString& val )
    {
        m_value = val;
    }

    void SimpleFunctionArgAST::writeToString( QString& buf ) const
    {
        FunctionArgAST::writeToString( buf );
        buf += m_value;
    }

}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
