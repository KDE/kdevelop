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

#include "lexer.h"

#include <qregexp.h>
#include <kdebug.h>

#define NEXT_CHAR( ptr ) { if(*ptr == '\n') {++ptr; newline(ptr);} else {++m_currentColumn; ++ptr;} }
#define NEXT_CHAR_N( ptr, n ) { m_currentColumn += (n); ptr += (n); }

using namespace std;

static struct {
    const char* name;
    int id;
} kw_table[] = {

    // KDE keywords -- start
    { "K_DCOP", Token_K_DCOP },
    { "k_dcop", Token_k_dcop },
    { "k_dcop_signals", Token_k_dcop_signals },
    // KDE keywords -- end

    // Qt keywords -- start
    { "Q_OBJECT", Token_Q_OBJECT },
    { "signals", Token_signals },
    { "slots", Token_slots },
    { "emit", Token_emit },
    // Qt keywords -- end

    { "__asm__", Token_asm },
    { "__int64", Token_int }, 
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
//    { "false", Token_false },
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
//    { "true", Token_true },
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

enum PreProcessorState
{
    PreProc_none,
	    
    // states
    PreProc_in_group,
    PreProc_skip
};


Lexer::Lexer( Driver* driver )
    : m_driver( driver ),
      m_recordComments( false ),
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
    m_startLineVector.resize( 5000 );
    m_source = QString::null;
    m_buffer = 0;
    m_endPtr = 0;
    m_startLine = false;
    
    m_currentLine = 0;
    m_currentColumn = 0;
}

void Lexer::newline( const QChar* ptr )
{
    if( m_lastLine == (int)m_startLineVector.size() ){
        m_startLineVector.resize( m_startLineVector.size() + 1000 );
    }
        
    m_startLineVector[ m_lastLine++ ] = ptr;
    m_startLine = true;        
    
    m_currentLine = m_lastLine;
    m_currentColumn = 0;        
}
  
void Lexer::getTokenPosition( const Token& token, int* line, int* col )
{
    token.getStartPosition( line, col );
}

