/* $ANTLR 2.7.2: "java.g" -> "JavaLexer.cpp"$ */
#include "JavaLexer.hpp"
#include <antlr/CharBuffer.hpp>
#include <antlr/TokenStreamException.hpp>
#include <antlr/TokenStreamIOException.hpp>
#include <antlr/TokenStreamRecognitionException.hpp>
#include <antlr/CharStreamException.hpp>
#include <antlr/CharStreamIOException.hpp>
#include <antlr/NoViableAltForCharException.hpp>

#line 1041 "java.g"

#include <string>

#line 16 "JavaLexer.cpp"
JavaLexer::JavaLexer(ANTLR_USE_NAMESPACE(std)istream& in)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(new ANTLR_USE_NAMESPACE(antlr)CharBuffer(in),true)
{
	initLiterals();
}

JavaLexer::JavaLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(ib,true)
{
	initLiterals();
}

JavaLexer::JavaLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(state,true)
{
	initLiterals();
}

void JavaLexer::initLiterals()
{
	literals["byte"] = 51;
	literals["public"] = 62;
	literals["case"] = 94;
	literals["short"] = 53;
	literals["break"] = 89;
	literals["while"] = 87;
	literals["new"] = 137;
	literals["instanceof"] = 122;
	literals["implements"] = 76;
	literals["synchronized"] = 68;
	literals["float"] = 55;
	literals["package"] = 44;
	literals["return"] = 91;
	literals["throw"] = 93;
	literals["null"] = 136;
	literals["threadsafe"] = 67;
	literals["protected"] = 63;
	literals["class"] = 70;
	literals["throws"] = 82;
	literals["do"] = 88;
	literals["strictfp"] = 41;
	literals["super"] = 80;
	literals["transient"] = 65;
	literals["native"] = 66;
	literals["interface"] = 72;
	literals["final"] = 39;
	literals["if"] = 84;
	literals["double"] = 57;
	literals["volatile"] = 69;
	literals["catch"] = 98;
	literals["try"] = 96;
	literals["int"] = 54;
	literals["for"] = 86;
	literals["extends"] = 71;
	literals["boolean"] = 50;
	literals["char"] = 52;
	literals["private"] = 61;
	literals["default"] = 95;
	literals["false"] = 135;
	literals["this"] = 79;
	literals["static"] = 64;
	literals["abstract"] = 40;
	literals["continue"] = 90;
	literals["finally"] = 97;
	literals["else"] = 85;
	literals["import"] = 46;
	literals["void"] = 49;
	literals["switch"] = 92;
	literals["true"] = 134;
	literals["long"] = 56;
}

