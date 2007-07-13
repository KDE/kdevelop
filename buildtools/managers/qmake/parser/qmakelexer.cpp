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

#include <QtCore/QString>
#include <QtCore/QDebug>
#include "qmakelexer.h"
#include "qmake_parser.h"
#include <kdev-pg-location-table.h>
#include <kdev-pg-token-stream.h>

namespace QMake
{


Lexer::Lexer( parser* _parser, const QString& content ):
        mContent(content), mParser(_parser),
        curpos(0), mContentSize(mContent.size()),
        mTokenBegin(0), mTokenEnd(0)
{
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
    int token = parser::Token_EOF;
    if( curpos >= mContentSize )
        return token;
    QChar* it = mContent.data();
    it += curpos;

    // Ignore whitespace and comments, but preserve the newline in a comment
    // that was started after a CONT marker
    while( curpos < mContentSize-1 && isWhitespaceOrComment(it) )
    {
        if( it->unicode() == '#' )
        {
            if( state() == QuoteState )
                popState();
            pushState(CommentState);
            ++curpos;
            ++it;
        }else if( it->unicode() == '\n' )
        {
            if( state() == CommentState )
            {
                popState();
                if( state() != ContState )
                {
                    ++curpos;
                    ++it;
                }
            }else
                break;
        }else
        {
            ++it;
            ++curpos;
        }
    }

    if( state() == ContState )
    {
        popState();
    }
    mTokenBegin = curpos;
    if( Lexer::isIdOrValueCharacter(it) )
    {
        token = parser::Token_IDENTIFIER;
        // it runs one in front of curpos, so we increment curpos only when
        // the next character still is part of the identifier
        while( !it->isSpace() && Lexer::isIdOrValueCharacter( it ) )
        {
            if( !Lexer::isIdentifierCharacter( it ) )
            {
                token = parser::Token_VALUE;
            }
            it++;
            curpos++;
        }
        curpos--;
    }else if( state() == QuoteState && it->isSpace() && it->unicode() != '\n' )
    {
        token = parser::Token_QUOTEDSPACE;
        while( it->isSpace() && it->unicode() != '\n' )
        {
            it++;
            curpos++;
        }
        curpos--;
    }else
    {
        //Now the stuff that will generate a proper token
        QChar* c2 = curpos < mContentSize ? it+1 : 0 ;
        switch(it->unicode())
        {
            case '$':
                if( !c2 && c2->unicode() == '$')
                {
                    curpos++;
                    token = parser::Token_DOUBLEDOLLAR;
                }else
                {
                    token = parser::Token_SINGLEDOLLAR;
                }
                break;
            case '(':
                token = parser::Token_LPAREN;
                break;
            case ')':
                token = parser::Token_RPAREN;
                break;
            case '[':
                token = parser::Token_LBRACKET;
                break;
            case ']':
                token = parser::Token_RBRACKET;
                break;
            case '{':
                token = parser::Token_LBRACE;
                break;
            case '}':
                token = parser::Token_RBRACE;
                break;
            case '"':
                pushState( QuoteState );
                token = parser::Token_QUOTE;
                break;
            case ':':
                token = parser::Token_COLON;
                break;
            case ',':
                token = parser::Token_COMMA;
                break;
            case '\\':
                if( c2 && ( c2->isSpace() || c2->unicode() == '\n' ) )
                {
                    qDebug() << "Found Cont at: " << curpos << *(mContent.data()+curpos);
                    pushState( ContState );
                    token = parser::Token_CONT;
                }else if( c2 )
                {
                    curpos++;
                    token = parser::Token_VALUE;
                }else
                {
                    token = parser::Token_ERROR;
                }
                break;
            case '~':
                if( !c2 || c2->unicode() != '=')
                {
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    token = parser::Token_TILDEEQ;
                }
                break;
            case '*':
                if( !c2 || c2->unicode() != '=')
                {
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    token = parser::Token_STAREQ;
                }
                break;
            case '-':
                if( !c2 || c2->unicode() != '=')
                {
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    token = parser::Token_MINUSEQ;
                }
                break;
            case '+':
                if( !c2 || c2->unicode() != '=')
                {
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    token = parser::Token_PLUSEQ;
                }
                break;
            case '=':
                token = parser::Token_EQUAL;
                break;
            case '\n':
                mParser->token_stream->location_table()->newline( curpos );
                token = parser::Token_NEWLINE;
                break;
            default:
                token = parser::Token_ERROR;
                break;
        }
    }
    mTokenEnd = curpos;
    curpos++;
    return token;
}

std::size_t Lexer::getTokenBegin() const
{
    return mTokenBegin;
}

std::size_t Lexer::getTokenEnd() const
{
    return mTokenEnd;
}

bool Lexer::isIdOrValueCharacter(QChar* c)
{
    return (   c->isLetter()
            || c->isDigit()
            || c->unicode() == '_'
            || c->unicode() == '.'
            || c->unicode() == '-'
            || c->unicode() == '/'
           );
}

bool Lexer::isIdentifierCharacter(QChar* c)
{
    return (   c->isLetter()
            || c->isDigit()
            || c->unicode() == '_'
            || c->unicode() == '.'
            || c->unicode() == '-'
           );
}

bool Lexer::isWhitespaceOrComment(QChar* c)
{
    return (
            ( state() != QuoteState && c->isSpace() && c->unicode() != '\n' )
            || ( state() == CommentState )
            || ( c->unicode() == '#' )
           );
}

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