void Lexer::tokenize()
{
    const QChar* ptr = m_buffer;
    
    QMap< QString, int >::Iterator op_it;
    QRegExp qt_rx( "Q[A-Z]{1,2}_[A-Z_]+" );
    QRegExp typelist_rx( "K_TYPELIST_[0-9]+" );

    m_startLine = true;

    m_size = 0;
    while( isValid(ptr) ){

	int preproc_state = PreProc_none;

	if( m_directiveStack.size() ){
	    preproc_state = m_directiveStack.top();
	}

	if( m_size == (int)m_tokens.size() ){
	    m_tokens.resize( m_tokens.size() + 5000 );
	}

	ptr = readWhiteSpaces( ptr );

	int len = ptr - m_buffer;
	QString ch2( ptr, QMIN(len,2) );
	QString ch3( ptr, QMIN(len,3) );

	int startLine = m_currentLine;
	int startColumn = m_currentColumn;
	
	if( !isValid(ptr) ){
	    break;
	} else if( *ptr == '/' && *(ptr+1) == '/' ){
	    const QChar* end = readLineComment( ptr );
	    if( recordComments() ){
		Token tk = Token( Token_comment, ptr, end - ptr );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
		m_tokens[ m_size++ ] = tk;
	    }
	    ptr = end;
	} else if( *ptr == '/' && *(ptr+1) == '*' ){
	    const QChar* end = readMultiLineComment( ptr );
	    if( recordComments() ){
		Token tk = Token( Token_comment, ptr, end - ptr );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
		m_tokens[ m_size++ ] = tk;
	    }
	    ptr = end;
	} else if( m_startLine && *ptr == '#' ){
	    
	    NEXT_CHAR( ptr ); // skip #
	    ptr = readWhiteSpaces( ptr, false );	    // skip white spaces
	    
	    const QChar* eptr = readIdentifier( ptr ); // read the directive
	    QString directive( ptr, eptr - ptr );	    
	    
	    ptr = handleDirective( directive, eptr );
	    //m_tokens[ m_size++ ] = Token( Token_preproc, ptr, 1 );	    
	} else if( preproc_state == PreProc_skip ){
	    // skip line and continue
	    while( isValid(ptr) && !ptr->isNull() && *ptr != '\n' )
		NEXT_CHAR( ptr );
	    continue;
	} else if( *ptr == '\'' ){
	    const QChar* end = readCharLiteral( ptr );
	    Token tk = Token( Token_char_literal, ptr, end - ptr );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	    ptr = end;
	} else if( *ptr == '"' ){
	    const QChar* end = readStringLiteral( ptr );
	    Token tk = Token( Token_string_literal, ptr, end - ptr );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	    ptr = end;
	} else if( ptr->isLetter() || *ptr == '_' ){
	    const QChar* end = readIdentifier( ptr );
	    QString ide( ptr, end - ptr );
	    QMap< QString, int >::Iterator it = m_keywords.find( ide );
	    
	    if( it != m_keywords.end() ){
		Token tk = Token( *it, ptr, end - ptr );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
		m_tokens[ m_size++ ] = tk;
	    } else {
		QMap< QString, SkipType >::Iterator pos = m_words.find( ide );
		if( pos != m_words.end() ){
		    if( *pos == SkipWordAndArguments ){
			end = skip( readWhiteSpaces(end, false), '(', ')' );
		    }
		} else if( /*qt_rx.exactMatch(ide) ||*/ ide.endsWith("EXPORT") || ide.startsWith("Q_EXPORT") || ide.startsWith("QM_EXPORT") || ide.startsWith("QM_TEMPLATE")){
		    end = skip( readWhiteSpaces(end, false), '(', ')' );
		} else if( typelist_rx.exactMatch(ide) ){
		    Token tk = Token( Token_identifier, ptr, end - ptr );
		    tk.setStartPosition( startLine, startColumn );
		    tk.setEndPosition( m_currentLine, m_currentColumn );
		    m_tokens[ m_size++ ] = tk;
		    end = skip( readWhiteSpaces(end, false), '(', ')' );
		} else{
		    Token tk = Token( Token_identifier, ptr, end - ptr );
		    tk.setStartPosition( startLine, startColumn );
		    tk.setEndPosition( m_currentLine, m_currentColumn );
		    m_tokens[ m_size++ ] = tk;
		}
	    }
	    ptr = end;
	} else if( ptr->isNumber() ){
	    const QChar* end = readNumberLiteral( ptr );
	    Token tk = Token( Token_number_literal, ptr, end - ptr );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	    ptr = end;
	} else if( (op_it = m_operators.find(ch3)) != m_operators.end() ){
	    Token tk = Token( *op_it, ptr, 3 );
	    NEXT_CHAR_N( ptr, 3 );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	} else if( (op_it = m_operators.find(ch2)) != m_operators.end() ){
	    Token tk = Token( *op_it, ptr, 2 );
	    NEXT_CHAR_N( ptr, 2 );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	} else {
	    Token tk = Token( ptr->latin1(), ptr, 1 );
	    NEXT_CHAR( ptr );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	}
	
	m_startLine = false;
    }
    
    Token tk = Token( Token_eof, ptr, 0 );
    tk.setEndPosition( m_currentLine, m_currentColumn );
    m_tokens[ m_size++ ] = tk;
}

const QChar* Lexer::readIdentifier( const QChar* ptr )
{
    while( isValid(ptr) && (ptr->isLetterOrNumber() || *ptr == '_') )
        NEXT_CHAR( ptr );

    return ptr;
}

const QChar* Lexer::readWhiteSpaces( const QChar* ptr, bool skipNewLine )
{
    while( isValid(ptr) && ptr->isSpace() ){
        if( *ptr == '\n' && !skipNewLine )
            break;

        NEXT_CHAR( ptr );
    }

    return ptr;
}

