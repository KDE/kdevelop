// Gideon support by Roberto Raggi (raggi@cli.di.unipi.it)

header "pre_include_hpp" {
	#include "classstore.h"

	#include <qstring.h>
	#include <qstringlist.h>
}

header "post_include_hpp" {
	#include "parsedmethod.h"
	#include "parsedclass.h"
	#include "parsedattribute.h"

	#include <kdebug.h>
}

options {
	language="Cpp";
}

/** Java 1.2 AST Recognizer Grammar
 *
 * Author:
 *	Terence Parr	parrt@magelang.com
 *
 * Version tracking now done with following ID:
 *
 * $Id$
 *
 * This grammar is in the PUBLIC DOMAIN
 *
 * BUGS
 */
class JavaStoreWalker extends TreeParser;

options {
	importVocab=Java;
	defaultErrorHandler = true;     
}
{
private:
	ClassStore* m_store;
	QString m_package;
	ANTLR_USE_NAMESPACE(std)string m_filename;
	ParsedClassContainer* global_ns;

public:
	void setClassStore( ClassStore* store )			{ m_store = store; }
	ClassStore* classStore()				{ return m_store; }
	const ClassStore* classStore() const			{ return m_store; }

	ANTLR_USE_NAMESPACE(std)string getFilename() const	{ return m_filename; }
	void setFilename( const ANTLR_USE_NAMESPACE(std)string& filename ) { m_filename = filename; }

	void init(){
		m_package = QString::null;
	}

	void wipeout()						{ m_store->wipeout(); }
	void out()						{ m_store->out(); }
	void removeWithReferences( const QString& fileName )	{ m_store->removeWithReferences( fileName ); }
}

compilationUnit { ParsedClass* kl; }
	: { init(); }
		(packageDefinition)?
		(importDefinition)*
		(kl=typeDefinition  { m_store->addClass( kl ); } )*
	;

packageDefinition { QString id; }
	:	#( PACKAGE_DEF id=identifier )	{ m_package = id; }
	;

importDefinition returns [ QString id ]
	:	#( IMPORT id=identifierStar )
	;

typeDefinition returns [ParsedClass* klass ] { QStringList bases; klass=0; }
	:	#(CLASS_DEF modifiers IDENT 
						{
						klass = new ParsedClass; 
						klass->setName( #IDENT->getText().c_str() );
						klass->setDeclaredInFile( getFilename().c_str() );
						klass->setDefinedInFile( getFilename().c_str() );
						}
		bases=extendsClause 
						{ 
						QStringList::Iterator it = bases.begin();
						while( it != bases.end() ){
							ParsedParent* parent = new ParsedParent;
							parent->setName( *it );
							klass->addParent( parent );
							++it;
						}
						}
		implementsClause objBlock[klass] 
		)
	|	#(INTERFACE_DEF modifiers IDENT 
						{
						klass = new ParsedClass; 
						klass->setName( #IDENT->getText().c_str() );
						klass->setDeclaredInFile( getFilename().c_str() );
						klass->setDefinedInFile( getFilename().c_str() );
						}
		bases=extendsClause
						{ 
						QStringList::Iterator it = bases.begin();
						while( it != bases.end() ){
							ParsedParent* parent = new ParsedParent;
							parent->setName( *it );
							klass->addParent( parent );
							++it;
						}
						}
		interfaceBlock[klass] )
	;

typeSpec
	:	#(TYPE typeSpecArray)
	;

typeSpecArray
	:	#( ARRAY_DECLARATOR typeSpecArray )
	|	type
	;

