/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   raggi@cli.di.unipi.it                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "lexer.h"
#include <kdebug.h>

using namespace std;

static struct {
    const char* name;
    int id;
} kw_table[] = {

    // KDE keywords -- start
    { "K_DCOP", Token_K_DCOP },
    { "k_dcop", Token_k_dcop },
    // KDE keywords -- end

    // Qt keywords -- start
    { "Q_OBJECT", Token_Q_OBJECT },
    { "signals", Token_signals },
    { "slots", Token_slots },
    { "emit", Token_emit },
    // Qt keywords -- end

    { "and", Token_and },
    { "and_eq", Token_and_eq },
    { "asm", Token_asm },
    { "auto", Token_auto },
    { "bitand", Token_bitand },
    { "bitor", Token_bitor },
    { "bool", Token_bool },
    { "break", Token_break },
    { "case", Token_case },
    { "catch", Token_catch },
    { "char", Token_char },
    { "class", Token_class },
    { "compl", Token_compl },
    { "const", Token_const },
    { "const_cast", Token_const_cast },
    { "continue", Token_continue },
    { "default", Token_default },
    { "delete", Token_delete },
    { "do", Token_do },
    { "double", Token_double },
    { "dynamic_cast", Token_dynamic_cast },
    { "else", Token_else },
    { "enum", Token_enum },
    { "explicit", Token_explicit },
    { "export", Token_export },
    { "extern", Token_extern },
    { "false", Token_false },
    { "float", Token_float },
    { "for", Token_for },
    { "friend", Token_friend },
    { "goto", Token_goto },
    { "if", Token_if },
    { "inline", Token_inline },
    { "int", Token_int },
    { "long", Token_long },
    { "mutable", Token_mutable },
    { "namespace", Token_namespace },
    { "new", Token_new },
    { "not", Token_not },
    { "not_eq", Token_not_eq },
    { "operator", Token_operator },
    { "or", Token_or },
    { "or_eq", Token_or_eq },
    { "private", Token_private },
    { "protected", Token_protected },
    { "public", Token_public },
    { "register", Token_register },
    { "reinterpret_cast", Token_reinterpret_cast },
    { "return", Token_return },
    { "short", Token_short },
    { "signed", Token_signed },
    { "sizeof", Token_sizeof },
    { "static", Token_static },
    { "static_cast", Token_static_cast },
    { "struct", Token_struct },
    { "switch", Token_switch },
    { "template", Token_template },
    { "this", Token_this },
    { "throw", Token_throw },
    { "true", Token_true },
    { "try", Token_try },
    { "typedef", Token_typedef },
    { "typeid", Token_typeid },
    { "typename", Token_typename },
    { "union", Token_union },
    { "unsigned", Token_unsigned },
    { "using", Token_using },
    { "virtual", Token_virtual },
    { "void", Token_void },
    { "volatile", Token_volatile },

//disabled for g++ compatibility(robe)    { "wchar_t", Token_wchar_t },

    { "while", Token_while },
    { "xor", Token_xor },
    { "xor_eq", Token_xor_eq }
};

static struct {
    const char* name;
    int id;
} op_table[] = {
    { "<<=", Token_assign },
    { ">>=", Token_assign },
    { "->*", Token_ptrmem },
    { "...", Token_ellipsis },
    { "::", Token_scope },
    { ".*", Token_ptrmem },
    { "+=", Token_assign },
    { "-=", Token_assign },
    { "*=", Token_assign },
    { "/=", Token_assign },
    { "%=", Token_assign },
    { "^=", Token_assign },
    { "&=", Token_assign },
    { "|=", Token_assign },
    { "<<", Token_shift },
    { ">>", Token_shift },
    { "==", Token_eq },
    { "!=", Token_not_eq },
    { "<=", Token_leq },
    { ">=", Token_geq },
    { "&&", Token_and },
    { "||", Token_or },
    { "++", Token_incr },
    { "--", Token_decr },
    { "->", Token_arrow }
};

Lexer::Lexer()
    : m_recordComments( false ),
      m_recordWhiteSpaces( false )
{
    reset();

    // setup keywords
    for( unsigned int i=0; i< (sizeof(kw_table)/sizeof(kw_table[0])); ++i ){
        m_keywords[ QString(kw_table[i].name) ] = kw_table[ i ].id;
    }

    // setup operators
    for( unsigned int i=0; i< (sizeof(op_table)/sizeof(op_table[0])); ++i ){
        m_operators[ QString(op_table[i].name) ] = op_table[ i ].id;
    }
}

