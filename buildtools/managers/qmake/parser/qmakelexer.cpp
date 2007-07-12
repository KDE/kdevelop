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
}

int Lexer::state() const
{
    return mState;
}

void Lexer::setState( int state )
{
    mState = state;
}

int Lexer::getNextTokenKind()
{
    int token = parser::Token_EOF;
    if( curpos >= mContentSize )
        return token;
    QChar* it = mContent.data();
    it += curpos;
    qDebug() << "Curpos:" << curpos << " - " << *it << " " << it->toAscii() << "|";

    // First the stuff we want to ignore, that is any whitespace except newline...
    if( state() != QuoteState && it->isSpace() && it->unicode() != '\n' )
    {
        // read all characters until a newline or non-whitespace is hit
        while( it->isSpace() && it->unicode() != '\n' )
        {
            ++it;
            ++curpos;
        }
        qDebug() << "Found whitespace";
    // and any comments, delimited by # and newline
    }else if ( it->unicode() == '#')
    {
        setState(DefaultState);
        // Same thing here, but we also eat whitespace until the eol
        qDebug() << "Found comment";
        while( it->unicode() != '\n' )
        {
            ++it;
            ++curpos;
        }
    }

    mTokenBegin = curpos;
    if( isIdOrValueCharacter(it) )
    {
        token = parser::Token_IDENTIFIER;
        // it runs one in front of curpos, so we increment curpos only when
        // the next character still is part of the identifier
        while( !it->isSpace() && isIdOrValueCharacter( it ) )
        {
            if( !isIdentifierCharacter( it ) )
                token = parser::Token_VALUE;
            it++;
            curpos++;
        }
        qDebug() << "Found identifier";
    }else if( state() == QuoteState && it->isSpace() && it->unicode() != '\n' )
    {
        token = parser::Token_QUOTEDSPACE;
        while( it->isSpace() && it->unicode() != '\n' )
        {
            it++;
            curpos++;
        }
        qDebug() << "Found Quote";
    }else
    {
        //Now the stuff that will generate a proper token
        QChar* c2 = curpos < mContentSize ? it+1 : 0 ;
        switch(it->unicode())
        {
            case '$':
                if( !c2 && c2->unicode() == '$')
                {
                    qDebug() << "Found 2 dollars";
                    curpos++;
                    token = parser::Token_DOUBLEDOLLAR;
                }else
                {
                    qDebug() << "Found 1 dollar";
                    token = parser::Token_SINGLEDOLLAR;
                }
                break;
            case '(':
                qDebug() << "Found lparen";
                token = parser::Token_LPAREN;
                break;
            case ')':
                qDebug() << "Found rparen";
                token = parser::Token_RPAREN;
                break;
            case '[':
                qDebug() << "Found lbracket";
                token = parser::Token_LBRACKET;
                break;
            case ']':
                qDebug() << "Found rbracket";
                token = parser::Token_RBRACKET;
                break;
            case '{':
                qDebug() << "Found lbrace";
                token = parser::Token_LBRACE;
                break;
            case '}':
                qDebug() << "Found rbrace";
                token = parser::Token_RBRACE;
                break;
            case '"':
                qDebug() << "Found rbrace";
                token = parser::Token_RBRACE;
                break;
            case ':':
                qDebug() << "Found colon";
                token = parser::Token_COLON;
                break;
            case ',':
                qDebug() << "Found comma";
                token = parser::Token_COMMA;
                break;
            case '\\':
                if( c2 && c2->isSpace() )
                {
                    qDebug() << "Found cont";
                    token = parser::Token_CONT;
                }else
                {
                    qDebug() << "Found Error";
                    token = parser::Token_ERROR;
                }
                break;
            case '~':
                if( !c2 || c2->unicode() != '=')
                {
                    qDebug() << "Found Error";
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    qDebug() << "Found ~=";
                    token = parser::Token_TILDEEQ;
                }
                break;
            case '*':
                if( !c2 || c2->unicode() != '=')
                {
                    qDebug() << "Found Error";
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    qDebug() << "Found *=";
                    token = parser::Token_STAREQ;
                }
                break;
            case '-':
                if( !c2 || c2->unicode() != '=')
                {
                    qDebug() << "Found Error";
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    qDebug() << "Found -=";
                    token = parser::Token_MINUSEQ;
                }
                break;
            case '+':
                if( !c2 || c2->unicode() != '=')
                {
                    qDebug() << "Found Error";
                    token = parser::Token_ERROR;
                }
                else
                {
                    curpos++;
                    qDebug() << "Found +=";
                    token = parser::Token_PLUSEQ;
                }
                break;
            case '=':
                qDebug() << "Found =";
                token = parser::Token_EQUAL;
                break;
            case '\n':
                qDebug() << "Found newline at" << curpos ;
                mParser->token_stream->location_table()->newline( curpos );
                token = parser::Token_NEWLINE;
                break;
            default:
                token = parser::Token_ERROR;
                break;
        }
        curpos++;
    }
    mTokenEnd = curpos;
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


}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