type returns [ QString tp ]
	:	tp=identifier
	|	b:builtInType			{ tp = #b->getText().c_str(); }
	;

builtInType
    :   "void"
    |   "boolean"
    |   "byte"
    |   "char"
    |   "short"
    |   "int"
    |   "float"
    |   "long"
    |   "double"
    ;

modifiers returns [ QStringList l ] 
	:	#( MODIFIERS (m:modifier { l << #m->getText().c_str(); } )* )
	;

modifier 
    :   "private"
    |   "public"
    |   "protected"
    |   "static"
    |   "transient"
    |   "final"
    |   "abstract"
    |   "native"
    |   "threadsafe"
    |   "synchronized"
    |   "const"
    |   "volatile"
    ;

extendsClause returns [ QStringList l ] { QString id; }
	:	#(EXTENDS_CLAUSE (id=identifier { l << id; } )* )
	;

implementsClause returns [ QStringList l ] { QString id; }
	:	#(IMPLEMENTS_CLAUSE (id=identifier { l << id; } )* )
	;


interfaceBlock [ ParsedClass* klass ] { ParsedMethod* meth; ParsedAttribute* attr; }
	:	#(	OBJBLOCK
			(	meth=methodDecl			{ klass->addMethod( meth ); }
			|	attr=variableDef		{ klass->addAttribute( attr ); }
			)*
		)
	;

objBlock [ ParsedClass* klass ] { ParsedClass* kl; ParsedMethod* meth; ParsedAttribute* attr; }
	:	#(	OBJBLOCK
			(	meth=ctorDef			{ klass->addMethod( meth ); }
			|	meth=methodDef			{ klass->addMethod( meth ); }
			|	attr=variableDef		{ klass->addAttribute( attr ); }
			|	kl=typeDefinition		{ klass->addClass( kl ); }
			|	#(STATIC_INIT slist)
			|	#(INSTANCE_INIT slist)
			)*
		)
	;

ctorDef returns [ ParsedMethod* meth ]
							{
							meth = new ParsedMethod;
							meth->setIsConstructor( TRUE );
							meth->setDeclaredInFile( getFilename().c_str() );
							meth->setDefinedInFile( getFilename().c_str() );
							}
	:	#(CTOR_DEF 
		modifiers methodHead[meth] slist
		)
	;

methodDecl returns [ ParsedMethod* meth ] 		{ 
							meth = new ParsedMethod;
							meth->setDeclaredInFile( getFilename().c_str() );
							meth->setDefinedInFile( getFilename().c_str() );
							}
	:	#(METHOD_DEF modifiers typeSpec methodHead[meth])
	;

methodDef returns [ ParsedMethod* meth ] 		{ 
							meth = new ParsedMethod; 
							meth->setDeclaredInFile( getFilename().c_str() );	
							meth->setDefinedInFile( getFilename().c_str() );
							}
	:	#(METHOD_DEF modifiers typeSpec methodHead[meth] (slist)?)
	;

variableDef returns [ ParsedAttribute* attr ]
							{
							attr = new ParsedAttribute;
							attr->setDeclaredInFile( getFilename().c_str() );
							attr->setDefinedInFile( getFilename().c_str() );
							}
	:	#(VARIABLE_DEF modifiers typeSpec variableDeclarator[attr] varInitializer)
	;

parameterDef
	:	#(PARAMETER_DEF modifiers typeSpec IDENT )
	;

objectinitializer
	:	#(INSTANCE_INIT slist)
	;

