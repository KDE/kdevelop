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
#include "lexer.moc"
#include "keywords.lut.h"

#include <kdebug.h>
#include <qregexp.h>

using namespace std;

Lexer::Lexer( Driver* driver )
    : m_driver( driver ),
      m_recordComments( false ),
      m_recordWhiteSpaces( false ),
      m_skipWordsEnabled( true ),
      m_preprocessorEnabled( true )
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
    m_ifLevel = 0;
    m_skipping.resize( 200 );
    m_skipping.fill( 0 );
    m_trueTest.resize( 200 );
    m_trueTest.fill( 0 );

    m_currentLine = 0;
    m_currentColumn = 0;
}

void Lexer::getTokenPosition( const Token& token, int* line, int* col )
{
    token.getStartPosition( line, col );
}

void Lexer::nextToken( Token& tk, bool stopOnNewline )
{
    int op = 0;

    if( m_size == (int)m_tokens.size() ){
	m_tokens.resize( m_tokens.size() + 5000 );
    }

    readWhiteSpaces( !stopOnNewline );

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
	m_startLine = false;

	int start = currentPosition();
	readIdentifier(); // read the directive
	QString directive = m_source.mid( start, currentPosition() - start );

	handleDirective( directive );
    } else if( m_startLine && m_skipping[ m_ifLevel ] ){
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
	} else if( m_preprocessorEnabled && m_driver->macros().contains(ide) &&
        		m_driver->macros()[ide].body().stripWhiteSpace() != ide ){
	    Macro m = m_driver->macros()[ ide ];
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
		(ide.startsWith("Q_EXPORT") && ide != "Q_EXPORT_INTERFACE") ||
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

        if( tk.type() != -1 )
	    addToken( tk );

	if( currentChar().isNull() )
	    break;
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
    bool skip = skipWordsEnabled();
    bool preproc = preprocessorEnabled();

    disableSkipWords();
    disablePreprocessor();

    if( directive == "define" ){
	if( !m_skipping[ m_ifLevel ] ){
	    Macro m;
	    processDefine( m );
	}
    } else if( directive == "else" ){
        processElse();
    } else if( directive == "elif" ){
        processElif();
    } else if( directive == "endif" ){
        processEndif();
    } else if( directive == "if" ){
        processIf();
    } else if( directive == "ifdef" ){
        processIfdef();
    } else if( directive == "ifndef" ){
        processIfndef();
    } else if( directive == "include" ){
        processInclude();
    } else if( directive == "undef" ){
        processUndef();
    }

    // skip line
    while( !currentChar().isNull() && currentChar() != '\n' )
        nextChar();

    m_skipWordsEnabled = skip;
    m_preprocessorEnabled = preproc;
}

/*
int Lexer::testDefined()
{
    int n = 0;
    readWhiteSpaces( false );
    if( currentChar() == '!' ){
	nextChar();
	n = 1;
	readWhiteSpaces( false );
    }

    int startWord = currentPosition();
    readIdentifier();
    QString word = m_source.mid( startWord, currentPosition() - startWord );

    if( word == "defined" ){
	return n ? -1 : 1;
    }

    return 0;
}
*/
int Lexer::testIfLevel()
{
    m_skipping[ m_ifLevel + 1 ] = m_skipping[ m_ifLevel ];
    return !m_skipping[ m_ifLevel++ ];
}

int Lexer::macroDefined()
{
    readWhiteSpaces( false );
    int startWord = currentPosition();
    readIdentifier();
    QString word = m_source.mid( startWord, currentPosition() - startWord );
    bool r = m_driver->macros().contains( word );

    return r;
}

