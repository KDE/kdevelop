#ifndef INC_JavaLexer_hpp_
#define INC_JavaLexer_hpp_

#line 2 "java.g"

	#include "driver.h"
	#include "JavaAST.hpp"

	#include <qlistview.h>
	#include <kdebug.h>

	#define SET_POSITION(ast,t)\
	{ \
		RefJavaAST(ast)->setLine( t->getLine() );\
		RefJavaAST(ast)->setColumn( t->getColumn() ); \
	}

#line 19 "JavaLexer.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.2: "java.g" -> "JavaLexer.hpp"$ */
#include <antlr/CommonToken.hpp>
#include <antlr/InputBuffer.hpp>
#include <antlr/BitSet.hpp>
#include "JavaTokenTypes.hpp"
#include <antlr/CharScanner.hpp>
class JavaLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public JavaTokenTypes
{
#line 1058 "java.g"

private:
    Driver* m_driver;

public:
	void setDriver( Driver* d )	{ m_driver = d; }
        void setFileName( const QString& fileName ) { m_driver->currentFileName() = fileName; }

        virtual void reportError( const ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex ){
		m_driver->addProblem( m_driver->currentFileName(), Problem( QString::fromLocal8Bit(ex.getMessage().c_str()), ex.getLine(), ex.getColumn()) );
	}

        virtual void reportError( const ANTLR_USE_NAMESPACE(std)string& errorMessage ){
		m_driver->addProblem( m_driver->currentFileName(), Problem( QString::fromLocal8Bit(errorMessage.c_str()), getLine(), getColumn()) );
	}

        virtual void reportWarning( const ANTLR_USE_NAMESPACE(std)string& warnMessage ){
		m_driver->addProblem( m_driver->currentFileName(), Problem( QString::fromLocal8Bit(warnMessage.c_str()), getLine(), getColumn()) );
	}
#line 30 "JavaLexer.hpp"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const
	{
		return true;
	}
public:
	JavaLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	JavaLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	JavaLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mQUESTION(bool _createToken);
	public: void mLPAREN(bool _createToken);
	public: void mRPAREN(bool _createToken);
	public: void mLBRACK(bool _createToken);
	public: void mRBRACK(bool _createToken);
	public: void mLCURLY(bool _createToken);
	public: void mRCURLY(bool _createToken);
	public: void mCOLON(bool _createToken);
	public: void mCOMMA(bool _createToken);
	public: void mASSIGN(bool _createToken);
	public: void mEQUAL(bool _createToken);
	public: void mLNOT(bool _createToken);
	public: void mBNOT(bool _createToken);
	public: void mNOT_EQUAL(bool _createToken);
	public: void mDIV(bool _createToken);
	public: void mDIV_ASSIGN(bool _createToken);
	public: void mPLUS(bool _createToken);
	public: void mPLUS_ASSIGN(bool _createToken);
	public: void mINC(bool _createToken);
	public: void mMINUS(bool _createToken);
	public: void mMINUS_ASSIGN(bool _createToken);
	public: void mDEC(bool _createToken);
	public: void mSTAR(bool _createToken);
	public: void mSTAR_ASSIGN(bool _createToken);
	public: void mMOD(bool _createToken);
	public: void mMOD_ASSIGN(bool _createToken);
	public: void mSR(bool _createToken);
	public: void mSR_ASSIGN(bool _createToken);
	public: void mBSR(bool _createToken);
	public: void mBSR_ASSIGN(bool _createToken);
	public: void mGE(bool _createToken);
	public: void mGT(bool _createToken);
	public: void mSL(bool _createToken);
	public: void mSL_ASSIGN(bool _createToken);
	public: void mLE(bool _createToken);
	public: void mLT_(bool _createToken);
	public: void mBXOR(bool _createToken);
	public: void mBXOR_ASSIGN(bool _createToken);
	public: void mBOR(bool _createToken);
	public: void mBOR_ASSIGN(bool _createToken);
	public: void mLOR(bool _createToken);
	public: void mBAND(bool _createToken);
	public: void mBAND_ASSIGN(bool _createToken);
	public: void mLAND(bool _createToken);
	public: void mSEMI(bool _createToken);
	public: void mWS(bool _createToken);
	public: void mSL_COMMENT(bool _createToken);
	public: void mML_COMMENT(bool _createToken);
	public: void mCHAR_LITERAL(bool _createToken);
	protected: void mESC(bool _createToken);
	public: void mSTRING_LITERAL(bool _createToken);
	protected: void mHEX_DIGIT(bool _createToken);
	protected: void mVOCAB(bool _createToken);
	public: void mIDENT(bool _createToken);
	public: void mNUM_INT(bool _createToken);
	protected: void mEXPONENT(bool _createToken);
	protected: void mFLOAT_SUFFIX(bool _createToken);
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
};

#endif /*INC_JavaLexer_hpp_*/
