/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   raggi@cli.di.unipi.it                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// c++ support
#include "parser.h"
#include "driver.h"
#include "lexer.h"
#include "errors.h"
#include "problemreporter.h"

// class store
#include "classstore.h"

// qt
#include <qstring.h>
#include <qstringlist.h>
#include <qasciidict.h>
#include <qdom.h>

// kde
#include <klocale.h>
#include <kdebug.h>

using namespace std;

#define ADVANCE(tk, descr) \
{ \
  Token token = lex->lookAhead( 0 ); \
		if( token != tk ){ \
				   reportError( i18n("'%1' expected found '%2'").arg(descr).arg(lex->lookAhead(0).toString()) ); \
				   return false; \
			       } \
lex->nextToken(); \
}

#define MATCH(tk, descr) \
{ \
  Token token = lex->lookAhead( 0 ); \
		if( token != tk ){ \
				   reportError( Errors::SyntaxError ); \
				   return false; \
			       } \
}

struct ParserPrivateData
{
    QDomDocument* dom;

    ParserPrivateData()
        : dom( 0 )
        {}
};


Parser::Parser( ProblemReporter* pr, Driver* drv, Lexer* lexer )
    : m_problemReporter( pr ),
      driver( drv ),
      lex( lexer )
{
    d = new ParserPrivateData();
    d->dom = new QDomDocument();
    m_fileName = "<stdin>";
    
    m_maxErrors = 5;
    resetErrors();
}

Parser::~Parser()
{
    delete( d->dom );
    delete( d );
    d = 0;
}

void Parser::setFileName( const QString& fileName )
{
    m_fileName = fileName;
}

bool Parser::reportError( const Error& err )
{
    //kdDebug(9007) << "Parser::reportError()" << endl;
    if( m_errors < m_maxErrors ){
	int line=0, col=0;
	const Token& token = lex->lookAhead( 0 );
	lex->getTokenPosition( token, &line, &col );
	
	QString s = lex->lookAhead( 0 ).toString();
	s = s.left( 30 ).stripWhiteSpace();
	if( s.isEmpty() )
	    s = i18n( "<eof>" );
	
	m_problemReporter->reportError( err.text.arg(s),
					m_fileName,
					line,
					col );
    }
    
    ++m_errors;
    
    return true;
}

bool Parser::reportError( const QString& msg )
{
    //kdDebug(9007) << "Parser::reportError()" << endl;
    if( m_errors < m_maxErrors ){
	int line=0, col=0;
	const Token& token = lex->lookAhead( 0 );
	lex->getTokenPosition( token, &line, &col );
	
	m_problemReporter->reportError( msg,
					m_fileName,
					line,
					col );
    }
    
    ++m_errors;
    
    return true;
}

void Parser::syntaxError()
{
    (void) reportError( Errors::SyntaxError );
}

void Parser::parseError()
{
    (void) reportError( Errors::ParseError );
}

bool Parser::skipUntil( int token )
{
    //kdDebug(9007) << "Parser::skipUntil()" << endl;
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == token )
	    return true;
	
	lex->nextToken();
    }
    
    return false;
}

bool Parser::skipUntilDeclaration()
{
    //kdDebug(9007) << "Parser::skipUntilDeclaration()" << endl;
    
    lex->nextToken();
    while( !lex->lookAhead(0).isNull() ){
	switch( lex->lookAhead(0) ){
	case ';':
	case '~':
	case Token_scope:
	case Token_identifier:
	case Token_operator:
	case Token_char:
	case Token_wchar_t:
	case Token_bool:
	case Token_short:
	case Token_int:
	case Token_long:
	case Token_signed:
	case Token_unsigned:
	case Token_float:
	case Token_double:
	case Token_void:
	case Token_extern:
	case Token_namespace:
	case Token_using:
	case Token_typedef:
	case Token_asm:
	case Token_template:
	case Token_export:

	case Token_const:       // cv
	case Token_volatile:    // cv
	    
	case Token_public:
	case Token_protected:
	case Token_private:
	case Token_signals:      // Qt
	case Token_slots:        // Qt
  	    return true;
	    
	default:
	    lex->nextToken();
	}
    }
    
    return false;
}

bool Parser::skipUntilStatement()
{
    //kdDebug(9007) << "Parser::skipUntilStatement() -- token = " << lex->lookAhead(0).toString() << endl;
    
    while( !lex->lookAhead(0).isNull() ){
	switch( lex->lookAhead(0) ){
		case ';':
		case '{':
		case '}':
		case Token_const:
		case Token_volatile:
		case Token_identifier:
		case Token_case:
		case Token_default:
		case Token_if:
		case Token_switch:
		case Token_while:
		case Token_do:
		case Token_for:
		case Token_break:
		case Token_continue:
		case Token_return:
		case Token_goto:
		case Token_try:
		case Token_catch:
		case Token_throw:
		case Token_char:
		case Token_wchar_t:
		case Token_bool:
		case Token_short:
		case Token_int:
		case Token_long:
		case Token_signed:
		case Token_unsigned:
		case Token_float:
		case Token_double:
		case Token_void:
		case Token_class:
		case Token_struct:
		case Token_union:
		case Token_enum:
		case Token_scope:
		case Token_template:
		case Token_using:
		    return true;
	    
	    default:
  	        lex->nextToken();
	}
    }
    
    return false;
}

bool Parser::skip( int l, int r )
{
    int count = 0;
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	if( tk == l )
	    ++count;
	else if( tk == r )
	    --count;
	
	if( count == 0 )
	    return true;
	
	lex->nextToken();
    }
    
    return false;
}

bool Parser::parseName( QDomElement& name )
{
    //kdDebug(9007) << "Parser::parseName()" << endl;
    
    if( lex->lookAhead(0) == Token_scope ){
	lex->nextToken();
    }

#warning "ignore namespace for now!!"    
    parseNestedNameSpecifier();
    return parseUnqualiedName( name );
}

bool Parser::parseTranslationUnit( ClassStore* store )
{
    //kdDebug(9007) << "Parser::parseTranslationUnit()" << endl;

    m_store = store;

    m_scopeStack.push( m_store->globalScope() );
            
    while( !lex->lookAhead(0).isNull() ){
        QDomElement def;
	if( !parseDefinition(def) ){
	    // error recovery
	    skipUntilDeclaration();
	}
    }
    
    return m_errors == 0;
}

bool Parser::parseDefinition( QDomElement& def )
{
    //kdDebug(9007) << "Parser::parseDefinition()" << endl;
    switch( lex->lookAhead(0) ){
	
    case ';':  
	lex->nextToken();
	return true;
	
    case Token_extern: 
	return parseLinkageSpecification();
	
    case Token_namespace: 
	return parseNamespace();
	
    case Token_using: 
	return parseUsing();
	
    case Token_typedef: 
	return parseTypedef();
	
    case Token_asm: 
	return parseAsmDefinition();
	
    case Token_template: 
    case Token_export:
	return parseTemplateDeclaration();
	
    default:
	if( parseEnumSpecifier(def) || parseClassSpecifier(def) ){
	    parseInitDeclaratorList();
	    ADVANCE( ';', ";" );
	    return true;
	}
	
	return parseDeclaration();
	
    } // end switch
}

bool Parser::parseLinkageSpecification()
{
    //kdDebug(9007) << "Parser::parseLinkageSpecification()" << endl;
    
    if( lex->lookAhead(0) != Token_extern ){
	return false;
    }
    lex->nextToken();
    
    QString type;
    if( lex->lookAhead(0) == Token_string_literal ){
	type = lex->lookAhead( 0 ).toString();
	lex->nextToken();
    }
    
    if( lex->lookAhead(0) == '{' ){
	parseLinkageBody();	
    } else {
        QDomElement def;
	if( !parseDefinition(def) ){
	    reportError( i18n("Declaration syntax error") );
	}
    }
    
    return true;
}

bool Parser::parseLinkageBody()
{
    //kdDebug(9007) << "Parser::parseLinkageBody()" << endl;
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
        
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	if( tk == '}' )
	    break;
	
	QDomElement def;
	if( !parseDefinition(def) ){
	    // error recovery
	    skipUntilDeclaration();
	}
    }
    
    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} expected") );
    } else
	lex->nextToken();
    
    return true;
}

bool Parser::parseNamespace()
{
    //kdDebug(9007) << "Parser::parseNamespace()" << endl;
    if( lex->lookAhead(0) != Token_namespace ){
	return false;
    }
    lex->nextToken();    
    
    QString namespaceName;
    if( lex->lookAhead(0) == Token_identifier ){
	namespaceName = lex->lookAhead( 0 ).toString();
	lex->nextToken();
    }
        
    if ( lex->lookAhead(0) == '=' ) {
	// namespace alias
	lex->nextToken();
	
	QDomElement name;
	if( parseName(name) ){	    
	    ADVANCE( ';', ";" );	    
	    return true;
	} else {
	    reportError( i18n("namespace expected") );
	    return false;
	}
    } else if( lex->lookAhead(0) != '{' ){
	reportError( i18n("{ expected") );
	return false;
    }

    ParsedScopeContainer* scope = new ParsedScopeContainer();
    scope->setName( namespaceName );
    m_scopeStack.top()->addScope( scope );
    m_scopeStack.push( scope );
    parseLinkageBody();
    m_scopeStack.pop();
    
    return true;
}

bool Parser::parseUsing()
{
    //kdDebug(9007) << "Parser::parseUsing()" << endl;
    
    if( lex->lookAhead(0) != Token_using ){
	return false;
    }
    lex->nextToken();
    
    if( lex->lookAhead(0) == Token_namespace ){
	return parseUsingDirective();
    }
    
    if( lex->lookAhead(0) == Token_typename )
	lex->nextToken();
    
    QDomElement name;
    if( !parseName(name) )
	return false;
        
    ADVANCE( ';', ";" );
      
    return true;
}

bool Parser::parseUsingDirective()
{
    //kdDebug(9007) << "Parser::parseUsingDirective()" << endl;
    
    if( lex->lookAhead(0) != Token_namespace ){
	return false;
    }
    lex->nextToken();
    
    QDomElement name;
    if( !parseName(name) ){
	reportError( i18n("Namespace name expected") );
	return false;
    }
    
    ADVANCE( ';', ";" );
        
    return true;
}


bool Parser::parseOperatorFunctionId( QDomElement& e )
{
    //kdDebug(9007) << "Parser::parseOperatorFunctionId()" << endl;

    e = d->dom->createElement( "operator-function-id" );

    if( lex->lookAhead(0) != Token_operator ){
	return false;
    }
    lex->nextToken();    
    
    if( parseOperator(e) )
	return true;
    else {
	// parse cast operator
        parseCvQualify();       
	
	QDomElement spec;
	if( !parseSimpleTypeSpecifier(spec) ){
	    parseError();
	}
	
	parseCvQualify();
	
	while( parsePtrOperator() )
  	    ;
	
	return true;
    }
}

bool Parser::parseTemplateArgumentList()
{
    //kdDebug(9007) << "Parser::parseTemplateArgumentList()" << endl;
    
    if( !parseTemplateArgument() )
	return false;
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseTemplateArgument() ){
	    parseError();
	    break;
	}
    }
    
    return true;
}

bool Parser::parseTypedef()
{
    //kdDebug(9007) << "Parser::parseTypedef()" << endl;
    
    if( lex->lookAhead(0) != Token_typedef ){
	return false;
    }
    lex->nextToken();
    
    //kdDebug(9007) << "token = " << lex->lookAhead(0).toString() << endl;
    QDomElement spec;
    if( !parseTypeSpecifierOrClassSpec(spec) ){
	reportError( i18n("Need a type specifier to declare") );
	return false;
    }
    
    //kdDebug(9007) << "token = " << lex->lookAhead(0).toString() << endl;
    if( !parseInitDeclaratorList() ){
	reportError( i18n("Need an identifier to declare") );
	return false;
    }
    
    ADVANCE( ';', ";" );
        
    return true;
}

bool Parser::parseAsmDefinition()
{
    //kdDebug(9007) << "Parser::parseAsmDefinition()" << endl;
    
    ADVANCE( Token_asm, "asm" );
    ADVANCE( '(', '(' );
    
    parseStringLiteral();
    
    ADVANCE( ')', ')' );
    ADVANCE( ';', ';' );
    
    return true;
}

bool Parser::parseTemplateDeclaration()
{
    //kdDebug(9007) << "Parser::parseTemplateDeclaration()" << endl;
    
    bool _export = false;
    if( lex->lookAhead(0) == Token_export ){
	_export = true;
	lex->nextToken();
    }
    
    if( lex->lookAhead(0) != Token_template ){
	if( _export ){
	    ADVANCE( Token_template, "template" );
	} else
	    return false;
    }
    
    ADVANCE( Token_template, "template" );
    
    if( lex->lookAhead(0) == '<' ){
	lex->nextToken();
	parseTemplateParameterList();
	
	ADVANCE( '>', ">" );
    }
    
    QDomElement def;
    if( !parseDefinition(def) ){
	reportError( i18n("expected a declaration") );
    }
        
    return true;
}