void Lexer::processDefine( Macro& m )
{
    readWhiteSpaces( false );

    int startMacroName = currentPosition();
    readIdentifier();
    QString macroName = m_source.mid( startMacroName, int(currentPosition()-startMacroName) );
    m.setName( macroName );

    if( currentChar() == '(' ){
	m.setHasArguments( true );
	nextChar();

	while( currentChar() != ')' ){
	    readWhiteSpaces( false );
	    int startArg = currentPosition();
	    readIdentifier();
	    QString arg = m_source.mid( startArg, int(currentPosition()-startArg) );

	    m.addArgument( Macro::Argument(arg, 0) );

	    readWhiteSpaces( false );
	    if( currentChar().isNull() || currentChar() != ',' )
		break;

	    nextChar(); // skip ','
	}

	if( currentChar() == ')' )
	    nextChar(); // skip ')'
    }

    QString body;
    while( !currentChar().isNull() ){

	if( currentChar() == '\n' ){
	    break;
	} else if( currentChar().isSpace() ){
	    readWhiteSpaces( false );
	    body += " ";
	} else if( currentChar() == '\\' ){
	    nextChar();
	    readWhiteSpaces( false );
	    if( currentChar() == '\n' ){
		nextChar();
	    }
	} else {

	    Token tk;
	    nextToken( tk, true );

	    if( tk == '\n' )
		break;

	    if( tk.type() != -1 ){
		body += toString( tk );
	    }
	}
    }

    m.setBody( body );
    m_driver->addMacro( m );

    // qDebug( "add macro %s with body %s", m.name().latin1(), m.body().latin1() );
}

void Lexer::processElse()
{
    if( m_ifLevel == 0 )
        // TODO: report error
	return;

    m_skipping[ m_ifLevel ] = m_trueTest[ m_ifLevel ];
}

void Lexer::processElif()
{
    if( m_ifLevel == 0 )
	// TODO: report error
	return;

    if( !m_trueTest[m_ifLevel] ){
#if 0
	int n;
	if( (n = testDefined()) != 0 ){
	    int isdef = macroDefined();
	    m_trueTest[m_ifLevel] = !((n == 1 && !isdef) || (n == -1 && isdef));
	}
	else
#endif
	    m_trueTest[ m_ifLevel ] = macroExpression() != 0;
	m_skipping[ m_ifLevel ] = !m_trueTest[ m_ifLevel ];
    }
    else
	m_skipping[ m_ifLevel ] = true;
}

void Lexer::processEndif()
{
    if( m_ifLevel == 0 )
	// TODO: report error
	return;

    m_skipping[ m_ifLevel ] = 0;
    m_trueTest[ m_ifLevel-- ] = 0;

}

