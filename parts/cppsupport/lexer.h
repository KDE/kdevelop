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

#ifndef LEXER_H
#define LEXER_H

#include <qstring.h>
#include <qmemarray.h>
#include <qmap.h>
#include <qvaluestack.h>

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

class Token{
public:
    Token();
    Token( int type, const QChar* position, int length );
    Token( const Token& source );

    Token& operator = ( const Token& source );
    bool operator == ( const Token& token ) const;
    operator int () const;

    QString toString() const;

    bool isNull() const;

    int type() const;
    void setType( int type );

    void getStartPosition( int* line, int* column ) const;
    void setStartPosition( int line, int column );
    void getEndPosition( int* line, int* column ) const;
    void setEndPosition( int line, int column );

    unsigned int length() const;
    void setLength( unsigned int length );

    bool startsWith( const QChar& ch ) const;

protected:
    const QChar* position() const;
    void setPosition( const QChar* position );

private:
    int m_type;
    const QChar* m_position;
    int m_length;
    int m_startLine;
    int m_startColumn;
    int m_endLine;
    int m_endColumn;

    friend class Lexer;
    friend class Parser;
}; // class Token

class Lexer {
public:
    Lexer();
    ~Lexer();

    bool recordComments() const;
    void setRecordComments( bool record );

    bool recordWhiteSpaces() const;
    void setRecordWhiteSpaces( bool record );

    void resetSkipWords();
    void addSkipWord( const QString& word, SkipType skipType=SkipWord );

    QString source() const;
    void setSource( const QString& source );

    int index() const;
    void setIndex( int index );

    void reset();

    int tokenPosition( const Token& token ) const;
    const Token& tokenAt( int position ) const;
    const Token& nextToken();
    const Token& lookAhead( int n ) const;

    void getTokenPosition( const Token& token, int* line, int* col );

private:
    void tokenize();
    void newline( const QChar* ptr );
    const QChar* skip( const QChar* ptr, const QChar& l, const QChar& r );
    const QChar* readIdentifier( const QChar* ptr);
    const QChar* readWhiteSpaces( const QChar* ptr, bool skipNewLine=true );
    const QChar* readLineComment( const QChar* ptr );
    const QChar* readMultiLineComment( const QChar* ptr );
    const QChar* readCharLiteral( const QChar* ptr );
    const QChar* readStringLiteral( const QChar* ptr );
    const QChar* readNumberLiteral( const QChar* ptr );
    const QChar* handleDirective( const QString& directive, const QChar* ptr );

private:
    QMemArray< Token > m_tokens;
    int m_size;
    QMemArray< const QChar* > m_startLineVector;
    int m_lastLine;
    QMap< QString, int > m_keywords;
    QMap< QString, int > m_operators;
    QMap< QString, SkipType > m_words;
    int m_index;
    QString m_source;
    const QChar* m_buffer;
    const QChar* m_endPtr;
    bool m_recordComments;
    bool m_recordWhiteSpaces;
    bool m_startLine;
    QValueStack<int> m_directiveStack;
    
    int m_currentLine;
    int m_currentColumn;
};


inline Token::Token()
    : m_type( -1 ),
      m_position( 0 ),
      m_length( 0 )
{
}

inline Token::Token( int type, const QChar* position, int length )
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

inline QString Token::toString() const
{
    return QString( m_position, m_length );
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

inline const QChar* Token::position() const
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

inline void Token::setPosition( const QChar* position )
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

inline bool Token::startsWith( const QChar& ch ) const
{
    return *m_position == ch;
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
    return token.position() - m_buffer;
}



#endif