bool Parser::parseOperator( QDomElement& e )
{
    //kdDebug(9007) << "Parser::parseOperator()" << endl;
    QString text = lex->lookAhead( 0 ).toString();
    
    switch( lex->lookAhead(0) ){
    case Token_new:
    case Token_delete:
	lex->nextToken();
	if( lex->lookAhead(0) == '[' && lex->lookAhead(1) == ']' ){
	    lex->nextToken();
	    lex->nextToken();
	    text += "[]";
	}
	e.setAttribute( "id", text );
	return true;
	
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case '&':
    case '|':
    case '~':
    case '!':
    case '=':
    case '<':
    case '>':
    case ',':
    case Token_assign:
    case Token_shift:
    case Token_eq:
    case Token_not_eq:
    case Token_leq:
    case Token_geq:
    case Token_and:
    case Token_or:
    case Token_incr:
    case Token_decr:
    case Token_ptrmem:
    case Token_arrow:
	e.setAttribute( "id", text );
	lex->nextToken();
	return true;
	
    default:
	if( lex->lookAhead(0) == '(' && lex->lookAhead(1) == ')' ){
	    lex->nextToken();
	    lex->nextToken();
	    e.setAttribute( "id", "()" );
	    return true;
	} else if( lex->lookAhead(0) == '[' && lex->lookAhead(1) == ']' ){
	    lex->nextToken();
	    lex->nextToken();
	    e.setAttribute( "id", "[]" );
	    return true;
	}
    }
    
    return false;
}

bool Parser::parseCvQualify()
{
    //kdDebug(9007) << "Parser::parseCvQualify()" << endl;
    
    int n = 0;
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	if( tk == Token_const || tk == Token_volatile ){
	    ++n;
	    lex->nextToken();
	} else
	    break;
    }
    return n != 0;
}

bool Parser::parseSimpleTypeSpecifier( QDomElement& spec )
{
    //kdDebug(9007) << "Parser::parseSimpleTypeSpecifier()" << endl;
    
    bool isIntegral = false;
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	if( tk == Token_char    || tk == Token_wchar_t  || 
	    tk == Token_bool    || tk == Token_short    ||
	    tk == Token_int     || tk == Token_long     || 
	    tk == Token_signed  || tk == Token_unsigned ||
	    tk == Token_float   || tk == Token_double   || 
	    tk == Token_void ){  
	    lex->nextToken();
	    isIntegral = true;
	} else if( isIntegral ){
	    return true;
	} else
	    break;
    }

    //kdDebug(9007) << "!! token = " << lex->lookAhead(0).toString() << endl;
    QDomElement name;
    return parseName( name );
}

bool Parser::parsePtrOperator()
{
    //kdDebug(9007) << "Parser::parsePtrOperator()" << endl;
    
    if( lex->lookAhead(0) == '&' ){
	lex->nextToken();
    } else if( lex->lookAhead(0) == '*' ){
	lex->nextToken();
    } else {
	int index = lex->index();
	if( !parsePtrToMember() ){
	    lex->setIndex( index );
	    return false;
	}
    }
    
    parseCvQualify();
    
    return true;
}


bool Parser::parseTemplateArgument()
{
    //kdDebug(9007) << "Parser::parseTemplateArgument()" << endl;
    
#warning "TODO Parser::parseTemplateArgument()"
#warning "parse type id"
    
#if 0
    if( parseTypeId() ){
	qWarning( "token = %s", lex->lookAhead(0).toString().latin1() );
	return true;
    }
#endif
    
    return parseAssignmentExpression();
}

bool Parser::parseTypeSpecifier( QDomElement& spec )
{
    //kdDebug(9007) << "Parser::parseTypeSpecifier()" << endl;
    
    parseCvQualify();
    
    if( parseElaboratedTypeSpecifier(spec) || parseSimpleTypeSpecifier(spec) ){
	parseCvQualify();
    } else
	return false;
    
    return true;
}

bool Parser::parseDeclarator( QDomElement& e )
{
    //kdDebug(9007) << "Parser::parseDeclarator()" << endl;

    e = d->dom->createElement( "declarator" );

    QDomElement ptrOp = d->dom->createElement( "ptr-op" );
    while( parsePtrOperator() ){
        // TODO: add ptr operator do current declarator
    }
    e.appendChild( ptrOp );
    
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();

	QDomElement subDeclarator;
	if( !parseDeclarator(subDeclarator) ){
	    return false;
	}	
	if( lex->lookAhead(0) != ')'){
	    return false;
	}
	lex->nextToken();
	e.appendChild( subDeclarator );	
    } else {
	
        QDomElement name;
	if( !parseDeclaratorId(name) ){
	    return false;
	}	
	e.appendChild( name );
	
	if( lex->lookAhead(0) == ':' ){
	    lex->nextToken();
	    if( !parseConstantExpression() ){
		reportError( i18n("Constant expression expected") );
		return true;
	    }
	    return true;
	}
    }
    
    while( lex->lookAhead(0) == '[' ){
	lex->nextToken();
	parseCommaExpression();
	
	ADVANCE( ']', "]" );
	// TODO: add vector
    }
    
    int index = lex->index();
    if( lex->lookAhead(0) == '(' ){     
	lex->nextToken();
	
	if( !parseParameterDeclarationClause() ){
	    lex->setIndex( index );
	    return true;
	}
	
	if( lex->lookAhead(0) != ')' ){
	    lex->setIndex( index );
	    return true;
	} else
	    lex->nextToken();

	e.setAttribute( "function", true );
	
	parseCvQualify();	
	parseExceptionSpecification();
    }
    
    return true;
}

bool Parser::parseEnumSpecifier( QDomElement& def )
{
    //kdDebug(9007) << "Parser::parseEnumSpecifier()" << endl;
    
    int index = lex->index();
    
    while( parseStorageClassSpecifier() )
	;

    parseCvQualify();
    
    if( lex->lookAhead(0) != Token_enum ){
	return false;
    }

    def = d->dom->createElement( "enum" );

    lex->nextToken();
    
    if( lex->lookAhead(0) == Token_identifier ){
        def.setAttribute( "id", lex->lookAhead(0).toString() );
	lex->nextToken();
    }
    
    if( lex->lookAhead(0) != '{' ){
	lex->setIndex( index );
	return false;
    }
    lex->nextToken();
        
    parseEnumeratorList();
    
    if( lex->lookAhead(0) != '}' )
	reportError( i18n("} missing") );
    else
	lex->nextToken();
    
    return true;
}

bool Parser::parseTemplateParameterList()
{
    //kdDebug(9007) << "Parser::parseTemplateParameterList()" << endl;
    
    if( !parseTemplateParameter() ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseTemplateParameter() ){
	    parseError();
	    break;
	}
    }
    
    return true;
}