const QChar* Lexer::readLineComment( const QChar* ptr )
{
    while( isValid(ptr) && *ptr != '\n' )
        NEXT_CHAR( ptr );

    return ptr;
}

const QChar* Lexer::readMultiLineComment( const QChar* ptr )
{
    while( isValid(ptr) ){
        int len = m_endPtr - ptr;

        if( QString(ptr,QMIN(len,2)) == "*/" ){
            NEXT_CHAR_N( ptr, 2 );
            return ptr;
        }

        NEXT_CHAR( ptr );
    }

    return ptr;
}

const QChar* Lexer::readCharLiteral( const QChar* ptr )
{
    if( *ptr != '\'' )
        return ptr;

    NEXT_CHAR( ptr ); // skip '

    while( isValid(ptr) ){
        int len = m_endPtr - ptr;

        if( len>=2 && (*ptr == '\\' && *(ptr+1) == '\'') ){
            NEXT_CHAR_N( ptr, 2 );
        } else if( len>=2 && (*ptr == '\\' && *(ptr+1) == '\\') ){
            NEXT_CHAR_N( ptr, 2 );
        } else if( *ptr == '\'' ){
            NEXT_CHAR( ptr );
            return ptr;
        } else {
	    NEXT_CHAR( ptr );
	}
    }

    return ptr;
}

const QChar* Lexer::readStringLiteral( const QChar* ptr )
{
    if( *ptr != '"' )
        return ptr;

    NEXT_CHAR( ptr ); // skip "

    while( isValid(ptr) ){
        int len = m_endPtr - ptr;

        if( len>=2 && *ptr == '\\' && *(ptr+1) == '"' ){
            NEXT_CHAR_N( ptr, 2 );
        } else if( len>=2 && *ptr == '\\' && *(ptr+1) == '\\' ){
            NEXT_CHAR_N( ptr, 2 );
        } else if( *ptr == '"' ){
            NEXT_CHAR( ptr );
            return ptr;
        } else {
	    NEXT_CHAR( ptr );
	}
    }

    return ptr;
}

const QChar* Lexer::readNumberLiteral( const QChar* ptr )
{
#warning "TODO: Lexer::readNumberLiteral()"
    while( isValid(ptr) && (ptr->isLetterOrNumber() || *ptr == '.') )
        NEXT_CHAR( ptr );

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

    if( isValid(ptr) && *ptr != l )
        return ptr;

    while( isValid(ptr) ){
        if( *ptr == l )
            ++count;
        else if( *ptr == r )
            --count;

        NEXT_CHAR( ptr );

        if( count == 0 )
            break;
    }

    return ptr;
}

