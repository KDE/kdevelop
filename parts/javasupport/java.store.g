// Gideon support by Roberto Raggi (roberto@kdevelop.org)

header "pre_include_hpp" {
	#include "classstore.h"
	#include "JavaAST.hpp"

	#include <qstring.h>
	#include <qstringlist.h>
        #include <qfileinfo.h>
}

header "post_include_hpp" {
	#include <parsedmethod.h>
	#include <parsedclass.h>
	#include <parsedattribute.h>
	#include <parsedargument.h>

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
	ASTLabelType = "RefJavaAST";
}
{
private:
	QString m_fileName;
	QStringList m_currentScope;
	ClassStore* m_store;
	ParsedClassContainer* m_currentContainer;
	ParsedClass* m_currentClass;
	PIAccess m_currentAccess;
	int m_anon;

public:
	void setClassStore( ClassStore* store )			{ m_store = store; }
	ClassStore* classStore()				{ return m_store; }
	const ClassStore* classStore() const			{ return m_store; }

	QString fileName() const	{ return m_fileName; }
	void setFileName( const QString& fileName ) { m_fileName = fileName; }

	void init(){
		m_currentScope.clear();
		m_currentContainer = m_store->globalScope();
		m_currentClass = 0;
		m_currentAccess = PIE_PUBLIC;
		m_anon = 0;
		m_store->removeWithReferences( m_fileName );
	}

	void wipeout()						{ m_store->wipeout(); }
	void out()						{ m_store->out(); }
	void removeWithReferences( const QString& fileName )	{ m_store->removeWithReferences( fileName ); }
}

compilationUnit { QString package; QString imp; QStringList imports; }
	: { init(); }
		(package=packageDefinition)?
		(imp=importDefinition { imports << imp; } )*
		(typeDefinition)*
	;

packageDefinition returns [ QString id ]
	:	#( PACKAGE_DEF id=identifier )
	;

importDefinition returns [ QString id ]
	:	#( IMPORT id=identifierStar )
	;

typeDefinition { QStringList bases; QString className; ParsedClass* klass = 0; }
	:	#(CLASS_DEF m:modifiers IDENT
						{
						klass = new ParsedClass;
						QString name = QString::fromUtf8( #IDENT->getText().c_str(), #IDENT->getText().length() );
						QStringList path = QStringList::split( ".", name );
						className = path.back();
						klass->setName( path.back() );

						klass->setDeclaredInFile( m_fileName );
						klass->setDefinedInFile( m_fileName );
						klass->setDeclaredOnLine( #IDENT->getLine() );
						klass->setDefinedOnLine( #IDENT->getLine() );
						klass->setDeclaredInScope( m_currentScope.join(".") );

						bool innerClass = !m_currentScope.isEmpty();
						if( innerClass )
						    m_currentContainer->addClass( klass );
						else
						    m_store->addClass( klass );

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
		implementsClause
						{
						m_currentScope.push_back( className );
						ParsedClass* oldClass = m_currentClass;
						ParsedClassContainer* oldContainer = m_currentContainer;

						m_currentContainer = klass;
						m_currentClass = klass;
						}
		objBlock[klass]
						{
						m_currentContainer = oldContainer;
						m_currentClass = oldClass;
						m_currentScope.pop_back();
						}
		)
	|	#(INTERFACE_DEF mm:modifiers IDENT
		bases=extendsClause
		interfaceBlock[klass] )
	;

typeSpec returns [ QString tp ]
	:	#(TYPE tp=typeSpecArray)
	;

typeSpecArray returns [ QString tp ]
	:	#( ARRAY_DECLARATOR tp=typeSpecArray ) { tp += "[]"; }
	|	tp=type
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
			(	meth=methodDecl			{
								ParsedMethod* m = m_currentClass->getMethod( meth );
								bool isStored = m != 0;
								if( isStored ){
								    m->setDefinedInFile( m_fileName );
								    m->setDefinedOnLine( meth->definedOnLine() );
								    delete( meth );
								    meth = m;
								} else
								    klass->addMethod( meth );
								}

			|	attr=variableDef		{
								ParsedAttribute* a = m_currentClass->getAttributeByName( attr->name() );
								bool isStored = a != 0;
								if( isStored ){
								    a->setDefinedInFile( m_fileName );
								    a->setDefinedOnLine( attr->definedOnLine() );
								    delete( attr );
								    attr = a;
								} else
								    klass->addAttribute( attr );
								}
			)*
		)
	;

objBlock [ ParsedClass* klass ] { ParsedMethod* meth; ParsedAttribute* attr; }
	:	#(	OBJBLOCK
			(	meth=ctorDef			{
								ParsedMethod* m = m_currentClass->getMethod( meth );
								bool isStored = m != 0;
								if( isStored ){
								    m->setDefinedInFile( m_fileName );
								    m->setDefinedOnLine( meth->definedOnLine() );
								    delete( meth );
								    meth = m;
								} else
								    klass->addMethod( meth );
								}
			|	meth=methodDef			{
								ParsedMethod* m = m_currentClass->getMethod( meth );
								bool isStored = m != 0;
								if( isStored ){
								    m->setDefinedInFile( m_fileName );
								    m->setDefinedOnLine( meth->definedOnLine() );
								    delete( meth );
								    meth = m;
								} else
								    klass->addMethod( meth );
								}
			|	attr=variableDef		{
								ParsedAttribute* a = m_currentClass->getAttributeByName( attr->name() );
								bool isStored = a != 0;
								if( isStored ){
								    a->setDefinedInFile( m_fileName );
								    a->setDefinedOnLine( attr->definedOnLine() );
								    delete( attr );
								    attr = a;
								} else
								    klass->addAttribute( attr );
								}
			|	typeDefinition
			|	#(STATIC_INIT slist)
			|	#(INSTANCE_INIT slist)
			)*
		)
	;

