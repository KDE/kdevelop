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

//@TODO: Lex and parse ! in default state

Lexer::Lexer( parser* _parser, const QString& content ):
        mContent(content), mParser(_parser),
        mCurpos(0), mContentSize(mContent.size()),
        mTokenBegin(0), mTokenEnd(0), mInQuote( false )
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
    QStringList lines = mContent.split("\n");
    QRegExp commentRE("^[^#]+#[^\n]*$");
    QRegExp fullLineCommentRE("^[ \t]*#[^\n]*$");
    int pos = 0;
    int lineendpos = 0;
    foreach( QString line, lines )
    {
        lineendpos = pos+line.size();
        mParser->token_stream->location_table()->newline(lineendpos);
        if( line.contains( fullLineCommentRE ) )
        {
            for( int i = pos; i < lineendpos+1 ; ++i)
            {
                mContent[i] = QChar(' ');
            }
        }else if( line.contains( commentRE ) )
        {
            int commentpos = pos+line.indexOf("#");
            for( int i = commentpos; i < lineendpos ; ++i )
            {
                mContent[i] = QChar(' ');
            }
        }
        pos = lineendpos+1;
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
    if( mCurpos >= mContentSize )
    {
        return parser::Token_EOF;
    }
    QChar* it = mContent.data();
    it += mCurpos;
    switch( state() )
    {
        case VariableValueState:
            // Fall through as these two states are very similar
        case FunctionArgState:
            if(!mInQuote)
            {
                it = ignoreWhitespace(it);
            }
            mTokenBegin = mCurpos;
            if( it->isSpace() && it->unicode() != '\n' && mInQuote )
            {
                while( it->isSpace() && it->unicode() != '\n' )
                {
                    ++it;
                    ++mCurpos;
                }
                mCurpos--;
                token = parser::Token_QUOTEDSPACE;
            }else
            {
                QChar* c2 = mCurpos < mContentSize ? it+1 : 0 ;
                switch(it->unicode())
                {
                    case '$':
                        if( c2->unicode() == '$')
                        {
                            ++mCurpos;
                            token = parser::Token_DOUBLEDOLLAR;
                        }else
                        {
                            token = parser::Token_SINGLEDOLLAR;
                        }
                        break;
                    case '(':
                        pushState( FunctionArgState );
                        token = parser::Token_LPAREN;
                        break;
                    case ')':
                        while( state() != FunctionArgState && state() != ErrorState )
                        {
                            popState();
                        }
                        popState();
                        token = parser::Token_RPAREN;
                        break;
                    case '{':
                        token = parser::Token_LBRACE;
                        break;
                    case '}':
                        token = parser::Token_RBRACE;
                        break;
                    case '[':
                        token = parser::Token_LBRACKET;
                        break;
                    case ']':
                        token = parser::Token_RBRACKET;
                        break;
                    case ',':
                        if( state() == FunctionArgState )
                        {
                            token = parser::Token_COMMA;
                        }else
                        {
                            token = parser::Token_IDENTIFIER;
                            while( !it->isSpace() && ( it->unicode() != ',' && !mInQuote )  )
                            {
                                if( !Lexer::isIdentifierCharacter( it ) )
                                {
                                    token = parser::Token_VALUE;
                                }
                                ++it;
                                ++mCurpos;
                            }
                            mCurpos--;
                        }
                        break;
                    case '"':
                        token = parser::Token_QUOTE;
                        mInQuote = !mInQuote;
                        break;
                    case '\n':
                        mInQuote = false;
                        token = parser::Token_NEWLINE;
                        popState();
                        break;
                    case '\\':
                        if( c2->isSpace() && !mInQuote )
                        {
                            pushState( ContState );
                            token = parser::Token_CONT;
                            break;
                        }else if( mInQuote && c2->isSpace() )
                        {
                            if( c2->unicode() == '\n' )
                            {
                                token = parser::Token_CONT;
                                pushState( ContState );
                                break;
                            }else
                            {
                                QChar* wsit = c2;
                                while( wsit->isSpace() && wsit->unicode() != '\n' )
                                {
                                    wsit++;
                                }
                                if( wsit->unicode() == '\n' )
                                {
                                    token = parser::Token_CONT;
                                    pushState( ContState );
                                    break;
                                }
                            }
                        }
                        // There's no break here by purpose, if \ is found
                        // before whitespace in quote or any other character
                        // it is part of a value so fall through to the default
                        // case.
                    default:
                        token = parser::Token_IDENTIFIER;
                        while( !it->isSpace()
                                && !isSpecialValueCharacter(it)
                                && !Lexer::isCont(it) )
                        {
                            if( !Lexer::isIdentifierCharacter( it ) )
                            {
                                token = parser::Token_VALUE;
                            }
                            ++it;
                            ++mCurpos;
                        }
                        mCurpos--;
                        break;
                }
            }
            break;
        case ContState:
            it = ignoreWhitespace(it);
            mTokenBegin = mCurpos;
            if( it->unicode() == '\n' )
            {
                mInQuote = false;
                token = parser::Token_NEWLINE;
                popState();
            }
            break;
        case DefaultState:
            it = ignoreWhitespace(it);
            mTokenBegin = mCurpos;
            if( Lexer::isIdentifierCharacter(it) )
            {
                token = parser::Token_IDENTIFIER;
                while( !it->isSpace() && Lexer::isIdentifierCharacter( it ) )
                {
                    it++;
                    mCurpos++;
                }
                mCurpos--;
            }else
            {
                //Now the stuff that will generate a proper token
                QChar* c2 = mCurpos < mContentSize ? it+1 : 0 ;
                switch(it->unicode())
                {
                    case '(':
                        pushState( FunctionArgState );
                        token = parser::Token_LPAREN;
                        break;
                    case ')':
                        while( state() != FunctionArgState && state() != ErrorState )
                        {
                            popState();
                        }
                        popState();
                        token = parser::Token_RPAREN;
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
                        if( c2 && c2->unicode() == '=')
                        {
                            pushState( VariableValueState );
                            mCurpos++;
                            token = parser::Token_TILDEEQ;
                        }
                        break;
                    case '*':
                        if( c2 && c2->unicode() == '=')
                        {
                            pushState( VariableValueState );
                            mCurpos++;
                            token = parser::Token_STAREQ;
                        }
                        break;
                    case '-':
                        if( c2 && c2->unicode() == '=')
                        {
                            pushState( VariableValueState );
                            mCurpos++;
                            token = parser::Token_MINUSEQ;
                        }
                        break;
                    case '+':
                        if( c2 && c2->unicode() == '=')
                        {
                            pushState( VariableValueState );
                            mCurpos++;
                            token = parser::Token_PLUSEQ;
                        }
                        break;
                    case '=':
                        pushState( VariableValueState );
                        token = parser::Token_EQUAL;
                        break;
                    case '\n':
                        mInQuote = false;
                        token = parser::Token_NEWLINE;
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }
    if( mCurpos >= mContentSize )
    {
        return parser::Token_EOF;
    }
    mTokenEnd = mCurpos;
    mCurpos++;
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

bool Lexer::isSpecialValueCharacter(QChar* c)
{
    return(
               c->unicode() == '$'
            || c->unicode() == '{'
            || c->unicode() == '}'
            || c->unicode() == '('
            || c->unicode() == ')'
            || c->unicode() == '['
            || c->unicode() == ']'
            || ( state() == FunctionArgState && c->unicode() == ',' )
            || c->unicode() == '"'
          );
}

bool Lexer::isIdentifierCharacter(QChar* c)
{
    return (   c->isLetter()
            || c->isDigit()
            || c->unicode() == '_'
            || c->unicode() == '.'
           );
}

bool Lexer::isCont(QChar* c)
{
    if( c->unicode() == '\\' )
    {
        c++;
        while( c->isSpace() && c->unicode() != '\n' )
        {
            c++;
        }
        if( c->unicode() == '\n' )
        {
            return true;
        }
    }
    return false;
}

QChar* Lexer::ignoreWhitespace(QChar* it)
{
    // Ignore whitespace, but preserve the newline
    while( mCurpos < mContentSize && ( ( !mInQuote || state() == ContState ) && it->isSpace() && it->unicode() != '\n' ) )
    {
        ++it;
        ++mCurpos;
    }
    return it;
}

}

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