void Lexer::processIf()
{
    if( testIfLevel() ) {
#if 0
	int n;
	if( (n = testDefined()) != 0 ) {
	    int isdef = macroDefined();
	    m_trueTest[ m_ifLevel ] = (n == 1 && isdef) || (n == -1 && !isdef);
	} else
#endif
	    m_trueTest[ m_ifLevel ] = macroExpression() != 0;
	m_skipping[ m_ifLevel ] = !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processIfdef()
{
    if( testIfLevel() ){
	m_trueTest[ m_ifLevel ] = macroDefined();
	m_skipping[ m_ifLevel] = !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processIfndef()
{
    if( testIfLevel() ){
	m_trueTest[ m_ifLevel ] = !macroDefined();
	m_skipping[ m_ifLevel] = !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processInclude()
{
    if( m_skipping[m_ifLevel] )
	return;

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
}

void Lexer::processUndef()
{
    readWhiteSpaces();
    int startWord = currentPosition();
    readIdentifier();
    QString word = m_source.mid( startWord, currentPosition() - startWord );
    m_driver->removeMacro( word );
    // qDebug( "remove definition for %s", word.latin1() );
}

int Lexer::macroPrimary()
{
    readWhiteSpaces( false );
    int result = 0;
    switch( currentChar().latin1() ) {
    case '(':
	nextChar();
	result = macroExpression();
	if( currentChar() != ')' ){
	    // TODO: report error
	    return 0;
	}
	nextChar();
	return result;

    case '+':
    case '-':
    case '!':
    case '~':
	{
	    char tk = currentChar().latin1();
	    nextChar();
	    int result = macroPrimary();
	    if( tk == '-' ) return -result;
	    else if( tk == '!' ) return !result;
	    else if( tk == '~' ) return ~result;
	}
	break;

    default:
	{
	    Token tk;
	    nextToken( tk, false );
	    switch( tk.type() ){
	    case Token_identifier:
		if( toString(tk) == "defined" ){
		    return macroPrimary();
		}
		// TODO: implement
		return m_driver->macros().contains( toString(tk) );
	    case Token_number_literal:
		return toString(tk).toInt();
	    case Token_char_literal:
		return 1;
		default:
		break;
	    } // end switch

	} // end default

    } // end switch

    return 0;
}

int Lexer::macroMultiplyDivide()
{
    int result = macroPrimary();
    int iresult, op;
    for (;;)    {
	readWhiteSpaces( false );
        if( currentChar() == '*' )
            op = 0;
        else if( currentChar() == '/' && !(peekChar() == '*' || peekChar() == '/') )
            op = 1;
        else if( currentChar() == '%' )
            op = 2;
        else
            break;
	nextChar();
        iresult = macroPrimary();
        result = op == 0 ? (result * iresult) :
                 op == 1 ? (iresult == 0 ? 0 : (result / iresult)) :
                           (iresult == 0 ? 0 : (result % iresult)) ;
    }
    return result;
}

int Lexer::macroAddSubtract()
{
    int result = macroMultiplyDivide();
    int iresult, ad;
    readWhiteSpaces( false );
    while( currentChar() == '+' || currentChar() == '-')    {
        ad = currentChar() == '+';
	nextChar();
        iresult = macroMultiplyDivide();
        result = ad ? (result+iresult) : (result-iresult);
    }
    return result;
}

int Lexer::macroRelational()
{
    int result = macroAddSubtract();
    int iresult;
    readWhiteSpaces( false );
    while( currentChar() == '<' || currentChar() == '>')    {
	int lt = currentChar() == '<';
	nextChar();
	if( currentChar() == '=') {
	    nextChar();

	    iresult = macroAddSubtract();
	    result = lt ? (result <= iresult) : (result >= iresult);
	}
	else {
	    iresult = macroAddSubtract();
	    result = lt ? (result < iresult) : (result > iresult);
	}
    }

    return result;
}

int Lexer::macroEquality()
{
    int result = macroRelational();
    int iresult, eq;
    readWhiteSpaces( false );
    while ((currentChar() == '=' || currentChar() == '!') && peekChar() == '=')  {
	eq = currentChar() == '=';
	nextChar( 2 );
	iresult = macroRelational();
	result = eq ? (result==iresult) : (result!=iresult);
    }
    return result;
}

int Lexer::macroBoolAnd()
{
    int result = macroEquality();
    readWhiteSpaces( false );
    while( currentChar() == '&' && peekChar() != '&')    {
	nextChar();
	result &= macroEquality();
    }
    return result;
}

int Lexer::macroBoolXor()
{
    int result = macroBoolAnd();
    readWhiteSpaces( false );
    while( currentChar() == '^')    {
	nextChar();
	result ^= macroBoolAnd();
    }
    return result;
}

int Lexer::macroBoolOr()
{
    int result = macroBoolXor();
    readWhiteSpaces( false );
    while( currentChar() == '|' && peekChar() != '|')    {
	nextChar();
	result |= macroBoolXor();
    }
    return result;
}

int Lexer::macroLogicalAnd()
{
    int result = macroBoolOr();
    readWhiteSpaces( false );
    while( currentChar() == '&' && peekChar() == '&')    {
	nextChar( 2 );
	int start = currentPosition();
        result = macroBoolOr() && result;
	QString s = m_source.mid( start, currentPosition() - start );
    }
    return result;
}

int Lexer::macroLogicalOr()
{
    int result = macroLogicalAnd();
    readWhiteSpaces( false );
    while( currentChar() == '|' && peekChar() == '|')    {
	nextChar( 2 );
        result = macroLogicalAnd() || result;
    }
    return result;
}

int Lexer::macroExpression()
{
    readWhiteSpaces( false );
    return macroLogicalOr();
}