bool Parser::parseTemplateParameter()
{
    //kdDebug(9007) << "Parser::parseTemplateParameter()" << endl;
    
    int tk = lex->lookAhead( 0 );
    if( tk == Token_class ||
	tk == Token_typename ||
	tk == Token_template )
	return parseTypeParameter();
    else
	return parseParameterDeclaration();
}

bool Parser::parseTypeParameter()
{
    //kdDebug(9007) << "Parser::parseTypeParameter()" << endl;
    
    switch( lex->lookAhead(0) ){
	
    case Token_class:
	{
	    
	    lex->nextToken(); // skip class
	    
	    // parse optional name
	    QString name;
	    if( lex->lookAhead(0) == Token_identifier ){
		name = lex->lookAhead( 0 ).toString();
		lex->nextToken();
	    }
	    if( name )
		
		if( lex->lookAhead(0) == '=' ){
		    lex->nextToken();
		    
		    if( !parseTypeId() ){
			parseError();
		    }
		}
	}
	return true;
	
    case Token_typename:
	{
	    
	    lex->nextToken(); // skip typename
	    
	    // parse optional name
	    QString name;
	    if( lex->lookAhead(0) == Token_identifier ){
		name = lex->lookAhead( 0 ).toString();
		lex->nextToken();
	    }
	    if( name )
		
		if( lex->lookAhead(0) == '=' ){
		    lex->nextToken();
		    
		    if( !parseTypeId() ){
			parseError();
		    }
		}
	}
	return true;
	
    case Token_template:
	{
	    
	    lex->nextToken(); // skip template
	    ADVANCE( '<', '<' );
	    if( !parseTemplateParameterList() ){
		return false;
	    }
	    
	    ADVANCE( '>', '>' );
	    ADVANCE( Token_class, "class" );
	    
	    // parse optional name
	    QString name;
	    if( lex->lookAhead(0) == Token_identifier ){
		name = lex->lookAhead( 0 ).toString();
		lex->nextToken();
	    }
	    
	    if( lex->lookAhead(0) == '=' ){
		lex->nextToken();
		
		QString templ_name = lex->lookAhead( 0 ).toString();
		ADVANCE( Token_identifier, "template name" );
	    }
	}
	return true;
	
    } // end switch
    
    
    return false;
}

bool Parser::parseStorageClassSpecifier()
{
    //kdDebug(9007) << "Parser::parseStorageClassSpecifier()" << endl;
    
    switch( lex->lookAhead(0) ){
    case Token_friend:
    case Token_auto:
    case Token_register:
    case Token_static:
    case Token_extern:
    case Token_mutable:
	lex->nextToken();
	return true;
    }
    
    return false;
}

bool Parser::parseFunctionSpecifier()
{
    //kdDebug(9007) << "Parser::parseFunctionSpecifier()" << endl;
    
    switch( lex->lookAhead(0) ){
    case Token_inline:
    case Token_virtual:
    case Token_explicit:
	lex->nextToken();
	return true;
    }
    
    return false;
}

bool Parser::parseTypeId()
{
    //kdDebug(9007) << "Parser::parseTypeId()" << endl;
    
    QDomElement spec;
    if( !parseTypeSpecifier(spec) ){
	return false;
    }
    
    parseAbstractDeclarator();
    return true;
}

bool Parser::parseAbstractDeclarator()
{
    //kdDebug(9007) << "Parser::parseAbstractDeclarator()" << endl;
    while( parsePtrOperator() ){
    }
    
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();
	
	QDomElement declarator;
	if( !parseDeclarator(declarator) ){
	    return false;	    
	}
	
	if( lex->lookAhead(0) != ')'){
	    return false;
	}
	lex->nextToken();
	
    }
    
    while( lex->lookAhead(0) == '[' ){
	lex->nextToken();
	parseCommaExpression();
	
	ADVANCE( ']', "]" );
    }
    
    int index = lex->index();
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();
	
	if( !parseParameterDeclarationClause() ){
	    lex->setIndex( index );
	    return true;
	}
	
	ADVANCE( ')', ")" );	
	parseCvQualify();	
	parseExceptionSpecification();
    }
    
    return true;
}

bool Parser::parseConstantExpression()
{
    //kdDebug(9007) << "Parser::parseConstantExpression()" << endl;
    
    QStringList l;
    
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	if( tk == '(' ){
	    l << "(...)";
	    if( !skip('(', ')') ){
		return false;
	    }
	    lex->nextToken();
	} else if( tk == ',' || tk == ';' || tk == '<' ||
		   tk == Token_assign || tk == ']' ||
		   tk == ')' || tk == '}' || tk == ':' ){
	    break;
	} else {
	    l << lex->lookAhead( 0 ).toString();
	    lex->nextToken();
	}
    }
    
    return !l.isEmpty();
}


bool Parser::parseInitDeclaratorList()
{
    //kdDebug(9007) << "Parser::parseInitDeclaratorList()" << endl;
    
    QDomElement declarator;
    if( !parseInitDeclarator(declarator) ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseInitDeclarator(declarator) ){
	    parseError();
	    break;
	}
    }
    //kdDebug(9007) << "Parser::parseInitDeclaratorList() -- end" << endl;
    return true;
}

bool Parser::parseParameterDeclarationClause()
{
    //kdDebug(9007) << "Parser::parseParameterDeclarationClause()" << endl;
    
    if( !parseParameterDeclarationList() ){
	
	if ( lex->lookAhead(0) == ')' )
	    return true;
	
	if( lex->lookAhead(0) == Token_ellipsis ){
	    lex->nextToken();
	    return true;
	}
	return false;
    }
    
    if( lex->lookAhead(0) == ',' ){
	lex->nextToken();
    }
    
    if( lex->lookAhead(0) == Token_ellipsis ){
	lex->nextToken();
    }
    
    return true;
}

bool Parser::parseParameterDeclarationList()
{
    //kdDebug(9007) << "Parser::parseParameterDeclarationList()" << endl;
    
    int index = lex->index();
    if( !parseParameterDeclaration() ){
	lex->setIndex( index );
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseParameterDeclaration() ){
	    lex->setIndex( index );
	    return false;
	}
    }
    return true;
}

bool Parser::parseParameterDeclaration()
{
    //kdDebug(9007) << "Parser::parseParameterDeclaration()" << endl;
    
    int index = lex->index();
    
    // parse decl spec
    QDomElement spec;
    if( !parseTypeSpecifier(spec) ){
	lex->setIndex( index );
	return false;
    }
    
    index = lex->index();
    
    QDomElement declarator;
    if( !parseDeclarator(declarator) ){
	lex->setIndex( index );
	parseAbstractDeclarator();
    }
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	if( !parseAssignmentExpression() ){
	    //reportError( i18n("Expression expected") );
	}
    }
    
    return true;
}