Lexer::~Lexer()
{
}

void Lexer::setSource( const QString& source )
{
    reset();
    m_source = source;
    m_buffer = source.unicode();
    m_endPtr = m_buffer + m_source.length();
    tokenize();
}

void Lexer::reset()
{
    m_index = 0;
    m_size = 0;
    m_lastLine = 0;
    m_tokens.resize( 5000 );
    m_endLines.resize( 5000 );
    m_source = QString::null;
    m_buffer = 0;
    m_endPtr = 0;
    m_startLine = false;
}

void Lexer::newline( const QChar* ptr )
{
    if( m_lastLine == (int)m_endLines.size() ){
        m_endLines.resize( m_endLines.size() + 1000 );
    }

    m_endLines[ m_lastLine++ ] = ptr;
    m_startLine = true;
}

void Lexer::getTokenPosition( const Token& token, int* line, int* col )
{
    const QChar* ptr = token.position();

    *line = m_lastLine;
    *col = 0;

    for( int i=0; i<m_lastLine; ++i ){
        const QChar* endLinePos = m_endLines[ i ];
        if( ptr < endLinePos ){
            *line = i + 1;
            break;
        }
    }
}

void Lexer::tokenize()
{
    const QChar* ptr = m_buffer;

    QMap< QString, int >::Iterator op_it;

    m_startLine = true;

    m_size = 0;
    while( ptr < m_endPtr ){

        if( m_size == (int)m_tokens.size() ){
            m_tokens.resize( m_tokens.size() + 5000 );
        }

        ptr = readWhiteSpaces( ptr );

        int len = ptr - m_buffer;
        QString ch2( ptr, QMIN(len,2) );
        QString ch3( ptr, QMIN(len,3) );

        if( ptr >= m_endPtr ){
            break;
        } else if( m_startLine && *ptr == '#' ){
            // skip for now
            while( ptr<m_endPtr ){
                // skip line
                const QChar* base = ptr;
                while( ptr<m_endPtr && *ptr != '\n' )
                    ++ptr;

                QString line( base, ptr - base );
                line = line.stripWhiteSpace();
                if( !line.endsWith("\\") )
                    break;

                if( ptr < m_endPtr ){
                    ++ptr; // skip \n
                    newline( ptr );
                }
            }
            //m_tokens[ m_size++ ] = Token( Token_preproc, ptr, 1 );
        } else if( *ptr == '\'' ){
            const QChar* end = readCharLiteral( ptr );
            m_tokens[ m_size++ ] = Token( Token_char_literal, ptr, end - ptr );
            ptr = end;
        } else if( *ptr == '"' ){
            const QChar* end = readStringLiteral( ptr );
            m_tokens[ m_size++ ] = Token( Token_string_literal, ptr, end - ptr );
            ptr = end;
        } else if( ptr->isLetter() || *ptr == '_' ){
            const QChar* end = readIdentifier( ptr );
            QString ide( ptr, end - ptr );
            QMap< QString, int >::Iterator it = m_keywords.find( ide );

            if( it != m_keywords.end() ){
                m_tokens[ m_size++ ] = Token( *it, ptr, end - ptr );
            } else {
                QMap< QString, SkipType >::Iterator pos = m_words.find( ide );
                if( pos != m_words.end() ){
                    if( *pos == SkipWordAndArguments ){
                        end = skip( end, '(', ')' );
                    }
                } else
                    m_tokens[ m_size++ ] = Token( Token_identifier, ptr, end - ptr );
            }
            ptr = end;
        } else if( ptr->isNumber() ){
            const QChar* end = readNumberLiteral( ptr );
            m_tokens[ m_size++ ] = Token( Token_number_literal, ptr, end - ptr );
            ptr = end;
        } else if( *ptr == '/' && *(ptr+1) == '/' ){
            const QChar* end = readLineComment( ptr );
            if( recordComments() ){
                m_tokens[ m_size++ ] = Token( Token_comment, ptr, end - ptr );
            }
            ptr = end;
        } else if( *ptr == '/' && *(ptr+1) == '*' ){
            const QChar* end = readMultiLineComment( ptr );
            if( recordComments() ){
                m_tokens[ m_size++ ] = Token( Token_comment, ptr, end - ptr );
            }
            ptr = end;
        } else if( (op_it = m_operators.find(ch3)) != m_operators.end() ){
            m_tokens[ m_size++ ] = Token( *op_it, ptr, 3 );
            ptr += 3;
        } else if( (op_it = m_operators.find(ch2)) != m_operators.end() ){
            m_tokens[ m_size++ ] = Token( *op_it, ptr, 2 );
            ptr += 2;
        } else {
            m_tokens[ m_size++ ] = Token( ptr->latin1(), ptr, 1 );
            ++ptr;
        }

        m_startLine = false;
    }

    m_tokens[ m_size++ ] = Token( Token_eof, ptr, 0 );
}

