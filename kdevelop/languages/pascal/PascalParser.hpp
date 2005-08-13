#ifndef INC_PascalParser_hpp_
#define INC_PascalParser_hpp_

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

#line 19 "PascalParser.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.2: "pascal.g" -> "PascalParser.hpp"$ */
#include <antlr/TokenStream.hpp>
#include <antlr/TokenBuffer.hpp>
#include "PascalTokenTypes.hpp"
#include <antlr/LLkParser.hpp>

class PascalParser : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public PascalTokenTypes
{
#line 90 "pascal.g"

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
	public: void compilationUnit();
	public: void program();
	public: void library();
	public: void unit();
	public: void programHeading();
	public: void usesClause();
	public: void block();
	public: void identifier();
	public: void libraryBlock();
	public: void exportsClause();
	public: void declarationPart();
	public: void statementPart();
	public: void exportsList();
	public: void exportsEntry();
	public: void integerConstant();
	public: void stringConstant();
	public: void identifierList();
	public: void interfacePart();
	public: void implementationPart();
	public: void initializationPart();
	public: void finalizationPart();
	public: void realizationPart();
	public: void constantDeclarationPart();
	public: void typeDeclarationPart();
	public: void procedureHeadersPart();
	public: void statement();
	public: void labelDeclarationPart();
	public: void resourcestringDeclarationPart();
	public: void variableDeclarationPart();
	public: void procedureAndFunctionDeclarationPart();
	public: void label();
	public: void constantDeclaration();
	public: void typedConstantDeclaration();
	public: void stringConstantDeclaration();
	public: void string();
	public: void typeDeclaration();
	public: void variableDeclaration();
	public: void type();
	public: void procedureAndFunctionDeclaration();
	public: void procedureDeclaration();
	public: void functionDeclaration();
	public: void constructorDeclaration();
	public: void destructorDeclaration();
	public: void compoundStatement();
	public: void procedureHeader();
	public: void subroutineBlock();
	public: void functionHeader();
	public: void qualifiedMethodIdentifier();
	public: void formalParameterList();
	public: void modifiers();
	public: void externalDirective();
	public: void functionHeaderEnding();
	public: void parameterDeclaration();
	public: void valueParameter();
	public: void variableParameter();
	public: void constantParameter();
	public: void untypedParameterPart();
	public: void callModifiers();
	public: void expression();
	public: void typedConstant();
	public: void constant();
	public: void recordConstant();
	public: void arrayConstant();
	public: void proceduralConstant();
	public: void addressConstant();
	public: void simpleType();
	public: void subrangeTypeOrTypeIdentifier();
	public: void enumeratedType();
	public: void stringType();
	public: void structuredType();
	public: void pointerType();
	public: void proceduralType();
	public: void ordinalType();
	public: void realType();
	public: void typeIdentifier();
	public: void subrangeType();
	public: void assignedEnumList();
	public: void unsignedInteger();
	public: void arrayType();
	public: void recordType();
	public: void objectType();
	public: void classType();
	public: void setType();
	public: void fileType();
	public: void arrayIndexType();
	public: void arraySubrangeType();
	public: void fieldList();
	public: void fixedField();
	public: void variantPart();
	public: void variant();
	public: void proceduralTypePart1();
	public: void heritage();
	public: void componentList();
	public: void objectVisibilitySpecifier();
	public: void fieldDefinition();
	public: void methodDefinition();
	public: void constructorHeader();
	public: void destructorHeader();
	public: void methodDirectives();
	public: void classComponentList();
	public: void classVisibilitySpecifier();
	public: void classMethodDefinition();
	public: void propertyDefinition();
	public: void classMethodDirectives();
	public: void directiveVariants();
	public: void propertyInterface();
	public: void propertySpecifiers();
	public: void propertyParameterList();
	public: void readSpecifier();
	public: void writeSpecifier();
	public: void defaultSpecifier();
	public: void fieldOrMethod();
	public: void simpleExpression();
	public: void expressionSign();
	public: void term();
	public: void factor();
	public: void identifierOrValueTypecastOrFunctionCall();
	public: void unsignedConstant();
	public: void setConstructor();
	public: void addressFactor();
	public: void expressions();
	public: void functionCall();
	public: void actualParameterList();
	public: void setGroup();
	public: void valueTypecast();
	public: void simpleStatement();
	public: void structuredStatement();
	public: void assignmentStatement();
	public: void procedureStatement();
	public: void gotoStatement();
	public: void raiseStatement();
	public: void identifierOrArrayIdentifier();
	public: void assignmentOperator();
	public: void repetitiveStatement();
	public: void conditionalStatement();
	public: void exceptionStatement();
	public: void withStatement();
	public: void ifStatement();
	public: void caseStatement();
	public: void forStatement();
	public: void repeatStatement();
	public: void whileStatement();
	public: void caseListElement();
	public: void constList();
	public: void forList();
	public: void initialValue();
	public: void finalValue();
	public: void recordVariableList();
	public: void variable();
	public: void operatorDefinition();
	public: void assignmentOperatorDefinition();
	public: void arithmeticOperatorDefinition();
	public: void comparisonOperatorDefinition();
	public: void tryStatement();
	public: void statements();
	public: void exceptOrFinallyPart();
	public: void exceptionHandlers();
	public: void exceptionHandler();
	public: void sign();
	public: void constantChr();
	public: void unsignedNumber();
	public: void unsignedReal();
public:
	RefPascalAST getAST();
	
protected:
	RefPascalAST returnAST;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 190;
#else
	enum {
		NUM_TOKENS = 190
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
	static const unsigned long _tokenSet_44_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_44;
	static const unsigned long _tokenSet_45_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_45;
	static const unsigned long _tokenSet_46_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_46;
	static const unsigned long _tokenSet_47_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_47;
	static const unsigned long _tokenSet_48_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_48;
	static const unsigned long _tokenSet_49_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_49;
	static const unsigned long _tokenSet_50_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_50;
	static const unsigned long _tokenSet_51_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_51;
	static const unsigned long _tokenSet_52_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_52;
	static const unsigned long _tokenSet_53_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_53;
	static const unsigned long _tokenSet_54_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_54;
	static const unsigned long _tokenSet_55_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_55;
	static const unsigned long _tokenSet_56_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_56;
	static const unsigned long _tokenSet_57_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_57;
	static const unsigned long _tokenSet_58_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_58;
	static const unsigned long _tokenSet_59_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_59;
	static const unsigned long _tokenSet_60_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_60;
	static const unsigned long _tokenSet_61_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_61;
	static const unsigned long _tokenSet_62_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_62;
	static const unsigned long _tokenSet_63_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_63;
	static const unsigned long _tokenSet_64_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_64;
	static const unsigned long _tokenSet_65_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_65;
	static const unsigned long _tokenSet_66_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_66;
	static const unsigned long _tokenSet_67_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_67;
	static const unsigned long _tokenSet_68_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_68;
	static const unsigned long _tokenSet_69_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_69;
	static const unsigned long _tokenSet_70_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_70;
	static const unsigned long _tokenSet_71_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_71;
	static const unsigned long _tokenSet_72_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_72;
	static const unsigned long _tokenSet_73_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_73;
	static const unsigned long _tokenSet_74_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_74;
	static const unsigned long _tokenSet_75_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_75;
	static const unsigned long _tokenSet_76_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_76;
};

#endif /*INC_PascalParser_hpp_*/