variableDeclarator [ ParsedAttribute* attr ]
	:	IDENT					{
							attr->setName( #IDENT->getText().c_str() );
							}
	|	LBRACK variableDeclarator[attr]
	;

varInitializer
	:	#(ASSIGN initializer)
	|
	;

initializer
	:	expression
	|	arrayInitializer
	;

arrayInitializer
	:	#(ARRAY_INIT (initializer)*)
	;

methodHead [ ParsedMethod* meth ]
	:	IDENT #( PARAMETERS (parameterDef)* ) (throwsClause)?
	{
		meth->setName( #IDENT->getText().c_str() );
	}
	;

throwsClause
	:	#( "throws" (identifier)* )
	;

identifier returns [ QString id ]
	:	IDENT 					{ id = #IDENT->getText().c_str(); }
	|	#( DOT id=identifier IDENT )		{ id += QString(".") + #IDENT->getText().c_str(); }
	;

identifierStar returns [ QString id ] 
	:	IDENT					{ id = #IDENT->getText().c_str(); }
	|	#( DOT id=identifier (STAR  { id += QString(".") + #STAR->getText().c_str(); } |
				      IDENT { id += QString(".") + #IDENT->getText().c_str(); }) ) 
	;

slist
	:	#( SLIST (stat)* )
	;

stat:	typeDefinition
	|	variableDef
	|	expression
	|	#(LABELED_STAT IDENT stat)
	|	#("if" expression stat (stat)? )
	|	#(	"for"
			#(FOR_INIT (variableDef | elist)?)
			#(FOR_CONDITION (expression)?)
			#(FOR_ITERATOR (elist)?)
			stat
		)
	|	#("while" expression stat)
	|	#("do" stat expression)
	|	#("break" (IDENT)? )
	|	#("continue" (IDENT)? )
	|	#("return" (expression)? )
	|	#("switch" expression (caseGroup)*)
	|	#("throw" expression)
	|	#("synchronized" expression stat)
	|	tryBlock
	|	slist // nested SLIST
	|	EMPTY_STAT
	;

caseGroup
	:	#(CASE_GROUP (#("case" expression) | "default")+ slist)
	;

tryBlock
	:	#( "try" slist (handler)* (#("finally" slist))? )
	;

handler
	:	#( "catch" parameterDef slist )
	;

elist
	:	#( ELIST (expression)* )
	;

expression
	:	#(EXPR expr)
	;

expr:	#(QUESTION expr expr expr)	// trinary operator
	|	#(ASSIGN expr expr)			// binary operators...
	|	#(PLUS_ASSIGN expr expr)
	|	#(MINUS_ASSIGN expr expr)
	|	#(STAR_ASSIGN expr expr)
	|	#(DIV_ASSIGN expr expr)
	|	#(MOD_ASSIGN expr expr)
	|	#(SR_ASSIGN expr expr)
	|	#(BSR_ASSIGN expr expr)
	|	#(SL_ASSIGN expr expr)
	|	#(BAND_ASSIGN expr expr)
	|	#(BXOR_ASSIGN expr expr)
	|	#(BOR_ASSIGN expr expr)
	|	#(LOR expr expr)
	|	#(LAND expr expr)
	|	#(BOR expr expr)
	|	#(BXOR expr expr)
	|	#(BAND expr expr)
	|	#(NOT_EQUAL expr expr)
	|	#(EQUAL expr expr)
	|	#(LT_ expr expr)
	|	#(GT expr expr)
	|	#(LE expr expr)
	|	#(GE expr expr)
	|	#(SL expr expr)
	|	#(SR expr expr)
	|	#(BSR expr expr)
	|	#(PLUS expr expr)
	|	#(MINUS expr expr)
	|	#(DIV expr expr)
	|	#(MOD expr expr)
	|	#(STAR expr expr)
	|	#(INC expr)
	|	#(DEC expr)
	|	#(POST_INC expr)
	|	#(POST_DEC expr)
	|	#(BNOT expr)
	|	#(LNOT expr)
	|	#("instanceof" expr expr)
	|	#(UNARY_MINUS expr)
	|	#(UNARY_PLUS expr)
	|	primaryExpression
	;

primaryExpression
    :   IDENT
    |   #(	DOT
			(	expr
				(	IDENT
				|	arrayIndex
				|	"this"
				|	"class"
				|	#( "new" IDENT elist )
				)
			|	#(ARRAY_DECLARATOR type)
			|	builtInType ("class")?
			)
		)
	|	arrayIndex
	|	#(METHOD_CALL primaryExpression elist)
	|	#(TYPECAST typeSpec expr)
	|   newExpression
	|   constant
    |   "super"
    |   "true"
    |   "false"
    |   "this"
    |   "null"
	|	typeSpec // type name used with instanceof
	;

arrayIndex
	:	#(INDEX_OP primaryExpression expression)
	;

constant
    :   NUM_INT
    |   CHAR_LITERAL
    |   STRING_LITERAL
    |   NUM_FLOAT
    ;

newExpression
	:	#(	"new" type
			(	newArrayDeclarator (arrayInitializer)?
			|	elist
			)
		)
			
	;

newArrayDeclarator
	:	#( ARRAY_DECLARATOR (newArrayDeclarator)? (expression)? )
	;
