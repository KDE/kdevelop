/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include <map>

#include <iostream>

#ifdef HAS_NOT_CSTRING_H
#include <string>
#else
#include <cstring>
#endif

#include "antlr/CharScanner.hpp"
#include "antlr/CommonToken.hpp"

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif
ANTLR_C_USING(exit)

CharScanner::CharScanner(InputBuffer& cb, bool case_sensitive )
	: saveConsumedInput(true) //, caseSensitiveLiterals(true)
	, caseSensitive(case_sensitive)
	, literals(CharScannerLiteralsLess(this))
	, inputState(new LexerInputState(cb))
	, commitToPath(false)
	, tabsize(8)
	, traceDepth(0)
{
	setTokenObjectFactory(&CommonToken::factory);
}

CharScanner::CharScanner(InputBuffer* cb, bool case_sensitive )
	: saveConsumedInput(true) //, caseSensitiveLiterals(true)
	, caseSensitive(case_sensitive)
	, literals(CharScannerLiteralsLess(this))
	, inputState(new LexerInputState(cb))
	, commitToPath(false)
	, tabsize(8)
	, traceDepth(0)
{
	setTokenObjectFactory(&CommonToken::factory);
}

CharScanner::CharScanner( const LexerSharedInputState& state, bool case_sensitive )
	: saveConsumedInput(true) //, caseSensitiveLiterals(true)
	, caseSensitive(case_sensitive)
	, literals(CharScannerLiteralsLess(this))
	, inputState(state)
	, commitToPath(false)
	, tabsize(8)
	, traceDepth(0)
{
	setTokenObjectFactory(&CommonToken::factory);
}

void CharScanner::consume()
{
	if (inputState->guessing == 0)
	{
		int c = LA(1);
		if (caseSensitive)
		{
			append(c);
		}
		else
		{
			// use input.LA(), not LA(), to get original case
			// CharScanner.LA() would toLower it.
			append(inputState->getInput().LA(1));
		}

		// RK: in a sense I don't like this automatic handling.
		if (c == '\t')
			tab();
		else
			inputState->column++;
	}
	inputState->getInput().consume();
}

//bool CharScanner::getCaseSensitiveLiterals() const
//{ return caseSensitiveLiterals; }

void CharScanner::panic()
{
	ANTLR_USE_NAMESPACE(std)cerr << "CharScanner: panic" << ANTLR_USE_NAMESPACE(std)endl;
	exit(1);
}

void CharScanner::panic(const ANTLR_USE_NAMESPACE(std)string& s)
{
	ANTLR_USE_NAMESPACE(std)cerr << "CharScanner: panic: " << s.c_str() << ANTLR_USE_NAMESPACE(std)endl;
	exit(1);
}

/** Report exception errors caught in nextToken() */
void CharScanner::reportError(const RecognitionException& ex)
{
	ANTLR_USE_NAMESPACE(std)cerr << ex.toString().c_str() << ANTLR_USE_NAMESPACE(std)endl;
}

/** Parser error-reporting function can be overridden in subclass */
void CharScanner::reportError(const ANTLR_USE_NAMESPACE(std)string& s)
{
	if (getFilename() == "")
		ANTLR_USE_NAMESPACE(std)cerr << "error: " << s.c_str() << ANTLR_USE_NAMESPACE(std)endl;
	else
		ANTLR_USE_NAMESPACE(std)cerr << getFilename().c_str() << ": error: " << s.c_str() << ANTLR_USE_NAMESPACE(std)endl;
}

/** Parser warning-reporting function can be overridden in subclass */
void CharScanner::reportWarning(const ANTLR_USE_NAMESPACE(std)string& s)
{
	if (getFilename() == "")
		ANTLR_USE_NAMESPACE(std)cerr << "warning: " << s.c_str() << ANTLR_USE_NAMESPACE(std)endl;
	else
		ANTLR_USE_NAMESPACE(std)cerr << getFilename().c_str() << ": warning: " << s.c_str() << ANTLR_USE_NAMESPACE(std)endl;
}

void CharScanner::traceIndent()
{
	for( int i = 0; i < traceDepth; i++ )
		ANTLR_USE_NAMESPACE(std)cout << " ";
}

void CharScanner::traceIn(const char* rname)
{
	traceDepth++;
	traceIndent();
	ANTLR_USE_NAMESPACE(std)cout << "> lexer " << rname
		<< "; c==" << LA(1) << ANTLR_USE_NAMESPACE(std)endl;
}

void CharScanner::traceOut(const char* rname)
{
	traceIndent();
	ANTLR_USE_NAMESPACE(std)cout << "< lexer " << rname
		<< "; c==" << LA(1) << ANTLR_USE_NAMESPACE(std)endl;
	traceDepth--;
}

#ifndef NO_STATIC_CONSTS
const int CharScanner::NO_CHAR;
const int CharScanner::EOF_CHAR;
#endif

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif

