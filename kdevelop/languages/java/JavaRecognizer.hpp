#ifndef INC_JavaRecognizer_hpp_
#define INC_JavaRecognizer_hpp_

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

#line 19 "JavaRecognizer.hpp"
#include <antlr/config.hpp>
/* $ANTLR 2.7.2: "java.g" -> "JavaRecognizer.hpp"$ */
#include <antlr/TokenStream.hpp>
#include <antlr/TokenBuffer.hpp>
#include "JavaTokenTypes.hpp"
#include <antlr/LLkParser.hpp>

/** Java 1.3 Recognizer
 *
 * \verbatim
 * Run 'java Main [-showtree] directory-full-of-java-files'
 * \endverbatim
 *
 * [The -showtree option pops up a Swing frame that shows
 *  the AST constructed from the parser.]
 *
 * \verbatim
 * Run 'java Main <directory full of java files>'
 * \endverbatim
 *
 * Contributing authors:
 *		John Mitchell		johnm@non.net
 *		Terence Parr		parrt@magelang.com
 *		John Lilley			jlilley@empathy.com
 *		Scott Stanchfield	thetick@magelang.com
 *		Markus Mohnen       mohnen@informatik.rwth-aachen.de
 *      Peter Williams      pete.williams@sun.com
 *      Allan Jacobs        Allan.Jacobs@eng.sun.com
 *      Steve Messick       messick@redhills.com
 *      John Pybus			john@pybus.org
 *
 * Version 1.00 December 9, 1997 -- initial release
 * Version 1.01 December 10, 1997
 *		fixed bug in octal def (0..7 not 0..8)
 * Version 1.10 August 1998 (parrt)
 *		added tree construction
 *		fixed definition of WS,comments for mac,pc,unix newlines
 *		added unary plus
 * Version 1.11 (Nov 20, 1998)
 *		Added "shutup" option to turn off last ambig warning.
 *		Fixed inner class def to allow named class defs as statements
 *		synchronized requires compound not simple statement
 *		add [] after builtInType DOT class in primaryExpression
 *		"const" is reserved but not valid..removed from modifiers
 * Version 1.12 (Feb 2, 1999)
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
 *		ESC lexer rule allowed 399 max not 377 max.
 *		java.tree.g didn't handle the expression of synchronized
 *		statements.
 * Version 1.18 (August 12, 2001)
 *      	Terence updated to Java 2 Version 1.3 by
 *		observing/combining work of Allan Jacobs and Steve
 *		Messick.  Handles 1.3 src.  Summary:
 *		o  primary didn't include boolean.class kind of thing
 *      	o  constructor calls parsed explicitly now:
 * 		   see explicitConstructorInvocation
 *		o  add strictfp modifier
 *      	o  missing objBlock after new expression in tree grammar
 *		o  merged local class definition alternatives, moved after declaration
 *		o  fixed problem with ClassName.super.field
 *      	o  reordered some alternatives to make things more efficient
 *		o  long and double constants were not differentiated from int/float
 *		o  whitespace rule was inefficient: matched only one char
 *		o  add an examples directory with some nasty 1.3 cases
 *		o  made Main.java use buffered IO and a Reader for Unicode support
 *		o  supports UNICODE?
 *		   Using Unicode charVocabulay makes code file big, but only
 *		   in the bitsets at the end. I need to make ANTLR generate
 *		   unicode bitsets more efficiently.
 * Version 1.19 (April 25, 2002)
 *		Terence added in nice fixes by John Pybus concerning floating
 *		constants and problems with super() calls.  John did a nice
 *		reorg of the primary/postfix expression stuff to read better
 *		and makes f.g.super() parse properly (it was METHOD_CALL not
 *		a SUPER_CTOR_CALL).  Also:
 *
 *		o  "finally" clause was a root...made it a child of "try"
 *		o  Added stuff for asserts too for Java 1.4, but *commented out*
 *		   as it is not backward compatible.
 *
 * Version 1.20 (October 27, 2002)
 *
 *      Terence ended up reorging John Pybus' stuff to
 *      remove some nondeterminisms and some syntactic predicates.
 *      Note that the grammar is stricter now; e.g., this(...) must
 *	be the first statement.
 *
 *      Trinary ?: operator wasn't working as array name:
 *          (isBig ? bigDigits : digits)[i];
 *
 *      Checked parser/tree parser on source for
 *          Resin-2.0.5, jive-2.1.1, jdk 1.3.1, Lucene, antlr 2.7.2a4,
 *	    and the 110k-line jGuru server source.
 *
 * Version tracking now done with following ID:
 *
 *
 * This grammar is in the PUBLIC DOMAIN
 */
