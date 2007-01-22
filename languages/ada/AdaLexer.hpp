#ifndef INC_AdaLexer_hpp_
#define INC_AdaLexer_hpp_

#line 28 "ada.g"

#include <antlr/SemanticException.hpp>  // antlr wants this
#include "AdaAST.hpp"
#include "preambles.h"

#line 11 "AdaLexer.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.7 (20061129): "ada.g" -> "AdaLexer.hpp"$ */
#include <antlr/CommonToken.hpp>
#include <antlr/InputBuffer.hpp>
#include <antlr/BitSet.hpp>
#include "AdaTokenTypes.hpp"
#include <antlr/CharScanner.hpp>
class CUSTOM_API AdaLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public AdaTokenTypes
{
#line 1848 "ada.g"

  ANTLR_LEXER_PREAMBLE
#line 22 "AdaLexer.hpp"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const
	{
		return false;
	}
public:
	AdaLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	AdaLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	AdaLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mCOMMENT_INTRO(bool _createToken);
	public: void mDOT_DOT(bool _createToken);
	public: void mLT_LT(bool _createToken);
	public: void mBOX(bool _createToken);
	public: void mGT_GT(bool _createToken);
	public: void mASSIGN(bool _createToken);
	public: void mRIGHT_SHAFT(bool _createToken);
	public: void mNE(bool _createToken);
	public: void mLE(bool _createToken);
	public: void mGE(bool _createToken);
	public: void mEXPON(bool _createToken);
	public: void mPIPE(bool _createToken);
	public: void mCONCAT(bool _createToken);
	public: void mDOT(bool _createToken);
	public: void mEQ(bool _createToken);
	public: void mLT_(bool _createToken);
	public: void mGT(bool _createToken);
	public: void mPLUS(bool _createToken);
	public: void mMINUS(bool _createToken);
	public: void mSTAR(bool _createToken);
	public: void mDIV(bool _createToken);
	public: void mLPAREN(bool _createToken);
	public: void mRPAREN(bool _createToken);
	public: void mCOLON(bool _createToken);
	public: void mCOMMA(bool _createToken);
	public: void mSEMI(bool _createToken);
	public: void mTIC(bool _createToken);
	public: void mIDENTIFIER(bool _createToken);
	public: void mCHARACTER_LITERAL(bool _createToken);
	public: void mCHAR_STRING(bool _createToken);
	public: void mNUMERIC_LIT(bool _createToken);
	protected: void mDIGIT(bool _createToken);
	protected: void mBASED_INTEGER(bool _createToken);
	protected: void mEXPONENT(bool _createToken);
	protected: void mEXTENDED_DIGIT(bool _createToken);
	public: void mWS_(bool _createToken);
	public: void mCOMMENT(bool _createToken);
private:
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_3;
	static const unsigned long _tokenSet_4_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_4;
	static const unsigned long _tokenSet_5_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_5;
	static const unsigned long _tokenSet_6_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_6;
	static const unsigned long _tokenSet_7_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_7;
};

#endif /*INC_AdaLexer_hpp_*/
