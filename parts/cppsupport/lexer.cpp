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
#include "lookup.h"
#include "keywords.lut.h"

//#include <qregexp.h>
#include <kdebug.h>

using namespace std;

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
      m_recordWhiteSpaces( false ),
      m_skipWordsEnabled( true )
{
    reset();
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
    m_tokens.resize( 15000 );
    m_startLineVector.resize( 5000 );
    m_source = QString::null;
    m_buffer = 0;
    m_endPtr = 0;
    m_startLine = false;
    
    m_currentLine = 0;
    m_currentColumn = 0;
}
  
void Lexer::getTokenPosition( const Token& token, int* line, int* col )
{
    token.getStartPosition( line, col );
}

void Lexer::tokenize()
{
    const QChar* ptr = m_buffer;    
    int op;

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

	    nextChar( ptr ); // skip #
	    ptr = readWhiteSpaces( ptr, false );	    // skip white spaces

	    const QChar* eptr = readIdentifier( ptr ); // read the directive
	    QString directive( ptr, eptr - ptr );

	    ptr = handleDirective( directive, eptr );
	    //m_tokens[ m_size++ ] = Token( Token_preproc, ptr, 1 );
	} else if( preproc_state == PreProc_skip ){
	    // skip line and continue
	    while( isValid(ptr) && !ptr->isNull() && *ptr != '\n' )
		nextChar( ptr );
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
	    int k = Lookup::find( &keyword, ide );
	    if( k != -1 ){
		Token tk = Token( k, ptr, end - ptr );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
		m_tokens[ m_size++ ] = tk;
	    } else {
	      if( m_skipWordsEnabled ){
		QMap< QString, SkipType >::Iterator pos = m_words.find( ide );
		if( pos != m_words.end() ){
		    if( *pos == SkipWordAndArguments ){
			end = skip( readWhiteSpaces(end, false), '(', ')' );
		    }
		} else if( /*qt_rx.exactMatch(ide) ||*/
		    ide.endsWith("EXPORT") ||
		    ide.startsWith("Q_EXPORT") ||
		    ide.startsWith("QM_EXPORT") ||
		    ide.startsWith("QM_TEMPLATE")){

		    end = skip( readWhiteSpaces(end, false), '(', ')' );
		} else if( ide.startsWith("K_TYPELIST_") ){
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
	      } else {
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
	} else if( -1 != (op = findOperator3(ptr)) ){
	    Token tk = Token( op, ptr, 3 );
	    nextChar( ptr, 3 );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	} else if( -1 != (op = findOperator2(ptr)) ){
	    Token tk = Token( op, ptr, 2 );
	    nextChar( ptr, 2 );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	    m_tokens[ m_size++ ] = tk;
	} else {
	    Token tk = Token( ptr->latin1(), ptr, 1 );
	    nextChar( ptr );
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

        nextChar( ptr );

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
	    while( isValid(ptr) && *ptr != '\n' ){
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
	    m_driver->addMacro( m_driver->currentFileName(), m );
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
	if( m_directiveStack.size() && m_directiveStack.top() == PreProc_skip )
	    m_directiveStack.push( PreProc_skip );
	else if( isValid(ptr) && *ptr == '0' )
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

	if( m_directiveStack.size() && m_directiveStack.top() == PreProc_skip )
	    m_directiveStack.push( PreProc_skip );
	else 
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
	m_directiveStack.pop();
	m_directiveStack.push( PreProc_skip ); // skip all elif
    } else if( directive == "else" ){
	(void) m_directiveStack.pop();
	int st = m_directiveStack.top();
	m_directiveStack.push( st == PreProc_skip ? PreProc_in_group : PreProc_skip );
    } else if( directive == "endif" ){
	(void) m_directiveStack.pop();
    }

    while( isValid(ptr) ){
	// skip line
	const QChar* base = ptr;
	while( isValid(ptr) && *ptr != '\n' )
	    nextChar( ptr );

	QString line( base, ptr - base );
	line = line.stripWhiteSpace();
	if( !line.endsWith("\\") )
	    break;

	if( isValid(ptr) ){
	    nextChar( ptr ); // skip \n
	}
    }
    return ptr;
}

#include "lexer.moc"
