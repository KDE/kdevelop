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
	literals["until"] = 143;
	literals["xor"] = 122;
	literals["abstract"] = 98;
	literals["shortint"] = 68;
	literals["with"] = 147;
	literals["packed"] = 90;
	literals["inherited"] = 171;
	literals["break"] = 165;
	literals["smallint"] = 69;
	literals["safecall"] = 63;
	literals["constructor"] = 101;
	literals["continue"] = 166;
	literals["uses"] = 29;
	literals["for"] = 144;
	literals["else"] = 139;
	literals["is"] = 118;
	literals["of"] = 51;
	literals["and"] = 127;
	literals["integer"] = 67;
	literals["byte"] = 72;
	literals["nil"] = 158;
	literals["begin"] = 34;
	literals["interface"] = 32;
	literals["as"] = 162;
	literals["div"] = 125;
	literals["write"] = 109;
	literals["qword"] = 75;
	literals["procedure"] = 47;
	literals["shl"] = 128;
	literals["var"] = 45;
	literals["private"] = 99;
	literals["function"] = 49;
	literals["unit"] = 30;
	literals["downto"] = 146;
	literals["name"] = 28;
	literals["resourcestring"] = 42;
	literals["register"] = 56;
	literals["popstack"] = 60;
	literals["label"] = 40;
	literals["try"] = 152;
	literals["raise"] = 151;
	literals["not"] = 130;
	literals["record"] = 91;
	literals["forward"] = 48;
	literals["in"] = 117;
	literals["except"] = 153;
	literals["file"] = 94;
	literals["operator"] = 150;
	literals["pascal"] = 57;
	literals["finalization"] = 39;
	literals["cdecl"] = 58;
	literals["extended"] = 85;
	literals["external"] = 52;
	literals["destructor"] = 102;
	literals["real"] = 82;
	literals["virtual"] = 97;
	literals["chr"] = 157;
	literals["near"] = 64;
	literals["object"] = 96;
	literals["public"] = 53;
	literals["repeat"] = 142;
	literals["library"] = 23;
	literals["false"] = 170;
	literals["longint"] = 70;
	literals["saveregisters"] = 61;
	literals["to"] = 145;
	literals["asm"] = 163;
	literals["case"] = 92;
	literals["export"] = 168;
	literals["true"] = 174;
	literals["do"] = 141;
	literals["stdcall"] = 59;
	literals["program"] = 35;
	literals["absolute"] = 161;
	literals["override"] = 104;
	literals["then"] = 138;
	literals["set"] = 93;
	literals["protected"] = 100;
	literals["or"] = 121;
	literals["word"] = 73;
	literals["finally"] = 154;
	literals["char"] = 79;
	literals["if"] = 137;
	literals["far"] = 65;
	literals["const"] = 41;
	literals["index"] = 27;
	literals["assembler"] = 164;
	literals["cardinal"] = 74;
	literals["string"] = 87;
	literals["dispose"] = 167;
	literals["read"] = 108;
	literals["default"] = 110;
	literals["new"] = 172;
	literals["array"] = 50;
	literals["self"] = 173;
	literals["end"] = 31;
	literals["single"] = 83;
	literals["property"] = 107;
	literals["mod"] = 126;
	literals["goto"] = 136;
	literals["on"] = 155;
	literals["comp"] = 86;
	literals["initialization"] = 38;
	literals["class"] = 103;
	literals["int64"] = 71;
	literals["published"] = 106;
	literals["nodefault"] = 111;
	literals["inline"] = 62;
	literals["while"] = 140;
	literals["boolean"] = 76;
	literals["type"] = 44;
	literals["double"] = 84;
	literals["implementation"] = 33;
	literals["exports"] = 25;
	literals["alias"] = 54;
	literals["exit"] = 169;
	literals["shr"] = 129;
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
#line 1083 "pascal.g"
		_ttype = DOTDOT;
#line 715 "PascalLexer.cpp"
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
#line 1101 "pascal.g"
		newline();
#line 855 "PascalLexer.cpp"
		break;
	}
	default:
	{
		throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());
	}
	}
	}
#line 1103 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 866 "PascalLexer.cpp"
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
#line 1111 "pascal.g"
			newline();
#line 888 "PascalLexer.cpp"
		}
		else if (((LA(1) == static_cast<unsigned char>('*')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))))&&( LA(2) != ')' )) {
			match(static_cast<unsigned char>('*'));
		}
		else if ((LA(1) == static_cast<unsigned char>('\r')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && ((LA(3) >= static_cast<unsigned char>('\0') && LA(3) <= static_cast<unsigned char>('\377'))) && (true)) {
			match(static_cast<unsigned char>('\r'));
#line 1112 "pascal.g"
			newline();
#line 897 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\n'))) {
			match(static_cast<unsigned char>('\n'));
#line 1113 "pascal.g"
			newline();
#line 903 "PascalLexer.cpp"
		}
		else if ((_tokenSet_0.member(LA(1)))) {
			{
			match(_tokenSet_0);
			}
		}
		else {
			goto _loop394;
		}
		
	}
	_loop394:;
	} // ( ... )*
	match("*)");
