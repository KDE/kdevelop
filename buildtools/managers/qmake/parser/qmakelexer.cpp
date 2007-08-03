/* KDevelop QMake Support
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

#include "qmakelexer.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include "qmake_parser.h"
#include <kdev-pg-location-table.h>
#include <kdev-pg-token-stream.h>

namespace QMake
{

/*
 * @TODO: functions do not always completely ignore whitespace:
 *        message( foo   bar  )
 *        this prints "foo   bar" on the output
 */

Lexer::Lexer( parser* _parser, const QString& content ):
        m_content( content ), m_parser( _parser ),
        m_curpos( 0 ), m_contentSize( m_content.size() ),
        m_tokenBegin( 0 ), m_tokenEnd( 0 )
{
    // preprocess input.
    // this is needed because comments make the lexer harder, unfortunately
    // we can't treat all comments the same, comments that fill a whole line
    // need to be removed completely including the newline. Those that
    // occur after some content need to be removed, but the newline needs
    // to be preserved.
    //
    // Examples of file contents that need this:
    // "FOO = bar \ <newline>   # some comment <newline> othervalue \n"
    // this is a variable FOO with the two values bar and othervalue.
    //
    // Unfortunately I didn't see a way to replace the parts of the content
    // string with whitespace and thus I have to iterate replacing each QChar
    // individually, if somebody knows a better way I'm open to suggestions
    //
    // Also this algorithm replaces newlines so we need to add the linebreaks
    // to the location table here.
    QStringList lines = m_content.split( "\n" );
    QRegExp commentRE( "^[^#]+#[^\n]*$" );
    QRegExp fullLineCommentRE( "^[ \t]*#[^\n]*$" );
    int pos = 0;
    int lineendpos = 0;
    foreach( QString line, lines )
    {
        lineendpos = pos + line.size();
        m_parser->token_stream->location_table()->newline( lineendpos );
        if ( line.contains( fullLineCommentRE ) )
        {
            for ( int i = pos; i < lineendpos + 1 ; ++i )
            {
                m_content[i] = QChar( ' ' );
            }
        }
        else if ( line.contains( commentRE ) )
        {
            int commentpos = pos + line.indexOf( "#" );
            for ( int i = commentpos; i < lineendpos ; ++i )
            {
                m_content[i] = QChar( ' ' );
            }
        }
        pos = lineendpos + 1;
    }
    pushState( ErrorState );
    pushState( DefaultState );
}

int Lexer::state() const
{
    return mState.top();
}

void Lexer::pushState( int state )
{
    mState.push( state );
}

void Lexer::popState()
{
    mState.pop();
}

int Lexer::getNextTokenKind()
{
    int token = parser::Token_INVALID;
    if ( m_curpos >= m_contentSize )
    {
        return 0;
    }
    QChar* it = m_content.data();
    it += m_curpos;
    switch ( state() )
    {
        case VariableValueState:
            it = ignoreWhitespace( it );
            m_tokenBegin = m_curpos;
            if( it->unicode() == '\n' )
            {
                popState();
                token = parser::Token_NEWLINE;
            }else if( it->unicode() == '\\' && isCont(it) )

            {
                pushState(ContState);
                token = parser::Token_CONT;
            }else if( it->unicode() == '"')
            {
                it++;
                m_curpos++;
                QChar* lastit = it;
                while( ( it->unicode() != '"' || lastit->unicode() == '\\' && it->unicode() == '"' ) && it->unicode() != '\n' && !isCont( it ) )
                {
                    lastit = it;
                    it++;
                    m_curpos++;
                }
                if( it->unicode() != '"' )
                {
                    m_curpos--;
                }
                token = parser::Token_VALUE;
            }else if( it->unicode() == '(' )
            {
                unsigned int bracecount = 0;
                while( ( it->unicode() != ';' || bracecount > 0 ) && it->unicode() != '\n' && !isCont( it ) )
                {
                    if( it->unicode() == '(' )
                    {
                        bracecount++;
                    }else if( it->unicode() == ')' && bracecount > 0 )
                    {
                        bracecount--;
                    }
                    ++it;
                    ++m_curpos;
                }
                if( it->unicode() != ';' )
                {
                    m_curpos--;
                }
                token = parser::Token_VALUE;
            }else
            {
                while( !it->isSpace() && !isCont(it) && m_curpos < m_contentSize )
                {
                    it++;
                    m_curpos++;
                }
                m_curpos--;
                token = parser::Token_VALUE;
            }
            break;
        case FunctionArgState:
            m_tokenBegin = m_curpos;
            if( it->unicode() == '\n' )
            {
                token = parser::Token_NEWLINE;
            }else if( it->unicode() == '\\' && isCont(it) )
            {
                pushState( ContState );
                token = parser::Token_CONT;
            }else if( it->unicode() == ',' )
            {
                token = parser::Token_COMMA;
            }else if( it->unicode() == ')' )
            {
                popState();
                token = parser::Token_RPAREN;
            }else
            {
                unsigned int parentCount = 0;
                while( parentCount > 0 || ( it->unicode() != ')' && it->unicode() != ',' ) && m_curpos < m_contentSize )
                {
                    if( it->unicode() == ')' )
                    {
                        parentCount--;
                    }else if( it->unicode() == '(' )
                    {
                        parentCount++;
                    }
                    ++it;
                    ++m_curpos;
                }
                m_curpos--;
                token = parser::Token_VALUE;
            }
            break;
        case ContState:
            it = ignoreWhitespace( it );
            m_tokenBegin = m_curpos;
            if ( it->unicode() == '\n' )
            {
                token = parser::Token_NEWLINE;
                popState();
            }
            break;
        case DefaultState:
            it = ignoreWhitespace( it );
            m_tokenBegin = m_curpos;
            if ( Lexer::isBeginIdentifierCharacter( it ) )
            {
                token = parser::Token_IDENTIFIER;
                while ( !it->isSpace() && Lexer::isIdentifierCharacter( it ) )
                {
                    it++;
                    m_curpos++;
                }
                if( !Lexer::isEndIdentifierCharacter( ( it-1 ) ) )
                {
                    token = parser::Token_INVALID;
                }
                m_curpos--;
            }
            else
            {
                //Now the stuff that will generate a proper token
                QChar* c2 = m_curpos < m_contentSize ? it + 1 : 0 ;
                switch ( it->unicode() )
                {
                    case '|':
                        token = parser::Token_OR;
                        break;
                    case '!':
                        token = parser::Token_EXCLAM;
                        break;
                    case '(':
                        pushState( FunctionArgState );
                        token = parser::Token_LPAREN;
                        break;
                    case '{':
                        token = parser::Token_LBRACE;
                        break;
                    case '}':
                        token = parser::Token_RBRACE;
                        break;
                    case ':':
                        token = parser::Token_COLON;
                        break;
                    case '~':
                        if ( c2 && c2->unicode() == '=' )
                        {
                            pushState( VariableValueState );
                            m_curpos++;
                            token = parser::Token_TILDEEQ;
                        }
                        break;
                    case '*':
                        if ( c2 && c2->unicode() == '=' )
                        {
                            pushState( VariableValueState );
                            m_curpos++;
                            token = parser::Token_STAREQ;
                        }
                        break;
                    case '-':
                        if ( c2 && c2->unicode() == '=' )
                        {
                            pushState( VariableValueState );
                            m_curpos++;
                            token = parser::Token_MINUSEQ;
                        }
                        break;
                    case '+':
                        if ( c2 && c2->unicode() == '=' )
                        {
                            pushState( VariableValueState );
                            m_curpos++;
                            token = parser::Token_PLUSEQ;
                        }
                        break;
                    case '=':
                        pushState( VariableValueState );
                        token = parser::Token_EQUAL;
                        break;
                    case '\n':
                        token = parser::Token_NEWLINE;
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            token = parser::Token_INVALID;
            break;
    }
    if ( m_curpos >= m_contentSize )
    {
        return 0;
    }
    m_tokenEnd = m_curpos;
    m_curpos++;
    return token;
}

std::size_t Lexer::getTokenBegin() const
{
    return m_tokenBegin;
}

std::size_t Lexer::getTokenEnd() const
{
    return m_tokenEnd;
}

bool Lexer::isIdentifierCharacter( QChar* c )
{
    return ( c->isLetter()
             || c->isDigit()
             || c->unicode() == '_'
             || c->unicode() == '.'
             || c->unicode() == '-'
             || c->unicode() == '$'
           );
}

bool Lexer::isBeginIdentifierCharacter( QChar* c )
{
    return ( c->isLetter()
             || c->isDigit()
             || c->unicode() == '_'
             || c->unicode() == '.'
             || c->unicode() == '$'
           );
}

bool Lexer::isEndIdentifierCharacter( QChar* c )
{
    return ( c->isLetter()
             || c->isDigit()
             || c->unicode() == '_'
             || c->unicode() == '.'
             || c->unicode() == '$'
           );
}


bool Lexer::isCont( QChar* c )
{
    if ( c->unicode() == '\\' )
    {
        c++;
        while ( c->isSpace() && c->unicode() != '\n' )
        {
            c++;
        }
        if ( c->unicode() == '\n' )
        {
            return true;
        }
    }
    return false;
}

QChar* Lexer::ignoreWhitespace( QChar* it )
{
    // Ignore whitespace, but preserve the newline
    while ( m_curpos < m_contentSize && it->isSpace() && it->unicode() != '\n' )
    {
        ++it;
        ++m_curpos;
    }
    return it;
}

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
