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
	literals["until"] = 146;
	literals["xor"] = 123;
	literals["abstract"] = 99;
	literals["shortint"] = 69;
	literals["with"] = 150;
	literals["packed"] = 91;
	literals["inherited"] = 172;
	literals["break"] = 168;
	literals["smallint"] = 70;
	literals["safecall"] = 64;
	literals["constructor"] = 102;
	literals["continue"] = 169;
	literals["uses"] = 30;
	literals["for"] = 147;
	literals["else"] = 142;
	literals["is"] = 119;
	literals["of"] = 51;
	literals["and"] = 128;
	literals["integer"] = 68;
	literals["byte"] = 73;
	literals["nil"] = 161;
	literals["begin"] = 34;
	literals["interface"] = 32;
	literals["as"] = 165;
	literals["div"] = 126;
	literals["write"] = 110;
	literals["qword"] = 76;
	literals["procedure"] = 47;
	literals["shl"] = 129;
	literals["var"] = 45;
	literals["private"] = 100;
	literals["function"] = 49;
	literals["unit"] = 31;
	literals["downto"] = 149;
	literals["name"] = 29;
	literals["resourcestring"] = 42;
	literals["register"] = 57;
	literals["popstack"] = 61;
	literals["label"] = 40;
	literals["try"] = 155;
	literals["raise"] = 154;
	literals["not"] = 131;
	literals["record"] = 92;
	literals["forward"] = 48;
	literals["in"] = 118;
	literals["except"] = 156;
	literals["file"] = 95;
	literals["operator"] = 153;
	literals["pascal"] = 58;
	literals["finalization"] = 39;
	literals["cdecl"] = 59;
	literals["extended"] = 86;
	literals["external"] = 52;
	literals["destructor"] = 103;
	literals["real"] = 83;
	literals["virtual"] = 98;
	literals["chr"] = 160;
	literals["near"] = 65;
	literals["object"] = 97;
	literals["public"] = 53;
	literals["repeat"] = 145;
	literals["library"] = 23;
	literals["false"] = 133;
	literals["longint"] = 71;
	literals["saveregisters"] = 62;
	literals["to"] = 148;
	literals["asm"] = 166;
	literals["case"] = 93;
	literals["export"] = 56;
	literals["true"] = 132;
	literals["do"] = 144;
	literals["stdcall"] = 60;
	literals["program"] = 35;
	literals["absolute"] = 164;
	literals["override"] = 105;
	literals["then"] = 141;
	literals["set"] = 94;
	literals["protected"] = 101;
	literals["or"] = 122;
	literals["word"] = 74;
	literals["finally"] = 157;
	literals["char"] = 80;
	literals["if"] = 140;
	literals["far"] = 66;
	literals["const"] = 41;
	literals["index"] = 28;
	literals["assembler"] = 167;
	literals["cardinal"] = 75;
	literals["string"] = 88;
	literals["dispose"] = 170;
	literals["read"] = 109;
	literals["default"] = 111;
	literals["new"] = 173;
	literals["array"] = 50;
	literals["self"] = 174;
	literals["end"] = 25;
	literals["single"] = 84;
	literals["property"] = 108;
	literals["mod"] = 127;
	literals["goto"] = 139;
	literals["on"] = 158;
	literals["comp"] = 87;
	literals["initialization"] = 38;
	literals["class"] = 104;
	literals["int64"] = 72;
	literals["published"] = 107;
	literals["nodefault"] = 112;
	literals["inline"] = 63;
	literals["while"] = 143;
	literals["boolean"] = 77;
	literals["type"] = 44;
	literals["double"] = 85;
	literals["implementation"] = 33;
	literals["exports"] = 26;
	literals["alias"] = 54;
	literals["exit"] = 171;
	literals["shr"] = 130;
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
				else if ((LA(1) == static_cast<unsigned char>('+')) && (LA(2) == static_cast<unsigned char>('='))) {
					mPLUSEQ(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('-')) && (LA(2) == static_cast<unsigned char>('='))) {
					mMINUSEQ(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('*')) && (LA(2) == static_cast<unsigned char>('='))) {
					mSTAREQ(true);
					theRetToken=_returnToken;
				}
				else if ((LA(1) == static_cast<unsigned char>('/')) && (LA(2) == static_cast<unsigned char>('='))) {
					mSLASHQE(true);
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
				else if ((LA(1) == static_cast<unsigned char>('/')) && (LA(2) == static_cast<unsigned char>('/'))) {
					mCOMMENT_3(true);
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
				else if ((LA(1) == static_cast<unsigned char>('/')) && (true)) {
					mSLASH(true);
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
#line 1124 "pascal.g"
		_ttype = DOTDOT;
#line 719 "PascalLexer.cpp"
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

void PascalLexer::mPLUSEQ(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = PLUSEQ;
	int _saveIndex;
	
	match("+=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mMINUSEQ(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = MINUSEQ;
	int _saveIndex;
	
	match("-=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mSTAREQ(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = STAREQ;
	int _saveIndex;
	
	match("*=");
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mSLASHQE(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = SLASHQE;
	int _saveIndex;
	
	match("/=");
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
#line 1142 "pascal.g"
		newline();
#line 859 "PascalLexer.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
#line 1144 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 870 "PascalLexer.cpp"
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
#line 1152 "pascal.g"
			newline();
#line 892 "PascalLexer.cpp"
		}
		else if (((LA(1) == static_cast<unsigned char>('*')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))))&&( LA(2) != ')' )) {
			match(static_cast<unsigned char>('*'));
		}
		else if ((LA(1) == static_cast<unsigned char>('\r')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))) && (true)) {
			match(static_cast<unsigned char>('\r'));
#line 1153 "pascal.g"
			newline();
#line 901 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\n'))) {
			match(static_cast<unsigned char>('\n'));
#line 1154 "pascal.g"
			newline();
#line 907 "PascalLexer.cpp"
		}
		else if ((_tokenSet_0.member(LA(1)))) {
			{
			match(_tokenSet_0);
			}
		}
		else {
			goto _loop427;
		}
		
	}
	_loop427:;
	} // ( ... )*
	match("*)");
