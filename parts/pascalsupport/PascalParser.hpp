#ifndef INC_PascalParser_hpp_
#define INC_PascalParser_hpp_

#line 20 "pascal.g"

        #include "problemreporter.h"
        #include "PascalAST.hpp"

        #include <qlistview.h>
        #include <kdebug.h>

        #define SET_POSITION(ast,t)\
        { \
                RefPascalAST(ast)->setLine( t->getLine() );\
                RefPascalAST(ast)->setColumn( t->getColumn() ); \
        }

#line 19 "PascalParser.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.2: "pascal.g" -> "PascalParser.hpp"$ */
#include <antlr/TokenStream.hpp>
#include <antlr/TokenBuffer.hpp>
#include "PascalTokenTypes.hpp"
#include <antlr/LLkParser.hpp>

class PascalParser : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public PascalTokenTypes
{
#line 81 "pascal.g"

private:
        unsigned int m_numberOfErrors;
        ProblemReporter* m_problemReporter;

public:
        void resetErrors()                              { m_numberOfErrors = 0; }
        unsigned int numberOfErrors() const             { return m_numberOfErrors; }
        void setProblemReporter( ProblemReporter* r )   { m_problemReporter = r; }

        void reportError( const ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex ){
                m_problemReporter->reportError( ex.getMessage().c_str(),
                                                ex.getFilename().c_str(),
                                                ex.getLine(),
                                                ex.getColumn() );
                ++m_numberOfErrors;
        }

        void reportError( const ANTLR_USE_NAMESPACE(std)string& errorMessage ){
                m_problemReporter->reportError( errorMessage.c_str(),
                                                getFilename().c_str(),
                                                LT(1)->getLine(),
                                                LT(1)->getColumn() );
                ++m_numberOfErrors;
        }

        void reportMessage( const ANTLR_USE_NAMESPACE(std)string& message ){
                m_problemReporter->reportMessage( message.c_str(),
                                                getFilename().c_str(),
                                                LT(1)->getLine(),
                                                LT(1)->getColumn() );
        }
#line 30 "PascalParser.hpp"
public:
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
protected:
	PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	PascalParser(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	PascalParser(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	int getNumTokens() const
	{
		return PascalParser::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return PascalParser::tokenNames[type];
	}
	const char* const* getTokenNames() const
	{
		return PascalParser::tokenNames;
	}
	public: void program();
	public: void programHeading();
	public: void block();
	public: void identifier();
	public: void identifierList();
	public: void labelDeclarationPart();
	public: void constantDefinitionPart();
	public: void typeDefinitionPart();
	public: void variableDeclarationPart();
	public: void procedureAndFunctionDeclarationPart();
	public: void usesUnitsPart();
	public: void compoundStatement();
	public: void label();
	public: void unsignedInteger();
	public: void constantDefinition();
	public: void constant();
	public: void constantChr();
	public: void unsignedNumber();
	public: void sign();
	public: void string();
	public: void unsignedReal();
	public: void typeDefinition();
	public: void type();
	public: void functionType();
	public: void procedureType();
	public: void formalParameterList();
	public: void resultType();
	public: void simpleType();
	public: void structuredType();
	public: void pointerType();
	public: void scalarType();
	public: void subrangeType();
	public: void typeIdentifier();
	public: void stringtype();
	public: void unpackedStructuredType();
	public: void arrayType();
	public: void recordType();
	public: void setType();
	public: void fileType();
	public: void typeList();
	public: void componentType();
	public: void indexType();
	public: void fieldList();
	public: void fixedPart();
	public: void variantPart();
	public: void recordSection();
	public: void tag();
	public: void variant();
	public: void constList();
	public: void baseType();
	public: void variableDeclaration();
	public: void procedureOrFunctionDeclaration();
	public: void procedureDeclaration();
	public: void functionDeclaration();
	public: void formalParameterSection();
	public: void parameterGroup();
	public: void statement();
	public: void unlabelledStatement();
	public: void simpleStatement();
	public: void structuredStatement();
	public: void assignmentStatement();
	public: void procedureStatement();
	public: void gotoStatement();
	public: void emptyStatement();
	public: void variable();
	public: void expression();
	public: void simpleExpression();
	public: void term();
	public: void signedFactor();
	public: void factor();
	public: void functionDesignator();
	public: void unsignedConstant();
	public: void set();
	public: void parameterList();
	public: void actualParameter();
	public: void elementList();
	public: void element();
	public: void empty();
	public: void conditionalStatement();
	public: void repetetiveStatement();
	public: void withStatement();
	public: void statements();
	public: void ifStatement();
	public: void caseStatement();
	public: void caseListElement();
	public: void whileStatement();
	public: void repeatStatement();
	public: void forStatement();
	public: void forList();
	public: void initialValue();
	public: void finalValue();
	public: void recordVariableList();
public:
	RefPascalAST getAST();
	
protected:
	RefPascalAST returnAST;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 109;
#else
	enum {
		NUM_TOKENS = 109
	};
#endif
	
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
	static const unsigned long _tokenSet_8_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_8;
	static const unsigned long _tokenSet_9_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_9;
	static const unsigned long _tokenSet_10_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_10;
	static const unsigned long _tokenSet_11_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_11;
	static const unsigned long _tokenSet_12_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_12;
	static const unsigned long _tokenSet_13_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_13;
	static const unsigned long _tokenSet_14_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_14;
	static const unsigned long _tokenSet_15_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_15;
	static const unsigned long _tokenSet_16_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_16;
	static const unsigned long _tokenSet_17_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_17;
	static const unsigned long _tokenSet_18_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_18;
	static const unsigned long _tokenSet_19_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_19;
	static const unsigned long _tokenSet_20_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_20;
	static const unsigned long _tokenSet_21_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_21;
	static const unsigned long _tokenSet_22_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_22;
	static const unsigned long _tokenSet_23_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_23;
	static const unsigned long _tokenSet_24_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_24;
	static const unsigned long _tokenSet_25_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_25;
	static const unsigned long _tokenSet_26_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_26;
	static const unsigned long _tokenSet_27_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_27;
	static const unsigned long _tokenSet_28_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_28;
	static const unsigned long _tokenSet_29_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_29;
	static const unsigned long _tokenSet_30_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_30;
	static const unsigned long _tokenSet_31_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_31;
	static const unsigned long _tokenSet_32_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_32;
	static const unsigned long _tokenSet_33_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_33;
	static const unsigned long _tokenSet_34_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_34;
	static const unsigned long _tokenSet_35_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_35;
	static const unsigned long _tokenSet_36_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_36;
	static const unsigned long _tokenSet_37_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_37;
	static const unsigned long _tokenSet_38_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_38;
	static const unsigned long _tokenSet_39_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_39;
	static const unsigned long _tokenSet_40_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_40;
	static const unsigned long _tokenSet_41_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_41;
	static const unsigned long _tokenSet_42_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_42;
	static const unsigned long _tokenSet_43_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_43;
};

#endif /*INC_PascalParser_hpp_*/
