/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LEXER_H
#define LEXER_H

#include "driver.h"

#include <qstring.h>
#include <qmemarray.h>
#include <qmap.h>
#include <qvaluestack.h>
#include <qpair.h>

enum Type {
    Token_eof = 0,
    Token_identifier = 1000,
    Token_number_literal,
    Token_char_literal,
    Token_string_literal,
    Token_whitespaces,
    Token_comment,
    Token_preproc,

    Token_assign = 2000,
    Token_ptrmem,
    Token_ellipsis,
    Token_scope,
    Token_shift,
    Token_eq,
    Token_leq,
    Token_geq,
    Token_incr,
    Token_decr,
    Token_arrow,

    Token_K_DCOP,
    Token_k_dcop,
    Token_k_dcop_signals,

    Token_Q_OBJECT,
    Token_signals,
    Token_slots,
    Token_emit,

    Token_and,
    Token_and_eq,
    Token_asm,
    Token_auto,
    Token_bitand,
    Token_bitor,
    Token_bool,
    Token_break,
    Token_case,
    Token_catch,
    Token_char,
    Token_class,
    Token_compl,
    Token_const,
    Token_const_cast,
    Token_continue,
    Token_default,
    Token_delete,
    Token_do,
    Token_double,
    Token_dynamic_cast,
    Token_else,
    Token_enum,
    Token_explicit,
    Token_export,
    Token_extern,
    Token_false,
    Token_float,
    Token_for,
    Token_friend,
    Token_goto,
    Token_if,
    Token_inline,
    Token_int,
    Token_long,
    Token_mutable,
    Token_namespace,
    Token_new,
    Token_not,
    Token_not_eq,
    Token_operator,
    Token_or,
    Token_or_eq,
    Token_private,
    Token_protected,
    Token_public,
    Token_register,
    Token_reinterpret_cast,
    Token_return,
    Token_short,
    Token_signed,
    Token_sizeof,
    Token_static,
    Token_static_cast,
    Token_struct,
    Token_switch,
    Token_template,
    Token_this,
    Token_throw,
    Token_true,
    Token_try,
    Token_typedef,
    Token_typeid,
    Token_typename,
    Token_union,
    Token_unsigned,
    Token_using,
    Token_virtual,
    Token_void,
    Token_volatile,
    Token_wchar_t,
    Token_while,
    Token_xor,
    Token_xor_eq
};

enum SkipType {
    SkipWord,
    SkipWordAndArguments
};

class Token
{
public:
    Token();
    Token( int type, int position, int length );
    Token( const Token& source );

    Token& operator = ( const Token& source );
    bool operator == ( const Token& token ) const;
    operator int () const;

    bool isNull() const;

    int type() const;
    void setType( int type );

    void getStartPosition( int* line, int* column ) const;
    void setStartPosition( int line, int column );
    void getEndPosition( int* line, int* column ) const;
    void setEndPosition( int line, int column );

    unsigned int length() const;
    void setLength( unsigned int length );

    int position() const;
    void setPosition( int position );

private:
    int m_type;
    int m_position;
    int m_length;
    int m_startLine;
    int m_startColumn;
    int m_endLine;
    int m_endColumn;

    friend class Lexer;
    friend class Parser;
}; // class Token

class Lexer 
{
public:
    Lexer( Driver* driver );
    ~Lexer();

    bool recordComments() const;
    void setRecordComments( bool record );

    bool recordWhiteSpaces() const;
    void setRecordWhiteSpaces( bool record );

    bool skipWordsEnabled() const;
    void enableSkipWords();
    void disableSkipWords();
    
    void resetSkipWords();
    void addSkipWord( const QString& word, SkipType skipType=SkipWord, const QString& str = QString::null );

    QString source() const;
    void setSource( const QString& source );

    int index() const;
    void setIndex( int index );

    void reset();

    const Token& tokenAt( int position ) const;
    const Token& nextToken();
    const Token& lookAhead( int n ) const;

    QString toString( const Token& token ) const;

    int tokenPosition( const Token& token ) const;
    void getTokenPosition( const Token& token, int* line, int* col );
    
protected:
    int nextToken( int pos, Token& token );

private:
    void tokenize();
    bool isValid( int pos ) const;
    void newline( int pos );
    void nextChar( int& ptr );
    void nextChar( int& ptr, int n );
    int skip( int pos, const QChar& l, const QChar& r );
    int readIdentifier( int pos);
    int readWhiteSpaces( int pos, bool skipNewLine=true );
    int readLineComment( int pos );
    int readMultiLineComment( int pos );
    int readCharLiteral( int pos );
    int readStringLiteral( int pos );
    int readNumberLiteral( int pos );
    int findOperator3( int pos );
    int findOperator2( int pos );
    int handleDirective( const QString& directive, int pos );
    void addToken( const Token& tk );

private:
    Driver* m_driver;
    QMemArray< Token > m_tokens;
    int m_size;
    QMemArray< int > m_startLineVector;
    int m_lastLine;
    QMap< QString, QPair<SkipType, QString> > m_words;
    int m_index;
    QString m_source;
    int m_endPtr;
    bool m_recordComments;
    bool m_recordWhiteSpaces;
    bool m_startLine;
    QValueStack<int> m_directiveStack;
    