#line 1117 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 920 "PascalLexer.cpp"
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
#line 1123 "pascal.g"
			newline();
#line 942 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\r')) && ((LA(2) >= static_cast<unsigned char>('\0') && LA(2) <= static_cast<unsigned char>('\377'))) && (true) && (true)) {
			match(static_cast<unsigned char>('\r'));
#line 1124 "pascal.g"
			newline();
#line 948 "PascalLexer.cpp"
		}
		else if ((LA(1) == static_cast<unsigned char>('\n'))) {
			match(static_cast<unsigned char>('\n'));
#line 1125 "pascal.g"
			newline();
#line 954 "PascalLexer.cpp"
		}
		else if ((_tokenSet_1.member(LA(1)))) {
			{
			match(_tokenSet_1);
			}
		}
		else {
			goto _loop398;
		}
		
	}
	_loop398:;
	} // ( ... )*
	match(static_cast<unsigned char>('}'));
#line 1129 "pascal.g"
	_ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP;
#line 971 "PascalLexer.cpp"
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
			goto _loop402;
		}
		}
	}
	_loop402:;
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
			goto _loop406;
		}
		
	}
	_loop406:;
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
	int _cnt409=0;
	for (;;) {
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
		else {
			if ( _cnt409>=1 ) { goto _loop409; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt409++;
	}
	_loop409:;
	}  // ( ... )+
	{
	if ((LA(1) == static_cast<unsigned char>('e'))) {
		mEXPONENT(false);
#line 1158 "pascal.g"
		_ttype = NUM_REAL;
#line 1119 "PascalLexer.cpp"
	}
	else {
		{
		if (((LA(1) == static_cast<unsigned char>('.')))&&((LA(2)!='.')&&(LA(2)!=')'))) {
			match(static_cast<unsigned char>('.'));
#line 1155 "pascal.g"
			_ttype = NUM_REAL;
#line 1127 "PascalLexer.cpp"
			{ // ( ... )+
			int _cnt413=0;
			for (;;) {
				if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
					matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
				}
				else {
					if ( _cnt413>=1 ) { goto _loop413; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
				}
				
				_cnt413++;
			}
			_loop413:;
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
	int _cnt419=0;
	for (;;) {
		if (((LA(1) >= static_cast<unsigned char>('0') && LA(1) <= static_cast<unsigned char>('9')))) {
			matchRange(static_cast<unsigned char>('0'),static_cast<unsigned char>('9'));
		}
		else {
			if ( _cnt419>=1 ) { goto _loop419; } else {throw ANTLR_USE_NAMESPACE(antlr)NoViableAltForCharException(LA(1), getFilename(), getLine(), getColumn());}
		}
		
		_cnt419++;
	}
	_loop419:;
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
// VARDECL ARGDECL ARGLIST TYPEDECL FIELD DOT "library" SEMI "exports" 
// COMMA "index" "name" "uses" "unit" "end" "interface" "implementation" 
// "begin" "program" LPAREN RPAREN "initialization" "finalization" "label" 
// "const" EQUAL "type" "var" COLON "procedure" "forward" "function" "array" 
// "of" "external" "public" "alias" INTERRUPT "register" "pascal" "cdecl" 
// "stdcall" "popstack" "saveregisters" "inline" "safecall" "near" "far" 
// NUM_INT "integer" "shortint" "smallint" "longint" "int64" "byte" "word" 
// "cardinal" "qword" "boolean" BYTEBOOL LONGBOOL "char" DOTDOT ASSIGN 
// "real" "single" "double" "extended" "comp" "string" LBRACK RBRACK "packed" 
// "record" "case" "set" "file" POINTER "object" "virtual" "abstract" "private" 
// "protected" "constructor" "destructor" "class" "override" MESSAGE "published" 
// "property" "read" "write" "default" "nodefault" LE GE LTH GT NOT_EQUAL 
// "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" "mod" "and" "shl" "shr" 
// "not" AT PLUSEQ MINUSEQ STAREQ SLASHQE "goto" "if" "then" "else" "while" 
// "do" "repeat" "until" "for" "to" "downto" "with" LBRACK2 RBRACK2 "operator" 
// "raise" "try" "except" "finally" "on" STRING_LITERAL "chr" "nil" NUM_REAL 
// IDENT "absolute" "as" "asm" "assembler" "break" "continue" "dispose" 
// "export" "exit" "false" "inherited" "new" "self" "true" METHOD ADDSUBOR 
// ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR LF LCURLY RCURLY WS COMMENT_1 
// COMMENT_2 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_0(_tokenSet_0_data_,16);
const unsigned long PascalLexer::_tokenSet_1_data_[] = { 4294958079UL, 4294967295UL, 4294967295UL, 3758096383UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE VARIANT_TAG VARIANT_TAG_NO_ID CONSTLIST FIELDLIST ARGDECLS 
// VARDECL ARGDECL ARGLIST TYPEDECL FIELD DOT "library" SEMI "exports" 
// COMMA "index" "name" "uses" "unit" "end" "interface" "implementation" 
// "begin" "program" LPAREN RPAREN "initialization" "finalization" "label" 
// "const" "resourcestring" EQUAL "type" "var" COLON "procedure" "forward" 
// "function" "array" "of" "external" "public" "alias" INTERRUPT "register" 
// "pascal" "cdecl" "stdcall" "popstack" "saveregisters" "inline" "safecall" 
// "near" "far" NUM_INT "integer" "shortint" "smallint" "longint" "int64" 
// "byte" "word" "cardinal" "qword" "boolean" BYTEBOOL LONGBOOL "char" 
// DOTDOT ASSIGN "real" "single" "double" "extended" "comp" "string" LBRACK 
// RBRACK "packed" "record" "case" "set" "file" POINTER "object" "virtual" 
// "abstract" "private" "protected" "constructor" "destructor" "class" 
// "override" MESSAGE "published" "property" "read" "write" "default" "nodefault" 
// LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "mod" 
// "and" "shl" "shr" "not" AT PLUSEQ MINUSEQ STAREQ SLASHQE "goto" "if" 
// "then" "else" "while" "do" "repeat" "until" "for" "to" "downto" "with" 
// LBRACK2 RBRACK2 "operator" "raise" "try" "except" "finally" "on" STRING_LITERAL 
// "chr" "nil" NUM_REAL IDENT "absolute" "as" "asm" "assembler" "break" 
// "continue" "dispose" "export" "exit" "false" "inherited" "new" "self" 
// "true" METHOD ADDSUBOR ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR 
// LF LCURLY RCURLY WS COMMENT_1 COMMENT_2 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_1(_tokenSet_1_data_,16);
const unsigned long PascalLexer::_tokenSet_2_data_[] = { 4294967295UL, 4294967167UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 4294967295UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL };
// null EOF null NULL_TREE_LOOKAHEAD BLOCK IDLIST ELIST FUNC_CALL PROC_CALL 
// SCALARTYPE TYPELIST VARIANT_TAG VARIANT_TAG_NO_ID VARIANT_CASE CONSTLIST 
// FIELDLIST ARGDECLS VARDECL ARGDECL ARGLIST TYPEDECL FIELD DOT "library" 
// SEMI "exports" COMMA "index" "name" "uses" "unit" "end" "interface" 
// "implementation" "begin" "program" LPAREN RPAREN "initialization" "label" 
// "const" "resourcestring" EQUAL "type" "var" COLON "procedure" "forward" 
// "function" "array" "of" "external" "public" "alias" INTERRUPT "register" 
// "pascal" "cdecl" "stdcall" "popstack" "saveregisters" "inline" "safecall" 
// "near" "far" NUM_INT "integer" "shortint" "smallint" "longint" "int64" 
// "byte" "word" "cardinal" "qword" "boolean" BYTEBOOL LONGBOOL "char" 
// DOTDOT ASSIGN "real" "single" "double" "extended" "comp" "string" LBRACK 
// RBRACK "packed" "record" "case" "set" "file" POINTER "object" "virtual" 
// "abstract" "private" "protected" "constructor" "destructor" "class" 
// "override" MESSAGE "published" "property" "read" "write" "default" "nodefault" 
// LE GE LTH GT NOT_EQUAL "in" "is" PLUS MINUS "or" "xor" STAR SLASH "div" 
// "mod" "and" "shl" "shr" "not" AT PLUSEQ MINUSEQ STAREQ SLASHQE "goto" 
// "if" "then" "else" "while" "do" "repeat" "until" "for" "to" "downto" 
// "with" LBRACK2 RBRACK2 "operator" "raise" "try" "except" "finally" "on" 
// STRING_LITERAL "chr" "nil" NUM_REAL IDENT "absolute" "as" "asm" "assembler" 
// "break" "continue" "dispose" "export" "exit" "false" "inherited" "new" 
// "self" "true" METHOD ADDSUBOR ASSIGNEQUAL SIGN FUNC NODE_NOT_EMIT MYASTVAR 
// LF LCURLY RCURLY WS COMMENT_1 COMMENT_2 EXPONENT 
const ANTLR_USE_NAMESPACE(antlr)BitSet PascalLexer::_tokenSet_2(_tokenSet_2_data_,16);

