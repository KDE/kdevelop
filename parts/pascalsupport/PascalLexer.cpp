/* $ANTLR 2.7.2: "pascal.g" -> "PascalLexer.cpp"$ */
#include "PascalLexer.hpp"
#include <antlr/CharBuffer.hpp>
#include <antlr/TokenStreamException.hpp>
#include <antlr/TokenStreamIOException.hpp>
#include <antlr/TokenStreamRecognitionException.hpp>
#include <antlr/CharStreamException.hpp>
#include <antlr/CharStreamIOException.hpp>
#include <antlr/NoViableAltForCharException.hpp>

#line 1 "pascal.g"
#line 13 "PascalLexer.cpp"
PascalLexer::PascalLexer(ANTLR_USE_NAMESPACE(std)istream& in)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(new ANTLR_USE_NAMESPACE(antlr)CharBuffer(in),false)
{
	initLiterals();
}

PascalLexer::PascalLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(ib,false)
{
	initLiterals();
}

PascalLexer::PascalLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state)
	: ANTLR_USE_NAMESPACE(antlr)CharScanner(state,false)
{
	initLiterals();
}

void PascalLexer::initLiterals()
{
	literals["type"] = 42;
	literals["case"] = 61;
	literals["while"] = 87;
	literals["repeat"] = 89;
	literals["end"] = 60;
	literals["integer"] = 49;
	literals["then"] = 85;
	literals["program"] = 24;
	literals["implementation"] = 30;
	literals["until"] = 90;
	literals["to"] = 92;
	literals["and"] = 79;
	literals["const"] = 34;
	literals["not"] = 80;
	literals["var"] = 65;
	literals["real"] = 50;
	literals["mod"] = 78;
	literals["packed"] = 52;
	literals["do"] = 88;
	literals["function"] = 43;
	literals["with"] = 94;
	literals["set"] = 62;
	literals["interface"] = 22;
	literals["of"] = 56;
	literals["array"] = 55;
	literals["file"] = 63;
	literals["or"] = 74;
	literals["if"] = 84;
	literals["record"] = 59;
	literals["chr"] = 36;
	literals["goto"] = 82;
	literals["for"] = 91;
	literals["unit"] = 28;
	literals["boolean"] = 48;
	literals["label"] = 32;
	literals["char"] = 47;
	literals["string"] = 51;
	literals["downto"] = 93;
	literals["begin"] = 83;
	literals["else"] = 86;
	literals["uses"] = 31;
	literals["in"] = 73;
	literals["procedure"] = 45;
	literals["nil"] = 81;
	literals["div"] = 77;
}