ctorDef returns [ ParsedMethod* meth ]	{ meth = new ParsedMethod; meth->setIsConstructor( TRUE ); }
	:	#(CTOR_DEF
		m:modifiers methodHead[meth] slist
		)
							{
							meth->setDeclaredInFile( m_fileName );
							meth->setDefinedInFile( m_fileName );
							}
	;

methodDecl returns [ ParsedMethod* meth ]  { QString tp; meth = new ParsedMethod; }
	:	#(METHOD_DEF m:modifiers tp=typeSpec methodHead[meth])
							{
							meth->setDeclaredInFile( m_fileName );
							meth->setDefinedInFile( m_fileName );
							meth->setType( tp );
							}
	;

methodDef returns [ ParsedMethod* meth ]  { QString tp; meth = new ParsedMethod; }
	:	#(METHOD_DEF m:modifiers tp=typeSpec methodHead[meth] (slist)?)
							{
							meth->setDeclaredInFile( m_fileName );
							meth->setDefinedInFile( m_fileName );
							meth->setType( tp );
							}
	;

variableDef returns [ ParsedAttribute* attr ] { QString tp; attr = new ParsedAttribute; }
	:	#(VARIABLE_DEF m:modifiers tp=typeSpec variableDeclarator[attr] varInitializer)
							{
							attr->setDeclaredInFile( m_fileName );
							attr->setDefinedInFile( m_fileName );
							attr->setType( tp );
							}
	;

parameterDef returns [ ParsedArgument* arg ] { QString tp; arg = new ParsedArgument; }
	:	#(PARAMETER_DEF modifiers tp=typeSpec IDENT )
							{
							arg->setName( #IDENT->getText().c_str() );
							arg->setType( tp );
							}
	;

objectinitializer
	:	#(INSTANCE_INIT slist)
	;

variableDeclarator [ ParsedAttribute* attr ]
	:	IDENT					{
							attr->setName( #IDENT->getText().c_str() );
							attr->setDeclaredOnLine( #IDENT->getLine() );
							attr->setDefinedOnLine( #IDENT->getLine() );
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

methodHead [ ParsedMethod* meth ] { ParsedArgument* p; }
	:	IDENT #( PARAMETERS (p=parameterDef { meth->addArgument( p ); } )* ) (throwsClause)?
	{
		meth->setName( #IDENT->getText().c_str() );
		meth->setDeclaredOnLine( #IDENT->getLine() );
		meth->setDefinedOnLine( #IDENT->getLine() );
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
