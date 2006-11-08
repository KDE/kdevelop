/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef LEXER_H
#define LEXER_H

#include "driver.h"

#include <qstring.h>
#include <qmap.h>
#include <qvaluestack.h>
#include <qpair.h>
#include <qptrvector.h>
#include <hashedstring.h>
#include <ext/hash_map>

#define CHARTYPE QChar

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

    Token_concat,

    Token_K_DCOP,
    Token_k_dcop,
    Token_k_dcop_signals,

    Token_Q_OBJECT,
    Token_signals,
    Token_slots,
    Token_emit,
    Token_foreach, // qt4 [erbsland]

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

struct LexerData;

class Token
{
    Token(const QString &);
    Token( int type, int position, int length, const QString& text );
    Token( const Token& source );

    Token& operator = ( const Token& source );
    bool operator == ( const Token& token ) const;
    operator int () const;

public:
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

    QString text() const;

private:
    int m_type;
    int m_position;
    int m_length;
    int m_startLine;
    int m_startColumn;
    int m_endLine;
    int m_endColumn;
    const QString & m_text;

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

    bool reportWarnings() const;
    void setReportWarnings( bool enable );

    bool reportMessages() const;
    void setReportMessages( bool enable );

    bool skipWordsEnabled() const;
    void setSkipWordsEnabled( bool enabled );

    bool preprocessorEnabled() const;
    void setPreprocessorEnabled( bool enabled );

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

    static int toInt( const Token& token );

    int tokenPosition( const Token& token ) const;
    void getTokenPosition( const Token& token, int* line, int* col );

    int currentLine() const { return m_currentLine; }
    int currentColumn() const { return m_currentColumn; }

    inline const CHARTYPE* offset( int offset ) const {
        return m_source.unicode() + offset;
    }

    inline int getOffset( const QChar* p ) const {
        return int(p - (m_source.unicode()));
    }
    
private:
    const QChar currentChar() const;
    QChar peekChar( int n=1 ) const;
    int currentPosition() const;

    void insertCurrent( const QString& str );
    
    void tokenize();
    void nextToken( Token& token, bool stopOnNewline=false );
    void nextChar();
    void nextChar( int n );
    void skip( int l, int r );
    void readIdentifier();
    void readWhiteSpaces( bool skipNewLine=true, bool skipOnlyOnce=false );
    void readLineComment();
    void readMultiLineComment();
    void readCharLiteral();
    void readStringLiteral();
    void readNumberLiteral();

    int findOperator3() const;
    int findOperator2() const;
    bool eof() const;

    // preprocessor (based on an article of Al Stevens on Dr.Dobb's journal)
    int testIfLevel();
    int macroDefined();
    QString readArgument();

    int macroPrimary();
    int macroMultiplyDivide();
    int macroAddSubtract();
    int macroRelational();
    int macroEquality();
    int macroBoolAnd();
    int macroBoolXor();
    int macroBoolOr();
    int macroLogicalAnd();
    int macroLogicalOr();
    int macroExpression();

    void handleDirective( const QString& directive );
    void processDefine( Macro& macro );
    void processElse();
    void processElif();
    void processEndif();
    void processIf();
    void processIfdef();
    void processIfndef();
    void processInclude();
    void processUndef();

private:
    LexerData* d;
    Driver* m_driver;
    QPtrVector< Token > m_tokens;
    int m_size;
    int m_index;
    QString m_source;
    const QChar* m_ptr;
    const QChar* m_endPtr;
    QChar m_currentChar;
    bool m_recordComments;
    bool m_recordWhiteSpaces;
    bool m_startLine;
    __gnu_cxx::hash_map< HashedString, QPair<SkipType, QString> > m_words;

    
    
    int m_currentLine;
    int m_currentColumn;
    bool m_skipWordsEnabled;