bool Parser::parseClassSpecifier( QDomElement& def )
{
    //kdDebug(9007) << "Parser::parseClassSpecifier()" << endl;
    
    int index = lex->index();
    
    while( parseStorageClassSpecifier() )
	;   

    parseCvQualify();
    
    int kind = lex->lookAhead( 0 );
    if( kind == Token_class || kind == Token_struct || kind == Token_union ){
        def = d->dom->createElement( lex->lookAhead(0).toString() );
	lex->nextToken();
    } else {
	return false;
    }

    QDomElement name;
    parseUnqualiedName( name );
    def.setAttribute( "id", name.attribute("id") );

    QDomElement parents;
    if( lex->lookAhead(0) == ':' ){      
	if( !parseBaseClause(parents) ){
	    skipUntil( '{' );
	}
    }
    
    if( lex->lookAhead(0) != '{' ){
	lex->setIndex( index );
	return false;
    }
    
    ADVANCE( '{', '{' );

    if( def.hasAttribute("id") ){

      if( def.tagName() == "class" ){
          ParsedClass* pClass = new ParsedClass();
          pClass->setName( def.attribute("id") );

	  QDomElement e = parents.firstChild().toElement();
	  while( !e.isNull() ){
  	      ParsedParent* parent = new ParsedParent();

	      QDomElement parentName = e.firstChild().toElement();
	      parent->setName( parentName.attribute("id") );

	      QString access = e.attribute( "access" );
	      if( access == "public" )
  		  parent->setAccess( PIE_PUBLIC );
	      else if( access == "protected" )
		  parent->setAccess( PIE_PROTECTED );
	      else if( access == "private" )
		  parent->setAccess( PIE_PRIVATE );

	      pClass->addParent( parent );

	      parent->out();
		  
	      e = e.nextSibling().toElement();
	  }

	  pClass->setDeclaredInScope( m_scopeStack.top()->path() );
	  m_store->addClass( pClass );	
	  m_scopeStack.top()->addClass( pClass );	  
      } else if( def.tagName() == "struct" || def.tagName() == "union" ){
  	  ParsedStruct* pStruct = new ParsedStruct();
	  pStruct->setName( def.attribute("id") );
	  pStruct->setDeclaredInScope( m_scopeStack.top()->path() );
	  m_store->addStruct( pStruct );
	  m_scopeStack.top()->addStruct( pStruct );
      }
    }
            
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;
	
	if( !parseMemberSpecification() ){
	    skipUntilDeclaration();
	}
    }
    
    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} missing") );
    } else
	lex->nextToken();
    
    return true;
}

bool Parser::parseAccessSpecifier( QString& access )
{
    //kdDebug(9007) << "Parser::parseAccessSpecifier()" << endl;
    
    switch( lex->lookAhead(0) ){
    case Token_public:
    case Token_protected:
    case Token_private:
        access = lex->lookAhead( 0 ).toString();
	lex->nextToken();
	return true;
    }
    
    return false;
}

bool Parser::parseMemberSpecification()
{
    //kdDebug(9007) << "Parser::parseMemberSpecification()" << endl;

    QString access;
    
    if( lex->lookAhead(0) == ';' ){
	lex->nextToken();
	return true;
    } else if( lex->lookAhead(0) == Token_Q_OBJECT || lex->lookAhead(0) == Token_K_DCOP ){
	lex->nextToken();
	return true;
    } else if( lex->lookAhead(0) == Token_signals || lex->lookAhead(0) == Token_k_dcop || lex->lookAhead(0) == Token_k_dcop_signals ){
	lex->nextToken();
	ADVANCE( ':', ":" );
	return true;
    } else if( parseTypedef() ){
	return true;
    } else if( parseUsing() ){
	return true;
    } else if( parseTemplateDeclaration() ){
	return true;
    } else if( parseAccessSpecifier(access) ){
	if( lex->lookAhead(0) == Token_slots ){
	    lex->nextToken();
	}
	ADVANCE( ':', ":" );
	return true;
    }
    
    QDomElement def;
    if( parseEnumSpecifier(def) || parseClassSpecifier(def) ){
	parseInitDeclaratorList();
	ADVANCE( ';', ";" );
	return true;
    }
    return parseDeclaration();
}

bool Parser::parseCtorInitializer()
{
    //kdDebug(9007) << "Parser::parseCtorInitializer()" << endl;
    
    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    lex->nextToken();
    
    if( !parseMemInitializerList() ){
	reportError( i18n("Member initializers expected") );
    }
    
    return true;
}

bool Parser::parseElaboratedTypeSpecifier( QDomElement& spec )
{
    //kdDebug(9007) << "Parser::parseElaboratedTypeSpecifier()" << endl;
    
    int index = lex->index();
    
    int tk = lex->lookAhead( 0 );
    if( tk == Token_class  ||
	tk == Token_struct ||
	tk == Token_union  ||
	tk == Token_enum   ||
	tk == Token_typename )
    {
	lex->nextToken();
	
	QDomElement name;
	if( parseName(name) ){
	    return true;
	}
    }
    
    lex->setIndex( index );
    return false;
}

bool Parser::parseDeclaratorId( QDomElement& name )
{
    //kdDebug(9007) << "Parser::parseDeclaratorId()" << endl;
    return parseName( name );
}

bool Parser::parseExceptionSpecification()
{
    //kdDebug(9007) << "Parser::parseExceptionSpecification()" << endl;
    
    if( lex->lookAhead(0) != Token_throw ){
	return false;
    }
    lex->nextToken();
    
    
    ADVANCE( '(', "(" );
    parseTypeIdList();
    ADVANCE( ')', ")" );
    
    return true;
}

bool Parser::parseEnumeratorList()
{
    //kdDebug(9007) << "Parser::parseEnumeratorList()" << endl;
    
    if( !parseEnumerator() ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseEnumerator() ){
	    //reportError( i18n("Enumerator expected") );
	    break;
	}
    }
    
    return true;
}

bool Parser::parseEnumerator()
{
    //kdDebug(9007) << "Parser::parseEnumerator()" << endl;
    
    if( lex->lookAhead(0) != Token_identifier ){
	return false;
    }
    lex->nextToken();
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	
	if( !parseExpression() ){
	    reportError( i18n("Constant expression expected") );
	}
    }
    
    return true;
}

bool Parser::parseInitDeclarator( QDomElement& declarator )
{
    //kdDebug(9007) << "Parser::parseInitDeclarator()" << endl;
    
    if( !parseDeclarator(declarator) ){
	return false;
    }
        
    parseInitializer();
    
    return true;
}