    int m_currentLine;
    int m_currentColumn;
    bool m_skipWordsEnabled;
    bool m_skipping;
};


inline Token::Token()
    : m_type( -1 ),
      m_position( 0 ),
      m_length( 0 )
{
}

inline Token::Token( int type, int position, int length )
    : m_type( type ),
      m_position( position ),
      m_length( length )
{
}

inline Token::Token( const Token& source )
    : m_type( source.m_type ),
      m_position( source.m_position ),
      m_length( source.m_length ),
      m_startLine( source.m_startLine ),
      m_startColumn( source.m_startColumn ),
      m_endLine( source.m_endLine ),
      m_endColumn( source.m_endColumn )
{
}

inline Token& Token::operator = ( const Token& source )
{
    m_type = source.m_type;
    m_position = source.m_position;
    m_length = source.m_length;
    m_startLine = source.m_startLine;
    m_startColumn = source.m_startColumn;
    m_endLine = source.m_endLine;
    m_endColumn = source.m_endColumn;
    return( *this );
}

inline Token::operator int () const
{
    return m_type;
}

inline bool Token::operator == ( const Token& token ) const
{
    return m_type == token.m_type &&
       m_position == token.m_position &&
         m_length == token.m_length &&
      m_startLine == token.m_startLine &&
    m_startColumn == token.m_startColumn &&
        m_endLine == token.m_endLine &&
      m_endColumn == token.m_endColumn;
}

inline bool Token::isNull() const
{
    return m_type == Token_eof || m_length == 0;
}

inline int Token::type() const
{
    return m_type;
}

inline void Token::setType( int type )
{
    m_type = type;
}

inline int Token::position() const
{
    return m_position;
}

inline void Token::setStartPosition( int line, int column )
{
    m_startLine = line;
    m_startColumn = column;
}

inline void Token::setEndPosition( int line, int column )
{
    m_endLine = line;
    m_endColumn = column;
}

inline void Token::getStartPosition( int* line, int* column ) const
{
    if( line ) *line = m_startLine;
    if( column ) *column = m_startColumn;
}

inline void Token::getEndPosition( int* line, int* column ) const
{
    if( line ) *line = m_endLine;
    if( column ) *column = m_endColumn;
}

inline void Token::setPosition( int position )
{
    m_position = position;
}

inline unsigned int Token::length() const
{
    return m_length;
}

inline void Token::setLength( unsigned int length )
{
    m_length = length;
}

inline bool Lexer::recordComments() const
{
    return m_recordComments;
}

inline void Lexer::setRecordComments( bool record )
{
    m_recordComments = record;
}

inline bool Lexer::recordWhiteSpaces() const
{
    return m_recordWhiteSpaces;
}

inline void Lexer::setRecordWhiteSpaces( bool record )
{
    m_recordWhiteSpaces = record;
}

inline QString Lexer::source() const
{
    return m_source;
}

inline int Lexer::index() const
{
    return m_index;
}

inline void Lexer::setIndex( int index )
{
    m_index = index;
}

inline const Token& Lexer::nextToken()
{
    if( m_index < m_size )
        return m_tokens[ m_index++ ];

    return m_tokens[ m_index ];
}

inline const Token& Lexer::tokenAt( int n ) const
{
    return m_tokens[ QMIN(n, m_size-1) ];
}

inline const Token& Lexer::lookAhead( int n ) const
{
    return m_tokens[ QMIN(m_index + n, m_size-1) ];
}

inline int Lexer::tokenPosition( const Token& token ) const
{
    return token.position();
}

inline bool Lexer::isValid( int ptr ) const
{
    return ptr < m_endPtr;
}

inline void Lexer::newline( int ptr )
{
    if( m_lastLine == (int)m_startLineVector.size() ){
        m_startLineVector.resize( m_startLineVector.size() + 1000 );
    }

    m_startLineVector[ m_lastLine++ ] = ptr;

    m_currentLine = m_lastLine;
    m_currentColumn = 0;
    m_startLine = true;
}

inline void Lexer::nextChar( int& ptr )
{
    if(m_source[ptr] == '\n') {
	++ptr;
	newline( ptr );
    } else {
	++m_currentColumn;
	++ptr;
    }
}

inline void Lexer::nextChar( int& ptr, int n )
{
    m_currentColumn += n;
    ptr += n;
}

inline int Lexer::readIdentifier( int ptr )
{
    while( isValid(ptr) && (m_source[ptr].isLetterOrNumber() || m_source[ptr] == '_') )
        nextChar( ptr );

    return ptr;
}

inline int Lexer::readWhiteSpaces( int ptr, bool skipNewLine )
{
    while( isValid(ptr) && m_source[ptr].isSpace() ){
        if( m_source[ptr] == '\n' && !skipNewLine )
            break;

        nextChar( ptr );
    }

    return ptr;
}