#line 1158 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 924 "PascalLexer.cpp"
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
#line 1164 "pascal.g"
			newline();
#line 946 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\r')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && (true) && (true)) {
			match(static_cast<unsigned char>('\r'));
#line 1165 "pascal.g"
			newline();
#line 952 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\n'))) {
			match(static_cast<unsigned char>('\n'));
#line 1166 "pascal.g"
			newline();
#line 958 "PascalLexer.cpp"
		}
		else if ((_tokenSet_1.member(LA(1)))) {
			{
			match(_tokenSet_1);
			}
		}
		else {
			goto _loop431;
		}
		
	}
	_loop431:;
	} // ( ... )*
	match(static_cast<unsigned char>('}'));
#line 1170 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 975 "PascalLexer.cpp"
	if ( _createToken && _token==ANTLR_USE_NAMESPACE(antlr)nullToken && _ttype!=ANTLR_USE_NAMESPACE(antlr)Token::SKIP ) {
	   _token = makeToken(_ttype);
	   _token->setText(text.substr(_begin, text.length()-_begin));
	}
	_returnToken = _token;
	_saveIndex=0;
}

void PascalLexer::mCOMMENT_3(bool _createToken) {
	int _ttype; ANTLR_USE_NAMESPACE(antlr)RefToken _token; int _begin=text.length();
	_ttype = COMMENT_3;
	int _saveIndex;
	
	match("//");
	{ // ( ... )*
	for (;;) {
		if ((_tokenSet_2.member(LA(1)))) {
			matchNot(static_cast<unsigned char>('\n'));
		}
		else {
			goto _loop434;
		}
		
	}
	_loop434:;
	} // ( ... )*
	match(static_cast<unsigned char>('\n'));
#line 1175 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 1005 "PascalLexer.cpp"
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
			goto _loop438;
		}
		}
	}
	_loop438:;
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
		else if ((_tokenSet_3.member(LA(1)))) {
			{
			match(_tokenSet_3);
			}
		}
		else {
			goto _loop442;
		}
		
	}
	_loop442:;
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
	int _cnt445=0;
	for (;;) {
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
		else {
			if ( _cnt445>=1 ) { goto _loop445; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt445++;
	}
	_loop445:;
	}  // ( ... )+
	{
	if ((LA(1) == static_cast<unsigned char>('e'))) {
		mEXPONENT(false);
#line 1204 "pascal.g"
		_ttype = NUM_REAL;
#line 1153 "PascalLexer.cpp"
	}
	else {
		{
		if (((LA(1) == static_cast<unsigned char>('.')))&&((LA(2)!='.')&&(LA(2)!=')'))) {
			match(static_cast<unsigned char>('.'));
#line 1201 "pascal.g"
			_ttype = NUM_REAL;
#line 1161 "PascalLexer.cpp"
			{ // ( ... )+
			int _cnt449=0;
			for (;;) {
				if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
					matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
				}
				else {
					if ( _cnt449>=1 ) { goto _loop449; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
				}
				
				_cnt449++;
			}
			_loop449:;
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
	int _cnt455=0;
	for (;;) {
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
		else {
			if ( _cnt455>=1 ) { goto _loop455; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt455++;
	}
	_loop455:;
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
// VARDECL ARGDECL ARGLIST TYPEDECL FIELD DOT "library" SEMI "end" "exports" 
// COMMA "index" "name" "uses" "unit" "interface" "implementation" "begin" 
// "program" LPAREN RPAREN "initialization" "finalization" "label" "const" 
// EQUAL "type" "var" COLON "procedure" "forward" "function" "array" "of" 
// "external" "public" "alias" INTERRUPT "export" "register" "pascal" "cdecl" 
// "stdcall" "popstack" "saveregisters" "inline" "safecall" "near" "far" 
// NUM_INT "integer" "shortint" "smallint" "longint" "int64" "byte" "word" 
// "cardinal" "qword" "boolean" BYTEBOOL LONGBOOL "char" DOTDOT ASSIGN 
// "real" "single" "double" "extended" "comp" "string" LBRACK RBRACK "packed" 
// "record" "case" "set" "file" POINTER "object" "virtual" "abstract" "private" 
// "protected" "constructor" "destructor" "class" "override" MESSAGE "published" 
// "property" "read" "write" "default" "nodefault" LE GE LTH GT NOT_EQUAL 
// "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" "mod" "and" "shl" "shr" 
// "not" "true" "false" AT PLUSEQ MINUSEQ STAREQ SLASHQE "goto" "if" "then" 
// "else" "while" "do" "repeat" "until" "for" "to" "downto" "with" LBRACK2 
// RBRACK2 "operator" "raise" "try" "except" "finally" "on" STRING_LITERAL 
// "chr" "nil" NUM_REAL IDENT "absolute" "as" "asm" "assembler" "break" 
// "continue" "dispose" "exit" "inherited" "new" "self" METHOD ADDSUBOR 
// ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR LF LCURLY RCURLY WS COMMENT_1 
// COMMENT_2 COMMENT_3 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_0(_tokenSet_0_data_,16);
const unsigned long PascalLexer::_tokenSet_1_data_[] = { 4294958079UL, 4294967295UL, 4294967295UL, 3758096383UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE VARIANT_TAG VARIANT_TAG_NO_ID CONSTLIST FIELDLIST ARGDECLS 
// VARDECL ARGDECL ARGLIST TYPEDECL FIELD DOT "library" SEMI "end" "exports" 
// COMMA "index" "name" "uses" "unit" "interface" "implementation" "begin" 
// "program" LPAREN RPAREN "initialization" "finalization" "label" "const" 
// "resourcestring" EQUAL "type" "var" COLON "procedure" "forward" "function" 
// "array" "of" "external" "public" "alias" INTERRUPT "export" "register" 
// "pascal" "cdecl" "stdcall" "popstack" "saveregisters" "inline" "safecall" 
// "near" "far" NUM_INT "integer" "shortint" "smallint" "longint" "int64" 
// "byte" "word" "cardinal" "qword" "boolean" BYTEBOOL LONGBOOL "char" 
// DOTDOT ASSIGN "real" "single" "double" "extended" "comp" "string" LBRACK 
// RBRACK "packed" "record" "case" "set" "file" POINTER "object" "virtual" 
// "abstract" "private" "protected" "constructor" "destructor" "class" 
// "override" MESSAGE "published" "property" "read" "write" "default" "nodefault" 
// LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR "div" "mod" 
// "and" "shl" "shr" "not" "true" "false" AT PLUSEQ MINUSEQ STAREQ SLASHQE 
// "goto" "if" "then" "else" "while" "do" "repeat" "until" "for" "to" "downto" 
// "with" LBRACK2 RBRACK2 "operator" "raise" "try" "except" "finally" "on" 
// STRING_LITERAL "chr" "nil" NUM_REAL IDENT "absolute" "as" "asm" "assembler" 
// "break" "continue" "dispose" "exit" "inherited" "new" "self" METHOD 
// ADDSUBOR ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR LF LCURLY RCURLY 
// WS COMMENT_1 COMMENT_2 COMMENT_3 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_1(_tokenSet_1_data_,16);
const unsigned long PascalLexer::_tokenSet_2_data_[] = { 4294966271UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE VARIANT_TAG VARIANT_TAG_NO_ID VARIANT_CASE CONSTLIST FIELDLIST 
// ARGDECLS VARDECL ARGDECL ARGLIST TYPEDECL FIELD DOT "library" SEMI "end" 
// "exports" COMMA "index" "name" "uses" "unit" "interface" "implementation" 
// "begin" "program" LPAREN RPAREN "initialization" "finalization" "label" 
// "const" "resourcestring" EQUAL "type" "var" COLON "procedure" "forward" 
// "function" "array" "of" "external" "public" "alias" INTERRUPT "export" 
// "register" "pascal" "cdecl" "stdcall" "popstack" "saveregisters" "inline" 
// "safecall" "near" "far" NUM_INT "integer" "shortint" "smallint" "longint" 
// "int64" "byte" "word" "cardinal" "qword" "boolean" BYTEBOOL LONGBOOL 
// "char" DOTDOT ASSIGN "real" "single" "double" "extended" "comp" "string" 
// LBRACK RBRACK "packed" "record" "case" "set" "file" POINTER "object" 
// "virtual" "abstract" "private" "protected" "constructor" "destructor" 
// "class" "override" MESSAGE "published" "property" "read" "write" "default" 
// "nodefault" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR 
// SLASH "div" "mod" "and" "shl" "shr" "not" "true" "false" AT PLUSEQ MINUSEQ 
// STAREQ SLASHQE "goto" "if" "then" "else" "while" "do" "repeat" "until" 
// "for" "to" "downto" "with" LBRACK2 RBRACK2 "operator" "raise" "try" 
// "except" "finally" "on" STRING_LITERAL "chr" "nil" NUM_REAL IDENT "absolute" 
// "as" "asm" "assembler" "break" "continue" "dispose" "exit" "inherited" 
// "new" "self" METHOD ADDSUBOR ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR 
// LF LCURLY RCURLY WS COMMENT_1 COMMENT_2 COMMENT_3 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_2(_tokenSet_2_data_,16);
const unsigned long PascalLexer::_tokenSet_3_data_[] = { 4294967295UL, 4294967167UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE TYPELIST VARIANT_TAG VARIANT_TAG_NO_ID VARIANT_CASE CONSTLIST 
// FIELDLIST ARGDECLS VARDECL ARGDECL ARGLIST TYPEDECL FIELD DOT "library" 
// SEMI "end" "exports" COMMA "index" "name" "uses" "unit" "interface" 
// "implementation" "begin" "program" LPAREN RPAREN "initialization" "label" 
// "const" "resourcestring" EQUAL "type" "var" COLON "procedure" "forward" 
// "function" "array" "of" "external" "public" "alias" INTERRUPT "export" 
// "register" "pascal" "cdecl" "stdcall" "popstack" "saveregisters" "inline" 
// "safecall" "near" "far" NUM_INT "integer" "shortint" "smallint" "longint" 
// "int64" "byte" "word" "cardinal" "qword" "boolean" BYTEBOOL LONGBOOL 
// "char" DOTDOT ASSIGN "real" "single" "double" "extended" "comp" "string" 
// LBRACK RBRACK "packed" "record" "case" "set" "file" POINTER "object" 
// "virtual" "abstract" "private" "protected" "constructor" "destructor" 
// "class" "override" MESSAGE "published" "property" "read" "write" "default" 
// "nodefault" LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR 
// SLASH "div" "mod" "and" "shl" "shr" "not" "true" "false" AT PLUSEQ MINUSEQ 
// STAREQ SLASHQE "goto" "if" "then" "else" "while" "do" "repeat" "until" 
// "for" "to" "downto" "with" LBRACK2 RBRACK2 "operator" "raise" "try" 
// "except" "finally" "on" STRING_LITERAL "chr" "nil" NUM_REAL IDENT "absolute" 
// "as" "asm" "assembler" "break" "continue" "dispose" "exit" "inherited" 
// "new" "self" METHOD ADDSUBOR ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR 
// LF LCURLY RCURLY WS COMMENT_1 COMMENT_2 COMMENT_3 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_3(_tokenSet_3_data_,16);

