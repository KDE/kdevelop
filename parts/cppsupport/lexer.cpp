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

#include "lexer.h"
#include "lookup.h"
#include "keywords.lut.h"

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
    m_ptr = 0;
    m_endPtr = m_source.length();

    tokenize();
}

void Lexer::reset()
{
    m_index = 0;
    m_size = 0;
    m_tokens.resize( 15000 );
    m_source = QString::null;
    m_ptr = 0;
    m_endPtr = 0;
    m_startLine = false;

    m_currentLine = 0;
    m_currentColumn = 0;
}

void Lexer::getTokenPosition( const Token& token, int* line, int* col )
{
    token.getStartPosition( line, col );
}

void Lexer::nextToken( Token& tk )
{
    int op = 0;
    int preproc_state = PreProc_none;

    if( m_directiveStack.size() ){
	preproc_state = m_directiveStack.top();
    }

    if( m_size == (int)m_tokens.size() ){
	m_tokens.resize( m_tokens.size() + 5000 );
    }

    readWhiteSpaces();

    int startLine = m_currentLine;
    int startColumn = m_currentColumn;

    QChar ch = currentChar();
    QChar ch1 = peekChar();

    if( ch.isNull() ){
	/* skip */
    } else if( ch == '/' && ch1 == '/' ){
	int start = currentPosition();
	readLineComment();
	if( recordComments() ){
	    tk = Token( Token_comment, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	}
    } else if( ch == '/' && ch1 == '*' ){
	int start = currentPosition();
	readMultiLineComment();
	if( recordComments() ){
	    tk = Token( Token_comment, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	}
    } else if( m_startLine && ch == '#' ){

	nextChar(); // skip #
	readWhiteSpaces( false );	    // skip white spaces

	int start = currentPosition();
	readIdentifier(); // read the directive
	QString directive = m_source.mid( start, currentPosition() - start );

	handleDirective( directive );
    } else if( preproc_state == PreProc_skip ){
	// skip line and continue
	while( !currentChar().isNull() && currentChar() != '\n' )
	    nextChar();
	/* skip */
    } else if( ch == '\'' ){
	int start = currentPosition();
	readCharLiteral();
	tk = Token( Token_char_literal, start, currentPosition() - start );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( ch == '"' ){
	int start = currentPosition();
	readStringLiteral();
	tk = Token( Token_string_literal, start, currentPosition() - start );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( ch.isLetter() || ch == '_' ){
	int start = currentPosition();
	readIdentifier();
	QString ide = m_source.mid( start, currentPosition() - start );
	int k = Lookup::find( &keyword, ide );
	if( k != -1 ){
	    tk = Token( k, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	} else if( m_driver->macros( m_driver->currentFileName() ).contains(ide) ){
	    const Macro& m = m_driver->macros( m_driver->currentFileName() )[ ide ];
	    if( m.hasArguments() ){
		readWhiteSpaces();
		if( currentChar() == '(' )
		    skip( '(', ')' );
	    }
	    m_source.insert( currentPosition(), QString(" ") + m.body() + QString(" ") );
	    m_endPtr = m_source.length();
	} else if( m_skipWordsEnabled ){
	    QMap< QString, QPair<SkipType, QString> >::Iterator pos = m_words.find( ide );
	    if( pos != m_words.end() ){
		if( (*pos).first == SkipWordAndArguments ){
		    readWhiteSpaces();
		    if( currentChar() == '(' )
			skip( '(', ')' );
		}
		if( !(*pos).second.isEmpty() ){
		    m_source.insert( currentPosition(), QString(" ") + (*pos).second + QString(" ") );
		    m_endPtr = m_source.length();
		}
	    } else if( /*qt_rx.exactMatch(ide) ||*/
		ide.endsWith("EXPORT") ||
		ide.startsWith("Q_EXPORT") ||
		ide.startsWith("QM_EXPORT") ||
		ide.startsWith("QM_TEMPLATE")){
		
		readWhiteSpaces();
		if( currentChar() == '(' )
		    skip( '(', ')' );
	    } else if( ide.startsWith("K_TYPELIST_") || ide.startsWith("TYPELIST_") ){
		tk = Token( Token_identifier, start, currentPosition() - start );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
		readWhiteSpaces();
		if( currentChar() == '(' )
		    skip( '(', ')' );
	    } else{
		tk = Token( Token_identifier, start, currentPosition() - start );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
	    }
	} else {
	    tk = Token( Token_identifier, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	} 
    } else if( ch.isNumber() ){
	int start = currentPosition();
	readNumberLiteral();
	tk = Token( Token_number_literal, start, currentPosition() - start );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( -1 != (op = findOperator3()) ){
	tk = Token( op, currentPosition(), 3 );
	nextChar( 3 );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( -1 != (op = findOperator2()) ){
	tk = Token( op, currentPosition(), 2 );
	nextChar( 2 );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else {
	tk = Token( ch.latin1(), currentPosition(), 1 );
	nextChar();
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    }
    
    m_startLine = false;
}


void Lexer::tokenize()
{
    m_startLine = true;
    m_size = 0;

    for( ;; ) {
	Token tk;
	nextToken( tk );

	if( currentChar().isNull() )
	    break;

        if( tk.type() != -1 )
	    addToken( tk );
    }

    Token tk = Token( Token_eof, currentPosition(), 0 );
    tk.setStartPosition( m_currentLine, m_currentColumn );
    tk.setEndPosition( m_currentLine, m_currentColumn );
    addToken( tk );
}

void Lexer::resetSkipWords()
{
    m_words.clear();
}

void Lexer::addSkipWord( const QString& word, SkipType skipType, const QString& str )
{
    m_words[ word ] = qMakePair( skipType, str );
}

void Lexer::skip( int l, int r )
{
    int svCurrentLine = m_currentLine;
    int svCurrentColumn = m_currentColumn;
    
    int count = 0;

    while( !eof() ){
	Token tk;
	nextToken( tk );
	
	if( (int)tk == l )
            ++count;
        else if( (int)tk == r )
            --count;

        if( count == 0 )
            break;
    }
    
    m_currentLine = svCurrentLine;
    m_currentColumn = svCurrentColumn;
}

void Lexer::handleDirective( const QString& directive )
{
    //kdDebug(9007) << "handle directive " << directive << endl;

    if( directive == "include" ){
	readWhiteSpaces( false );
	if( !currentChar().isNull() ){
	    QChar ch = currentChar();
	    if( ch == '"' || ch == '<' ){
		nextChar();
		QChar ch2 = ch == QChar('"') ? QChar('"') : QChar('>');
		
		int startWord = currentPosition();
		while( !currentChar().isNull() && currentChar() != ch2 )
		    nextChar();
		if( !currentChar().isNull() ){
		    QString word = m_source.mid( startWord, int(currentPosition()-startWord) );
		    m_driver->addDependence( m_driver->currentFileName(),
					     Dependence(word, ch == '"' ? Dep_Local : Dep_Global) );
		    nextChar();
		}
	    }
	}
    } else if( directive == "define" ){
	readWhiteSpaces( false );
	if( !currentChar().isNull() ) {
	    Macro m;

	    int startMacroName = currentPosition();
	    readIdentifier();
	    QString macroName = m_source.mid( startMacroName, int(currentPosition()-startMacroName) );
	    m.setName( macroName );

	    if( !currentChar().isNull() && currentChar() == '(' ){
		m.setHasArguments( true );
		nextChar();

		while( !currentChar().isNull() && currentChar() != ')' ){
		    readWhiteSpaces( false );
		    int startArg = currentPosition();
		    readIdentifier();
		    QString arg = m_source.mid( startArg, int(currentPosition()-startArg) );

		    m.addArgument( Macro::Argument(arg, 0) );

		    readWhiteSpaces( false );
		    if( !!currentChar().isNull() || currentChar() != ',' )
			break;

		    nextChar(); // skip ','
		}

		if( !currentChar().isNull() && currentChar() == ')' )
		    nextChar(); // skip ')'
	    }

	    readWhiteSpaces( false );
	    QString body;
	    while( !currentChar().isNull() && currentChar() != '\n' ){
		if( currentChar() == '\\' ){
		    nextChar();
		    readWhiteSpaces( false );
		    if( !currentChar().isNull() && currentChar() == '\n' ){
			nextChar();
			body += "\n";
			readWhiteSpaces( false );
			continue;
		    }
		}
		body += currentChar();
		nextChar();
	    }
	    m.setBody( body );
	    m_driver->addMacro( m_driver->currentFileName(), m );
	}
    } else if( directive == "undef" ){
	readWhiteSpaces( false );
	if( !currentChar().isNull() ) {
	    int startMacroName = currentPosition();
	    readIdentifier();
	    QString macroName = m_source.mid( startMacroName, int(currentPosition()-startMacroName) );
	}
    } else if( directive == "line" ){
    } else if( directive == "error" ){
    } else if( directive == "pragma" ){
    } else if( directive == "if" ){
	readWhiteSpaces( false );
	if( m_directiveStack.size() && m_directiveStack.top() == PreProc_skip )
	    m_directiveStack.push( PreProc_skip );
	else if( !currentChar().isNull() && currentChar() == '0' )
	    m_directiveStack.push( PreProc_skip );
	else
	    m_directiveStack.push( PreProc_in_group );
    } else if( directive == "ifdef" ){
	readWhiteSpaces( false );
	if( !currentChar().isNull() ) {
	    int startMacroName = currentPosition();
	    readIdentifier();
	    QString macroName = m_source.mid( startMacroName, int(currentPosition()-startMacroName) );
	}
	if( m_directiveStack.size() && m_directiveStack.top() == PreProc_skip )
	    m_directiveStack.push( PreProc_skip );
	else
	    m_directiveStack.push( PreProc_in_group );
    } else if( directive == "ifndef" ){
	readWhiteSpaces( false );
	if( !currentChar().isNull() ) {
	    int startMacroName = currentPosition();
	    readIdentifier();
	    QString macroName = m_source.mid( startMacroName, int(currentPosition()-startMacroName) );
	}
	m_directiveStack.push( PreProc_in_group );
    } else if( directive == "elif" ){
	readWhiteSpaces( false );
	if( !currentChar().isNull() ) {
	    int startMacroName = currentPosition();
	    readIdentifier();
	    QString macroName = m_source.mid( startMacroName, int(currentPosition()-startMacroName) );
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

    while( !currentChar().isNull() ){
	// skip line
	int base = currentPosition();
	while( !currentChar().isNull() && currentChar() != '\n' )
	    nextChar();

	QString line = m_source.mid( base, currentPosition() - base );
	line = line.stripWhiteSpace();

	if( !line.endsWith("\\") )
	    break;

	if( !currentChar().isNull() ){
	    nextChar(); // skip \n
	}
    }
}

void Lexer::handleDefineDirective( Macro& macro )
{
    Q_UNUSED( macro );
    
    if( currentChar() == '(' ){
	// read argument
	nextChar();
	
	while( !eof() ){
	    readWhiteSpaces( false );
	    if( currentChar().isLetter() || currentChar() == '_' ){
		int startArg = currentPosition();
		readIdentifier();
		QString arg = m_source.mid( startArg, currentPosition() - startArg );
	    } 
	    
	    if( findOperator3() == Token_ellipsis ){
		nextChar( 3 );
		readWhiteSpaces( false );
		break;
	    }
	    
	    readWhiteSpaces( false );
	    if( currentChar() != ',' )
		break;
	}
	
	if( currentChar() != ')' ){
	    // TODO: report error
	} else {
	    nextChar();
	    
	    // TODO: read macro body
	}
    }
    
    // TODO: fill macro
}


#include "lexer.moc"