inline int Lexer::readLineComment( int ptr )
{
    while( isValid(ptr) && m_source[ptr] != '\n' )
        nextChar( ptr );

    return ptr;
}

inline int Lexer::readMultiLineComment( int ptr )
{
    while( isValid(ptr) ){
        if( isValid(ptr+1) && m_source[ptr] == '*' && m_source[ptr+1] == '/' ){
            nextChar( ptr, 2 );
            return ptr;
        }
        nextChar( ptr );
    }

    return ptr;
}

inline int Lexer::readCharLiteral( int ptr )
{
    if( m_source[ptr] != '\'' )
        return ptr;

    nextChar( ptr ); // skip '

    while( isValid(ptr) ){
        int len = m_endPtr - ptr;

        if( len>=2 && (m_source[ptr] == '\\' && m_source[ptr+1] == '\'') ){
            nextChar( ptr, 2 );
        } else if( len>=2 && (m_source[ptr] == '\\' && m_source[ptr+1] == '\\') ){
            nextChar( ptr, 2 );
        } else if( m_source[ptr] == '\'' ){
            nextChar( ptr );
            return ptr;
        } else {
	    nextChar( ptr );
	}
    }

    return ptr;
}

inline int Lexer::readStringLiteral( int ptr )
{
    if( m_source[ptr] != '"' )
        return ptr;

    nextChar( ptr ); // skip "

    while( isValid(ptr) ){
        int len = m_endPtr - ptr;

        if( len>=2 && m_source[ptr] == '\\' && m_source[ptr+1] == '"' ){
            nextChar( ptr, 2 );
        } else if( len>=2 && m_source[ptr] == '\\' && m_source[ptr+1] == '\\' ){
            nextChar( ptr, 2 );
        } else if( m_source[ptr] == '"' ){
            nextChar( ptr );
            return ptr;
        } else {
	    nextChar( ptr );
	}
    }

    return ptr;
}

inline int Lexer::readNumberLiteral( int ptr )
{
    while( isValid(ptr) && (m_source[ptr].isLetterOrNumber() || m_source[ptr] == '.') )
        nextChar( ptr );

    return ptr;
}

inline int Lexer::findOperator3( int ptr )
{
    int n = int(m_endPtr - ptr);

    if( n >= 3){
	if( m_source[ptr] == '<' && m_source[ptr+1] == '<' && m_source[ptr+2] == '=' ) return Token_assign;
	else if( m_source[ptr] == '>' && m_source[ptr+1] == '<' && m_source[ptr+2] == '=' ) return Token_assign; 
	else if( m_source[ptr] == '-' && m_source[ptr+1] == '>' && m_source[ptr+2] == '*' ) return Token_ptrmem; 
	else if( m_source[ptr] == '.' && m_source[ptr+1] == '.' && m_source[ptr+2] == '.' ) return Token_ellipsis;
    } 
    
    return -1;
}

inline int Lexer::findOperator2( int ptr )
{
    int n = int(m_endPtr - ptr);

    if( n>=2 ){
	if( m_source[ptr] == ':' && m_source[ptr+1] == ':' ) return Token_scope;
	else if( m_source[ptr] == '.' && m_source[ptr+1] == '*' ) return Token_ptrmem;
	else if( m_source[ptr] == '+' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '-' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '*' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '/' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '%' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '^' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '&' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '|' && m_source[ptr+1] == '=' ) return Token_assign;
	else if( m_source[ptr] == '<' && m_source[ptr+1] == '<' ) return Token_shift;
	else if( m_source[ptr] == '>' && m_source[ptr+1] == '>' ) return Token_shift;
	else if( m_source[ptr] == '=' && m_source[ptr+1] == '=' ) return Token_eq;
	else if( m_source[ptr] == '!' && m_source[ptr+1] == '=' ) return Token_eq;
	else if( m_source[ptr] == '<' && m_source[ptr+1] == '=' ) return Token_leq;
	else if( m_source[ptr] == '>' && m_source[ptr+1] == '=' ) return Token_geq;
	else if( m_source[ptr] == '&' && m_source[ptr+1] == '&' ) return Token_and;
	else if( m_source[ptr] == '|' && m_source[ptr+1] == '|' ) return Token_or;
	else if( m_source[ptr] == '+' && m_source[ptr+1] == '+' ) return Token_incr;
	else if( m_source[ptr] == '-' && m_source[ptr+1] == '-' ) return Token_decr;
	else if( m_source[ptr] == '-' && m_source[ptr+1] == '>' ) return Token_arrow;
    }
    
    return -1;
}

inline bool Lexer::skipWordsEnabled() const
{
    return m_skipWordsEnabled;
}

inline void Lexer::enableSkipWords()
{
    m_skipWordsEnabled = true;
}

inline void Lexer::disableSkipWords()
{
    m_skipWordsEnabled = false;
}

inline QString Lexer::toString( const Token& token ) const
{
    return m_source.mid( token.position(), token.length() );
}

inline void Lexer::addToken( const Token& tk )
{
    if( !m_skipping )
	m_tokens[ m_size++ ] = tk;
}


#endif