bool Parser::parseAssignmentExpression()
{
    //kdDebug(9007) << "Parser::parseAssignmentExpression()" << endl;
    
#warning "TODO Parser::parseAssignmentExpression()"
    
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	if( tk == '(' ){
	    if( !skip('(', ')') ){
		return false;
	    } else
		lex->nextToken();
	} else if( tk == '<' ){
	    if( !skip('<', '>') ){
		return false;
	    } else
		lex->nextToken();
	} else if( tk == '[' ){
	    if( !skip('[', ']') ){
		return false;
	    } else
		lex->nextToken();
	} else if( tk == ',' || tk == ';' ||
		   tk == '>' || tk == ']' || tk == ')' ||
		   tk == Token_assign ){
	    break;
	} else
	    lex->nextToken();
    }
    
    return true;
}

bool Parser::parseBaseClause( QDomElement& parents )
{
    //kdDebug(9007) << "Parser::parseBaseClause()" << endl;
    
    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    lex->nextToken();
    
    if( !parseBaseSpecifierList(parents) ){
	reportError( i18n("expected base specifier list") );
	return false;
    }
    
    return true;
}

bool Parser::parseInitializer()
{
    //kdDebug(9007) << "Parser::parseInitializer()" << endl;
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	
	if( !parseInitializerClause() ){
	    reportError( i18n("Initializer clause expected") );
	    return false;
	}
    } else if( lex->lookAhead(0) == '(' ){
	lex->nextToken();
	parseCommaExpression();
	
	ADVANCE( ')', ")" );
    }
    
    return false;
}

bool Parser::parseMemInitializerList()
{
    //kdDebug(9007) << "Parser::parseMemInitializerList()" << endl;
    
    if( !parseMemInitializer() ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( parseMemInitializer() ){
	} else {
	    break;
	}
    }
    
    return true;
}

bool Parser::parseMemInitializer()
{
    //kdDebug(9007) << "Parser::parseMemInitializer()" << endl;
    
    if( !parseMemInitializerId() ){
	reportError( i18n("Identifier expected") );
	return false;
    }
    ADVANCE( '(', '(' );
    parseCommaExpression();
    ADVANCE( ')', ')' );
    
    return true;
}

bool Parser::parseTypeIdList()
{
    //kdDebug(9007) << "Parser::parseTypeIdList()" << endl;
    
    if( !parseTypeId() ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	if( parseTypeId() ){
	} else {
	    reportError( i18n("Type id expected") );
	    break;
	}
    }
    
    return true;
}

bool Parser::parseBaseSpecifierList( QDomElement& parents )
{
    //kdDebug(9007) << "Parser::parseBaseSpecifierList()" << endl;
    parents = d->dom->createElement( "parents" );

    QDomElement parent;
    if( !parseBaseSpecifier(parent) ){
	return false;
    }
    parents.appendChild( parent );
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseBaseSpecifier(parent) ){
	    reportError( i18n("Base class specifier expected") );
	    return false;
	}
	parents.appendChild( parent );
    }
    
    return true;
}

bool Parser::parseBaseSpecifier( QDomElement& e )
{
    //kdDebug(9007) << "Parser::parseBaseSpecifier()" << endl;
    QString access;
    e = d->dom->createElement( "base-spec" );
    
    if( lex->lookAhead(0) == Token_virtual ){
        e.setAttribute( "virtual", true );
	lex->nextToken();
	
	parseAccessSpecifier( access );
	e.setAttribute( "access", access );
    } else {	
        parseAccessSpecifier( access );
	e.setAttribute( "access", access );
	
	if( lex->lookAhead(0) == Token_virtual ){
   	    e.setAttribute( "virtual", true );
	    lex->nextToken();
	}
    }
        
    QDomElement name;
    if( !parseName(name) ){
	reportError( i18n("Identifier expected") );
    }
    e.appendChild( name );

    return true;
}


bool Parser::parseInitializerClause()
{
    //kdDebug(9007) << "Parser::parseInitializerClause()" << endl;
    
#warning "TODO Parser::initializer-list"
    
    if( lex->lookAhead(0) == '{' ){
	if( !skip('{','}') ){
	    reportError( i18n("} missing") );
	} else
	    lex->nextToken();
    } else {
	if( !parseAssignmentExpression() ){
	    //reportError( i18n("Expression expected") );
	}
    }
    
    return true;
}

bool Parser::parseMemInitializerId()
{
    //kdDebug(9007) << "Parser::parseMemInitializerId()" << endl;
    
    QDomElement name;
    return parseName( name );
}


bool Parser::parseCommaExpression()
{
    //kdDebug(9007) << "Parser::parseCommaExpression()" << endl;
    
    if( !parseExpression() )
	return false;
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseExpression() ){
	    reportError( i18n("expression expected") );
	    return false;
	}
    }
    return true;
}

bool Parser::parseNestedNameSpecifier()
{
    //kdDebug(9007) << "Parser::parseNestedNameSpecifier()" << endl;
    
    int index = lex->index();
    bool ok = false;
    
    while( lex->lookAhead(0) == Token_identifier ){
	index = lex->index();
	
	if( lex->lookAhead(1) == '<' ){
	    lex->nextToken(); // skip template name
	    lex->nextToken(); // skip <
	    
	    if( !parseTemplateArgumentList() ){
		lex->setIndex( index );
		return false;
	    }
	    
	    if( lex->lookAhead(0) != '>' ){
		lex->setIndex( index );
		return false;
	    } else
		lex->nextToken(); // skip >
	    
	    if ( lex->lookAhead(0) == Token_scope ) {
		lex->nextToken();
		ok = true;
	    } else {
		lex->setIndex( index );
		break;
	    }
	    
	} else if( lex->lookAhead(1) == Token_scope ){
	    lex->nextToken(); // skip name
	    lex->nextToken(); // skip SymbolTable
	    ok = true;
	} else
	    break;
    }
    
    if ( !ok ) {
	lex->setIndex( index );
	return false;
    }
    return true;
}

bool Parser::parsePtrToMember()
{
    //kdDebug(9007) << "Parser::parsePtrToMember()" << endl;
    
    if( lex->lookAhead(0) == Token_scope ){
	lex->nextToken();
    }
    
    while( lex->lookAhead(0) == Token_identifier ){
	lex->nextToken();
	
	if( lex->lookAhead(0) == Token_scope && lex->lookAhead(1) == '*' ){
	    lex->nextToken(); // skip SymbolTable
	    lex->nextToken(); // skip *
	    return true;
	} else
	    break;
    }
    
    return false;
}

