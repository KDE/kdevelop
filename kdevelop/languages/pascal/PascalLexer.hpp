#ifndef INC_PascalLexer_hpp_
#define INC_PascalLexer_hpp_

#line 29 "pascal.g"

        #include "problemreporter.h"
        #include "PascalAST.hpp"

        #include <qlistview.h>
        #include <kdebug.h>

        #define SET_POSITION(ast,t)\
        { \
                RefPascalAST(ast)->setLine( t->getLine() );\
                RefPascalAST(ast)->setColumn( t->getColumn() ); \
        }

#line 19 "PascalLexer.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.2: "pascal.g" -> "PascalLexer.hpp"$ */
#include <antlr/CommonToken.hpp>
#include <antlr/InputBuffer.hpp>
#include <antlr/BitSet.hpp>
#include "PascalTokenTypes.hpp"
#include <antlr/CharScanner.hpp>
class PascalLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public PascalTokenTypes
{
#line 1067 "pascal.g"

private:
        ProblemReporter* m_problemReporter;
        unsigned int m_numberOfErrors;

public:
        void resetErrors()                              { m_numberOfErrors = 0; }
        unsigned int numberOfErrors() const             { return m_numberOfErrors; }
        void setProblemReporter( ProblemReporter* r )   { m_problemReporter = r; }

        virtual void reportError( const ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex ){
                m_problemReporter->reportError( ex.getMessage().c_str(),
                                                ex.getFilename().c_str(),
                                                ex.getLine(),
                                                ex.getColumn() );
                ++m_numberOfErrors;
        }

        virtual void reportError( const ANTLR_USE_NAMESPACE(std)string& errorMessage ){
                m_problemReporter->reportError( errorMessage.c_str(),
                                                getFilename().c_str(),
                                                getLine(), getColumn() );
                ++m_numberOfErrors;
        }

        virtual void reportWarning( const ANTLR_USE_NAMESPACE(std)string& warnMessage ){
                m_problemReporter->reportWarning( warnMessage.c_str(),
                                                getFilename().c_str(),
                                                getLine(), getColumn() );
        }
#line 30 "PascalLexer.hpp"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const
	{
		return false;
	}
public:
	PascalLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	PascalLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	PascalLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mPLUS(bool _createToken);
	public: void mMINUS(bool _createToken);
	public: void mSTAR(bool _createToken);
	public: void mSLASH(bool _createToken);
	public: void mASSIGN(bool _createToken);
	public: void mCOMMA(bool _createToken);
	public: void mSEMI(bool _createToken);
	public: void mCOLON(bool _createToken);
	public: void mEQUAL(bool _createToken);
	public: void mNOT_EQUAL(bool _createToken);
	public: void mLTH(bool _createToken);
	public: void mLE(bool _createToken);
	public: void mGE(bool _createToken);
	public: void mGT(bool _createToken);
	public: void mLPAREN(bool _createToken);
	public: void mRPAREN(bool _createToken);
	public: void mLBRACK(bool _createToken);
	public: void mLBRACK2(bool _createToken);
	public: void mRBRACK(bool _createToken);
	public: void mRBRACK2(bool _createToken);
	public: void mPOINTER(bool _createToken);
	public: void mAT(bool _createToken);
	public: void mDOT(bool _createToken);
	public: void mLCURLY(bool _createToken);
	public: void mRCURLY(bool _createToken);
	public: void mPLUSEQ(bool _createToken);
	public: void mMINUSEQ(bool _createToken);
	public: void mSTAREQ(bool _createToken);
	public: void mSLASHQE(bool _createToken);
	public: void mWS(bool _createToken);
	public: void mCOMMENT_1(bool _createToken);
	public: void mCOMMENT_2(bool _createToken);
	public: void mCOMMENT_3(bool _createToken);
	public: void mIDENT(bool _createToken);
	public: void mSTRING_LITERAL(bool _createToken);
	public: void mNUM_INT(bool _createToken);
	protected: void mEXPONENT(bool _createToken);
private:
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
	static const unsigned long _tokenSet_3_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_3;
};

#endif /*INC_PascalLexer_hpp_*/