class JavaRecognizer : public ANTLR_USE_NAMESPACE(antlr)LLkParser, public JavaTokenTypes
{
#line 154 "java.g"

private:
	Driver* m_driver;

public:
	void setDriver( Driver* d )	{ m_driver = d; }
        void setFileName( const QString& fileName ) { m_driver->currentFileName() = fileName; }

	void reportError( const ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex ){
		m_driver->addProblem( m_driver->currentFileName(), Problem( QString::fromLocal8Bit(ex.getMessage().c_str()), ex.getLine(), ex.getColumn()) );
	}

	void reportError( const ANTLR_USE_NAMESPACE(std)string& errorMessage ){
		m_driver->addProblem( m_driver->currentFileName(), Problem( QString::fromLocal8Bit(errorMessage.c_str()), LT(1)->getLine(), LT(1)->getColumn()) );
	}

	void reportMessage( const ANTLR_USE_NAMESPACE(std)string& message ){
		m_driver->addProblem( m_driver->currentFileName(), Problem( QString::fromLocal8Bit(message.c_str()), LT(1)->getLine(), LT(1)->getColumn()) );
	}
#line 142 "JavaRecognizer.hpp"
public:
	void initializeASTFactory( ANTLR_USE_NAMESPACE(antlr)ASTFactory& factory );
protected:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf, int k);
public:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenBuffer& tokenBuf);
protected:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer, int k);
public:
	JavaRecognizer(ANTLR_USE_NAMESPACE(antlr)TokenStream& lexer);
	JavaRecognizer(const ANTLR_USE_NAMESPACE(antlr)ParserSharedInputState& state);
	int getNumTokens() const
	{
		return JavaRecognizer::NUM_TOKENS;
	}
	const char* getTokenName( int type ) const
	{
		if( type > getNumTokens() ) return 0;
		return JavaRecognizer::tokenNames[type];
	}
	const char* const* getTokenNames() const
	{
		return JavaRecognizer::tokenNames;
	}
	public: void compilationUnit();
	public: void packageDefinition();
	public: void importDefinition();
	public: void typeDefinition();
	public: void identifier();
	public: void identifierStar();
	public: void modifiers();
	public: void classDefinition(
		RefJavaAST modifiers
	);
	public: void interfaceDefinition(
		RefJavaAST modifiers
	);
	public: void declaration();
	public: void typeSpec(
		bool addImagNode
	);
	public: void variableDefinitions(
		RefJavaAST mods, RefJavaAST t
	);
	public: void classTypeSpec(
		bool addImagNode
	);
	public: void builtInTypeSpec(
		bool addImagNode
	);
	public: void builtInType();
	public: void type();
	public: void modifier();
	public: void superClassClause();
	public: void implementsClause();
	public: void classBlock();
	public: void interfaceExtends();
	public: void field();
	public: void ctorHead();
	public: void constructorBody();
	public: void parameterDeclarationList();
	public: void declaratorBrackets(
		RefJavaAST typ
	);
	public: void throwsClause();
	public: void compoundStatement();
	public: void explicitConstructorInvocation();
	public: void statement();
	public: void argList();
	public: void variableDeclarator(
		RefJavaAST mods, RefJavaAST t
	);
	public: void varInitializer();
	public: void initializer();
	public: void arrayInitializer();
	public: void expression();
	public: void parameterDeclaration();
	public: void parameterModifier();
	public: void forInit();
	public: void forCond();
	public: void forIter();
	public: void casesGroup();
	public: void tryBlock();
	public: void aCase();
	public: void caseSList();
	public: void expressionList();
	public: void handler();
	public: void finallyClause();
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
	public: void identPrimary();
	public: void constant();
	public: void newArrayDeclarator();
public:
	RefJavaAST getAST();
	
protected:
	RefJavaAST returnAST;
private:
	static const char* tokenNames[];
#ifndef NO_STATIC_CONSTS
	static const int NUM_TOKENS = 152;
#else
	enum {
		NUM_TOKENS = 152
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
};

#endif /*INC_JavaRecognizer_hpp_*/
