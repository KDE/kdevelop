/* $ANTLR 2.7.2: "ada.g" -> "AdaLexer.cpp"$ */
#include "AdaLexer.hpp"
#include <antlr/CharBuffer.hpp>
#include <antlr/TokenStreamException.hpp>
#include <antlr/TokenStreamIOException.hpp>
#include <antlr/TokenStreamRecognitionException.hpp>
#include <antlr/CharStreamException.hpp>
#include <antlr/CharStreamIOException.hpp>
#include <antlr/NoViableAltForCharException.hpp>

#line 1506 "ada.g"

#include "preambles.h"

#line 16 "AdaLexer.cpp"
AdaLexer::AdaLexer(ANTLR_USE_NAMESPACE(std)istream& in)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(new ANTLR_USE_NAMESPACE(antlr)CharBuffer(in),false)
{
	initLiterals();
}

AdaLexer::AdaLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(ib,false)
{
	initLiterals();
}

AdaLexer::AdaLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(state,false)
{
	initLiterals();
}

void AdaLexer::initLiterals()
{
	literals["until"] = 78;
	literals["xor"] = 86;
	literals["abstract"] = 40;
	literals["reverse"] = 71;
	literals["use"] = 13;
	literals["requeue"] = 84;
	literals["with"] = 11;
	literals["task"] = 42;
	literals["at"] = 49;
	literals["for"] = 47;
	literals["else"] = 68;
	literals["is"] = 23;
	literals["of"] = 55;
	literals["range"] = 16;
	literals["and"] = 85;
	literals["begin"] = 62;
	literals["procedure"] = 24;
	literals["separate"] = 39;
	literals["private"] = 20;
	literals["pragma"] = 4;
	literals["delay"] = 77;
	literals["abort"] = 80;
	literals["function"] = 25;
	literals["digits"] = 17;
	literals["declare"] = 72;
	literals["raise"] = 83;
	literals["not"] = 87;
	literals["record"] = 38;
	literals["all"] = 30;
	literals["in"] = 32;
	literals["generic"] = 61;
	literals["terminate"] = 82;
	literals["subtype"] = 51;
	literals["delta"] = 18;
	literals["when"] = 58;
	literals["null"] = 37;
	literals["entry"] = 46;
	literals["elsif"] = 67;
	literals["case"] = 57;
	literals["others"] = 27;
	literals["do"] = 76;
	literals["abs"] = 100;
	literals["constant"] = 53;
	literals["renames"] = 34;
	literals["then"] = 66;
	literals["exception"] = 52;
	literals["protected"] = 43;
	literals["accept"] = 75;
	literals["or"] = 81;
	literals["if"] = 65;
	literals["aliased"] = 56;
	literals["loop"] = 69;
	literals["return"] = 41;
	literals["limited"] = 60;
	literals["new"] = 26;
	literals["array"] = 54;
	literals["rem"] = 99;
	literals["end"] = 48;
	literals["body"] = 22;
	literals["mod"] = 50;
	literals["goto"] = 74;
	literals["tagged"] = 59;
	literals["select"] = 79;
	literals["while"] = 70;
	literals["package"] = 21;
	literals["type"] = 14;
	literals["access"] = 19;
	literals["out"] = 33;
	literals["exit"] = 73;
}

