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

#include <kdebug.h>
#include <klocale.h>

#include <qregexp.h>
#include <qmap.h>
#include <q3valuelist.h>

#if defined( KDEVELOP_BGPARSER )
#include <qthread.h>

class KDevTread: public QThread
{
public:
    static void yield()
    {
	msleep( 0 );
    }
};

inline void qthread_yield()
{
    KDevTread::yield();
}

#endif

#define CREATE_TOKEN(type, start, len) Token( (type), (start), (len), m_source )
#define ADD_TOKEN(tk) m_tokens.insert( m_size++, new Token(tk) );

using namespace std;

struct LexerData
{
    typedef QMap<QString, QString> Scope;
    typedef Q3ValueList<Scope> StaticChain;

    StaticChain staticChain;

    void beginScope()
    {
        Scope scope;
        staticChain.push_front( scope );
    }

    void endScope()
    {
        staticChain.pop_front();
    }

    void bind( const QString& name, const QString& value )
    {
        Q_ASSERT( staticChain.size() > 0 );
        staticChain.front().insert( name, value );
    }

    bool hasBind( const QString& name ) const
    {
        StaticChain::ConstIterator it = staticChain.begin();
        while( it != staticChain.end() ){
            const Scope& scope = *it;
            ++it;

            if( scope.contains(name) )
                return true;
        }

        return false;
    }

    QString apply( const QString& name ) const
    {
        StaticChain::ConstIterator it = staticChain.begin();
        while( it != staticChain.end() ){
            const Scope& scope = *it;
            ++it;

            if( scope.contains(name) )
                return scope[ name ];
        }

        return QString::null;
    }

};

Lexer::Lexer( Driver* driver )
    : d( new LexerData),
      m_driver( driver ),
      m_recordComments( false ),
      m_recordWhiteSpaces( false ),
      m_skipWordsEnabled( true ),
      m_preprocessorEnabled( true ),
      m_reportWarnings( false ),
      m_reportMessages( false )
{
    m_tokens.setAutoDelete( true );
    reset();
    d->beginScope();
}

Lexer::~Lexer()
{
    d->endScope();
    delete( d );
}

void Lexer::setSource( const QString& source )
{
    reset();
    m_source = source;
    m_ptr = 0;
    m_endPtr = m_source.length();
    m_inPreproc = false;

    tokenize();
}

void Lexer::reset()
{
    m_index = 0;
    m_size = 0;
    m_tokens.clear();
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

// ### should all be done with a "long" type IMO
int Lexer::toInt( const Token& token )
{
    QString s = token.text();
    if( token.type() == Token_number_literal ){
        // hex literal ?
	if( s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
	    return s.mid( 2 ).toInt( 0, 16 );
        QString n;
        int i = 0;
        while( i < int(s.length()) && s[i].isDigit() )
            n += s[i++];
        // ### respect more prefixes and suffixes ?
        return n.toInt();
    } else if( token.type() == Token_char_literal ){
	int i = s[0] == 'L' ? 2 : 1; // wide char ?
	if( s[i] == '\\' ){
	    // escaped char
	    int c = s[i+1].unicode();
	    switch( c ) {
	    case '0':
		return 0;
	    case 'n':
		return '\n';
	    // ### more
	    default:
		return c;
	    }
	} else {
	    return s[i].unicode();
	}
    } else {
	return 0;
    }
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

    if( ch.isNull() || ch.isSpace() ){
	/* skip */
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
        m_startLine = false;
        int ppe = preprocessorEnabled();
	setPreprocessorEnabled( false );
	while( currentChar() && currentChar() != '\n' ){
            Token tok;
            nextToken( tok, true );
        }
        m_startLine = true;
        setPreprocessorEnabled( ppe );
        return;
    } else if( ch == '/' && ch1 == '/' ){
	int start = currentPosition();
	readLineComment();
	if( recordComments() ){
	    tk = CREATE_TOKEN( Token_comment, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	}
    } else if( ch == '/' && ch1 == '*' ){
	int start = currentPosition();
	nextChar( 2 );
	readMultiLineComment();

	if( recordComments() ){
	    tk = CREATE_TOKEN( Token_comment, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	}
    } else if( ch == '\'' || (ch == 'L' && ch1 == '\'') ){
	int start = currentPosition();
	readCharLiteral();
	tk = CREATE_TOKEN( Token_char_literal, start, currentPosition() - start );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( ch == '"' ){
	int start = currentPosition();
	readStringLiteral();
	tk = CREATE_TOKEN( Token_string_literal, start, currentPosition() - start );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( ch.isLetter() || ch == '_' ){
	int start = currentPosition();
	readIdentifier();
	QString ide = m_source.mid( start, currentPosition() - start );
	int k = Lookup::find( ide );
	if( m_preprocessorEnabled && m_driver->hasMacro(ide) &&
	    (k == -1 || !m_driver->macro(ide).body().isEmpty()) ){


            bool preproc = m_preprocessorEnabled;
            m_preprocessorEnabled = false;

            d->beginScope();

	    int svLine = currentLine();
	    int svColumn = currentColumn();

//	    Macro& m = m_driver->macro( ide );
	    Macro m = m_driver->macro( ide );
	    //m_driver->removeMacro( m.name() );

            QString ellipsisArg;

	    if( m.hasArguments() ){
                int endIde = currentPosition();

		readWhiteSpaces();
		if( currentChar() == '(' ){
		    nextChar();
		    int argIdx = 0;
		    int argCount = m.argumentList().size();
		    while( currentChar() && argIdx<argCount ){
			readWhiteSpaces();

                        QString argName = m.argumentList()[ argIdx ];

                        bool ellipsis = argName == "...";

			QString arg = readArgument();

                        if( !ellipsis )
                            d->bind( argName, arg );
                        else
                            ellipsisArg += arg;

			if( currentChar() == ',' ){
			    nextChar();
                            if( !ellipsis ){
                                ++argIdx;
                            } else {
                                ellipsisArg += ", ";
                            }
			} else if( currentChar() == ')' ){
			    break;
			}
		    }
                    if( currentChar() == ')' ){
                        // valid macro
                        nextChar();
                    }
		} else {
		    tk = CREATE_TOKEN( Token_identifier, start, endIde - start );
		    tk.setStartPosition( svLine, svColumn );
		    tk.setEndPosition( svLine, svColumn + (endIde - start) );

                    m_startLine = false;

                    d->endScope();        // OPS!!
                    m_preprocessorEnabled = preproc;
                    return;
                }
	    }

	    int argsEndAtLine = currentLine();
	    int argsEndAtColumn = currentColumn();

#if defined( KDEVELOP_BGPARSER )
	    qthread_yield();
#endif
	    m_source.insert( currentPosition(), m.body() );

            // tokenize the macro body

            QString textToInsert;

            m_endPtr = currentPosition() + m.body().length();
            while( currentChar() ){

                readWhiteSpaces();

                Token tok;
                nextToken( tok );

                bool stringify = !m_inPreproc && tok == '#';
                bool merge = !m_inPreproc && tok == Token_concat;

                if( stringify || merge )
                    nextToken( tok );

                if( tok == Token_eof )
                     break;

                QString tokText = tok.text();
                QString str = (tok == Token_identifier && d->hasBind(tokText)) ? d->apply( tokText ) : tokText;
                if( str == ide ){
                    //Problem p( i18n("unsafe use of macro '%1'").arg(ide), m_currentLine, m_currentColumn );
                    //m_driver->addProblem( m_driver->currentFileName(), p );
                    m_driver->removeMacro( ide );
                    // str = QString::null;
                }

                if( stringify ) {
                    textToInsert.append( QString::fromLatin1("\"") + str + QString::fromLatin1("\" ") );
                } else if( merge ){
                    textToInsert.truncate( textToInsert.length() - 1 );
                    textToInsert.append( str );
                } else if( tok == Token_ellipsis && d->hasBind("...") ){
                    textToInsert.append( ellipsisArg );
                } else {
                    textToInsert.append( str + QString::fromLatin1(" ") );
                }
            }

#if defined( KDEVELOP_BGPARSER )
	    qthread_yield();
#endif
            m_source.insert( currentPosition(), textToInsert );

            d->endScope();
            m_preprocessorEnabled = preproc;
	    //m_driver->addMacro( m );
	    m_currentLine = argsEndAtLine;
	    m_currentColumn = argsEndAtColumn;
	    m_endPtr = m_source.length();
	} else if( k != -1 ){
	    tk = CREATE_TOKEN( k, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	} else if( m_skipWordsEnabled ){
	    QMap< QString, QPair<SkipType, QString> >::Iterator pos = m_words.find( ide );
	    if( pos != m_words.end() ){
		if( (*pos).first == SkipWordAndArguments ){
		    readWhiteSpaces();
		    if( currentChar() == '(' )
			skip( '(', ')' );
		}
		if( !(*pos).second.isEmpty() ){
#if defined( KDEVELOP_BGPARSER )
	    qthread_yield();
#endif
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
		tk = CREATE_TOKEN( Token_identifier, start, currentPosition() - start );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
		readWhiteSpaces();
		if( currentChar() == '(' )
		    skip( '(', ')' );
	    } else{
		tk = CREATE_TOKEN( Token_identifier, start, currentPosition() - start );
		tk.setStartPosition( startLine, startColumn );
		tk.setEndPosition( m_currentLine, m_currentColumn );
	    }
	} else {
	    tk = CREATE_TOKEN( Token_identifier, start, currentPosition() - start );
	    tk.setStartPosition( startLine, startColumn );
	    tk.setEndPosition( m_currentLine, m_currentColumn );
	}
    } else if( ch.isNumber() ){
	int start = currentPosition();
	readNumberLiteral();
	tk = CREATE_TOKEN( Token_number_literal, start, currentPosition() - start );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( -1 != (op = findOperator3()) ){
	tk = CREATE_TOKEN( op, currentPosition(), 3 );
	nextChar( 3 );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else if( -1 != (op = findOperator2()) ){
	tk = CREATE_TOKEN( op, currentPosition(), 2 );
	nextChar( 2 );
	tk.setStartPosition( startLine, startColumn );
	tk.setEndPosition( m_currentLine, m_currentColumn );
    } else {
	tk = CREATE_TOKEN( ch, currentPosition(), 1 );
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
	    ADD_TOKEN( tk );

	if( currentChar().isNull() )
	    break;
    }

    Token tk = CREATE_TOKEN( Token_eof, currentPosition(), 0 );
    tk.setStartPosition( m_currentLine, m_currentColumn );
    tk.setEndPosition( m_currentLine, m_currentColumn );
    ADD_TOKEN( tk );
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

QString Lexer::readArgument()
{
    int count = 0;

    QString arg;

    readWhiteSpaces();
    while( currentChar() ){

	readWhiteSpaces();
	QChar ch = currentChar();

	if( ch.isNull() || (!count && (ch == ',' || ch == ')')) )
	    break;

	Token tk;
	nextToken( tk );

	if( tk == '(' ){
	    ++count;
	} else if( tk == ')' ){
	    --count;
	}

	if( tk != -1 )
            arg += tk.text() + " ";
    }

    return arg.stripWhiteSpace();
}

void Lexer::handleDirective( const QString& directive )
{
    m_inPreproc = true;

    bool skip = skipWordsEnabled();
    bool preproc = preprocessorEnabled();

    setSkipWordsEnabled( false );
    setPreprocessorEnabled( false );

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
	if( !m_skipping[ m_ifLevel ] ){
            processInclude();
        }
    } else if( directive == "undef" ){
	if( !m_skipping[ m_ifLevel ] ){
            processUndef();
        }
    }

    // skip line
    while( currentChar() && currentChar() != '\n' ){
        Token tk;
        nextToken( tk, true );
    }

    setSkipWordsEnabled( skip );
    setPreprocessorEnabled( preproc );

    m_inPreproc = false;
}

int Lexer::testIfLevel()
{
    int rtn = !m_skipping[ m_ifLevel++ ];
    m_skipping[ m_ifLevel ] = m_skipping[ m_ifLevel - 1 ];
    return rtn;
}

int Lexer::macroDefined()
{
    readWhiteSpaces( false );
    int startWord = currentPosition();
    readIdentifier();
    QString word = m_source.mid( startWord, currentPosition() - startWord );
    bool r = m_driver->hasMacro( word );

    return r;
}

void Lexer::processDefine( Macro& m )
{
    m.setFileName( m_driver->currentFileName() );
    readWhiteSpaces( false );

    int startMacroName = currentPosition();
    readIdentifier();
    QString macroName = m_source.mid( startMacroName, int(currentPosition()-startMacroName) );
    m_driver->removeMacro( macroName );
    m.setName( macroName );

    if( currentChar() == '(' ){
	m.setHasArguments( true );
	nextChar();

        readWhiteSpaces( false );

	while( currentChar() && currentChar() != ')' ){
	    readWhiteSpaces( false );

	    int startArg = currentPosition();

            if( currentChar() == '.' && peekChar() == '.' && peekChar(2) == '.' )
                nextChar( 3 );
            else
	        readIdentifier();

	    QString arg = m_source.mid( startArg, int(currentPosition()-startArg) );

	    m.addArgument( Macro::Argument(arg) );

	    readWhiteSpaces( false );
	    if( currentChar() != ',' )
		break;

	    nextChar(); // skip ','
	}

	if( currentChar() == ')' )
	    nextChar(); // skip ')'
    }

    setPreprocessorEnabled( true );

    QString body;
    while( currentChar() && currentChar() != '\n' ){

        if( currentChar().isSpace() ){
	    readWhiteSpaces( false );
	    body += " ";
	} else {

	    Token tk;
	    nextToken( tk, true );

	    if( tk.type() != -1 ){
                QString s = tk.text();
		body += s;
	    }
	}
    }

    m.setBody( body );
    m_driver->addMacro( m );
}

void Lexer::processElse()
{
    if( m_ifLevel == 0 )
        /// @todo report error
	return;

    if( m_ifLevel > 0 && m_skipping[m_ifLevel-1] )
       m_skipping[ m_ifLevel ] = m_skipping[ m_ifLevel - 1 ];
    else
       m_skipping[ m_ifLevel ] = m_trueTest[ m_ifLevel ];
}

void Lexer::processElif()
{
    if( m_ifLevel == 0 )
	/// @todo report error
	return;

    if( !m_trueTest[m_ifLevel] ){
        /// @todo implement the correct semantic for elif!!
        bool inSkip = m_ifLevel > 0 && m_skipping[ m_ifLevel-1 ];
        m_trueTest[ m_ifLevel ] = macroExpression() != 0;
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
    else
	m_skipping[ m_ifLevel ] = true;
}

void Lexer::processEndif()
{
    if( m_ifLevel == 0 )
	/// @todo report error
	return;

    m_skipping[ m_ifLevel ] = 0;
    m_trueTest[ m_ifLevel-- ] = 0;
}

void Lexer::processIf()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if( testIfLevel() ) {
#if 0
	int n;
	if( (n = testDefined()) != 0 ) {
	    int isdef = macroDefined();
	    m_trueTest[ m_ifLevel ] = (n == 1 && isdef) || (n == -1 && !isdef);
	} else
#endif
        m_trueTest[ m_ifLevel ] = macroExpression() != 0;
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processIfdef()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if( testIfLevel() ){
	m_trueTest[ m_ifLevel ] = macroDefined();
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processIfndef()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if( testIfLevel() ){
	m_trueTest[ m_ifLevel ] = !macroDefined();
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processInclude()
{
    if( m_skipping[m_ifLevel] )
	return;

    readWhiteSpaces( false );
    if( currentChar() ){
	QChar ch = currentChar();
	if( ch == '"' || ch == '<' ){
	    nextChar();
	    QChar ch2 = ch == QChar('"') ? QChar('"') : QChar('>');

	    int startWord = currentPosition();
	    while( currentChar() && currentChar() != ch2 )
		nextChar();
	    if( currentChar() ){
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
}

int Lexer::macroPrimary()
{
    readWhiteSpaces( false );
    int result = 0;
    switch( currentChar() ) {
    case '(':
	nextChar();
	result = macroExpression();
	if( currentChar() != ')' ){
	    /// @todo report error
	    return 0;
	}
	nextChar();
	return result;

    case '+':
    case '-':
    case '!':
    case '~':
	{
	    QChar tk = currentChar();
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
		if( tk.text() == "defined" ){
		    return macroPrimary();
		}
		/// @todo implement
		return m_driver->hasMacro( tk.text() );
	    case Token_number_literal:
	    case Token_char_literal:
		return toInt( tk );
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

// *IMPORTANT*
// please, don't include lexer.moc here, because Lexer isn't a QObject class!!
// if you have problem while recompiling try to remove cppsupport/.deps,
// cppsupport/Makefile.in and rerun automake/autoconf