    // preprocessor
    QMemArray<bool> m_skipping;
    QMemArray<bool> m_trueTest;
    int m_ifLevel;
    bool m_preprocessorEnabled;
    bool m_inPreproc;

    bool m_reportWarnings;
    bool m_reportMessages;

private:
    Lexer( const Lexer& source );
    void operator = ( const Lexer& source );
};


inline Token::Token(const QString & text = "")
    : m_type( -1 ),
      m_position( 0 ),
      m_length( 0 ),
      m_text( text )
{
}

inline Token::Token( int type, int position, int length, const QString& text )
    : m_type( type ),
      m_position( position ),
      m_length( length ),
      m_text( text )
{
}

inline Token::Token( const Token& source )
    : m_type( source.m_type ),
      m_position( source.m_position ),
      m_length( source.m_length ),
      m_startLine( source.m_startLine ),
      m_startColumn( source.m_startColumn ),
      m_endLine( source.m_endLine ),
      m_endColumn( source.m_endColumn ),
      m_text( source.m_text )
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
//    m_text = source.m_text;
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
      m_endColumn == token.m_endColumn &&
           m_text == token.m_text;
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

inline QString Token::text() const
{
    return m_text.mid(m_position, m_length);
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
        return *m_tokens[ m_index++ ];

    return *m_tokens[ m_index ];
}

inline const Token& Lexer::tokenAt( int n ) const
{
    return *m_tokens[ QMIN(n, m_size-1) ];
}

inline const Token& Lexer::lookAhead( int n ) const
{
    return *m_tokens[ QMIN(m_index + n, m_size-1) ];
}

inline int Lexer::tokenPosition( const Token& token ) const
{
    return token.position();
}

inline void Lexer::nextChar()
{
    if(*m_ptr == '\n') {
        ++m_currentLine;
        m_currentColumn = 0;
        m_startLine = true;
    } else {
	++m_currentColumn;
    }
    ++m_ptr;

    if( m_ptr <  m_endPtr )
        m_currentChar = *m_ptr;
    else
        m_currentChar = QChar::null;

}

inline void Lexer::nextChar( int n )
{
    m_currentColumn += n;
    m_ptr += n;
    
    if( m_ptr <  m_endPtr )
        m_currentChar = *m_ptr;
    else
        m_currentChar = QChar::null;
}

inline void Lexer::readIdentifier()
{
    while( currentChar().isLetterOrNumber() || currentChar() == '_' )
        nextChar();
}

inline void Lexer::readWhiteSpaces( bool skipNewLine, bool skipOnlyOnce )
{
    while( !currentChar().isNull() ){
        QChar ch = currentChar();

        if( ch == '\n' && !skipNewLine ){
            break;
        } else if( ch.isSpace() ){
            nextChar();
        } else if( m_inPreproc && currentChar() == '\\' ){
            nextChar();
            readWhiteSpaces( true, true );
        } else {
            break;
        }
        if( skipOnlyOnce && ch == '\n' ) {
          skipNewLine = false;
        }
    }
}

//little hack for better performance
inline bool isTodo( const QString& txt, int position ) {
    if( txt.length() < position + 4 ) return false;
    return (txt[ position ] == 't' || txt[ position ] == 'T')
            && (txt[ position+1 ] == 'o' || txt[ position+1 ] == 'O')
            && (txt[ position+2 ] == 'd' || txt[ position+2 ] == 'D')
            && (txt[ position+3 ] == 'o' || txt[ position+3 ] == 'O');
}

inline bool isFixme( const QString& txt, int position ) {
    if( txt.length() < position + 5 ) return false;
    return (txt[ position ] == 'f' || txt[ position ] == 'F')
            && (txt[ position+1 ] == 'i' || txt[ position+1 ] == 'I')
            && (txt[ position+2 ] == 'x' || txt[ position+2 ] == 'X')
            && (txt[ position+3 ] == 'm' || txt[ position+3 ] == 'M')
            && (txt[ position+4 ] == 'e' || txt[ position+4 ] == 'E');
}

inline void Lexer::readLineComment()
{
    while( !currentChar().isNull() && currentChar() != '\n' ){
        if( m_reportMessages && isTodo( m_source, currentPosition() ) ){
	    nextChar( 4 );
	    QString msg;
	    int line = m_currentLine;
	    int col = m_currentColumn;

	    while( currentChar() ){
		if( currentChar() == '*' && peekChar() == '/' )
		    break;
		else if( currentChar() == '\n' )
		    break;

		msg += currentChar();
		nextChar();
	    }
	    m_driver->addProblem( m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Todo) );
	} else
        if( m_reportMessages && isFixme( m_source, currentPosition() ) ){
            nextChar( 5 );
            QString msg;
            int line = m_currentLine;
            int col = m_currentColumn;

            while( currentChar() ){
            if( currentChar() == '*' && peekChar() == '/' )
                break;
            else if( currentChar() == '\n' )
                break;

            msg += currentChar();
            nextChar();
            }
            m_driver->addProblem( m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Fixme) );
        } else
            nextChar();
    }
}

inline void Lexer::readMultiLineComment()
{
    while( !currentChar().isNull() ){
        if( currentChar() == '*' && peekChar() == '/' ){
            nextChar( 2 );
            return;
        } else if( m_reportMessages && isTodo( m_source, currentPosition() ) ){
	    nextChar( 4 );
	    QString msg;
	    int line = m_currentLine;
	    int col = m_currentColumn;

	    while( currentChar() ){
		if( currentChar() == '*' && peekChar() == '/' )
		    break;
		else if( currentChar() == '\n' )
		    break;
		msg += currentChar();
		nextChar();
	    }
	    m_driver->addProblem( m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Todo) );
        } else
            if( m_reportMessages && isFixme( m_source, currentPosition() ) ) {
            nextChar( 5 );
            QString msg;
            int line = m_currentLine;
            int col = m_currentColumn;

            while( currentChar() ){
            if( currentChar() == '*' && peekChar() == '/' )
                break;
            else if( currentChar() == '\n' )
                break;

            msg += currentChar();
            nextChar();
            }
            m_driver->addProblem( m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Fixme) );
        } else
            nextChar();
    }
}

inline void Lexer::readCharLiteral()
{
    if( currentChar() == '\'' )
        nextChar(); // skip '
    else if( currentChar() == 'L' && peekChar() == '\'' )
	nextChar( 2 ); // slip L'
    else
        return;

    while( !currentChar().isNull() ){
        int len = getOffset( m_endPtr ) - currentPosition();

        if( len>=2 && (currentChar() == '\\' && peekChar() == '\'') ){
            nextChar( 2 );
        } else if( len>=2 && (currentChar() == '\\' && peekChar() == '\\') ){
            nextChar( 2 );
        } else if( currentChar() == '\'' ){
            nextChar();
            break;
        } else {
	    nextChar();
	}
    }
}

inline void Lexer::readStringLiteral()
{
    if( currentChar() != '"' )
        return;

    nextChar(); // skip "

    while( !currentChar().isNull() ){
        int len = getOffset( m_endPtr ) - currentPosition();

        if( len>=2 && currentChar() == '\\' && peekChar() == '"' ){
            nextChar( 2 );
        } else if( len>=2 && currentChar() == '\\' && peekChar() == '\\' ){
            nextChar( 2 );
        } else if( currentChar() == '"' ){
            nextChar();
            break;
        } else {
	    nextChar();
	}
    }
}

inline void Lexer::readNumberLiteral()
{
    while( currentChar().isLetterOrNumber() || currentChar() == '.' )
        nextChar();
}

inline int Lexer::findOperator3() const
{
    int n = getOffset( m_endPtr ) - currentPosition();

    if( n >= 3){
	QChar ch = currentChar(), ch1=peekChar(), ch2=peekChar(2);

	if( ch == '<' && ch1 == '<' && ch2 == '=' ) return Token_assign;
	else if( ch == '>' && ch1 == '>' && ch2 == '=' ) return Token_assign;
	else if( ch == '-' && ch1 == '>' && ch2 == '*' ) return Token_ptrmem;
	else if( ch == '.' && ch1 == '.' && ch2 == '.' ) return Token_ellipsis;
    }

    return -1;
}

inline int Lexer::findOperator2() const
{
    int n = getOffset( m_endPtr ) - currentPosition();

    if( n>=2 ){
	QChar ch = currentChar(), ch1=peekChar();

	if( ch == ':' && ch1 == ':' ) return Token_scope;
	else if( ch == '.' && ch1 == '*' ) return Token_ptrmem;
	else if( ch == '+' && ch1 == '=' ) return Token_assign;
	else if( ch == '-' && ch1 == '=' ) return Token_assign;
	else if( ch == '*' && ch1 == '=' ) return Token_assign;
	else if( ch == '/' && ch1 == '=' ) return Token_assign;
	else if( ch == '%' && ch1 == '=' ) return Token_assign;
	else if( ch == '^' && ch1 == '=' ) return Token_assign;
	else if( ch == '&' && ch1 == '=' ) return Token_assign;
	else if( ch == '|' && ch1 == '=' ) return Token_assign;
	else if( ch == '<' && ch1 == '<' ) return Token_shift;
	else if( ch == '>' && ch1 == '>' ) return Token_shift;
	else if( ch == '=' && ch1 == '=' ) return Token_eq;
	else if( ch == '!' && ch1 == '=' ) return Token_eq;
	else if( ch == '<' && ch1 == '=' ) return Token_leq;
	else if( ch == '>' && ch1 == '=' ) return Token_geq;
	else if( ch == '&' && ch1 == '&' ) return Token_and;
	else if( ch == '|' && ch1 == '|' ) return Token_or;
	else if( ch == '+' && ch1 == '+' ) return Token_incr;
	else if( ch == '-' && ch1 == '-' ) return Token_decr;
	else if( ch == '-' && ch1 == '>' ) return Token_arrow;
	else if( ch == '#' && ch1 == '#' ) return Token_concat;
    }

    return -1;
}

inline bool Lexer::skipWordsEnabled() const
{
    return m_skipWordsEnabled;
}

inline void Lexer::setSkipWordsEnabled( bool enabled )
{
    m_skipWordsEnabled = enabled;
}

inline bool Lexer::preprocessorEnabled() const
{
    return m_preprocessorEnabled;
}

inline void Lexer::setPreprocessorEnabled( bool enabled )
{
    m_preprocessorEnabled = enabled;
}

inline int Lexer::currentPosition() const
{
    return getOffset( m_ptr );
}

inline const QChar Lexer::currentChar() const
{
    return m_currentChar;
}

inline QChar Lexer::peekChar( int n ) const
{
    const QChar* p = m_ptr + n;
    
    if( p <  m_endPtr )
        return *p;
    else
        return QChar::null;
}

inline bool Lexer::eof() const
{
    return m_ptr >= m_endPtr;
}

inline bool Lexer::reportWarnings() const
{
    return m_reportWarnings;
}

inline void Lexer::setReportWarnings( bool enable )
{
    m_reportWarnings = enable;
}

inline bool Lexer::reportMessages() const
{
    return m_reportMessages;
}

inline void Lexer::setReportMessages( bool enable )
{
    m_reportMessages = enable;
}

inline void Lexer::insertCurrent( const QString& str ) {
    int posi = currentPosition();
    m_source.insert( posi, str );

    m_ptr = offset( posi );
    m_endPtr = offset( m_source.length() );
    if( m_ptr < m_endPtr )
      m_currentChar = *m_ptr;
}

#endif