ANTLR_USE_NAMESPACE(antlr)RefToken AdaLexer::nextToken()
{
	ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
	for (;;) {
		ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
		int _ttype = ANTLR_USE_NAMESPACE(antlr)Token::INVALID_TYPE;
		resetText();
		try {   // for lexical and char stream error handling
			switch ( LA(1)) {
			case static_cast<unsigned char>('|'):
			{
				mPIPE(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('&'):
			{
				mCONCAT(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('+'):
			{
				mPLUS(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('('):
			{
				mLPAREN(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>(')'):
			{
				mRPAREN(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>(','):
			{
				mCOMMA(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>(';'):
			{
				mSEMI(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('a'):
			case static_cast<unsigned char>('b'):
			case static_cast<unsigned char>('c'):
			case static_cast<unsigned char>('d'):
			case static_cast<unsigned char>('e'):
			case static_cast<unsigned char>('f'):
			case static_cast<unsigned char>('g'):
			case static_cast<unsigned char>('h'):
			case static_cast<unsigned char>('i'):
			case static_cast<unsigned char>('j'):
			case static_cast<unsigned char>('k'):
			case static_cast<unsigned char>('l'):
			case static_cast<unsigned char>('m'):
			case static_cast<unsigned char>('n'):
			case static_cast<unsigned char>('o'):
			case static_cast<unsigned char>('p'):
			case static_cast<unsigned char>('q'):
			case static_cast<unsigned char>('r'):
			case static_cast<unsigned char>('s'):
			case static_cast<unsigned char>('t'):
			case static_cast<unsigned char>('u'):
			case static_cast<unsigned char>('v'):
			case static_cast<unsigned char>('w'):
			case static_cast<unsigned char>('x'):
			case static_cast<unsigned char>('y'):
			case static_cast<unsigned char>('z'):
			{
				mIDENTIFIER(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('"'):
			{
				mCHAR_STRING(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('0'):
			case static_cast<unsigned char>('1'):
			case static_cast<unsigned char>('2'):
			case static_cast<unsigned char>('3'):
			case static_cast<unsigned char>('4'):
			case static_cast<unsigned char>('5'):
			case static_cast<unsigned char>('6'):
			case static_cast<unsigned char>('7'):
			case static_cast<unsigned char>('8'):
			case static_cast<unsigned char>('9'):
			{
				mNUMERIC_LIT(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('\t'):
			case static_cast<unsigned char>('\n'):
			case static_cast<unsigned char>('\14'):
			case static_cast<unsigned char>('\r'):
			case static_cast<unsigned char>(' '):
			{
				mWS_(true);
				theRetToken=_returnToken;
				break;
			}
			default:
				if ((LA(1) == static_cast<unsigned char>('-')) && (LA(2) == static_cast<unsigned char>('-')) && ((LA(3) >= static_cast<unsigned char>('\3') && LA(3) <= static_cast<unsigned char>('\377')))) {
					mCOMMENT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('-')) && (LA(2) == static_cast<unsigned char>('-')) && (true)) {
					mCOMMENT_INTRO(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('.')) && (LA(2) == static_cast<unsigned char>('.'))) {
					mDOT_DOT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (LA(2) == static_cast<unsigned char>('<'))) {
					mLT_LT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (LA(2) == static_cast<unsigned char>('>'))) {
					mBOX(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (LA(2) == static_cast<unsigned char>('>'))) {
					mGT_GT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>(':')) && (LA(2) == static_cast<unsigned char>('='))) {
					mASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('=')) && (LA(2) == static_cast<unsigned char>('>'))) {
					mRIGHT_SHAFT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('/')) && (LA(2) == static_cast<unsigned char>('='))) {
					mNE(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (LA(2) == static_cast<unsigned char>('='))) {
					mLE(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (LA(2) == static_cast<unsigned char>('='))) {
					mGE(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('*')) && (LA(2) == static_cast<unsigned char>('*'))) {
					mEXPON(true);
					theRetToken=_returnToken;
				}
				else if (((LA(1) == static_cast<unsigned char>('\'')) && ((LA(2) >= static_cast<unsigned char>('\3') && LA(2) <= static_cast<unsigned char>('\377'))))&&( LA(3)=='\'' )) {
					mCHARACTER_LITERAL(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('.')) && (true)) {
					mDOT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('=')) && (true)) {
					mEQ(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (true)) {
					mLT_(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (true)) {
					mGT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('-')) && (true)) {
					mMINUS(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('*')) && (true)) {
					mSTAR(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('/')) && (true)) {
					mDIV(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>(':')) && (true)) {
					mCOLON(true);
					theRetToken=_returnToken;
				}
				else if (((LA(1) == static_cast<unsigned char>('\'')) && (true))&&( LA(3)!='\'' )) {
					mTIC(true);
					theRetToken=_returnToken;
				}
			else {
				if (LA(1)==EOF_CHAR)
				{
					uponEOF();
					_returnToken = makeToken(ANTLR_USE_NAMESPACE(antlr)Token::EOF_TYPE);
				}
				else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
			}
			}
			if ( !_returnToken )
				goto tryAgain; // found SKIP token

			_ttype = _returnToken->getType();
			_returnToken->setType(_ttype);
			return _returnToken;
		}
		catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& e) {
			{
				reportError(e);
				consume();
			}
		}
		catch (ANTLR_USE_NAMESPACE(antlr)CharStreamIOException& csie) {
			throw ANTLR_USE_NAMESPACE(antlr)TokenStreamIOException(csie.io);
		}
		catch (ANTLR_USE_NAMESPACE(antlr)CharStreamException& cse) {
			throw ANTLR_USE_NAMESPACE(antlr)TokenStreamException(cse.getMessage());
		}
tryAgain:;
	}
}

void AdaLexer::mCOMMENT_INTRO(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COMMENT_INTRO;
	int _saveIndex;
	
	try {      // for error handling
		match("--");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_0);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mDOT_DOT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DOT_DOT;
	int _saveIndex;
	
	try {      // for error handling
		match("..");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mLT_LT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LT_LT;
	int _saveIndex;
	
	try {      // for error handling
		match("<<");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mBOX(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BOX;
	int _saveIndex;
	
	try {      // for error handling
		match("<>");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mGT_GT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = GT_GT;
	int _saveIndex;
	
	try {      // for error handling
		match(">>");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = ASSIGN;
	int _saveIndex;
	
	try {      // for error handling
		match(":=");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mRIGHT_SHAFT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = RIGHT_SHAFT;
	int _saveIndex;
	
	try {      // for error handling
		match("=>");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mNE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = NE;
	int _saveIndex;
	
	try {      // for error handling
		match("/=");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mLE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LE;
	int _saveIndex;
	
	try {      // for error handling
		match("<=");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mGE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = GE;
	int _saveIndex;
	
	try {      // for error handling
		match(">=");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mEXPON(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EXPON;
	int _saveIndex;
	
	try {      // for error handling
		match("**");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mPIPE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = PIPE;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('|'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mCONCAT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = CONCAT;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('&'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mDOT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DOT;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('.'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mEQ(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EQ;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('='));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mLT_(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LT_;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('<'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mGT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = GT;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('>'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mPLUS(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = PLUS;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('+'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mMINUS(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = MINUS;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('-'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mSTAR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = STAR;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('*'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mDIV(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DIV;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('/'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mLPAREN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LPAREN;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('('));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mRPAREN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = RPAREN;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>(')'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mCOLON(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COLON;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>(':'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mCOMMA(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COMMA;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>(','));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mSEMI(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SEMI;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>(';'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mTIC(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = TIC;
	int _saveIndex;
	
	try {      // for error handling
		if (!( LA(3)!='\'' ))
			throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" LA(3)!='\\'' ");
		match(static_cast<unsigned char>('\''));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mIDENTIFIER(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = IDENTIFIER;
	int _saveIndex;
	
	try {      // for error handling
		{
		matchRange(static_cast<unsigned char>('a'),static_cast<unsigned char>('z'));
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_2.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case static_cast<unsigned char>('_'):
				{
					match(static_cast<unsigned char>('_'));
					break;
				}
				case static_cast<unsigned char>('0'):
				case static_cast<unsigned char>('1'):
				case static_cast<unsigned char>('2'):
				case static_cast<unsigned char>('3'):
				case static_cast<unsigned char>('4'):
				case static_cast<unsigned char>('5'):
				case static_cast<unsigned char>('6'):
				case static_cast<unsigned char>('7'):
				case static_cast<unsigned char>('8'):
				case static_cast<unsigned char>('9'):
				case static_cast<unsigned char>('a'):
				case static_cast<unsigned char>('b'):
				case static_cast<unsigned char>('c'):
				case static_cast<unsigned char>('d'):
				case static_cast<unsigned char>('e'):
				case static_cast<unsigned char>('f'):
				case static_cast<unsigned char>('g'):
				case static_cast<unsigned char>('h'):
				case static_cast<unsigned char>('i'):
				case static_cast<unsigned char>('j'):
				case static_cast<unsigned char>('k'):
				case static_cast<unsigned char>('l'):
				case static_cast<unsigned char>('m'):
				case static_cast<unsigned char>('n'):
				case static_cast<unsigned char>('o'):
				case static_cast<unsigned char>('p'):
				case static_cast<unsigned char>('q'):
				case static_cast<unsigned char>('r'):
				case static_cast<unsigned char>('s'):
				case static_cast<unsigned char>('t'):
				case static_cast<unsigned char>('u'):
				case static_cast<unsigned char>('v'):
				case static_cast<unsigned char>('w'):
				case static_cast<unsigned char>('x'):
				case static_cast<unsigned char>('y'):
				case static_cast<unsigned char>('z'):
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
				}
				}
				}
				{
				switch ( LA(1)) {
				case static_cast<unsigned char>('a'):
				case static_cast<unsigned char>('b'):
				case static_cast<unsigned char>('c'):
				case static_cast<unsigned char>('d'):
				case static_cast<unsigned char>('e'):
				case static_cast<unsigned char>('f'):
				case static_cast<unsigned char>('g'):
				case static_cast<unsigned char>('h'):
				case static_cast<unsigned char>('i'):
				case static_cast<unsigned char>('j'):
				case static_cast<unsigned char>('k'):
				case static_cast<unsigned char>('l'):
				case static_cast<unsigned char>('m'):
				case static_cast<unsigned char>('n'):
				case static_cast<unsigned char>('o'):
				case static_cast<unsigned char>('p'):
				case static_cast<unsigned char>('q'):
				case static_cast<unsigned char>('r'):
				case static_cast<unsigned char>('s'):
				case static_cast<unsigned char>('t'):
				case static_cast<unsigned char>('u'):
				case static_cast<unsigned char>('v'):
				case static_cast<unsigned char>('w'):
				case static_cast<unsigned char>('x'):
				case static_cast<unsigned char>('y'):
				case static_cast<unsigned char>('z'):
				{
					matchRange(static_cast<unsigned char>('a'),static_cast<unsigned char>('z'));
					break;
				}
				case static_cast<unsigned char>('0'):
				case static_cast<unsigned char>('1'):
				case static_cast<unsigned char>('2'):
				case static_cast<unsigned char>('3'):
				case static_cast<unsigned char>('4'):
				case static_cast<unsigned char>('5'):
				case static_cast<unsigned char>('6'):
				case static_cast<unsigned char>('7'):
				case static_cast<unsigned char>('8'):
				case static_cast<unsigned char>('9'):
				{
					matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
				}
				}
				}
			}
			else {
				goto _loop508;
			}
			
		}
		_loop508:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	_ttype = testLiteralsTable(_ttype);
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mCHARACTER_LITERAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = CHARACTER_LITERAL;
	int _saveIndex;
	
	try {      // for error handling
		if (!( LA(3)=='\'' ))
			throw ANTLR_USE_NAMESPACE(antlr)SemanticException(" LA(3)=='\\'' ");
		match("'");
		matchNot(EOF/*_CHAR*/);
		match("'");
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mCHAR_STRING(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = CHAR_STRING;
	int _saveIndex;
	
	try {      // for error handling
		match(static_cast<unsigned char>('"'));
		{ // ( ... )*
		for (;;) {
			if ((LA(1) == static_cast<unsigned char>('"')) && (LA(2) == static_cast<unsigned char>('"'))) {
				match("\"\"");
			}
			else if ((_tokenSet_3.member(LA(1)))) {
				{
				match(_tokenSet_3);
				}
			}
			else {
				goto _loop513;
			}
			
		}
		_loop513:;
		} // ( ... )*
		match(static_cast<unsigned char>('"'));
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mNUMERIC_LIT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = NUMERIC_LIT;
	int _saveIndex;
	
	try {      // for error handling
		{ // ( ... )+
		int _cnt516=0;
		for (;;) {
			if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
				mDIGIT(false);
			}
			else {
				if ( _cnt516>=1 ) { goto _loop516; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
			}
			
			_cnt516++;
		}
		_loop516:;
		}  // ( ... )+
		{
		switch ( LA(1)) {
		case static_cast<unsigned char>('#'):
		{
			match(static_cast<unsigned char>('#'));
			mBASED_INTEGER(false);
			{
			switch ( LA(1)) {
			case static_cast<unsigned char>('.'):
			{
				match(static_cast<unsigned char>('.'));
				mBASED_INTEGER(false);
				break;
			}
			case static_cast<unsigned char>('#'):
			{
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
			}
			}
			}
			match(static_cast<unsigned char>('#'));
			break;
		}
		case static_cast<unsigned char>('_'):
		{
			{ // ( ... )+
			int _cnt522=0;
			for (;;) {
				if ((LA(1) == static_cast<unsigned char>('_'))) {
					match(static_cast<unsigned char>('_'));
					{ // ( ... )+
					int _cnt521=0;
					for (;;) {
						if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
							mDIGIT(false);
						}
						else {
							if ( _cnt521>=1 ) { goto _loop521; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
						}
						
						_cnt521++;
					}
					_loop521:;
					}  // ( ... )+
				}
				else {
					if ( _cnt522>=1 ) { goto _loop522; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
				}
				
				_cnt522++;
			}
			_loop522:;
			}  // ( ... )+
			break;
		}
		default:
			{
			}
		}
		}
		{
		if (((LA(1) == static_cast<unsigned char>('.') || LA(1) == static_cast<unsigned char>('e')))&&( LA(2)!='.' )) {
			{
			switch ( LA(1)) {
			case static_cast<unsigned char>('.'):
			{
				match(static_cast<unsigned char>('.'));
				{ // ( ... )+
				int _cnt526=0;
				for (;;) {
					if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
						mDIGIT(false);
					}
					else {
						if ( _cnt526>=1 ) { goto _loop526; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
					}
					
					_cnt526++;
				}
				_loop526:;
				}  // ( ... )+
				{ // ( ... )*
				for (;;) {
					if ((LA(1) == static_cast<unsigned char>('_'))) {
						match(static_cast<unsigned char>('_'));
						{ // ( ... )+
						int _cnt529=0;
						for (;;) {
							if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
								mDIGIT(false);
							}
							else {
								if ( _cnt529>=1 ) { goto _loop529; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
							}
							
							_cnt529++;
						}
						_loop529:;
						}  // ( ... )+
					}
					else {
						goto _loop530;
					}
					
				}
				_loop530:;
				} // ( ... )*
				{
				if ((LA(1) == static_cast<unsigned char>('e'))) {
					mEXPONENT(false);
				}
				else {
				}
				
				}
				break;
			}
			case static_cast<unsigned char>('e'):
			{
				mEXPONENT(false);
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
			}
			}
			}
		}
		else {
		}
		
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mDIGIT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DIGIT;
	int _saveIndex;
	
	try {      // for error handling
		{
		matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_4);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mBASED_INTEGER(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BASED_INTEGER;
	int _saveIndex;
	
	try {      // for error handling
		{
		mEXTENDED_DIGIT(false);
		}
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_5.member(LA(1)))) {
				{
				switch ( LA(1)) {
				case static_cast<unsigned char>('_'):
				{
					match(static_cast<unsigned char>('_'));
					break;
				}
				case static_cast<unsigned char>('0'):
				case static_cast<unsigned char>('1'):
				case static_cast<unsigned char>('2'):
				case static_cast<unsigned char>('3'):
				case static_cast<unsigned char>('4'):
				case static_cast<unsigned char>('5'):
				case static_cast<unsigned char>('6'):
				case static_cast<unsigned char>('7'):
				case static_cast<unsigned char>('8'):
				case static_cast<unsigned char>('9'):
				case static_cast<unsigned char>('a'):
				case static_cast<unsigned char>('b'):
				case static_cast<unsigned char>('c'):
				case static_cast<unsigned char>('d'):
				case static_cast<unsigned char>('e'):
				case static_cast<unsigned char>('f'):
				{
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
				}
				}
				}
				mEXTENDED_DIGIT(false);
			}
			else {
				goto _loop545;
			}
			
		}
		_loop545:;
		} // ( ... )*
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_6);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mEXPONENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EXPONENT;
	int _saveIndex;
	
	try {      // for error handling
		{
		match(static_cast<unsigned char>('e'));
		}
		{
		switch ( LA(1)) {
		case static_cast<unsigned char>('+'):
		{
			match(static_cast<unsigned char>('+'));
			break;
		}
		case static_cast<unsigned char>('-'):
		{
			match(static_cast<unsigned char>('-'));
			break;
		}
		case static_cast<unsigned char>('0'):
		case static_cast<unsigned char>('1'):
		case static_cast<unsigned char>('2'):
		case static_cast<unsigned char>('3'):
		case static_cast<unsigned char>('4'):
		case static_cast<unsigned char>('5'):
		case static_cast<unsigned char>('6'):
		case static_cast<unsigned char>('7'):
		case static_cast<unsigned char>('8'):
		case static_cast<unsigned char>('9'):
		{
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
		}
		}
		}
		{ // ( ... )+
		int _cnt538=0;
		for (;;) {
			if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
				mDIGIT(false);
			}
			else {
				if ( _cnt538>=1 ) { goto _loop538; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
			}
			
			_cnt538++;
		}
		_loop538:;
		}  // ( ... )+
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mEXTENDED_DIGIT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EXTENDED_DIGIT;
	int _saveIndex;
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case static_cast<unsigned char>('0'):
		case static_cast<unsigned char>('1'):
		case static_cast<unsigned char>('2'):
		case static_cast<unsigned char>('3'):
		case static_cast<unsigned char>('4'):
		case static_cast<unsigned char>('5'):
		case static_cast<unsigned char>('6'):
		case static_cast<unsigned char>('7'):
		case static_cast<unsigned char>('8'):
		case static_cast<unsigned char>('9'):
		{
			mDIGIT(false);
			break;
		}
		case static_cast<unsigned char>('a'):
		case static_cast<unsigned char>('b'):
		case static_cast<unsigned char>('c'):
		case static_cast<unsigned char>('d'):
		case static_cast<unsigned char>('e'):
		case static_cast<unsigned char>('f'):
		{
			matchRange(static_cast<unsigned char>('a'),static_cast<unsigned char>('f'));
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
		}
		}
		}
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_4);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mWS_(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = WS_;
	int _saveIndex;
	
	try {      // for error handling
		{
		switch ( LA(1)) {
		case static_cast<unsigned char>(' '):
		{
			match(static_cast<unsigned char>(' '));
			break;
		}
		case static_cast<unsigned char>('\t'):
		{
			match(static_cast<unsigned char>('\t'));
			break;
		}
		case static_cast<unsigned char>('\14'):
		{
			match(static_cast<unsigned char>('\14'));
			break;
		}
		case static_cast<unsigned char>('\n'):
		case static_cast<unsigned char>('\r'):
		{
			{
			if ((LA(1) == static_cast<unsigned char>('\r')) && (LA(2) == static_cast<unsigned char>('\n'))) {
				match("\r\n");
			}
			else if ((LA(1) == static_cast<unsigned char>('\r')) && (true)) {
				match(static_cast<unsigned char>('\r'));
			}
			else if ((LA(1) == static_cast<unsigned char>('\n'))) {
				match(static_cast<unsigned char>('\n'));
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
			}
			
			}
#line 1939 "ada.g"
			newline();
#line 1538 "AdaLexer.cpp"
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
		}
		}
		}
#line 1941 "ada.g"
		_ttype = antlr::Token::SKIP;
#line 1549 "AdaLexer.cpp"
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void AdaLexer::mCOMMENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COMMENT;
	int _saveIndex;
	
	try {      // for error handling
		{
		mCOMMENT_INTRO(false);
		{ // ( ... )*
		for (;;) {
			if ((_tokenSet_7.member(LA(1)))) {
				{
				match(_tokenSet_7);
				}
			}
			else {
				goto _loop553;
			}
			
		}
		_loop553:;
		} // ( ... )*
		{
		switch ( LA(1)) {
		case static_cast<unsigned char>('\n'):
		{
			match(static_cast<unsigned char>('\n'));
			break;
		}
		case static_cast<unsigned char>('\r'):
		{
			match(static_cast<unsigned char>('\r'));
			{
			if ((LA(1) == static_cast<unsigned char>('\n'))) {
				match(static_cast<unsigned char>('\n'));
			}
			else {
			}
			
			}
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
		}
		}
		}
		}
#line 1946 "ada.g"
		_ttype = antlr::Token::SKIP; newline();
#line 1615 "AdaLexer.cpp"
	}
	catch (ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex) {
		reportError(ex);
		consume();
		consumeUntil(_tokenSet_1);
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}


const unsigned long AdaLexer::_tokenSet_0_data_[] = { 4294967288UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NULL_TREE_LOOKAHEAD "pragma" IDENTIFIER SEMI LPAREN COMMA RPAREN RIGHT_SHAFT 
// "with" DOT "use" "type" TIC "range" "digits" "delta" "access" "private" 
// "package" "body" "is" "procedure" "function" "new" "others" PIPE DOT_DOT 
// "all" COLON "in" "out" "renames" CHARACTER_LITERAL CHAR_STRING "null" 
// "record" "separate" "abstract" "return" "task" "protected" BOX ASSIGN 
// "entry" "for" "end" "at" "mod" "subtype" "exception" "constant" "array" 
// "of" "aliased" "case" "when" "tagged" "limited" "generic" "begin" LT_LT 
// GT_GT "if" "then" "elsif" "else" "loop" "while" "reverse" "declare" 
// "exit" "goto" "accept" "do" "delay" "until" "select" "abort" "or" "terminate" 
// "raise" "requeue" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT 
// STAR DIV "rem" "abs" EXPON NUMERIC_LIT ABORTABLE_PART ABORT_STATEMENT 
// ABSTRACT_SUBPROGRAM_DECLARATION ACCEPT_ALTERNATIVE ACCEPT_STATEMENT 
// ALLOCATOR ASSIGNMENT_STATEMENT ASYNCHRONOUS_SELECT ATTRIBUTE_DEFINITION_CLAUSE 
// AT_CLAUSE BLOCK_STATEMENT CASE_STATEMENT CASE_STATEMENT_ALTERNATIVE 
// CODE_STATEMENT COMPONENT_DECLARATION COMPONENT_LIST CONDITION CONDITIONAL_ENTRY_CALL 
// CONTEXT_CLAUSE DECLARATIVE_ITEM DECLARATIVE_PART DEFINING_IDENTIFIER_LIST 
// DELAY_ALTERNATIVE DELAY_STATEMENT DELTA_CONSTRAINT DIGITS_CONSTRAINT 
// DISCRETE_RANGE DISCRIMINANT_ASSOCIATION DISCRIMINANT_CONSTRAINT DISCRIMINANT_SPECIFICATION 
// ENTRY_BODY ENTRY_CALL_ALTERNATIVE ENTRY_CALL_STATEMENT ENTRY_DECLARATION 
// ENTRY_INDEX_SPECIFICATION ENUMERATION_REPESENTATION_CLAUSE EXCEPTION_DECLARATION 
// EXCEPTION_HANDLER EXCEPTION_RENAMING_DECLARATION EXIT_STATEMENT FORMAL_PACKAGE_DECLARATION 
// FORMAL_TYPE_DECLARATION FULL_TYPE_DECLARATION GENERIC_FORMAL_PART GENERIC_INSTANTIATION 
// GENERIC_PACKAGE_DECLARATION GENERIC_RENAMING_DECLARATION GENERIC_SUBPROGRAM_DECLARATION 
// GOTO_STATEMENT HANDLED_SEQUENCE_OF_STATEMENTS IF_STATEMENT INCOMPLETE_TYPE_DECLARATION 
// INDEXED_COMPONENT INDEX_CONSTRAINT LIBRARY_ITEM LOOP_STATEMENT NAME 
// NULL_STATEMENT NUMBER_DECLARATION OBJECT_DECLARATION OBJECT_RENAMING_DECLARATION 
// OPERATOR_SYMBOL PACKAGE_BODY PACKAGE_BODY_STUB PACKAGE_RENAMING_DECLARATION 
// PACKAGE_SPECIFICATION PARAMETER_SPECIFICATION PREFIX PRIMARY PRIVATE_EXTENSION_DECLARATION 
// PRIVATE_TYPE_DECLARATION PROCEDURE_CALL_STATEMENT PROTECTED_BODY PROTECTED_BODY_STUB 
// PROTECTED_TYPE_DECLARATION RAISE_STATEMENT RANGE_ATTRIBUTE_REFERENCE 
// RECORD_REPRESENTATION_CLAUSE REQUEUE_STATEMENT RETURN_STATEMENT SELECTIVE_ACCEPT 
// SELECT_ALTERNATIVE SELECT_STATEMENT SEQUENCE_OF_STATEMENTS SINGLE_PROTECTED_DECLARATION 
// SINGLE_TASK_DECLARATION STATEMENT SUBPROGRAM_BODY SUBPROGRAM_BODY_STUB 
// SUBPROGRAM_DECLARATION SUBPROGRAM_RENAMING_DECLARATION SUBTYPE_DECLARATION 
// SUBTYPE_INDICATION SUBTYPE_MARK SUBUNIT TASK_BODY TASK_BODY_STUB TASK_TYPE_DECLARATION 
// TERMINATE_ALTERNATIVE TIMED_ENTRY_CALL TRIGGERING_ALTERNATIVE TYPE_DECLARATION 
// USE_CLAUSE USE_TYPE_CLAUSE VARIANT VARIANT_PART WITH_CLAUSE ABSTRACT_FUNCTION_DECLARATION 
// ABSTRACT_PROCEDURE_DECLARATION ACCESS_TO_FUNCTION_DECLARATION ACCESS_TO_OBJECT_DECLARATION 
// ACCESS_TO_PROCEDURE_DECLARATION ACCESS_TYPE_DECLARATION ARRAY_OBJECT_DECLARATION 
// ARRAY_TYPE_DECLARATION AND_THEN BASIC_DECLARATIVE_ITEMS_OPT BLOCK_BODY 
// BLOCK_BODY_OPT CALL_STATEMENT COMPONENT_CLAUSES_OPT COMPONENT_ITEMS 
// COND_CLAUSE DECIMAL_FIXED_POINT_DECLARATION DECLARE_OPT DERIVED_RECORD_EXTENSION 
// DERIVED_TYPE_DECLARATION DISCRETE_SUBTYPE_DEF_OPT DISCRIMINANT_SPECIFICATIONS 
// DISCRIM_PART_OPT ELSE_OPT ELSIFS_OPT ENTRY_INDEX_OPT ENUMERATION_TYPE_DECLARATION 
// EXCEPT_HANDLER_PART_OPT EXTENSION_OPT FLOATING_POINT_DECLARATION FORMAL_DECIMAL_FIXED_POINT_DECLARATION 
// FORMAL_DISCRETE_TYPE_DECLARATION FORMAL_FLOATING_POINT_DECLARATION FORMAL_FUNCTION_DECLARATION 
// FORMAL_MODULAR_TYPE_DECLARATION FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION 
// FORMAL_ORDINARY_FIXED_POINT_DECLARATION FORMAL_PART_OPT FORMAL_PRIVATE_EXTENSION_DECLARATION 
// FORMAL_PRIVATE_TYPE_DECLARATION FORMAL_PROCEDURE_DECLARATION FORMAL_SIGNED_INTEGER_TYPE_DECLARATION 
// FUNCTION_BODY FUNCTION_BODY_STUB FUNCTION_DECLARATION FUNCTION_RENAMING_DECLARATION 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_0(_tokenSet_0_data_,16);
const unsigned long AdaLexer::_tokenSet_1_data_[] = { 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_1(_tokenSet_1_data_,10);
const unsigned long AdaLexer::_tokenSet_2_data_[] = { 0UL, 67043328UL, 2147483648UL, 134217726UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "at" "mod" "subtype" "exception" "constant" "array" "of" "aliased" 
// "case" MINUS STAR DIV "rem" "abs" EXPON NUMERIC_LIT ABORTABLE_PART ABORT_STATEMENT 
// ABSTRACT_SUBPROGRAM_DECLARATION ACCEPT_ALTERNATIVE ACCEPT_STATEMENT 
// ALLOCATOR ASSIGNMENT_STATEMENT ASYNCHRONOUS_SELECT ATTRIBUTE_DEFINITION_CLAUSE 
// AT_CLAUSE BLOCK_STATEMENT CASE_STATEMENT CASE_STATEMENT_ALTERNATIVE 
// CODE_STATEMENT COMPONENT_DECLARATION COMPONENT_LIST CONDITION CONDITIONAL_ENTRY_CALL 
// CONTEXT_CLAUSE DECLARATIVE_ITEM 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_2(_tokenSet_2_data_,10);
const unsigned long AdaLexer::_tokenSet_3_data_[] = { 4294967288UL, 4294967291UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NULL_TREE_LOOKAHEAD "pragma" IDENTIFIER SEMI LPAREN COMMA RPAREN RIGHT_SHAFT 
// "with" DOT "use" "type" TIC "range" "digits" "delta" "access" "private" 
// "package" "body" "is" "procedure" "function" "new" "others" PIPE DOT_DOT 
// "all" COLON "in" "out" CHARACTER_LITERAL CHAR_STRING "null" "record" 
// "separate" "abstract" "return" "task" "protected" BOX ASSIGN "entry" 
// "for" "end" "at" "mod" "subtype" "exception" "constant" "array" "of" 
// "aliased" "case" "when" "tagged" "limited" "generic" "begin" LT_LT GT_GT 
// "if" "then" "elsif" "else" "loop" "while" "reverse" "declare" "exit" 
// "goto" "accept" "do" "delay" "until" "select" "abort" "or" "terminate" 
// "raise" "requeue" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT 
// STAR DIV "rem" "abs" EXPON NUMERIC_LIT ABORTABLE_PART ABORT_STATEMENT 
// ABSTRACT_SUBPROGRAM_DECLARATION ACCEPT_ALTERNATIVE ACCEPT_STATEMENT 
// ALLOCATOR ASSIGNMENT_STATEMENT ASYNCHRONOUS_SELECT ATTRIBUTE_DEFINITION_CLAUSE 
// AT_CLAUSE BLOCK_STATEMENT CASE_STATEMENT CASE_STATEMENT_ALTERNATIVE 
// CODE_STATEMENT COMPONENT_DECLARATION COMPONENT_LIST CONDITION CONDITIONAL_ENTRY_CALL 
// CONTEXT_CLAUSE DECLARATIVE_ITEM DECLARATIVE_PART DEFINING_IDENTIFIER_LIST 
// DELAY_ALTERNATIVE DELAY_STATEMENT DELTA_CONSTRAINT DIGITS_CONSTRAINT 
// DISCRETE_RANGE DISCRIMINANT_ASSOCIATION DISCRIMINANT_CONSTRAINT DISCRIMINANT_SPECIFICATION 
// ENTRY_BODY ENTRY_CALL_ALTERNATIVE ENTRY_CALL_STATEMENT ENTRY_DECLARATION 
// ENTRY_INDEX_SPECIFICATION ENUMERATION_REPESENTATION_CLAUSE EXCEPTION_DECLARATION 
// EXCEPTION_HANDLER EXCEPTION_RENAMING_DECLARATION EXIT_STATEMENT FORMAL_PACKAGE_DECLARATION 
// FORMAL_TYPE_DECLARATION FULL_TYPE_DECLARATION GENERIC_FORMAL_PART GENERIC_INSTANTIATION 
// GENERIC_PACKAGE_DECLARATION GENERIC_RENAMING_DECLARATION GENERIC_SUBPROGRAM_DECLARATION 
// GOTO_STATEMENT HANDLED_SEQUENCE_OF_STATEMENTS IF_STATEMENT INCOMPLETE_TYPE_DECLARATION 
// INDEXED_COMPONENT INDEX_CONSTRAINT LIBRARY_ITEM LOOP_STATEMENT NAME 
// NULL_STATEMENT NUMBER_DECLARATION OBJECT_DECLARATION OBJECT_RENAMING_DECLARATION 
// OPERATOR_SYMBOL PACKAGE_BODY PACKAGE_BODY_STUB PACKAGE_RENAMING_DECLARATION 
// PACKAGE_SPECIFICATION PARAMETER_SPECIFICATION PREFIX PRIMARY PRIVATE_EXTENSION_DECLARATION 
// PRIVATE_TYPE_DECLARATION PROCEDURE_CALL_STATEMENT PROTECTED_BODY PROTECTED_BODY_STUB 
// PROTECTED_TYPE_DECLARATION RAISE_STATEMENT RANGE_ATTRIBUTE_REFERENCE 
// RECORD_REPRESENTATION_CLAUSE REQUEUE_STATEMENT RETURN_STATEMENT SELECTIVE_ACCEPT 
// SELECT_ALTERNATIVE SELECT_STATEMENT SEQUENCE_OF_STATEMENTS SINGLE_PROTECTED_DECLARATION 
// SINGLE_TASK_DECLARATION STATEMENT SUBPROGRAM_BODY SUBPROGRAM_BODY_STUB 
// SUBPROGRAM_DECLARATION SUBPROGRAM_RENAMING_DECLARATION SUBTYPE_DECLARATION 
// SUBTYPE_INDICATION SUBTYPE_MARK SUBUNIT TASK_BODY TASK_BODY_STUB TASK_TYPE_DECLARATION 
// TERMINATE_ALTERNATIVE TIMED_ENTRY_CALL TRIGGERING_ALTERNATIVE TYPE_DECLARATION 
// USE_CLAUSE USE_TYPE_CLAUSE VARIANT VARIANT_PART WITH_CLAUSE ABSTRACT_FUNCTION_DECLARATION 
// ABSTRACT_PROCEDURE_DECLARATION ACCESS_TO_FUNCTION_DECLARATION ACCESS_TO_OBJECT_DECLARATION 
// ACCESS_TO_PROCEDURE_DECLARATION ACCESS_TYPE_DECLARATION ARRAY_OBJECT_DECLARATION 
// ARRAY_TYPE_DECLARATION AND_THEN BASIC_DECLARATIVE_ITEMS_OPT BLOCK_BODY 
// BLOCK_BODY_OPT CALL_STATEMENT COMPONENT_CLAUSES_OPT COMPONENT_ITEMS 
// COND_CLAUSE DECIMAL_FIXED_POINT_DECLARATION DECLARE_OPT DERIVED_RECORD_EXTENSION 
// DERIVED_TYPE_DECLARATION DISCRETE_SUBTYPE_DEF_OPT DISCRIMINANT_SPECIFICATIONS 
// DISCRIM_PART_OPT ELSE_OPT ELSIFS_OPT ENTRY_INDEX_OPT ENUMERATION_TYPE_DECLARATION 
// EXCEPT_HANDLER_PART_OPT EXTENSION_OPT FLOATING_POINT_DECLARATION FORMAL_DECIMAL_FIXED_POINT_DECLARATION 
// FORMAL_DISCRETE_TYPE_DECLARATION FORMAL_FLOATING_POINT_DECLARATION FORMAL_FUNCTION_DECLARATION 
// FORMAL_MODULAR_TYPE_DECLARATION FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION 
// FORMAL_ORDINARY_FIXED_POINT_DECLARATION FORMAL_PART_OPT FORMAL_PRIVATE_EXTENSION_DECLARATION 
// FORMAL_PRIVATE_TYPE_DECLARATION FORMAL_PROCEDURE_DECLARATION FORMAL_SIGNED_INTEGER_TYPE_DECLARATION 
// FUNCTION_BODY FUNCTION_BODY_STUB FUNCTION_DECLARATION FUNCTION_RENAMING_DECLARATION 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_3(_tokenSet_3_data_,16);
const unsigned long AdaLexer::_tokenSet_4_data_[] = { 0UL, 67059720UL, 2147483648UL, 126UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// CHARACTER_LITERAL "entry" "end" "at" "mod" "subtype" "exception" "constant" 
// "array" "of" "aliased" "case" MINUS STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_4(_tokenSet_4_data_,10);
const unsigned long AdaLexer::_tokenSet_5_data_[] = { 0UL, 67043328UL, 2147483648UL, 126UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// "end" "at" "mod" "subtype" "exception" "constant" "array" "of" "aliased" 
// "case" MINUS STAR DIV "rem" "abs" EXPON NUMERIC_LIT 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_5(_tokenSet_5_data_,10);
const unsigned long AdaLexer::_tokenSet_6_data_[] = { 0UL, 16392UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// CHARACTER_LITERAL "entry" 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_6(_tokenSet_6_data_,10);
const unsigned long AdaLexer::_tokenSet_7_data_[] = { 4294958072UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NULL_TREE_LOOKAHEAD "pragma" IDENTIFIER SEMI LPAREN COMMA RPAREN "with" 
// DOT "type" TIC "range" "digits" "delta" "access" "private" "package" 
// "body" "is" "procedure" "function" "new" "others" PIPE DOT_DOT "all" 
// COLON "in" "out" "renames" CHARACTER_LITERAL CHAR_STRING "null" "record" 
// "separate" "abstract" "return" "task" "protected" BOX ASSIGN "entry" 
// "for" "end" "at" "mod" "subtype" "exception" "constant" "array" "of" 
// "aliased" "case" "when" "tagged" "limited" "generic" "begin" LT_LT GT_GT 
// "if" "then" "elsif" "else" "loop" "while" "reverse" "declare" "exit" 
// "goto" "accept" "do" "delay" "until" "select" "abort" "or" "terminate" 
// "raise" "requeue" "and" "xor" "not" EQ NE LT_ LE GT GE PLUS MINUS CONCAT 
// STAR DIV "rem" "abs" EXPON NUMERIC_LIT ABORTABLE_PART ABORT_STATEMENT 
// ABSTRACT_SUBPROGRAM_DECLARATION ACCEPT_ALTERNATIVE ACCEPT_STATEMENT 
// ALLOCATOR ASSIGNMENT_STATEMENT ASYNCHRONOUS_SELECT ATTRIBUTE_DEFINITION_CLAUSE 
// AT_CLAUSE BLOCK_STATEMENT CASE_STATEMENT CASE_STATEMENT_ALTERNATIVE 
// CODE_STATEMENT COMPONENT_DECLARATION COMPONENT_LIST CONDITION CONDITIONAL_ENTRY_CALL 
// CONTEXT_CLAUSE DECLARATIVE_ITEM DECLARATIVE_PART DEFINING_IDENTIFIER_LIST 
// DELAY_ALTERNATIVE DELAY_STATEMENT DELTA_CONSTRAINT DIGITS_CONSTRAINT 
// DISCRETE_RANGE DISCRIMINANT_ASSOCIATION DISCRIMINANT_CONSTRAINT DISCRIMINANT_SPECIFICATION 
// ENTRY_BODY ENTRY_CALL_ALTERNATIVE ENTRY_CALL_STATEMENT ENTRY_DECLARATION 
// ENTRY_INDEX_SPECIFICATION ENUMERATION_REPESENTATION_CLAUSE EXCEPTION_DECLARATION 
// EXCEPTION_HANDLER EXCEPTION_RENAMING_DECLARATION EXIT_STATEMENT FORMAL_PACKAGE_DECLARATION 
// FORMAL_TYPE_DECLARATION FULL_TYPE_DECLARATION GENERIC_FORMAL_PART GENERIC_INSTANTIATION 
// GENERIC_PACKAGE_DECLARATION GENERIC_RENAMING_DECLARATION GENERIC_SUBPROGRAM_DECLARATION 
// GOTO_STATEMENT HANDLED_SEQUENCE_OF_STATEMENTS IF_STATEMENT INCOMPLETE_TYPE_DECLARATION 
// INDEXED_COMPONENT INDEX_CONSTRAINT LIBRARY_ITEM LOOP_STATEMENT NAME 
// NULL_STATEMENT NUMBER_DECLARATION OBJECT_DECLARATION OBJECT_RENAMING_DECLARATION 
// OPERATOR_SYMBOL PACKAGE_BODY PACKAGE_BODY_STUB PACKAGE_RENAMING_DECLARATION 
// PACKAGE_SPECIFICATION PARAMETER_SPECIFICATION PREFIX PRIMARY PRIVATE_EXTENSION_DECLARATION 
// PRIVATE_TYPE_DECLARATION PROCEDURE_CALL_STATEMENT PROTECTED_BODY PROTECTED_BODY_STUB 
// PROTECTED_TYPE_DECLARATION RAISE_STATEMENT RANGE_ATTRIBUTE_REFERENCE 
// RECORD_REPRESENTATION_CLAUSE REQUEUE_STATEMENT RETURN_STATEMENT SELECTIVE_ACCEPT 
// SELECT_ALTERNATIVE SELECT_STATEMENT SEQUENCE_OF_STATEMENTS SINGLE_PROTECTED_DECLARATION 
// SINGLE_TASK_DECLARATION STATEMENT SUBPROGRAM_BODY SUBPROGRAM_BODY_STUB 
// SUBPROGRAM_DECLARATION SUBPROGRAM_RENAMING_DECLARATION SUBTYPE_DECLARATION 
// SUBTYPE_INDICATION SUBTYPE_MARK SUBUNIT TASK_BODY TASK_BODY_STUB TASK_TYPE_DECLARATION 
// TERMINATE_ALTERNATIVE TIMED_ENTRY_CALL TRIGGERING_ALTERNATIVE TYPE_DECLARATION 
// USE_CLAUSE USE_TYPE_CLAUSE VARIANT VARIANT_PART WITH_CLAUSE ABSTRACT_FUNCTION_DECLARATION 
// ABSTRACT_PROCEDURE_DECLARATION ACCESS_TO_FUNCTION_DECLARATION ACCESS_TO_OBJECT_DECLARATION 
// ACCESS_TO_PROCEDURE_DECLARATION ACCESS_TYPE_DECLARATION ARRAY_OBJECT_DECLARATION 
// ARRAY_TYPE_DECLARATION AND_THEN BASIC_DECLARATIVE_ITEMS_OPT BLOCK_BODY 
// BLOCK_BODY_OPT CALL_STATEMENT COMPONENT_CLAUSES_OPT COMPONENT_ITEMS 
// COND_CLAUSE DECIMAL_FIXED_POINT_DECLARATION DECLARE_OPT DERIVED_RECORD_EXTENSION 
// DERIVED_TYPE_DECLARATION DISCRETE_SUBTYPE_DEF_OPT DISCRIMINANT_SPECIFICATIONS 
// DISCRIM_PART_OPT ELSE_OPT ELSIFS_OPT ENTRY_INDEX_OPT ENUMERATION_TYPE_DECLARATION 
// EXCEPT_HANDLER_PART_OPT EXTENSION_OPT FLOATING_POINT_DECLARATION FORMAL_DECIMAL_FIXED_POINT_DECLARATION 
// FORMAL_DISCRETE_TYPE_DECLARATION FORMAL_FLOATING_POINT_DECLARATION FORMAL_FUNCTION_DECLARATION 
// FORMAL_MODULAR_TYPE_DECLARATION FORMAL_ORDINARY_DERIVED_TYPE_DECLARATION 
// FORMAL_ORDINARY_FIXED_POINT_DECLARATION FORMAL_PART_OPT FORMAL_PRIVATE_EXTENSION_DECLARATION 
// FORMAL_PRIVATE_TYPE_DECLARATION FORMAL_PROCEDURE_DECLARATION FORMAL_SIGNED_INTEGER_TYPE_DECLARATION 
// FUNCTION_BODY FUNCTION_BODY_STUB FUNCTION_DECLARATION FUNCTION_RENAMING_DECLARATION 
const ANTLR_USE_NAMESPACE(antlr)BitSet AdaLexer::_tokenSet_7(_tokenSet_7_data_,16);