bool Parser::parseUnqualiedName( QDomElement& e )
{
    //kdDebug(9007) << "Parser::parseUnqualiedName()" << endl;
    
    bool isDestructor = false;

    e = d->dom->createElement( "unqualified-name" );
    
    if( lex->lookAhead(0) == Token_identifier ){
        e.setAttribute( "id", lex->lookAhead(0).toString() );
	lex->nextToken();
    } else if( lex->lookAhead(0) == '~' && lex->lookAhead(1) == Token_identifier ){
	lex->nextToken(); // skip ~
        e.setAttribute( "id", QString("~") + lex->lookAhead(0).toString() );
	lex->nextToken(); // skip classname
	isDestructor = true;
    } else if( lex->lookAhead(0) == Token_operator ){
	return parseOperatorFunctionId( e );
    } else
	return false;
    
    if( !isDestructor ){
	
	int index = lex->index();
	
	if( lex->lookAhead(0) == '<' ){
	    lex->nextToken();
	    
	    // optional template arguments
	    parseTemplateArgumentList();
	    
	    if( lex->lookAhead(0) != '>' ){
		lex->setIndex( index );
	    } else
		lex->nextToken();

	    e.setAttribute( "template", true );
	}
    }
    
    return true;
}

void Parser::dump()
{
}

bool Parser::parseStringLiteral()
{
    while( !lex->lookAhead(0).isNull() ) {
	if( lex->lookAhead(0) == Token_identifier &&
	    lex->lookAhead(0).toString() == "L" &&
					  lex->lookAhead(1) == Token_string_literal ) {
	    
	    lex->nextToken();
	    lex->nextToken();
	} else if( lex->lookAhead(0) == Token_string_literal ) {
	    lex->nextToken();
	} else
	    return false;
    }
    return true;
}

bool Parser::parseExpression()
{
    //kdDebug(9007) << "Parser::parseExpression()" << endl;
    
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	switch( tk ){
		case '(':
	    skip( '(', ')' );
	    lex->nextToken();
	    break;
	    
	case '[':
	    skip( '[', ']' );
	    lex->nextToken();
	    break;
	    
		case ';':
		case ',':
		case ']':
		case ')':
		case '{':
		case '}':
		case Token_case:
		case Token_default:
		case Token_if:
		case Token_while:
		case Token_do:
		case Token_for:
		case Token_break:
		case Token_continue:
		case Token_return:
		case Token_goto:
	    return true;
	    
	    default:
	    lex->nextToken();
	}
    }
    
    return false;
}


bool Parser::parseExpressionStatement()
{
    //kdDebug(9007) << "Parser::parseExpressionStatement()" << endl;
    parseCommaExpression();
    
    ADVANCE( ';', ";" );
    
    return true;
}

bool Parser::parseStatement() // thanks to fiore@8080.it ;-)
{
    //kdDebug(9007) << "Parser::parseStatement()" << endl;
    switch( lex->lookAhead(0) ){
	
    case Token_while:
	return parseWhileStatement();
	
    case Token_do:
	return parseDoStatement();
	
    case Token_for:
	return parseForStatement();
	
    case Token_if:
	return parseIfStatement();
	
    case Token_switch:
	return parseSwitchStatement();
	
    case Token_try:
	return parseTryBlockStatement();
	
    case Token_case:
    case Token_default:
	return parseLabeledStatement();
	
    case Token_break:
    case Token_continue:
	lex->nextToken();
	ADVANCE( ';', ";" );
	return true;
	
    case Token_goto:
	lex->nextToken();
	ADVANCE( Token_identifier, "identifier" );
	ADVANCE( ';', ";" );
	return true;
	
    case Token_return:
	lex->nextToken();
	parseCommaExpression();
	ADVANCE( ';', ";" );
	return true;
	
    case '{':
	return parseCompoundStatement();
	
    case Token_identifier:
	if( parseLabeledStatement() )
	    return true;
	break;
    }
    
    if ( parseDeclarationStatement() )
	return true;
    
    return parseExpressionStatement();
}

bool Parser::parseCondition()
{
    //kdDebug(9007) << "Parser::parseCondition()" << endl;
    
    int index = lex->index();
    
    QDomElement spec;
    if( parseTypeSpecifier(spec) ){
	
        QDomElement declarator;
	if( parseDeclarator(declarator) && lex->lookAhead(0) == '=' ) {
	    lex->nextToken();
	    
	    if( parseAssignmentExpression() )
		return true;
	}
    }
    
    lex->setIndex( index );
    return parseCommaExpression();
}