ANTLR_USE_NAMESPACE(antlr)RefToken JavaLexer::nextToken()
{
	ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
	for (;;) {
		ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
		int _ttype = ANTLR_USE_NAMESPACE(antlr)Token::INVALID_TYPE;
		resetText();
		try {   // for lexical and char stream error handling
			switch ( LA(1)) {
			case static_cast<unsigned char>('?'):
			{
				mQUESTION(true);
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
			case static_cast<unsigned char>('['):
			{
				mLBRACK(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>(']'):
			{
				mRBRACK(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('{'):
			{
				mLCURLY(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('}'):
			{
				mRCURLY(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>(':'):
			{
				mCOLON(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>(','):
			{
				mCOMMA(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('~'):
			{
				mBNOT(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>(';'):
			{
				mSEMI(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('\t'):
			case static_cast<unsigned char>('\n'):
			case static_cast<unsigned char>('\14'):
			case static_cast<unsigned char>('\r'):
			case static_cast<unsigned char>(' '):
			{
				mWS(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('\''):
			{
				mCHAR_LITERAL(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('"'):
			{
				mSTRING_LITERAL(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('$'):
			case static_cast<unsigned char>('A'):
			case static_cast<unsigned char>('B'):
			case static_cast<unsigned char>('C'):
			case static_cast<unsigned char>('D'):
			case static_cast<unsigned char>('E'):
			case static_cast<unsigned char>('F'):
			case static_cast<unsigned char>('G'):
			case static_cast<unsigned char>('H'):
			case static_cast<unsigned char>('I'):
			case static_cast<unsigned char>('J'):
			case static_cast<unsigned char>('K'):
			case static_cast<unsigned char>('L'):
			case static_cast<unsigned char>('M'):
			case static_cast<unsigned char>('N'):
			case static_cast<unsigned char>('O'):
			case static_cast<unsigned char>('P'):
			case static_cast<unsigned char>('Q'):
			case static_cast<unsigned char>('R'):
			case static_cast<unsigned char>('S'):
			case static_cast<unsigned char>('T'):
			case static_cast<unsigned char>('U'):
			case static_cast<unsigned char>('V'):
			case static_cast<unsigned char>('W'):
			case static_cast<unsigned char>('X'):
			case static_cast<unsigned char>('Y'):
			case static_cast<unsigned char>('Z'):
			case static_cast<unsigned char>('_'):
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
				mIDENT(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('.'):
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
				mNUM_INT(true);
				theRetToken=_returnToken;
				break;
			}
			default:
				if ((LA(1) == static_cast<unsigned char>('>')) && (LA(2) == static_cast<unsigned char>('>')) && (LA(3) == static_cast<unsigned char>('>')) && (LA(4) == static_cast<unsigned char>('='))) {
					mBSR_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (LA(2) == static_cast<unsigned char>('>')) && (LA(3) == static_cast<unsigned char>('='))) {
					mSR_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (LA(2) == static_cast<unsigned char>('>')) && (LA(3) == static_cast<unsigned char>('>')) && (true)) {
					mBSR(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (LA(2) == static_cast<unsigned char>('<')) && (LA(3) == static_cast<unsigned char>('='))) {
					mSL_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('=')) && (LA(2) == static_cast<unsigned char>('='))) {
					mEQUAL(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('!')) && (LA(2) == static_cast<unsigned char>('='))) {
					mNOT_EQUAL(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('/')) && (LA(2) == static_cast<unsigned char>('='))) {
					mDIV_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('+')) && (LA(2) == static_cast<unsigned char>('='))) {
					mPLUS_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('+')) && (LA(2) == static_cast<unsigned char>('+'))) {
					mINC(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('-')) && (LA(2) == static_cast<unsigned char>('='))) {
					mMINUS_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('-')) && (LA(2) == static_cast<unsigned char>('-'))) {
					mDEC(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('*')) && (LA(2) == static_cast<unsigned char>('='))) {
					mSTAR_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('%')) && (LA(2) == static_cast<unsigned char>('='))) {
					mMOD_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (LA(2) == static_cast<unsigned char>('>')) && (true)) {
					mSR(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (LA(2) == static_cast<unsigned char>('='))) {
					mGE(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (LA(2) == static_cast<unsigned char>('<')) && (true)) {
					mSL(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (LA(2) == static_cast<unsigned char>('='))) {
					mLE(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('^')) && (LA(2) == static_cast<unsigned char>('='))) {
					mBXOR_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('|')) && (LA(2) == static_cast<unsigned char>('='))) {
					mBOR_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('|')) && (LA(2) == static_cast<unsigned char>('|'))) {
					mLOR(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('&')) && (LA(2) == static_cast<unsigned char>('='))) {
					mBAND_ASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('&')) && (LA(2) == static_cast<unsigned char>('&'))) {
					mLAND(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('/')) && (LA(2) == static_cast<unsigned char>('/'))) {
					mSL_COMMENT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('/')) && (LA(2) == static_cast<unsigned char>('*'))) {
					mML_COMMENT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('=')) && (true)) {
					mASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('!')) && (true)) {
					mLNOT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('/')) && (true)) {
					mDIV(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('+')) && (true)) {
					mPLUS(true);
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
				else if ((LA(1) == static_cast<unsigned char>('%')) && (true)) {
					mMOD(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (true)) {
					mGT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (true)) {
					mLT_(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('^')) && (true)) {
					mBXOR(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('|')) && (true)) {
					mBOR(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('&')) && (true)) {
					mBAND(true);
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
				throw ANTLR_USE_NAMESPACE(antlr)TokenStreamRecognitionException(e);
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

void JavaLexer::mQUESTION(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = QUESTION;
	int _saveIndex;
	
	match(static_cast<unsigned char>('?'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLPAREN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LPAREN;
	int _saveIndex;
	
	match(static_cast<unsigned char>('('));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mRPAREN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = RPAREN;
	int _saveIndex;
	
	match(static_cast<unsigned char>(')'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLBRACK(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LBRACK;
	int _saveIndex;
	
	match(static_cast<unsigned char>('['));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mRBRACK(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = RBRACK;
	int _saveIndex;
	
	match(static_cast<unsigned char>(']'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLCURLY(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LCURLY;
	int _saveIndex;
	
	match(static_cast<unsigned char>('{'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mRCURLY(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = RCURLY;
	int _saveIndex;
	
	match(static_cast<unsigned char>('}'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mCOLON(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COLON;
	int _saveIndex;
	
	match(static_cast<unsigned char>(':'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mCOMMA(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COMMA;
	int _saveIndex;
	
	match(static_cast<unsigned char>(','));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = ASSIGN;
	int _saveIndex;
	
	match(static_cast<unsigned char>('='));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mEQUAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EQUAL;
	int _saveIndex;
	
	match("==");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLNOT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LNOT;
	int _saveIndex;
	
	match(static_cast<unsigned char>('!'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBNOT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BNOT;
	int _saveIndex;
	
	match(static_cast<unsigned char>('~'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mNOT_EQUAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = NOT_EQUAL;
	int _saveIndex;
	
	match("!=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mDIV(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DIV;
	int _saveIndex;
	
	match(static_cast<unsigned char>('/'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mDIV_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DIV_ASSIGN;
	int _saveIndex;
	
	match("/=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mPLUS(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = PLUS;
	int _saveIndex;
	
	match(static_cast<unsigned char>('+'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mPLUS_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = PLUS_ASSIGN;
	int _saveIndex;
	
	match("+=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mINC(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = INC;
	int _saveIndex;
	
	match("++");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mMINUS(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = MINUS;
	int _saveIndex;
	
	match(static_cast<unsigned char>('-'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mMINUS_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = MINUS_ASSIGN;
	int _saveIndex;
	
	match("-=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mDEC(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DEC;
	int _saveIndex;
	
	match("--");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSTAR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = STAR;
	int _saveIndex;
	
	match(static_cast<unsigned char>('*'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSTAR_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = STAR_ASSIGN;
	int _saveIndex;
	
	match("*=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mMOD(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = MOD;
	int _saveIndex;
	
	match(static_cast<unsigned char>('%'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mMOD_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = MOD_ASSIGN;
	int _saveIndex;
	
	match("%=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SR;
	int _saveIndex;
	
	match(">>");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSR_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SR_ASSIGN;
	int _saveIndex;
	
	match(">>=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBSR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BSR;
	int _saveIndex;
	
	match(">>>");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBSR_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BSR_ASSIGN;
	int _saveIndex;
	
	match(">>>=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mGE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = GE;
	int _saveIndex;
	
	match(">=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mGT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = GT;
	int _saveIndex;
	
	match(">");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SL;
	int _saveIndex;
	
	match("<<");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSL_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SL_ASSIGN;
	int _saveIndex;
	
	match("<<=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LE;
	int _saveIndex;
	
	match("<=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLT_(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LT_;
	int _saveIndex;
	
	match(static_cast<unsigned char>('<'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBXOR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BXOR;
	int _saveIndex;
	
	match(static_cast<unsigned char>('^'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBXOR_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BXOR_ASSIGN;
	int _saveIndex;
	
	match("^=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBOR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BOR;
	int _saveIndex;
	
	match(static_cast<unsigned char>('|'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBOR_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BOR_ASSIGN;
	int _saveIndex;
	
	match("|=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLOR(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LOR;
	int _saveIndex;
	
	match("||");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBAND(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BAND;
	int _saveIndex;
	
	match(static_cast<unsigned char>('&'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mBAND_ASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = BAND_ASSIGN;
	int _saveIndex;
	
	match("&=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mLAND(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LAND;
	int _saveIndex;
	
	match("&&");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSEMI(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SEMI;
	int _saveIndex;
	
	match(static_cast<unsigned char>(';'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mWS(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = WS;
	int _saveIndex;
	
	{ // ( ... )+
	int _cnt245=0;
	for (;;) {
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
			if ((LA(1) == static_cast<unsigned char>('\r')) && (LA(2) == static_cast<unsigned char>('\n')) && (true) && (true)) {
				match("\r\n");
			}
			else if ((LA(1) == static_cast<unsigned char>('\r')) && (true) && (true) && (true)) {
				match(static_cast<unsigned char>('\r'));
			}
			else if ((LA(1) == static_cast<unsigned char>('\n'))) {
				match(static_cast<unsigned char>('\n'));
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
			}
			
			}
#line 1137 "java.g"
			newline();
#line 1109 "JavaLexer.cpp"
			break;
		}
		default:
		{
			if ( _cnt245>=1 ) { goto _loop245; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		}
		_cnt245++;
	}
	_loop245:;
	}  // ( ... )+
#line 1139 "java.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 1123 "JavaLexer.cpp"
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSL_COMMENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SL_COMMENT;
	int _saveIndex;
	
	match("//");
	{ // ( ... )*
	for (;;) {
		if ((_tokenSet_0.member(LA(1)))) {
			{
			match(_tokenSet_0);
			}
		}
		else {
			goto _loop249;
		}
		
	}
	_loop249:;
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
#line 1146 "java.g"
	
				_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
				newline();
			
#line 1183 "JavaLexer.cpp"
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mML_COMMENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = ML_COMMENT;
	int _saveIndex;
	
	match("/*");
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == static_cast<unsigned char>('\r')) && (LA(2) == static_cast<unsigned char>('\n')) && ((LA(3) >= static_cast<unsigned char>('\3') && LA(3) <= static_cast<unsigned char>('\377'))) && ((LA(4) >= static_cast<unsigned char>('\3') && LA(4) <= static_cast<unsigned char>('\377')))) {
			match(static_cast<unsigned char>('\r'));
			match(static_cast<unsigned char>('\n'));
#line 1167 "java.g"
			newline();
#line 1205 "JavaLexer.cpp"
		}
		else if (((LA(1) == static_cast<unsigned char>('*')) && ((LA(2) >= static_cast<unsigned char>('\3') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\3') && LA(3) <= static_cast<unsigned char>('\377'))))&&( LA(2)!='/' )) {
			match(static_cast<unsigned char>('*'));
		}
		else if ((LA(1) == static_cast<unsigned char>('\r')) && ((LA(2) >= static_cast<unsigned char>('\3') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\3') && LA(3) <= static_cast<unsigned char>('\377'))) && (true)) {
			match(static_cast<unsigned char>('\r'));
#line 1168 "java.g"
			newline();
#line 1214 "JavaLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\n'))) {
			match(static_cast<unsigned char>('\n'));
#line 1169 "java.g"
			newline();
#line 1220 "JavaLexer.cpp"
		}
		else if ((_tokenSet_1.member(LA(1)))) {
			{
			match(_tokenSet_1);
			}
		}
		else {
			goto _loop255;
		}
		
	}
	_loop255:;
	} // ( ... )*
	match("*/");
#line 1173 "java.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 1237 "JavaLexer.cpp"
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mCHAR_LITERAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = CHAR_LITERAL;
	int _saveIndex;
	
	match(static_cast<unsigned char>('\''));
	{
	if ((LA(1) == static_cast<unsigned char>('\\'))) {
		mESC(false);
	}
	else if ((_tokenSet_2.member(LA(1)))) {
		matchNot(static_cast<unsigned char>('\''));
	}
	else {
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	
	}
	match(static_cast<unsigned char>('\''));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mESC(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = ESC;
	int _saveIndex;
	
	match(static_cast<unsigned char>('\\'));
	{
	switch ( LA(1)) {
	case static_cast<unsigned char>('n'):
	{
		match(static_cast<unsigned char>('n'));
		break;
	}
	case static_cast<unsigned char>('r'):
	{
		match(static_cast<unsigned char>('r'));
		break;
	}
	case static_cast<unsigned char>('t'):
	{
		match(static_cast<unsigned char>('t'));
		break;
	}
	case static_cast<unsigned char>('b'):
	{
		match(static_cast<unsigned char>('b'));
		break;
	}
	case static_cast<unsigned char>('f'):
	{
		match(static_cast<unsigned char>('f'));
		break;
	}
	case static_cast<unsigned char>('"'):
	{
		match(static_cast<unsigned char>('"'));
		break;
	}
	case static_cast<unsigned char>('\''):
	{
		match(static_cast<unsigned char>('\''));
		break;
	}
	case static_cast<unsigned char>('\\'):
	{
		match(static_cast<unsigned char>('\\'));
		break;
	}
	case static_cast<unsigned char>('u'):
	{
		{ // ( ... )+
		int _cnt265=0;
		for (;;) {
			if ((LA(1) == static_cast<unsigned char>('u'))) {
				match(static_cast<unsigned char>('u'));
			}
			else {
				if ( _cnt265>=1 ) { goto _loop265; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
			}
			
			_cnt265++;
		}
		_loop265:;
		}  // ( ... )+
		mHEX_DIGIT(false);
		mHEX_DIGIT(false);
		mHEX_DIGIT(false);
		mHEX_DIGIT(false);
		break;
	}
	case static_cast<unsigned char>('0'):
	case static_cast<unsigned char>('1'):
	case static_cast<unsigned char>('2'):
	case static_cast<unsigned char>('3'):
	{
		matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('3'));
		{
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('7'))) && ((LA(2) >= static_cast<unsigned char>('\3') && LA(2) <= static_cast<unsigned char>('\377'))) && (true) && (true)) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('7'));
			{
			if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('7'))) && ((LA(2) >= static_cast<unsigned char>('\3') && LA(2) <= static_cast<unsigned char>('\377'))) && (true) && (true)) {
				matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('7'));
			}
			else if (((LA(1) >= static_cast<unsigned char>('\3') && LA(1) <= static_cast<unsigned char>('\377'))) && (true) && (true) && (true)) {
			}
			else {
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
			}
			
			}
		}
		else if (((LA(1) >= static_cast<unsigned char>('\3') && LA(1) <= static_cast<unsigned char>('\377'))) && (true) && (true) && (true)) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
		}
		
		}
		break;
	}
	case static_cast<unsigned char>('4'):
	case static_cast<unsigned char>('5'):
	case static_cast<unsigned char>('6'):
	case static_cast<unsigned char>('7'):
	{
		matchRange(static_cast<unsigned char>('4'),static_cast<unsigned char>('7'));
		{
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('7'))) && ((LA(2) >= static_cast<unsigned char>('\3') && LA(2) <= static_cast<unsigned char>('\377'))) && (true) && (true)) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('7'));
		}
		else if (((LA(1) >= static_cast<unsigned char>('\3') && LA(1) <= static_cast<unsigned char>('\377'))) && (true) && (true) && (true)) {
		}
		else {
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
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
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mSTRING_LITERAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = STRING_LITERAL;
	int _saveIndex;
	
	match(static_cast<unsigned char>('"'));
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == static_cast<unsigned char>('\\'))) {
			mESC(false);
		}
		else if ((_tokenSet_3.member(LA(1)))) {
			{
			match(_tokenSet_3);
			}
		}
		else {
			goto _loop261;
		}
		
	}
	_loop261:;
	} // ( ... )*
	match(static_cast<unsigned char>('"'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mHEX_DIGIT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = HEX_DIGIT;
	int _saveIndex;
	
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
		matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		break;
	}
	case static_cast<unsigned char>('A'):
	case static_cast<unsigned char>('B'):
	case static_cast<unsigned char>('C'):
	case static_cast<unsigned char>('D'):
	case static_cast<unsigned char>('E'):
	case static_cast<unsigned char>('F'):
	{
		matchRange(static_cast<unsigned char>('A'),static_cast<unsigned char>('F'));
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
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mVOCAB(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = VOCAB;
	int _saveIndex;
	
	matchRange(static_cast<unsigned char>('\3'),static_cast<unsigned char>('\377'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mIDENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = IDENT;
	int _saveIndex;
	
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
	case static_cast<unsigned char>('A'):
	case static_cast<unsigned char>('B'):
	case static_cast<unsigned char>('C'):
	case static_cast<unsigned char>('D'):
	case static_cast<unsigned char>('E'):
	case static_cast<unsigned char>('F'):
	case static_cast<unsigned char>('G'):
	case static_cast<unsigned char>('H'):
	case static_cast<unsigned char>('I'):
	case static_cast<unsigned char>('J'):
	case static_cast<unsigned char>('K'):
	case static_cast<unsigned char>('L'):
	case static_cast<unsigned char>('M'):
	case static_cast<unsigned char>('N'):
	case static_cast<unsigned char>('O'):
	case static_cast<unsigned char>('P'):
	case static_cast<unsigned char>('Q'):
	case static_cast<unsigned char>('R'):
	case static_cast<unsigned char>('S'):
	case static_cast<unsigned char>('T'):
	case static_cast<unsigned char>('U'):
	case static_cast<unsigned char>('V'):
	case static_cast<unsigned char>('W'):
	case static_cast<unsigned char>('X'):
	case static_cast<unsigned char>('Y'):
	case static_cast<unsigned char>('Z'):
	{
		matchRange(static_cast<unsigned char>('A'),static_cast<unsigned char>('Z'));
		break;
	}
	case static_cast<unsigned char>('_'):
	{
		match(static_cast<unsigned char>('_'));
		break;
	}
	case static_cast<unsigned char>('$'):
	{
		match(static_cast<unsigned char>('$'));
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
	{ // ( ... )*
	for (;;) {
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
		case static_cast<unsigned char>('A'):
		case static_cast<unsigned char>('B'):
		case static_cast<unsigned char>('C'):
		case static_cast<unsigned char>('D'):
		case static_cast<unsigned char>('E'):
		case static_cast<unsigned char>('F'):
		case static_cast<unsigned char>('G'):
		case static_cast<unsigned char>('H'):
		case static_cast<unsigned char>('I'):
		case static_cast<unsigned char>('J'):
		case static_cast<unsigned char>('K'):
		case static_cast<unsigned char>('L'):
		case static_cast<unsigned char>('M'):
		case static_cast<unsigned char>('N'):
		case static_cast<unsigned char>('O'):
		case static_cast<unsigned char>('P'):
		case static_cast<unsigned char>('Q'):
		case static_cast<unsigned char>('R'):
		case static_cast<unsigned char>('S'):
		case static_cast<unsigned char>('T'):
		case static_cast<unsigned char>('U'):
		case static_cast<unsigned char>('V'):
		case static_cast<unsigned char>('W'):
		case static_cast<unsigned char>('X'):
		case static_cast<unsigned char>('Y'):
		case static_cast<unsigned char>('Z'):
		{
			matchRange(static_cast<unsigned char>('A'),static_cast<unsigned char>('Z'));
			break;
		}
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
		{
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
			break;
		}
		case static_cast<unsigned char>('$'):
		{
			match(static_cast<unsigned char>('$'));
			break;
		}
		default:
		{
			goto _loop275;
		}
		}
	}
	_loop275:;
	} // ( ... )*
	_ttype = testLiteralsTable(_ttype);
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mNUM_INT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = NUM_INT;
	int _saveIndex;
	ANTLR_USE_NAMESPACE(antlr)RefToken f1;
	ANTLR_USE_NAMESPACE(antlr)RefToken f2;
	ANTLR_USE_NAMESPACE(antlr)RefToken f3;
	ANTLR_USE_NAMESPACE(antlr)RefToken f4;
#line 1251 "java.g"
	
			bool isDecimal = false;
			ANTLR_USE_NAMESPACE(antlr)RefToken t = ANTLR_USE_NAMESPACE(antlr)nullToken;
		
#line 1707 "JavaLexer.cpp"
	
	switch ( LA(1)) {
	case static_cast<unsigned char>('.'):
	{
		match(static_cast<unsigned char>('.'));
#line 1256 "java.g"
		_ttype = DOT;
#line 1715 "JavaLexer.cpp"
		{
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			{ // ( ... )+
			int _cnt279=0;
			for (;;) {
				if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
					matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
				}
				else {
					if ( _cnt279>=1 ) { goto _loop279; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
				}
				
				_cnt279++;
			}
			_loop279:;
			}  // ( ... )+
			{
			if ((LA(1) == static_cast<unsigned char>('E') || LA(1) == static_cast<unsigned char>('e'))) {
				mEXPONENT(false);
			}
			else {
			}
			
			}
			{
			if ((LA(1) == static_cast<unsigned char>('D') || LA(1) == static_cast<unsigned char>('F') || LA(1) == static_cast<unsigned char>('d') || LA(1) == static_cast<unsigned char>('f'))) {
				mFLOAT_SUFFIX(true);
				f1=_returnToken;
#line 1257 "java.g"
				t=f1;
#line 1746 "JavaLexer.cpp"
			}
			else {
			}
			
			}
#line 1258 "java.g"
			
								if ( t &&
									  (t->getText().find('f') != ANTLR_USE_NAMESPACE(std)string::npos ||
										t->getText().find('F') != ANTLR_USE_NAMESPACE(std)string::npos ) ) {
									_ttype = NUM_FLOAT;
								}
								else {
									_ttype = NUM_DOUBLE; // assume double
								}
							
#line 1763 "JavaLexer.cpp"
		}
		else {
		}
		
		}
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
		{
		switch ( LA(1)) {
		case static_cast<unsigned char>('0'):
		{
			match(static_cast<unsigned char>('0'));
#line 1270 "java.g"
			isDecimal = true;
#line 1789 "JavaLexer.cpp"
			{
			switch ( LA(1)) {
			case static_cast<unsigned char>('X'):
			case static_cast<unsigned char>('x'):
			{
				{
				switch ( LA(1)) {
				case static_cast<unsigned char>('x'):
				{
					match(static_cast<unsigned char>('x'));
					break;
				}
				case static_cast<unsigned char>('X'):
				{
					match(static_cast<unsigned char>('X'));
					break;
				}
				default:
				{
					throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
				}
				}
				}
				{ // ( ... )+
				int _cnt286=0;
				for (;;) {
					if ((_tokenSet_4.member(LA(1))) && (true) && (true) && (true)) {
						mHEX_DIGIT(false);
					}
					else {
						if ( _cnt286>=1 ) { goto _loop286; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
					}
					
					_cnt286++;
				}
				_loop286:;
				}  // ( ... )+
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
			{
				{ // ( ... )+
				int _cnt288=0;
				for (;;) {
					if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('7')))) {
						matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('7'));
					}
					else {
						if ( _cnt288>=1 ) { goto _loop288; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
					}
					
					_cnt288++;
				}
				_loop288:;
				}  // ( ... )+
				break;
			}
			default:
				{
				}
			}
			}
			break;
		}
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
			{
			matchRange(static_cast<unsigned char>('1'),static_cast<unsigned char>('9'));
			}
			{ // ( ... )*
			for (;;) {
				if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
					matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
				}
				else {
					goto _loop291;
				}
				
			}
			_loop291:;
			} // ( ... )*
#line 1285 "java.g"
			isDecimal=true;
#line 1888 "JavaLexer.cpp"
			break;
		}
		default:
		{
			throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
		}
		}
		}
		{
		if ((LA(1) == static_cast<unsigned char>('L') || LA(1) == static_cast<unsigned char>('l'))) {
			{
			switch ( LA(1)) {
			case static_cast<unsigned char>('l'):
			{
				match(static_cast<unsigned char>('l'));
				break;
			}
			case static_cast<unsigned char>('L'):
			{
				match(static_cast<unsigned char>('L'));
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
			}
			}
			}
#line 1287 "java.g"
			_ttype = NUM_LONG;
#line 1919 "JavaLexer.cpp"
		}
		else if (((LA(1) == static_cast<unsigned char>('.') || LA(1) == static_cast<unsigned char>('D') || LA(1) == static_cast<unsigned char>('E') || LA(1) == static_cast<unsigned char>('F') || LA(1) == static_cast<unsigned char>('d') || LA(1) == static_cast<unsigned char>('e') || LA(1) == static_cast<unsigned char>('f')))&&(isDecimal)) {
			{
			switch ( LA(1)) {
			case static_cast<unsigned char>('.'):
			{
				match(static_cast<unsigned char>('.'));
				{ // ( ... )*
				for (;;) {
					if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
						matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
					}
					else {
						goto _loop296;
					}
					
				}
				_loop296:;
				} // ( ... )*
				{
				if ((LA(1) == static_cast<unsigned char>('E') || LA(1) == static_cast<unsigned char>('e'))) {
					mEXPONENT(false);
				}
				else {
				}
				
				}
				{
				if ((LA(1) == static_cast<unsigned char>('D') || LA(1) == static_cast<unsigned char>('F') || LA(1) == static_cast<unsigned char>('d') || LA(1) == static_cast<unsigned char>('f'))) {
					mFLOAT_SUFFIX(true);
					f2=_returnToken;
#line 1291 "java.g"
					t=f2;
#line 1953 "JavaLexer.cpp"
				}
				else {
				}
				
				}
				break;
			}
			case static_cast<unsigned char>('E'):
			case static_cast<unsigned char>('e'):
			{
				mEXPONENT(false);
				{
				if ((LA(1) == static_cast<unsigned char>('D') || LA(1) == static_cast<unsigned char>('F') || LA(1) == static_cast<unsigned char>('d') || LA(1) == static_cast<unsigned char>('f'))) {
					mFLOAT_SUFFIX(true);
					f3=_returnToken;
#line 1292 "java.g"
					t=f3;
#line 1971 "JavaLexer.cpp"
				}
				else {
				}
				
				}
				break;
			}
			case static_cast<unsigned char>('D'):
			case static_cast<unsigned char>('F'):
			case static_cast<unsigned char>('d'):
			case static_cast<unsigned char>('f'):
			{
				mFLOAT_SUFFIX(true);
				f4=_returnToken;
#line 1293 "java.g"
				t=f4;
#line 1988 "JavaLexer.cpp"
				break;
			}
			default:
			{
				throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
			}
			}
			}
#line 1295 "java.g"
			
								if ( t &&
									  (t->getText().find('f') != ANTLR_USE_NAMESPACE(std)string::npos ||
										t->getText().find('F') != ANTLR_USE_NAMESPACE(std)string::npos ) ) {
									_ttype = NUM_FLOAT;
								}
								else {
									_ttype = NUM_DOUBLE; // assume double
								}
							
#line 2008 "JavaLexer.cpp"
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
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mEXPONENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EXPONENT;
	int _saveIndex;
	
	{
	switch ( LA(1)) {
	case static_cast<unsigned char>('e'):
	{
		match(static_cast<unsigned char>('e'));
		break;
	}
	case static_cast<unsigned char>('E'):
	{
		match(static_cast<unsigned char>('E'));
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
	int _cnt304=0;
	for (;;) {
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
		else {
			if ( _cnt304>=1 ) { goto _loop304; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt304++;
	}
	_loop304:;
	}  // ( ... )+
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void JavaLexer::mFLOAT_SUFFIX(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = FLOAT_SUFFIX;
	int _saveIndex;
	
	switch ( LA(1)) {
	case static_cast<unsigned char>('f'):
	{
		match(static_cast<unsigned char>('f'));
		break;
	}
	case static_cast<unsigned char>('F'):
	{
		match(static_cast<unsigned char>('F'));
		break;
	}
	case static_cast<unsigned char>('d'):
	{
		match(static_cast<unsigned char>('d'));
		break;
	}
	case static_cast<unsigned char>('D'):
	{
		match(static_cast<unsigned char>('D'));
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}


const unsigned long JavaLexer::_tokenSet_0_data_[] = { 4294958072UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NULL_TREE_LOOKAHEAD BLOCK MODIFIERS OBJBLOCK SLIST CTOR_DEF METHOD_DEF 
// INSTANCE_INIT STATIC_INIT CLASS_DEF INTERFACE_DEF PACKAGE_DEF ARRAY_DECLARATOR 
// EXTENDS_CLAUSE IMPLEMENTS_CLAUSE PARAMETERS PARAMETER_DEF LABELED_STAT 
// TYPECAST INDEX_OP POST_INC POST_DEC METHOD_CALL EXPR ARRAY_INIT IMPORT 
// UNARY_MINUS UNARY_PLUS CASE_GROUP ELIST FOR_INIT FOR_CONDITION FOR_ITERATOR 
// EMPTY_STAT "final" "abstract" "strictfp" SUPER_CTOR_CALL CTOR_CALL "package" 
// SEMI "import" LBRACK RBRACK "void" "boolean" "byte" "char" "short" "int" 
// "float" "long" "double" IDENT DOT STAR "private" "public" "protected" 
// "static" "transient" "native" "threadsafe" "synchronized" "volatile" 
// "class" "extends" "interface" LCURLY RCURLY COMMA "implements" LPAREN 
// RPAREN "this" "super" ASSIGN "throws" COLON "if" "else" "for" "while" 
// "do" "break" "continue" "return" "switch" "throw" "case" "default" "try" 
// "finally" "catch" PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN 
// SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION 
// LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL 
// SR BSR PLUS MINUS DIV MOD INC DEC BNOT LNOT "true" "false" "null" "new" 
// NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE WS 
// SL_COMMENT ML_COMMENT ESC HEX_DIGIT VOCAB EXPONENT FLOAT_SUFFIX 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaLexer::_tokenSet_0(_tokenSet_0_data_,16);
const unsigned long JavaLexer::_tokenSet_1_data_[] = { 4294958072UL, 4294966271UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NULL_TREE_LOOKAHEAD BLOCK MODIFIERS OBJBLOCK SLIST CTOR_DEF METHOD_DEF 
// INSTANCE_INIT STATIC_INIT CLASS_DEF INTERFACE_DEF PACKAGE_DEF ARRAY_DECLARATOR 
// EXTENDS_CLAUSE IMPLEMENTS_CLAUSE PARAMETERS PARAMETER_DEF LABELED_STAT 
// TYPECAST INDEX_OP POST_INC POST_DEC METHOD_CALL EXPR ARRAY_INIT IMPORT 
// UNARY_MINUS UNARY_PLUS CASE_GROUP ELIST FOR_INIT FOR_CONDITION FOR_ITERATOR 
// EMPTY_STAT "final" "abstract" "strictfp" CTOR_CALL "package" SEMI "import" 
// LBRACK RBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" 
// "double" IDENT DOT STAR "private" "public" "protected" "static" "transient" 
// "native" "threadsafe" "synchronized" "volatile" "class" "extends" "interface" 
// LCURLY RCURLY COMMA "implements" LPAREN RPAREN "this" "super" ASSIGN 
// "throws" COLON "if" "else" "for" "while" "do" "break" "continue" "return" 
// "switch" "throw" "case" "default" "try" "finally" "catch" PLUS_ASSIGN 
// MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN SR_ASSIGN BSR_ASSIGN 
// SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION LOR LAND BOR BXOR 
// BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL SR BSR PLUS MINUS 
// DIV MOD INC DEC BNOT LNOT "true" "false" "null" "new" NUM_INT CHAR_LITERAL 
// STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE WS SL_COMMENT ML_COMMENT 
// ESC HEX_DIGIT VOCAB EXPONENT FLOAT_SUFFIX 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaLexer::_tokenSet_1(_tokenSet_1_data_,16);
const unsigned long JavaLexer::_tokenSet_2_data_[] = { 4294967288UL, 4294967167UL, 4026531839UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NULL_TREE_LOOKAHEAD BLOCK MODIFIERS OBJBLOCK SLIST CTOR_DEF METHOD_DEF 
// VARIABLE_DEF INSTANCE_INIT STATIC_INIT TYPE CLASS_DEF INTERFACE_DEF 
// PACKAGE_DEF ARRAY_DECLARATOR EXTENDS_CLAUSE IMPLEMENTS_CLAUSE PARAMETERS 
// PARAMETER_DEF LABELED_STAT TYPECAST INDEX_OP POST_INC POST_DEC METHOD_CALL 
// EXPR ARRAY_INIT IMPORT UNARY_MINUS UNARY_PLUS CASE_GROUP ELIST FOR_INIT 
// FOR_CONDITION FOR_ITERATOR EMPTY_STAT "abstract" "strictfp" SUPER_CTOR_CALL 
// CTOR_CALL "package" SEMI "import" LBRACK RBRACK "void" "boolean" "byte" 
// "char" "short" "int" "float" "long" "double" IDENT DOT STAR "private" 
// "public" "protected" "static" "transient" "native" "threadsafe" "synchronized" 
// "volatile" "class" "extends" "interface" LCURLY RCURLY COMMA "implements" 
// LPAREN RPAREN "this" "super" ASSIGN "throws" COLON "if" "else" "for" 
// "while" "do" "break" "continue" "return" "throw" "case" "default" "try" 
// "finally" "catch" PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN 
// SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION 
// LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL 
// SR BSR PLUS MINUS DIV MOD INC DEC BNOT LNOT "true" "false" "null" "new" 
// NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE WS 
// SL_COMMENT ML_COMMENT ESC HEX_DIGIT VOCAB EXPONENT FLOAT_SUFFIX 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaLexer::_tokenSet_2(_tokenSet_2_data_,16);
const unsigned long JavaLexer::_tokenSet_3_data_[] = { 4294967288UL, 4294967291UL, 4026531839UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// NULL_TREE_LOOKAHEAD BLOCK MODIFIERS OBJBLOCK SLIST CTOR_DEF METHOD_DEF 
// VARIABLE_DEF INSTANCE_INIT STATIC_INIT TYPE CLASS_DEF INTERFACE_DEF 
// PACKAGE_DEF ARRAY_DECLARATOR EXTENDS_CLAUSE IMPLEMENTS_CLAUSE PARAMETERS 
// PARAMETER_DEF LABELED_STAT TYPECAST INDEX_OP POST_INC POST_DEC METHOD_CALL 
// EXPR ARRAY_INIT IMPORT UNARY_MINUS UNARY_PLUS CASE_GROUP FOR_INIT FOR_CONDITION 
// FOR_ITERATOR EMPTY_STAT "final" "abstract" "strictfp" SUPER_CTOR_CALL 
// CTOR_CALL "package" SEMI "import" LBRACK RBRACK "void" "boolean" "byte" 
// "char" "short" "int" "float" "long" "double" IDENT DOT STAR "private" 
// "public" "protected" "static" "transient" "native" "threadsafe" "synchronized" 
// "volatile" "class" "extends" "interface" LCURLY RCURLY COMMA "implements" 
// LPAREN RPAREN "this" "super" ASSIGN "throws" COLON "if" "else" "for" 
// "while" "do" "break" "continue" "return" "throw" "case" "default" "try" 
// "finally" "catch" PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN MOD_ASSIGN 
// SR_ASSIGN BSR_ASSIGN SL_ASSIGN BAND_ASSIGN BXOR_ASSIGN BOR_ASSIGN QUESTION 
// LOR LAND BOR BXOR BAND NOT_EQUAL EQUAL LT_ GT LE GE "instanceof" SL 
// SR BSR PLUS MINUS DIV MOD INC DEC BNOT LNOT "true" "false" "null" "new" 
// NUM_INT CHAR_LITERAL STRING_LITERAL NUM_FLOAT NUM_LONG NUM_DOUBLE WS 
// SL_COMMENT ML_COMMENT ESC HEX_DIGIT VOCAB EXPONENT FLOAT_SUFFIX 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaLexer::_tokenSet_3(_tokenSet_3_data_,16);
const unsigned long JavaLexer::_tokenSet_4_data_[] = { 0UL, 67043328UL, 126UL, 126UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// RBRACK "void" "boolean" "byte" "char" "short" "int" "float" "long" "double" 
// "transient" "native" "threadsafe" "synchronized" "volatile" "class" 
// "finally" "catch" PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN DIV_ASSIGN 
const ANTLR_USE_NAMESPACE(antlr)BitSet JavaLexer::_tokenSet_4(_tokenSet_4_data_,10);

