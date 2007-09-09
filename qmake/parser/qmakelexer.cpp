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

bool isIdentifierCharacter( QChar* c )
{
    return ( c->isLetter()
             || c->isDigit()
             || c->unicode() == '_'
             || c->unicode() == '.'
             || c->unicode() == '-'
             || c->unicode() == '$'
           );
}

bool isBeginIdentifierCharacter( QChar* c )
{
    return ( c->isLetter()
             || c->isDigit()
             || c->unicode() == '_'
             || c->unicode() == '.'
             || c->unicode() == '$'
           );
}

bool isEndIdentifierCharacter( QChar* c )
{
    return ( c->isLetter()
             || c->isDigit()
             || c->unicode() == '_'
             || c->unicode() == '.'
             || c->unicode() == '$'
           );
}


bool isCont( QChar* c )
{
    if ( c->unicode() == '\\' )
    {
        c++;
        while ( c->isSpace() && c->unicode() != '\n' )
        {
            c++;
        }
        if ( c->unicode() == '\n' || c->unicode() == '#' )
        {
            return true;
        }
    }
    return false;
}


Lexer::Lexer( parser* _parser, const QString& content ):
        m_content( content ), m_parser( _parser ),
        m_curpos( 0 ), m_contentSize( m_content.size() ),
        m_tokenBegin( 0 ), m_tokenEnd( 0 )
{
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

int Lexer::nextTokenKind()
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
            it = ignoreWhitespaceAndComment( it );
            m_tokenBegin = m_curpos;
            if( m_curpos < m_contentSize )
            {
                if( it->unicode() == '\n' )
                {
                    popState();
                    createNewline( m_curpos );
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
                    while( ( it->unicode() != '"' || lastit->unicode() == '\\' && it->unicode() == '"' ) && it->unicode() != '\n' && it->unicode() != '#' && !isCont( it ) && m_curpos < m_contentSize )
                    {
                        lastit = it;
                        it++;
                        m_curpos++;
                    }
                    if( it->unicode() != '"' && it->unicode() != '#' )
                    {
                        m_curpos--;
                    }
                    token = parser::Token_VALUE;
                    if( it->unicode() == '#' )
                    {
                        m_tokenEnd = m_curpos - 1;
                        do
                        {
                            it++;
                            m_curpos++;
                        }while( it->unicode() != '\n' && m_curpos < m_contentSize );
                        if( it->unicode() == '\n')
                        {
                            m_curpos--;
                        }
                        return token;
                    }
                }else if( it->unicode() == '(' )
                {
                    unsigned int bracecount = 0;
                    while( ( it->unicode() != ';' || bracecount > 0 ) && it->unicode() != '\n' && !isCont( it )  && m_curpos < m_contentSize )
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
                    while( !it->isSpace() && !isCont(it) && it->unicode() != '#' && m_curpos < m_contentSize )
                    {
                        it++;
                        m_curpos++;
                    }
                    m_curpos--;
                    token = parser::Token_VALUE;
                }
            }
            break;
        case FunctionArgState:
            m_tokenBegin = m_curpos;
            if( it->unicode() == '\n' )
            {
                createNewline( m_curpos );
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
            it = ignoreWhitespaceAndComment( it );
            m_tokenBegin = m_curpos;
            if( m_curpos < m_contentSize )
            {
                if ( it->unicode() == '\n' )
                {
                    createNewline( m_curpos );
                    token = parser::Token_NEWLINE;
                    m_tokenEnd = m_curpos;
                    popState();
                    QChar* temp = it;
                    int newpos = m_curpos;
                    do
                    {
                        temp++;
                        newpos++;
                        if(temp->unicode() == '#' )
                        {
                            while( temp->unicode() != '\n' && newpos < m_contentSize )
                            {
                                temp++;
                                newpos++;
                            }
                            createNewline( m_curpos );
                            temp++;
                            m_curpos = newpos;
                            newpos++;
                        }
                    }while( temp->isSpace() && temp->unicode() != '\n' && m_curpos < m_contentSize );
                    m_curpos++;
                    return token;
                }
            }
            break;
        case DefaultState:
            it = ignoreWhitespaceAndComment( it );
            m_tokenBegin = m_curpos;
            if( m_curpos < m_contentSize )
            {
                if ( isBeginIdentifierCharacter( it ) )
                {
                    token = parser::Token_IDENTIFIER;
                    while ( !it->isSpace() && isIdentifierCharacter( it )  && m_curpos < m_contentSize )
                    {
                        it++;
                        m_curpos++;
                    }
                    if( !isEndIdentifierCharacter( ( it-1 ) ) )
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
                            createNewline( m_curpos );
                            token = parser::Token_NEWLINE;
                            break;
                        default:
                            break;
                    }
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

std::size_t Lexer::tokenBegin() const
{
    return m_tokenBegin;
}

std::size_t Lexer::tokenEnd() const
{
    return m_tokenEnd;
}

QChar* Lexer::ignoreWhitespaceAndComment( QChar* it )
{
    // Ignore whitespace, but preserve the newline
    bool comment = false;
    while ( m_curpos < m_contentSize && ( it->isSpace() || comment || it->unicode() == '#' ) && it->unicode() != '\n' )
    {
        if( it->unicode() == '#' )
        {
            comment = true;
        }
        ++it;
        ++m_curpos;
    }
    return it;
}

void Lexer::createNewline( int pos )
{
    if( m_parser )
        m_parser->token_stream->location_table()->newline( pos );
}

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