ANTLR_USE_NAMESPACE(antlr)RefToken PascalLexer::nextToken()
{
	ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
	for (;;) {
		ANTLR_USE_NAMESPACE(antlr)RefToken theRetToken;
		int _ttype = ANTLR_USE_NAMESPACE(antlr)Token::INVALID_TYPE;
		resetText();
		try {   // for lexical and char stream error handling
			switch ( LA(1)) {
			case static_cast<unsigned char>('+'):
			{
				mPLUS(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('-'):
			{
				mMINUS(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('*'):
			{
				mSTAR(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('/'):
			{
				mSLASH(true);
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
			case static_cast<unsigned char>('='):
			{
				mEQUAL(true);
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
			case static_cast<unsigned char>('^'):
			{
				mPOINTER(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('@'):
			{
				mAT(true);
				theRetToken=_returnToken;
				break;
			}
			case static_cast<unsigned char>('}'):
			{
				mRCURLY(true);
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
			case static_cast<unsigned char>('\''):
			{
				mSTRING_LITERAL(true);
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
				mNUM_INT(true);
				theRetToken=_returnToken;
				break;
			}
			default:
				if ((LA(1) == static_cast<unsigned char>(':')) && (LA(2) == static_cast<unsigned char>('='))) {
					mASSIGN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (LA(2) == static_cast<unsigned char>('>'))) {
					mNOT_EQUAL(true);
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
				else if ((LA(1) == static_cast<unsigned char>('(')) && (LA(2) == static_cast<unsigned char>('.'))) {
					mLBRACK2(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('.')) && (LA(2) == static_cast<unsigned char>(')'))) {
					mRBRACK2(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('(')) && (LA(2) == static_cast<unsigned char>('*'))) {
					mCOMMENT_1(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('{')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377')))) {
					mCOMMENT_2(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>(':')) && (true)) {
					mCOLON(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('<')) && (true)) {
					mLTH(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('>')) && (true)) {
					mGT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('(')) && (true)) {
					mLPAREN(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('.')) && (true)) {
					mDOT(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('{')) && (true)) {
					mLCURLY(true);
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

void PascalLexer::mPLUS(bool _createToken) {
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

void PascalLexer::mMINUS(bool _createToken) {
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

void PascalLexer::mSTAR(bool _createToken) {
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

void PascalLexer::mSLASH(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SLASH;
	int _saveIndex;
	
	match(static_cast<unsigned char>('/'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mASSIGN(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = ASSIGN;
	int _saveIndex;
	
	match(":=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mCOMMA(bool _createToken) {
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

void PascalLexer::mSEMI(bool _createToken) {
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

void PascalLexer::mCOLON(bool _createToken) {
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

void PascalLexer::mEQUAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EQUAL;
	int _saveIndex;
	
	match(static_cast<unsigned char>('='));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mNOT_EQUAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = NOT_EQUAL;
	int _saveIndex;
	
	match("<>");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mLTH(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LTH;
	int _saveIndex;
	
	match(static_cast<unsigned char>('<'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mLE(bool _createToken) {
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

void PascalLexer::mGE(bool _createToken) {
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

void PascalLexer::mGT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = GT;
	int _saveIndex;
	
	match(static_cast<unsigned char>('>'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mLPAREN(bool _createToken) {
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

void PascalLexer::mRPAREN(bool _createToken) {
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

void PascalLexer::mLBRACK(bool _createToken) {
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

void PascalLexer::mLBRACK2(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LBRACK2;
	int _saveIndex;
	
	match("(.");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mRBRACK(bool _createToken) {
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

void PascalLexer::mRBRACK2(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = RBRACK2;
	int _saveIndex;
	
	match(".)");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mPOINTER(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = POINTER;
	int _saveIndex;
	
	match(static_cast<unsigned char>('^'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mAT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = AT;
	int _saveIndex;
	
	match(static_cast<unsigned char>('@'));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mDOT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = DOT;
	int _saveIndex;
	
	match(static_cast<unsigned char>('.'));
	{
	if ((LA(1) == static_cast<unsigned char>('.'))) {
		match(static_cast<unsigned char>('.'));
#line 732 "pascal.g"
		_ttype = DOTDOT;
#line 635 "PascalLexer.cpp"
	}
	else {
	}
	
	}
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mLCURLY(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = LCURLY;
	int _saveIndex;
	
	match("{");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mRCURLY(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = RCURLY;
	int _saveIndex;
	
	match("}");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mWS(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = WS;
	int _saveIndex;
	
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
#line 746 "pascal.g"
		newline();
#line 719 "PascalLexer.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
#line 748 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 730 "PascalLexer.cpp"
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mCOMMENT_1(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COMMENT_1;
	int _saveIndex;
	
	match("(*");
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == static_cast<unsigned char>('\r')) && (LA(2) == static_cast<unsigned char>('\n')) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))) && ((LA(4) >= static_cast<unsigned char>('\0') && LA(4) <= static_cast<unsigned char>('\377')))) {
			match(static_cast<unsigned char>('\r'));
			match(static_cast<unsigned char>('\n'));
#line 756 "pascal.g"
			newline();
#line 752 "PascalLexer.cpp"
		}
		else if (((LA(1) == static_cast<unsigned char>('*')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))))&&( LA(2) != ')' )) {
			match(static_cast<unsigned char>('*'));
		}
		else if ((LA(1) == static_cast<unsigned char>('\r')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))) && (true)) {
			match(static_cast<unsigned char>('\r'));
#line 757 "pascal.g"
			newline();
#line 761 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\n'))) {
			match(static_cast<unsigned char>('\n'));
#line 758 "pascal.g"
			newline();
#line 767 "PascalLexer.cpp"
		}
		else if ((_tokenSet_0.member(LA(1)))) {
			{
			match(_tokenSet_0);
			}
		}
		else {
			goto _loop188;
		}
		
	}
	_loop188:;
	} // ( ... )*
	match("*)");
#line 762 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 784 "PascalLexer.cpp"
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mCOMMENT_2(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COMMENT_2;
	int _saveIndex;
	
	match(static_cast<unsigned char>('{'));
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == static_cast<unsigned char>('\r')) && (LA(2) == static_cast<unsigned char>('\n')) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))) && (true)) {
			match(static_cast<unsigned char>('\r'));
			match(static_cast<unsigned char>('\n'));
#line 768 "pascal.g"
			newline();
#line 806 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\r')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && (true) && (true)) {
			match(static_cast<unsigned char>('\r'));
#line 769 "pascal.g"
			newline();
#line 812 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\n'))) {
			match(static_cast<unsigned char>('\n'));
#line 770 "pascal.g"
			newline();
#line 818 "PascalLexer.cpp"
		}
		else if ((_tokenSet_1.member(LA(1)))) {
			{
			match(_tokenSet_1);
			}
		}
		else {
			goto _loop192;
		}
		
	}
	_loop192:;
	} // ( ... )*
	match(static_cast<unsigned char>('}'));
#line 774 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 835 "PascalLexer.cpp"
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mIDENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = IDENT;
	int _saveIndex;
	
	{
	matchRange(static_cast<unsigned char>('a'),static_cast<unsigned char>('z'));
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
		case static_cast<unsigned char>('_'):
		{
			match(static_cast<unsigned char>('_'));
			break;
		}
		default:
		{
			goto _loop196;
		}
		}
	}
	_loop196:;
	} // ( ... )*
	_ttype = testLiteralsTable(_ttype);
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mSTRING_LITERAL(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = STRING_LITERAL;
	int _saveIndex;
	
	match(static_cast<unsigned char>('\''));
	{ // ( ... )*
	for (;;) {
		if ((LA(1) == static_cast<unsigned char>('\'')) && (LA(2) == static_cast<unsigned char>('\''))) {
			match("\'\'");
		}
		else if ((_tokenSet_2.member(LA(1)))) {
			{
			match(_tokenSet_2);
			}
		}
		else {
			goto _loop200;
		}
		
	}
	_loop200:;
	} // ( ... )*
	match(static_cast<unsigned char>('\''));
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

/** a numeric literal.  Form is (from Wirth)
 *  digits
 *  digits . digits
 *  digits . digits exponent
 *  digits exponent
 */
void PascalLexer::mNUM_INT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = NUM_INT;
	int _saveIndex;
	
	{ // ( ... )+
	int _cnt203=0;
	for (;;) {
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
		else {
			if ( _cnt203>=1 ) { goto _loop203; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt203++;
	}
	_loop203:;
	}  // ( ... )+
	{
	if ((LA(1) == static_cast<unsigned char>('e'))) {
		mEXPONENT(false);
#line 803 "pascal.g"
		_ttype = NUM_REAL;
#line 983 "PascalLexer.cpp"
	}
	else {
		{
		if (((LA(1) == static_cast<unsigned char>('.')))&&((LA(2)!='.')&&(LA(2)!=')'))) {
			match(static_cast<unsigned char>('.'));
#line 800 "pascal.g"
			_ttype = NUM_REAL;
#line 991 "PascalLexer.cpp"
			{ // ( ... )+
			int _cnt207=0;
			for (;;) {
				if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
					matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
				}
				else {
					if ( _cnt207>=1 ) { goto _loop207; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
				}
				
				_cnt207++;
			}
			_loop207:;
			}  // ( ... )+
			{
			if ((LA(1) == static_cast<unsigned char>('e'))) {
				mEXPONENT(false);
			}
			else {
			}
			
			}
		}
		else {
		}
		
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

void PascalLexer::mEXPONENT(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = EXPONENT;
	int _saveIndex;
	
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
	int _cnt213=0;
	for (;;) {
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
		else {
			if ( _cnt213>=1 ) { goto _loop213; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt213++;
	}
	_loop213:;
	}  // ( ... )+
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}


const unsigned long PascalLexer::_tokenSet_0_data_[] = { 4294958079UL, 4294966271UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE VARIANT_TAG VARIANT_TAG_NO_ID CONSTLIST FIELDLIST ARGDECLS 
// VARDECL ARGDECL ARGLIST TYPEDECL FIELD "interface" DOT "program" LPAREN 
// RPAREN SEMI "unit" IDENT "implementation" "uses" "label" COMMA "const" 
// EQUAL "chr" NUM_INT NUM_REAL PLUS MINUS STRING_LITERAL "function" COLON 
// "procedure" DOTDOT "char" "boolean" "integer" "real" "string" "packed" 
// LBRACK RBRACK "array" "of" LBRACK2 RBRACK2 "record" "end" "case" "set" 
// "file" POINTER "var" ASSIGN AT NOT_EQUAL LTH LE GE GT "in" "or" STAR 
// SLASH "div" "mod" "and" "not" "nil" "goto" "begin" "if" "then" "else" 
// "while" "do" "repeat" "until" "for" "to" "downto" "with" METHOD ADDSUBOR 
// ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR LF LCURLY RCURLY WS COMMENT_1 
// COMMENT_2 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_0(_tokenSet_0_data_,16);
const unsigned long PascalLexer::_tokenSet_1_data_[] = { 4294958079UL, 4294967295UL, 4294967295UL, 3758096383UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE VARIANT_TAG VARIANT_TAG_NO_ID CONSTLIST FIELDLIST ARGDECLS 
// VARDECL ARGDECL ARGLIST TYPEDECL FIELD "interface" DOT "program" LPAREN 
// RPAREN SEMI "unit" IDENT "implementation" "uses" "label" COMMA "const" 
// EQUAL "chr" NUM_INT NUM_REAL PLUS MINUS STRING_LITERAL "type" "function" 
// COLON "procedure" DOTDOT "char" "boolean" "integer" "real" "string" 
// "packed" LBRACK RBRACK "array" "of" LBRACK2 RBRACK2 "record" "end" "case" 
// "set" "file" POINTER "var" ASSIGN AT NOT_EQUAL LTH LE GE GT "in" "or" 
// STAR SLASH "div" "mod" "and" "not" "nil" "goto" "begin" "if" "then" 
// "else" "while" "do" "repeat" "until" "for" "to" "downto" "with" METHOD 
// ADDSUBOR ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR LF LCURLY RCURLY 
// WS COMMENT_1 COMMENT_2 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_1(_tokenSet_1_data_,16);
const unsigned long PascalLexer::_tokenSet_2_data_[] = { 4294967295UL, 4294967167UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE TYPELIST VARIANT_TAG VARIANT_TAG_NO_ID VARIANT_CASE CONSTLIST 
// FIELDLIST ARGDECLS VARDECL ARGDECL ARGLIST TYPEDECL FIELD "interface" 
// DOT "program" LPAREN RPAREN SEMI "unit" IDENT "implementation" "uses" 
// "label" COMMA "const" EQUAL "chr" NUM_INT NUM_REAL MINUS STRING_LITERAL 
// "type" "function" COLON "procedure" DOTDOT "char" "boolean" "integer" 
// "real" "string" "packed" LBRACK RBRACK "array" "of" LBRACK2 RBRACK2 
// "record" "end" "case" "set" "file" POINTER "var" ASSIGN AT NOT_EQUAL 
// LTH LE GE GT "in" "or" STAR SLASH "div" "mod" "and" "not" "nil" "goto" 
// "begin" "if" "then" "else" "while" "do" "repeat" "until" "for" "to" 
// "downto" "with" METHOD ADDSUBOR ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT 
// MYASTVAR LF LCURLY RCURLY WS COMMENT_1 COMMENT_2 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_2(_tokenSet_2_data_,16);