const QChar* Lexer::readIdentifier( const QChar* ptr )
{
    while( ptr<m_endPtr && (ptr->isLetterOrNumber() || *ptr == '_') )
        ++ptr;

    return ptr;
}

const QChar* Lexer::readWhiteSpaces( const QChar* ptr, bool skipNewLine )
{
    while( ptr<m_endPtr && ptr->isSpace() ){
        if( *ptr == '\n' && !skipNewLine )
            break;

        if( *ptr == '\n' )
            newline( ptr+1 );

        ++ptr;
    }

    return ptr;
}

const QChar* Lexer::readLineComment( const QChar* ptr )
{
    while( ptr<m_endPtr && *ptr != '\n' )
        ++ptr;

    return ptr;
}

const QChar* Lexer::readMultiLineComment( const QChar* ptr )
{
    while( ptr<m_endPtr ){
        int len = m_endPtr - ptr;

        if( QString(ptr,QMIN(len,2)) == "*/" ){
            ptr += 2;
            return ptr;
        } else if( *ptr == '\n' ){
            newline( ptr+1 );
        }

        ++ptr;
    }

    kdDebug(9007) << "Lexer::readMultiLineComment() -- unexpected eof" << endl;
    return ptr;
}

const QChar* Lexer::readCharLiteral( const QChar* ptr )
{
    if( *ptr != '\'' )
        return ptr;

    ++ptr; // skip '

    while( ptr<m_endPtr ){
        int len = m_endPtr - ptr;

        if( len>=2 && (*ptr == '\\' && *(ptr+1) == '\'') ){
            ptr += 2;
        } else if( len>=2 && (*ptr == '\\' && *(ptr+1) == '\\') ){
            ptr += 2;
        } else if( *ptr == '\'' ){
            ++ptr;
            return ptr;
        } else
            ++ptr;
    }

    return ptr;
}

const QChar* Lexer::readStringLiteral( const QChar* ptr )
{
    if( *ptr != '"' )
        return ptr;

    ++ptr; // skip "

    while( ptr<m_endPtr ){
        int len = m_endPtr - ptr;

        if( len>=2 && *ptr == '\\' && *(ptr+1) == '"' ){
            ptr += 2;
        } else if( len>=2 && *ptr == '\\' && *(ptr+1) == '\\' ){
            ptr += 2;
        } else if( *ptr == '"' ){
            ++ptr;
            return ptr;
        } else
            ++ptr;
    }

    kdDebug(9007) << "Lexer::readStringLiteral() -- unexpected eof" << endl;
    return ptr;
}

const QChar* Lexer::readNumberLiteral( const QChar* ptr )
{
#warning "TODO: Lexer::readNumberLiteral()"
    while( ptr<m_endPtr && (ptr->isLetterOrNumber() || *ptr == '.') )
        ++ptr;

    return ptr;
}

void Lexer::resetSkipWords()
{
    m_words.clear();
}

void Lexer::addSkipWord( const QString& word, SkipType skipType )
{
    m_words[ word ] = skipType;
}

const QChar* Lexer::skip( const QChar* ptr, const QChar& l, const QChar& r )
{
    int count = 0;
    while( ptr<m_endPtr ){
        if( *ptr == l )
            ++count;
        else if( *ptr == r )
            --count;

        ++ptr;

        if( count == 0 )
            break;

    }
    return ptr;
}