const QChar* Lexer::handleDirective( const QString& directive, const QChar* ptr )
{
    //kdDebug(9007) << "handle directive " << directive << endl;
    
    if( directive == "include" ){
	ptr = readWhiteSpaces( ptr, false );
	if( isValid(ptr) && *ptr == '"' || *ptr == '<' ){
	    QChar ch = *ptr++;
	    QChar ch2 = ch == QChar('"') ? QChar('"') : QChar('>');
	 
	    const QChar* startWord = ptr;
	    while( isValid(ptr) && *ptr != ch2 )
		++ptr;
	    if( isValid(ptr) ){
		QString word( startWord, int(ptr-startWord) );
		m_driver->addDependence( m_driver->currentFileName(), 
					 Dependence(word, ch == '"' ? Dep_Local : Dep_Global) );
		++ptr;
	    }
	}
    } else if( directive == "define" ){
	ptr = readWhiteSpaces( ptr, false );
	if( isValid(ptr) ) {
	    Macro m;
	    
	    const QChar* startMacroName = ptr;
	    ptr = readIdentifier( ptr );
	    QString macroName( startMacroName, int(ptr-startMacroName) );
	    m.setName( macroName );
	    
	    ptr = readWhiteSpaces( ptr, false );
	    if( isValid(ptr) && *ptr == '(' ){
		m.setHasArguments( true );
		++ptr;
		
		while( isValid(ptr) && *ptr != ')' ){
		    ptr = readWhiteSpaces( ptr, false );
		    const QChar* startArg = ptr;
		    ptr = readIdentifier( ptr );
		    QString arg( startArg, int(ptr-startArg) );
		    
		    m.addArgument( arg );
		    
		    ptr = readWhiteSpaces( ptr, false );
		    if( !isValid(ptr) || *ptr != ',' )
			break;
		    
		    ++ptr; // skip ','
		}
		
		if( isValid(ptr) && *ptr == ')' )
		    ++ptr; // skip ')'		
	    }
	    
	    ptr = readWhiteSpaces( ptr, false );
	    QString body;
	    while( isValid(ptr) && *ptr != "\n" ){
		if( *ptr == '\\' ){
		    const QChar* p = readWhiteSpaces( ptr + 1, false );
		    if( isValid(p) && *p == '\n' ){
			newline( p );
			ptr = readWhiteSpaces( p + 1, false );
			continue;
		    }
		}
		body += *ptr;
		++ptr;
	    }
	    m.setBody( body );
	    qDebug( "body of %s is %s", m.name().latin1(), m.body().latin1() );
	}
    } else if( directive == "undef" ){
	ptr = readWhiteSpaces( ptr, false );
	if( isValid(ptr) ) {
	    const QChar* startMacroName = ptr;
	    ptr = readIdentifier( ptr );
	    QString macroName( startMacroName, int(ptr-startMacroName) );
	}
    } else if( directive == "line" ){
    } else if( directive == "error" ){
    } else if( directive == "pragma" ){
    } else if( directive == "if" ){
	ptr = readWhiteSpaces( ptr, false );
	if( isValid(ptr) && *ptr == '0' )
	    m_directiveStack.push( PreProc_skip );
	else
	    m_directiveStack.push( PreProc_in_group );
    } else if( directive == "ifdef" ){
	ptr = readWhiteSpaces( ptr, false );
	if( isValid(ptr) ) {
	    const QChar* startMacroName = ptr;
	    ptr = readIdentifier( ptr );
	    QString macroName( startMacroName, int(ptr-startMacroName) );
	}
	
	m_directiveStack.push( PreProc_in_group );
    } else if( directive == "ifndef" ){
	ptr = readWhiteSpaces( ptr, false );
	if( isValid(ptr) ) {
	    const QChar* startMacroName = ptr;
	    ptr = readIdentifier( ptr );
	    QString macroName( startMacroName, int(ptr-startMacroName) );
	}
	
	m_directiveStack.push( PreProc_in_group );
    } else if( directive == "elif" ){
	ptr = readWhiteSpaces( ptr, false );
	if( isValid(ptr) ) {
	    const QChar* startMacroName = ptr;
	    ptr = readIdentifier( ptr );
	    QString macroName( startMacroName, int(ptr-startMacroName) );
	}
	
	// ignore elif
	(void) m_directiveStack.pop();
	m_directiveStack.push( PreProc_skip );	
    } else if( directive == "else" ){
	// ignore else
	int st = m_directiveStack.top();
	(void) m_directiveStack.pop();
	m_directiveStack.push( st == PreProc_skip ? PreProc_in_group : PreProc_skip );
    } else if( directive == "endif" ){	
	(void) m_directiveStack.pop();
    }
    
    while( isValid(ptr) ){
	// skip line
	const QChar* base = ptr;
	while( isValid(ptr) && *ptr != '\n' )
	    NEXT_CHAR( ptr );
	
	QString line( base, ptr - base );
	line = line.stripWhiteSpace();
	if( !line.endsWith("\\") )
	    break;
	
	if( isValid(ptr) ){
	    NEXT_CHAR( ptr ); // skip \n
	}
    }
    return ptr;
}

#include "lexer.moc"
