#ifndef INC_JavaRecognizer_hpp_
#define INC_JavaRecognizer_hpp_

#line 2 "java.g"

	#include "problemreporter.h"

	#include <qlistview.h>
	#include <kdebug.h>

#line 12 "JavaRecognizer.hpp"
#include "antlr/config.hpp"
/* $ANTLR 2.7.1: "java.g" -> "JavaRecognizer.hpp"$ */
#include "antlr/TokenStream.hpp"
#include "antlr/TokenBuffer.hpp"
#include "JavaTokenTypes.hpp"
#include "antlr/LLkParser.hpp"

/** Java 1.2 Recognizer
 *
 * Run 'java Main <directory full of java files>'
 *
 * Contributing authors:
 *		John Mitchell		johnm@non.net
 *		Terence Parr		parrt@magelang.com
 *		John Lilley			jlilley@empathy.com
 *		Scott Stanchfield	thetick@magelang.com
 *		Markus Mohnen       mohnen@informatik.rwth-aachen.de
 *		Peter Williams		pwilliams@netdynamics.com
 *
 * Version 1.00 December 9, 1997 -- initial release
 * Version 1.01 December 10, 1997
 *		fixed bug in octal def (0..7 not 0..8)
 * Version 1.10 August 1998 (parrt)
 *		added tree construction
 *		fixed definition of WS_,comments for mac,pc,unix newlines
 *		added unary plus
 * Version 1.11 (Nov 20, 1998)
 *		Added "shutup" option to turn off last ambig warning.
 *		Fixed inner class def to allow named class defs as statements
 *		synchronized requires compound not simple statement
 *		add [] after builtInType DOT class in primaryExpression
 *		"const" is reserved but not valid..removed from modifiers
 *	Version 1.12 (Feb 2, 1999)
 *		Changed LITERAL_xxx to xxx in tree grammar.
 *		Updated java.g to use tokens {...} now for 2.6.0 (new feature).
 *
 * Version 1.13 (Apr 23, 1999)
 *		Didn't have (stat)? for else clause in tree parser.
 *		Didn't gen ASTs for interface extends.  Updated tree parser too.
 *		Updated to 2.6.0.
 * Version 1.14 (Jun 20, 1999)
 *		Allowed final/abstract on local classes.
 *		Removed local interfaces from methods
 *		Put instanceof precedence where it belongs...in relationalExpr
 *			It also had expr not type as arg; fixed it.
 *		Missing ! on SEMI in classBlock
 *		fixed: (expr) + "string" was parsed incorrectly (+ as unary plus).
 *		fixed: didn't like Object[].class in parser or tree parser
 * Version 1.15 (Jun 26, 1999)
 *		Screwed up rule with instanceof in it. :(  Fixed.
 *		Tree parser didn't like (expr).something; fixed.
 *		Allowed multiple inheritance in tree grammar. oops.
 * Version 1.16 (August 22, 1999)
 *		Extending an interface built a wacky tree: had extra EXTENDS.
 *		Tree grammar didn't allow multiple superinterfaces.
 *		Tree grammar didn't allow empty var initializer: {}
 * Version 1.17 (October 12, 1999)
 *    ESC lexer rule allowed 399 max not 377 max.
 *    java.tree.g didn't handle the expression of synchronized
 *       statements.
 *
 * Version tracking now done with following ID:
 *
 * $Id$
 *
 * This grammar is in the PUBLIC DOMAIN
 *
 * BUGS
 *
 */
class JavaRecognizer : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public JavaTokenTypes
 {
#line 97 "java.g"

private:
	unsigned int m_numberOfErrors;
	ProblemReporter* m_problemReporter;

public:
	void resetErrors()				{ m_numberOfErrors = 0; }
	unsigned int numberOfErrors() const		{ return m_numberOfErrors; }
	void setProblemReporter( ProblemReporter* r )	{ m_problemReporter = r; }

	void reportError( const ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex ){
		m_problemReporter->reportError( ex.getErrorMessage().c_str(),
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
#line 86 "JavaRecognizer.hpp"
protected:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	JavaRecognizer(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	public: void compilationUnit();
	public: void packageDefinition();
	public: void importDefinition();
	public: void typeDefinition();
	public: void identifier();
	public: void identifierStar();
	public: void modifiers();
	public: void classDefinition(
		ANTLR_USE_NAMESPACE(antlr)RefAST modifiers
	);
	public: void interfaceDefinition(
		ANTLR_USE_NAMESPACE(antlr)RefAST modifiers
	);
	public: void declaration();
	public: void typeSpec(
		bool addImagNode
	);
	public: void variableDefinitions(
		ANTLR_USE_NAMESPACE(antlr)RefAST mods, ANTLR_USE_NAMESPACE(antlr)RefAST t
	);
	public: void modifier();
	public: void classTypeSpec(
		bool addImagNode
	);
	public: void builtInTypeSpec(
		bool addImagNode
	);
	public: void builtInType();
	public: void type();
	public: void superClassClause();
	public: void implementsClause();
	public: void classBlock();
	public: void interfaceExtends();
	public: void field();
	public: void ctorHead();
	public: void compoundStatement();
	public: void parameterDeclarationList();
	public: void returnTypeBrackersOnEndOfMethodHead(
		ANTLR_USE_NAMESPACE(antlr)RefAST typ
	);
	public: void throwsClause();
	public: void variableDeclarator(
		ANTLR_USE_NAMESPACE(antlr)RefAST mods, ANTLR_USE_NAMESPACE(antlr)RefAST t
	);
	public: void declaratorBrackets(
		ANTLR_USE_NAMESPACE(antlr)RefAST typ
	);
	public: void varInitializer();
	public: void initializer();
	public: void arrayInitializer();
	public: void expression();
	public: void parameterDeclaration();
	public: void parameterModifier();
	public: void parameterDeclaratorBrackets(
		ANTLR_USE_NAMESPACE(antlr)RefAST t
	);
	public: void statement();
	public: void forInit();
	public: void forCond();
	public: void forIter();
	public: void casesGroup();
	public: void tryBlock();
	public: void aCase();
	public: void caseSList();
	public: void expressionList();
	public: void handler();
	public: void assignmentExpression();
	public: void conditionalExpression();
	public: void logicalOrExpression();
	public: void logicalAndExpression();
	public: void inclusiveOrExpression();
	public: void exclusiveOrExpression();
	public: void andExpression();
	public: void equalityExpression();
	public: void relationalExpression();
	public: void shiftExpression();
	public: void additiveExpression();
	public: void multiplicativeExpression();
	public: void unaryExpression();
	public: void unaryExpressionNotPlusMinus();
	public: void postfixExpression();
	public: void primaryExpression();
	public: void newExpression();
	public: void argList();
	public: void constant();
	public: void newArrayDeclarator();
public:
	ANTLR_USE_NAMESPACE(antlr)RefAST getAST();
	
protected:
	ANTLR_USE_NAMESPACE(antlr)RefAST returnAST;
private:
	static const char* _tokenNames[];
	
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
};

#endif /*INC_JavaRecognizer_hpp_*/