bool Parser::parseWhileStatement()
{
    //kdDebug(9007) << "Parser::parseWhileStatement()" << endl;
    ADVANCE( Token_while, "while" );
    ADVANCE( '(' , "(" );
    
    if( !parseCondition() ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    if( !parseStatement() ){
	reportError( i18n("statement expected") );
	return false;
    }
    
    return true;
}

bool Parser::parseDoStatement()
{
    //kdDebug(9007) << "Parser::parseDoStatement()" << endl;
    ADVANCE( Token_do, "do" );
    if( !parseStatement() ){
	reportError( i18n("statement expected") );
	return false;
    }
    ADVANCE( Token_while, "while" );
    ADVANCE( '(' , "(" );
    if( !parseCommaExpression() ){
	reportError( i18n("expression expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    ADVANCE( ';', ";" );
    
    return true;
}

bool Parser::parseForStatement()
{
    //kdDebug(9007) << "Parser::parseForStatement()" << endl;
    ADVANCE( Token_for, "for" );
    ADVANCE( '(', "(" );
        
    if( !parseForInitStatement() ){
	reportError( i18n("for initialization expected") );
	return false;
    }
    
    parseCondition();
    ADVANCE( ';', ";" );
    parseCommaExpression();
    ADVANCE( ')', ")" );
    
    return parseStatement();
}

bool Parser::parseForInitStatement()
{
    //kdDebug(9007) << "Parser::parseForInitStatement()" << endl;
    
    if ( parseDeclarationStatement() )
	return true;
    return parseExpressionStatement();
}

bool Parser::parseCompoundStatement()
{
    //kdDebug(9007) << "Parser::parseCompoundStatement()" << endl;
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
        
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;
	
	if( !parseStatement() ){
	    skipUntilStatement();
	}
    }
    
    ADVANCE( '}', "}" );
    return true;
}

bool Parser::parseIfStatement()
{
    //kdDebug(9007) << "Parser::parseIfStatement()" << endl;
    
    ADVANCE( Token_if, "if" );
    
    ADVANCE( '(' , "(" );
    
    if( !parseCondition() ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    if( !parseStatement() ){
	reportError( i18n("statement expected") );
	return false;
    }
    
    if( lex->lookAhead(0) == Token_else ){
	lex->nextToken();
	if( !parseStatement() ) {
	    reportError( i18n("statement expected") );
	    return false;
	}
    }
    
    return true;
}

bool Parser::parseSwitchStatement()
{
    //kdDebug(9007) << "Parser::parseSwitchStatement()" << endl;
    ADVANCE( Token_switch, "switch" );
    
    ADVANCE( '(' , "(" );
        
    if( !parseCondition() ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    if( !parseCompoundStatement() ){
	syntaxError();
	return false;
    }
    return true;
}

bool Parser::parseLabeledStatement()
{
    //kdDebug(9007) << "Parser::parseLabeledStatement()" << endl;
    switch( lex->lookAhead(0) ){
    case Token_identifier:
    case Token_default:
	if( lex->lookAhead(1) == ':' ){
	    lex->nextToken();
	    lex->nextToken();
	    return parseStatement();
	}
	break;
	
    case Token_case:
	lex->nextToken();
	if( !parseConstantExpression() ){
	    reportError( i18n("expression expected") );
	}
	ADVANCE( ':', ":" );
	return parseStatement();
    }
    return false;
}

bool Parser::parseBlockDeclaration()
{
    //kdDebug(9007) << "Parser::parseBlockDeclaration()" << endl;
    switch( lex->lookAhead(0) ) {
    case Token_using:
	return parseUsing();
    case Token_asm:
	return parseAsmDefinition();
    case Token_namespace:
	return parseNamespaceAliasDefinition();
    }
    
    int index = lex->index();
    
    while( parseStorageClassSpecifier() )
	;
    
    parseCvQualify();
    
    QDomElement spec;
    if ( !parseTypeSpecifierOrClassSpec(spec) ) { // replace with simpleTypeSpecifier?!?!
	lex->setIndex( index );
	return false;
    }
    
    parseInitDeclaratorList();
    
    return true;
}

bool Parser::parseNamespaceAliasDefinition()
{
    if ( lex->lookAhead(0) != Token_namespace ) {
	return false;
    }
    lex->nextToken();

    ADVANCE( Token_identifier,  "identifier" );
    ADVANCE( '=', "=" );
    
    QDomElement name;
    if( !parseName(name) ){
	reportError( i18n("Namespace name expected") );
    }
    
    ADVANCE( ';', ";" );
        
    return true;
    
}

bool Parser::parseDeclarationStatement()
{
    //kdDebug(9007) << "Parser::parseDeclarationStatement()" << endl;
    
    int index = lex->index();
    
    if ( !parseBlockDeclaration() )
	return false;
    
    if ( lex->lookAhead(0) != ';' ) {
	lex->setIndex( index );
	return false;
    }
    lex->nextToken();
    
    return true;
}

bool Parser::parseDeclaration()
{
#warning "TODO: Parser::parseDeclaration() -- fill abstract syntax tree"

    while( parseFunctionSpecifier() )
        ;
    
    while(  parseStorageClassSpecifier() )
        ;

    parseCvQualify();
        
    int index = lex->index();

    QDomElement name;
    if( parseName(name) && lex->lookAhead(0) == '(' ){
	// no type specifier, maybe a constructor or a cast operator??
	
	lex->setIndex( index );
	
	parseNestedNameSpecifier();
	QString nestedName = toString( index, lex->index() );
	
	QDomElement declarator;
	if( parseInitDeclarator(declarator) ){
	    switch( lex->lookAhead(0) ){
	    case ';':
		if( !nestedName ){
		    lex->nextToken();
		    return true;
		}
		syntaxError();
		return false;
		case ':':
		if( parseCtorInitializer() && parseFunctionBody() )
		    return true;
		syntaxError();
		return false;
	    case '{':
		if( !parseFunctionBody() ){
		    syntaxError();
		    return false;
		}
		return true;
	    }
	}
	syntaxError();
	return false;
    }
    
    lex->setIndex( index );
    
    if( lex->lookAhead(0) == Token_const && lex->lookAhead(1) == Token_identifier && lex->lookAhead(2) == '=' ){
	// constant definition
	lex->nextToken();
	if( parseInitDeclaratorList() ){
	    ADVANCE( ';', ";" );
	    //kdDebug(9007) << "found constant definition" << endl;
	    return true;
	}
	syntaxError();
	return false;
    }
        
    QDomElement spec;
    if( parseTypeSpecifier(spec) ){
	
	if( lex->lookAhead(0) == ';' ){
	    // type definition
	    lex->nextToken();
	    return true;
	}
	
	if( parseNestedNameSpecifier() ) {
	    // maybe a method declaration/definition
	    
  	    QDomElement declarator;
	    if ( !parseInitDeclarator(declarator) ) {
		syntaxError();
		return false;
	    }
	} else if ( !parseInitDeclaratorList() ) {
	    syntaxError();
	    return false;
	}
	
	switch( lex->lookAhead(0) ){
	case ';':
	    lex->nextToken();
	    //kdDebug(9007) << "found function/field declaration" << endl;
	    return true;

	case '=':
	    return parseInitializer();

	case '{':
	    if ( parseFunctionBody() ) {
		//kdDebug(9007) << "found function definition" << endl;
		return true;
	    }
	    break;
	    
	default:
	    syntaxError();
	    return false;
	}
    }
    
    syntaxError();
    return false;
}

bool Parser::parseFunctionBody()
{
    //kdDebug(9007) << "Parser::parseFunctionBody()" << endl;
    
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
    
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;
	
	if( !parseStatement() ){
	    skipUntilStatement();
	}
    }
    
    ADVANCE( '}', "}" );
    
    return true;
}

QString Parser::toString( int start, int end, const QString& sep ) const
{
    QStringList l;
    
    for( int i=start; i<end; ++i ){
	l << lex->tokenAt( i ).toString();
    }
    
    return l.join( sep ).stripWhiteSpace();
}

bool Parser::parseTypeSpecifierOrClassSpec( QDomElement& spec )
{
    if( parseClassSpecifier(spec) )
	return true;
    else if( parseEnumSpecifier(spec) )
	return true;
    else if( parseTypeSpecifier(spec) )
	return true;
    else
	return false;
}

bool Parser::parseTryBlockStatement()
{
    //kdDebug(9007) << "Parser::parseTryBlockStatement()" << endl;
    
    if( lex->lookAhead(0) != Token_try )
	return false;
    lex->nextToken();
    
    if( !parseCompoundStatement() ){
	syntaxError();
	return false;
    }
    
    if( lex->lookAhead(0) != Token_catch ){
	reportError( i18n("catch expected") );
	return false;
    }
    
    while( lex->lookAhead(0) == Token_catch ){
	lex->nextToken();
	ADVANCE( '(', "(" );
	if( !parseCondition() ){
	    reportError( i18n("condition expected") );
	    return false;
	}
	ADVANCE( ')', ")" );
	if( !parseCompoundStatement() ){
	    syntaxError();
	    return false;
	}
    }
    
    return true;
}
