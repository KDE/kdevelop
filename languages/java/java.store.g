// KDevelop support by Roberto Raggi (roberto@kdevelop.org)

header "pre_include_hpp" {
	#include <codemodel.h>
	#include "JavaAST.hpp"

	#include <qstring.h>
	#include <qstringlist.h>
	#include <qvaluestack.h>
        #include <qfileinfo.h>
}

header "post_include_hpp" {
	#include <codemodel.h>

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
	QStringList m_currentScope;
	CodeModel* m_model;
	FileDom m_file;
	QValueStack<ClassDom> m_currentClass;
	int m_currentAccess;
	int m_anon;
        ANTLR_USE_NAMESPACE(antlr)JavaASTFactory ast_factory;

public:
	void setCodeModel( CodeModel* model )
	{
		m_model = model;
	}

	void setFile( FileDom file )
	{
		m_file = file;
	}

	void init()
	{
		m_currentScope.clear();
		m_currentAccess = CodeModelItem::Public;
		m_anon = 0;

        	initializeASTFactory (ast_factory);
        	setASTFactory (&ast_factory);
	}
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

typeDefinition { QStringList bases; QString className; ClassDom klass; QStringList m;}
	:	#(CLASS_DEF m=modifiers IDENT
						{
						klass = m_model->create<ClassModel>();
						QString name = QString::fromUtf8( #IDENT->getText().c_str(), #IDENT->getText().length() );
						QStringList path = QStringList::split( ".", name );
						className = path.back();

						klass->setName( path.back() );
						klass->setScope( m_currentScope );
						klass->setStartPosition( #IDENT->getLine(), #IDENT->getColumn() );
						/// @todo klass->setEndPositon()

						klass->setFileName( m_file->name() );
						if( m_currentClass.top() )
						    m_currentClass.top()->addClass( klass );
						else
						    m_file->addClass( klass );
						}
		bases=extendsClause
						{
						for( QStringList::Iterator it = bases.begin(); it != bases.end(); ++it )
							klass->addBaseClass( *it );
						}
		implementsClause
						{
						m_currentClass.push( klass );
						m_currentScope.push_back( className );
						}
		objBlock[klass]
						{
						m_currentClass.pop();
						m_currentScope.pop_back();
						}
		)
	|	#(INTERFACE_DEF m=modifiers IDENT
						{
						klass = m_model->create<ClassModel>();
						QString name = QString::fromUtf8( #IDENT->getText().c_str(), #IDENT->getText().length() );
						QStringList path = QStringList::split( ".", name );
						className = path.back();

						klass->setName( path.back() );
						klass->setScope( m_currentScope );
						klass->setStartPosition( #IDENT->getLine(), #IDENT->getColumn() );
						/// @todo klass->setEndPositon()

						klass->setFileName( m_file->name() );

						if( m_currentClass.top() )
						    m_currentClass.top()->addClass( klass );
						else
						    m_file->addClass( klass );
						}
		bases=extendsClause
						{
						m_currentClass.push( klass );
						m_currentScope.push_back( className );
						}
		interfaceBlock[klass]
						{
						m_currentClass.pop();
						m_currentScope.pop_back();
						}
		)
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

interfaceBlock [ ClassDom klass ] { FunctionDom meth; VariableDom attr; }
	:	#(	OBJBLOCK
			(	meth=methodDecl			{
								}

			|	attr=variableDef		{
								}
			)*
		)
	;

objBlock [ ClassDom klass ] { FunctionDom meth; VariableDom attr; }
	:	#(	OBJBLOCK
			(	meth=ctorDef			{
								klass->addFunction( meth );
								}
			|	meth=methodDef			{
								klass->addFunction( meth );
								}
			|	attr=variableDef		{
								klass->addVariable( attr );
								}
			|	typeDefinition
			|	#(STATIC_INIT slist)
			|	#(INSTANCE_INIT slist)
			)*
		)
	;

ctorDef returns [ FunctionDom meth ]				{
								QStringList m;
								meth = m_model->create<FunctionModel>();
								meth->setFileName( m_file->name() );
								}
	:	#(CTOR_DEF
		m=modifiers methodHead[meth] slist
		)
								{
								if( m.contains("public") )
								    meth->setAccess( CodeModelItem::Public );
								else if( m.contains("protected") )
								    meth->setAccess( CodeModelItem::Protected );
								else
								    meth->setAccess( CodeModelItem::Private );
								}
	;

methodDecl returns [ FunctionDom meth ]  			{
								QStringList m;
								QString tp;
								meth = m_model->create<FunctionModel>();
								meth->setFileName( m_file->name() );
								}
	:	#(METHOD_DEF m=modifiers tp=typeSpec methodHead[meth])
								{
								meth->setResultType( tp );
								if( m.contains("public") )
								    meth->setAccess( CodeModelItem::Public );
								else if( m.contains("protected") )
								    meth->setAccess( CodeModelItem::Protected );
								else
								    meth->setAccess( CodeModelItem::Private );
								}
	;

methodDef returns [ FunctionDom meth ]  			{
								QStringList m;
								QString tp;
								meth = m_model->create<FunctionModel>();
								meth->setFileName( m_file->name() );
								}
	:	#(METHOD_DEF m=modifiers tp=typeSpec methodHead[meth] (slist)?)
								{
								meth->setResultType( tp );
								if( m.contains("public") )
								    meth->setAccess( CodeModelItem::Public );
								else if( m.contains("protected") )
								    meth->setAccess( CodeModelItem::Protected );
								else
								    meth->setAccess( CodeModelItem::Private );
								}
	;

variableDef returns [ VariableDom attr ] 			{
								QStringList m;
								QString tp;
								attr = m_model->create<VariableModel>();
								attr->setFileName( m_file->name() );
								}
	:	#(VARIABLE_DEF m=modifiers tp=typeSpec variableDeclarator[attr] varInitializer)
								{
								attr->setType( tp );
								if( m.contains("public") )
								    attr->setAccess( CodeModelItem::Public );
								else if( m.contains("protected") )
								    attr->setAccess( CodeModelItem::Protected );
								else
								    attr->setAccess( CodeModelItem::Private );

								attr->setStatic( m.contains("static") );
								}
	;

parameterDef returns [ ArgumentDom arg ] 			{
								QString tp;
								arg = m_model->create<ArgumentModel>();
								}
	:	#(PARAMETER_DEF modifiers tp=typeSpec IDENT )
								{
								arg->setType( tp );
								arg->setName( #IDENT->getText().c_str() );
								}
	;

objectinitializer
	:	#(INSTANCE_INIT slist)
	;

variableDeclarator [ VariableDom attr ]
	:	IDENT						{
								attr->setName( #IDENT->getText().c_str() );
								attr->setStartPosition( #IDENT->getLine(), #IDENT->getColumn() );
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

methodHead [ FunctionDom meth ] { ArgumentDom arg; }
	:	IDENT #( PARAMETERS (arg=parameterDef { meth->addArgument(arg); } )* ) (throwsClause)?
	{
	meth->setName( #IDENT->getText().c_str() );
	meth->setScope( m_currentScope );
	meth->setStartPosition( #IDENT->getLine(), #IDENT->getColumn() );
	}
	;

throwsClause
	:	#( "throws" (identifier)* )
	;

identifier returns [ QString id ]
	:	IDENT 						{
								id = #IDENT->getText().c_str();
								}
	|	#( DOT id=identifier IDENT )			{
								id += QString(".") + #IDENT->getText().c_str();
								}
	;

identifierStar returns [ QString id ]
	:	IDENT						{
								id = #IDENT->getText().c_str();
								}
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
