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

// qt
#include <qstring.h>
#include <qstringlist.h>
#include <qasciidict.h>

#include <kdebug.h>
#include <klocale.h>

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
    ParserPrivateData()
        {}
};


Parser::Parser( ProblemReporter* pr, Driver* drv, Lexer* lexer )
    : m_problemReporter( pr ),
      driver( drv ),
      lex( lexer )
{
    d = new ParserPrivateData();
    m_fileName = "<stdin>";
    
    m_maxErrors = 5;
    resetErrors();
}

Parser::~Parser()
{
    delete( d );
    d = 0;
}

void Parser::setFileName( const QString& fileName )
{
    m_fileName = fileName;
}

bool Parser::reportError( const Error& err )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::reportError()" << endl;
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
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::reportError()" << endl;
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
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipUntil()" << endl;
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == token )
	    return true;
	
	lex->nextToken();
    }
    
    return false;
}

bool Parser::skipUntilDeclaration()
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipUntilDeclaration()" << endl;
    
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "-->token = " << lex->lookAhead(0).toString() << endl;

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
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipUntilStatement() -- token = " << lex->lookAhead(0).toString() << endl;
    
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

bool Parser::parseName( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseName()" << endl;

#warning "ignore namespace for now!!"    

    AST::Ptr nestedName, unqualifedName;
            
    if( lex->lookAhead(0) == Token_scope ){
	lex->nextToken();
    }
    
    parseNestedNameSpecifier( nestedName );
    if( parseUnqualifiedName(unqualifedName) ){
        //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "----------> name parsed!!" << endl;
	return true;
    }
    
    return false;
}

bool Parser::parseTranslationUnit( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTranslationUnit()" << endl;
            
    while( !lex->lookAhead(0).isNull() ){
        AST::Ptr def;
        if( !parseDefinition(def) ){
	    // error recovery
	    skipUntilDeclaration();
	}
    }
    
    return m_errors == 0;
}

bool Parser::parseDefinition( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDefinition()" << endl;
    
    int start = lex->lookAhead( 0 );
    
    switch( lex->lookAhead(0) ){
	
    case ';':  
	lex->nextToken();
	return true;
	
    case Token_extern: 
	return parseLinkageSpecification( node );
	
    case Token_namespace: 
	return parseNamespace( node );
	
    case Token_using: 
	return parseUsing( node );
	
    case Token_typedef: 
	return parseTypedef( node );
	
    case Token_asm: 
	return parseAsmDefinition( node );
	
    case Token_template: 
    case Token_export:
	return parseTemplateDeclaration( node );
	
    default:
        {
	    AST::Ptr spec, declarators, declarator;
	    
	    if( parseEnumSpecifier(spec) || parseClassSpecifier(spec) ){
	        parseInitDeclaratorList(declarators);
	        ADVANCE( ';', ";" );
		
		node = AST::Ptr( new AST() );
		node->start = start;
		node->end = lex->lookAhead( 0 );
		// node->spec = spec;
		// node->declarators = declarators;
		
	        return true;
	    }
	
	    return parseDeclaration( node );
	}
	
    } // end switch
}

bool Parser::parseLinkageSpecification( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLinkageSpecification()" << endl;
    
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
        AST::Ptr linkageBody;
	parseLinkageBody( linkageBody );	
    } else {
        AST::Ptr def;
	if( !parseDefinition(def) ){
	    reportError( i18n("Declaration syntax error") );
	}
    }
    
    return true;
}

bool Parser::parseLinkageBody( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLinkageBody()" << endl;
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
        
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	if( tk == '}' )
	    break;
	
	AST::Ptr def;
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

bool Parser::parseNamespace( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNamespace()" << endl;
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
	
	AST::Ptr name;
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

    AST::Ptr linkageBody;
    parseLinkageBody( linkageBody );
    
    return true;
}

bool Parser::parseUsing( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseUsing()" << endl;
    
    if( lex->lookAhead(0) != Token_using ){
	return false;
    }
    lex->nextToken();
    
    if( lex->lookAhead(0) == Token_namespace ){
        AST::Ptr usingDirective;
	return parseUsingDirective( usingDirective );
    }
    
    if( lex->lookAhead(0) == Token_typename )
	lex->nextToken();
    
    AST::Ptr name;
    if( !parseName(name) )
	return false;
        
    ADVANCE( ';', ";" );
      
    return true;
}

bool Parser::parseUsingDirective( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseUsingDirective()" << endl;
    
    if( lex->lookAhead(0) != Token_namespace ){
	return false;
    }
    lex->nextToken();
    
    AST::Ptr name;
    if( !parseName(name) ){
	reportError( i18n("Namespace name expected") );
	return false;
    }
    
    ADVANCE( ';', ";" );
        
    return true;
}


bool Parser::parseOperatorFunctionId( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseOperatorFunctionId()" << endl;

    if( lex->lookAhead(0) != Token_operator ){
	return false;
    }
    lex->nextToken();    

    AST::Ptr op;    
    if( parseOperator(op) )
	return true;
    else {
	// parse cast operator
	AST::Ptr cv;
        parseCvQualify(cv);       
	
	AST::Ptr spec;
	if( !parseSimpleTypeSpecifier(spec) ){
	    parseError();
	}
	
	AST::Ptr cv2;
	parseCvQualify(cv2);
	
	AST::Ptr ptrOp;
	while( parsePtrOperator(ptrOp) )
  	    ;
	
	return true;
    }
}

bool Parser::parseTemplateArgumentList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateArgumentList()" << endl;
    
    AST::Ptr templArg;
    if( !parseTemplateArgument(templArg) )
	return false;
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseTemplateArgument(templArg) ){
	    parseError();
	    break;
	}
    }
    
    return true;
}

bool Parser::parseTypedef( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypedef()" << endl;
    
    if( lex->lookAhead(0) != Token_typedef ){
	return false;
    }
    lex->nextToken();
    
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "token = " << lex->lookAhead(0).toString() << endl;
    AST::Ptr spec;
    if( !parseTypeSpecifierOrClassSpec(spec) ){
	reportError( i18n("Need a type specifier to declare") );
	return false;
    }
    
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "token = " << lex->lookAhead(0).toString() << endl;
    AST::Ptr declarators;
    if( !parseInitDeclaratorList(declarators) ){
	reportError( i18n("Need an identifier to declare") );
	return false;
    }
    
    ADVANCE( ';', ";" );
        
    return true;
}

bool Parser::parseAsmDefinition( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAsmDefinition()" << endl;
    
    ADVANCE( Token_asm, "asm" );
    ADVANCE( '(', '(' );
    
    AST::Ptr lit;
    parseStringLiteral( lit );
    
    ADVANCE( ')', ')' );
    ADVANCE( ';', ';' );
    
    return true;
}

bool Parser::parseTemplateDeclaration( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateDeclaration()" << endl;
    
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
	lex->nextToken()
	;
	AST::Ptr params;
	parseTemplateParameterList(params);
	
	ADVANCE( '>', ">" );
    }
    
    AST::Ptr def;
    if( !parseDefinition(def) ){
	reportError( i18n("expected a declaration") );
    }
        
    return true;
}

bool Parser::parseOperator( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseOperator()" << endl;
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
	lex->nextToken();
	return true;
	
    default:
	if( lex->lookAhead(0) == '(' && lex->lookAhead(1) == ')' ){
	    lex->nextToken();
	    lex->nextToken();
	    return true;
	} else if( lex->lookAhead(0) == '[' && lex->lookAhead(1) == ']' ){
	    lex->nextToken();
	    lex->nextToken();
	    return true;
	}
    }
    
    return false;
}

bool Parser::parseCvQualify( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCvQualify()" << endl;
    
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

bool Parser::parseSimpleTypeSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseSimpleTypeSpecifier()" << endl;
    
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

    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "!! token = " << lex->lookAhead(0).toString() << endl;
    AST::Ptr name;
    return parseName( name );
}

bool Parser::parsePtrOperator( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parsePtrOperator()" << endl;
    
    if( lex->lookAhead(0) == '&' ){
	lex->nextToken();
    } else if( lex->lookAhead(0) == '*' ){
	lex->nextToken();
    } else {
	int index = lex->index();
	AST::Ptr memPtr;
	if( !parsePtrToMember(memPtr) ){
	    lex->setIndex( index );
	    return false;
	}
    }
    
    AST::Ptr cv;
    parseCvQualify( cv );
    
    return true;
}


bool Parser::parseTemplateArgument( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateArgument()" << endl;
    
#warning "TODO Parser::parseTemplateArgument()"
#warning "parse type id"
    
#if 0
    if( parseTypeId() ){
	qWarning( "token = %s", lex->lookAhead(0).toString().latin1() );
	return true;
    }
#endif
    
    return skipAssignmentExpression();
}

bool Parser::parseTypeSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypeSpecifier()" << endl;
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "----> token = " << lex->lookAhead(0).toString() << endl;

    AST::Ptr cv;    
    parseCvQualify(cv);
    
    AST::Ptr spec;
    if( parseElaboratedTypeSpecifier(spec) || parseSimpleTypeSpecifier(spec) ){
        AST::Ptr cv2;
	parseCvQualify(cv2);
    } else
	return false;
    
    return true;
}

bool Parser::parseDeclarator( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclarator()" << endl;

    AST::Ptr ptrOp, decl, declId;
    
    while( parsePtrOperator(ptrOp) ){
        // TODO: add ptr operator do current declarator
    }
    
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();

	if( !parseDeclarator(decl) ){
	    return false;
	}	
	if( lex->lookAhead(0) != ')'){
	    return false;
	}
	lex->nextToken();
    } else {
	
	if( !parseDeclaratorId(declId) ){
	    return false;
	}	
	
	if( lex->lookAhead(0) == ':' ){
	    lex->nextToken();
	    if( !skipConstantExpression() ){
		reportError( i18n("Constant expression expected") );
		return true;
	    }
	    return true;
	}
    }
    
    while( lex->lookAhead(0) == '[' ){
	lex->nextToken();
	skipCommaExpression();
	
	ADVANCE( ']', "]" );
	// TODO: add vector
    }
    
    int index = lex->index();
    if( lex->lookAhead(0) == '(' ){     
	lex->nextToken();
	
	AST::Ptr params;
	if( !parseParameterDeclarationClause(params) ){
	    lex->setIndex( index );
	    return true;
	}
	
	if( lex->lookAhead(0) != ')' ){
	    lex->setIndex( index );
	    return true;
	} else
	    lex->nextToken();
	
	AST::Ptr cv;
	parseCvQualify( cv );	
	
	AST::Ptr except;
	parseExceptionSpecification( except );
    }
    
    return true;
}

bool Parser::parseEnumSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseEnumSpecifier()" << endl;
    
    int index = lex->index();
    
    AST::Ptr storageSpec;
    while( parseStorageClassSpecifier(storageSpec) )
	;


    AST::Ptr cv;
    parseCvQualify( cv );
    
    if( lex->lookAhead(0) != Token_enum ){
	return false;
    }

    lex->nextToken();
    
    if( lex->lookAhead(0) == Token_identifier ){
	lex->nextToken();
    }
    
    if( lex->lookAhead(0) != '{' ){
	lex->setIndex( index );
	return false;
    }
    lex->nextToken();
        
    AST::Ptr enumerators;
    parseEnumeratorList( enumerators );
    
    if( lex->lookAhead(0) != '}' )
	reportError( i18n("} missing") );
    else
	lex->nextToken();
    
    return true;
}

bool Parser::parseTemplateParameterList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateParameterList()" << endl;
    
    AST::Ptr param;
    if( !parseTemplateParameter(param) ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseTemplateParameter(param) ){
	    parseError();
	    break;
	}
    }
    
    return true;
}

bool Parser::parseTemplateParameter( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateParameter()" << endl;
    
    int tk = lex->lookAhead( 0 );
    if( tk == Token_class ||
	tk == Token_typename ||
	tk == Token_template )
	return parseTypeParameter( node );
    else
	return parseParameterDeclaration( node );
}

bool Parser::parseTypeParameter( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypeParameter()" << endl;
    
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
		    
		    AST::Ptr typeId;
		    if( !parseTypeId(typeId) ){
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
		    
		    AST::Ptr typeId;
		    if( !parseTypeId(typeId) ){
			parseError();
		    }
		}
	}
	return true;
	
    case Token_template:
	{
	    
	    lex->nextToken(); // skip template
	    ADVANCE( '<', '<' );
	    
	    AST::Ptr params;
	    if( !parseTemplateParameterList(params) ){
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

bool Parser::parseStorageClassSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseStorageClassSpecifier()" << endl;
    
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

bool Parser::parseFunctionSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseFunctionSpecifier()" << endl;
    
    switch( lex->lookAhead(0) ){
    case Token_inline:
    case Token_virtual:
    case Token_explicit:
	lex->nextToken();
	return true;
    }
    
    return false;
}

bool Parser::parseTypeId( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypeId()" << endl;
    
    AST::Ptr spec;
    if( !parseTypeSpecifier(spec) ){
	return false;
    }
    
    AST::Ptr decl;
    parseAbstractDeclarator( decl );
    
    return true;
}

bool Parser::parseAbstractDeclarator( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAbstractDeclarator()" << endl;
    
    AST::Ptr ptrOp;
    while( parsePtrOperator(ptrOp) )
       ;
    
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();
	
	AST::Ptr decl;
	if( !parseDeclarator(decl) ){
	    return false;	    
	}
	
	if( lex->lookAhead(0) != ')'){
	    return false;
	}
	lex->nextToken();
	
    }
    
    while( lex->lookAhead(0) == '[' ){
	lex->nextToken();
	skipCommaExpression();
	
	ADVANCE( ']', "]" );
    }
    
    int index = lex->index();
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();
	
	AST::Ptr param;
	if( !parseParameterDeclarationClause(param) ){
	    lex->setIndex( index );
	    return true;
	}
	
	ADVANCE( ')', ")" );	
	
	AST::Ptr cv, except;
	
	parseCvQualify( cv );	
	parseExceptionSpecification( except );
    }
    
    return true;
}

bool Parser::skipConstantExpression()
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipConstantExpression()" << endl;
    
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


bool Parser::parseInitDeclaratorList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitDeclaratorList()" << endl;
    
    AST::Ptr decl;
    
    if( !parseInitDeclarator(decl) ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseInitDeclarator(decl) ){
	    parseError();
	    break;
	}
    }
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitDeclaratorList() -- end" << endl;
    return true;
}

bool Parser::parseParameterDeclarationClause( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseParameterDeclarationClause()" << endl;
    
    AST::Ptr params;
    if( !parseParameterDeclarationList(params) ){
	
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

bool Parser::parseParameterDeclarationList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseParameterDeclarationList()" << endl;
    
    int index = lex->index();
    
    AST::Ptr param;
    if( !parseParameterDeclaration(param) ){
	lex->setIndex( index );
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseParameterDeclaration(param) ){
	    lex->setIndex( index );
	    return false;
	}
    }
    return true;
}

bool Parser::parseParameterDeclaration( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseParameterDeclaration()" << endl;
    
    int index = lex->index();
    
    // parse decl spec
    
    AST::Ptr spec;
    if( !parseTypeSpecifier(spec) ){
	lex->setIndex( index );
	return false;
    }
    
    index = lex->index();
    
    AST::Ptr decl;
    if( !parseDeclarator(decl) ){
	lex->setIndex( index );
	parseAbstractDeclarator(decl);
    }
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	if( !skipAssignmentExpression() ){
	    //reportError( i18n("Expression expected") );
	}
    }
    
    return true;
}

bool Parser::parseClassSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseClassSpecifier()" << endl;
    
    int index = lex->index();

    AST::Ptr storageSpec;    
    while( parseStorageClassSpecifier(storageSpec) )
	;   

    AST::Ptr cv;
    parseCvQualify( cv );
    
    int kind = lex->lookAhead( 0 );
    if( kind == Token_class || kind == Token_struct || kind == Token_union ){
	lex->nextToken();
    } else {
	return false;
    }

    int line, col;
    lex->lookAhead(0).getStartPosition( &line, &col );

    AST::Ptr unqualifedName;
    parseUnqualifiedName( unqualifedName );

    AST::Ptr bases;
    if( lex->lookAhead(0) == ':' ){      
	if( !parseBaseClause(bases) ){
	    skipUntil( '{' );
	}
    }
    
    if( lex->lookAhead(0) != '{' ){
	lex->setIndex( index );
	return false;
    }
    
    ADVANCE( '{', '{' );

    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;
	
	AST::Ptr memSpec;
	if( !parseMemberSpecification(memSpec) ){
	    skipUntilDeclaration();
	}
    }
    
    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} missing") );
    } else
	lex->nextToken();
    
    return true;
}

bool Parser::parseAccessSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAccessSpecifier()" << endl;
    
    switch( lex->lookAhead(0) ){
    case Token_public:
    case Token_protected:
    case Token_private:
	lex->nextToken();
	return true;
    }
    
    return false;
}

bool Parser::parseMemberSpecification( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMemberSpecification()" << endl;

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
    } else if( parseTypedef(node) ){
	return true;
    } else if( parseUsing(node) ){
	return true;
    } else if( parseTemplateDeclaration(node) ){
	return true;
    } else if( parseAccessSpecifier(node) ){
	if( lex->lookAhead(0) == Token_slots ){
	    lex->nextToken();
	}
	ADVANCE( ':', ":" );
	return true;
    }
    
    AST::Ptr spec;
    if( parseEnumSpecifier(spec) || parseClassSpecifier(spec) ){
    	AST::Ptr declarators;
	parseInitDeclaratorList( declarators );
	ADVANCE( ';', ";" );
	return true;
    }
    
    return parseDeclaration( node );
}

bool Parser::parseCtorInitializer( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCtorInitializer()" << endl;
    
    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    lex->nextToken();
    
    AST::Ptr inits;
    if( !parseMemInitializerList(inits) ){
	reportError( i18n("Member initializers expected") );
    }
    
    return true;
}

bool Parser::parseElaboratedTypeSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseElaboratedTypeSpecifier()" << endl;
    
    int index = lex->index();
    
    int tk = lex->lookAhead( 0 );
    if( tk == Token_class  ||
	tk == Token_struct ||
	tk == Token_union  ||
	tk == Token_enum   ||
	tk == Token_typename )
    {
	lex->nextToken();
	
	AST::Ptr name;
	if( parseName(name) ){
	    return true;
	}
    }
    
    lex->setIndex( index );
    return false;
}

bool Parser::parseDeclaratorId( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclaratorId()" << endl;
    return parseName( node );
}

bool Parser::parseExceptionSpecification( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseExceptionSpecification()" << endl;
    
    if( lex->lookAhead(0) != Token_throw ){
	return false;
    }
    lex->nextToken();
    
    ADVANCE( '(', "(" );
    parseTypeIdList( node );
    ADVANCE( ')', ")" );
    
    return true;
}

bool Parser::parseEnumeratorList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseEnumeratorList()" << endl;
    
    AST::Ptr enumerator;
    if( !parseEnumerator(enumerator) ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseEnumerator(enumerator) ){
	    //reportError( i18n("Enumerator expected") );
	    break;
	}
    }
    
    return true;
}

bool Parser::parseEnumerator( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseEnumerator()" << endl;
    
    if( lex->lookAhead(0) != Token_identifier ){
	return false;
    }
    lex->nextToken();
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	
	if( !skipExpression() ){
	    reportError( i18n("Constant expression expected") );
	}
    }
    
    return true;
}

bool Parser::parseInitDeclarator( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitDeclarator()" << endl;
    
    AST::Ptr decl, init;
    if( !parseDeclarator(decl) ){
	return false;
    }
        
    parseInitializer( init );
    
    return true;
}

bool Parser::skipAssignmentExpression()
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipAssignmentExpression()" << endl;
    
#warning "TODO Parser::skipAssignmentExpression()"
    
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

bool Parser::parseBaseClause( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseBaseClause()" << endl;
    
    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    lex->nextToken();
    
    if( !parseBaseSpecifierList(node) ){
	reportError( i18n("expected base specifier list") );
	return false;
    }
    
    return true;
}

bool Parser::parseInitializer( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitializer()" << endl;
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	
	AST::Ptr init;
	if( !parseInitializerClause(node) ){
	    reportError( i18n("Initializer clause expected") );
	    return false;
	}
    } else if( lex->lookAhead(0) == '(' ){
	lex->nextToken();
	skipCommaExpression();
	
	ADVANCE( ')', ")" );
    }
    
    return false;
}

bool Parser::parseMemInitializerList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMemInitializerList()" << endl;
    
    AST::Ptr init;
    if( !parseMemInitializer(init) ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( parseMemInitializer(init) ){
	} else {
	    break;
	}
    }
    
    return true;
}

bool Parser::parseMemInitializer( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMemInitializer()" << endl;
    
    AST::Ptr initId;
    if( !parseMemInitializerId(initId) ){
	reportError( i18n("Identifier expected") );
	return false;
    }
    ADVANCE( '(', '(' );
    skipCommaExpression();
    ADVANCE( ')', ')' );
    
    return true;
}

bool Parser::parseTypeIdList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypeIdList()" << endl;
    
    AST::Ptr typeId;
    if( !parseTypeId(typeId) ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	if( parseTypeId(typeId) ){
	    // ...
	} else {
	    reportError( i18n("Type id expected") );
	    break;
	}
    }
    
    return true;
}

bool Parser::parseBaseSpecifierList( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseBaseSpecifierList()" << endl;

    AST::Ptr baseSpec;
    if( !parseBaseSpecifier(baseSpec) ){
	return false;
    }
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseBaseSpecifier(baseSpec) ){
	    reportError( i18n("Base class specifier expected") );
	    return false;
	}
    }
    
    return true;
}

bool Parser::parseBaseSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseBaseSpecifier()" << endl;
    AST::Ptr access;
    
    if( lex->lookAhead(0) == Token_virtual ){
	lex->nextToken();
	
	parseAccessSpecifier( access );
    } else {	
        parseAccessSpecifier( access );
	
	if( lex->lookAhead(0) == Token_virtual ){
	    lex->nextToken();
	}
    }
        
    AST::Ptr name;
    if( !parseName(name) ){
	reportError( i18n("Identifier expected") );
    }

    return true;
}


bool Parser::parseInitializerClause( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitializerClause()" << endl;
    
#warning "TODO Parser::initializer-list"
    
    if( lex->lookAhead(0) == '{' ){
	if( !skip('{','}') ){
	    reportError( i18n("} missing") );
	} else
	    lex->nextToken();
    } else {
	if( !skipAssignmentExpression() ){
	    //reportError( i18n("Expression expected") );
	}
    }
    
    return true;
}

bool Parser::parseMemInitializerId( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMemInitializerId()" << endl;
    
    return parseName( node );
}


bool Parser::skipCommaExpression()
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipCommaExpression()" << endl;
    
    if( !skipExpression() )
	return false;
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !skipExpression() ){
	    reportError( i18n("expression expected") );
	    return false;
	}
    }
    return true;
}

bool Parser::parseNestedNameSpecifier( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNestedNameSpecifier()" << endl;
    
    int index = lex->index();
    bool ok = false;
    
    while( lex->lookAhead(0) == Token_identifier ){
	index = lex->index();
	
	if( lex->lookAhead(1) == '<' ){
	    lex->nextToken(); // skip template name
	    lex->nextToken(); // skip <
	    
	    AST::Ptr args;
	    if( !parseTemplateArgumentList(args) ){
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
	    lex->nextToken(); // skip ::
	    if( lex->lookAhead(0) == Token_template && lex->lookAhead(1) == Token_identifier )
		lex->nextToken(); // skip optional template keyword
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

bool Parser::parsePtrToMember( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parsePtrToMember()" << endl;
    
    if( lex->lookAhead(0) == Token_scope ){
	lex->nextToken();
    }
    
    while( lex->lookAhead(0) == Token_identifier ){
	lex->nextToken();
	
	if( lex->lookAhead(0) == Token_scope && lex->lookAhead(1) == '*' ){
	    lex->nextToken(); // skip ::
	    lex->nextToken(); // skip *
	    return true;
	} else
	    break;
    }
    
    return false;
}

bool Parser::parseUnqualifiedName( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseUnqualifiedName()" << endl;
    
    bool isDestructor = false;

    if( lex->lookAhead(0) == Token_identifier ){
	lex->nextToken();
    } else if( lex->lookAhead(0) == '~' && lex->lookAhead(1) == Token_identifier ){
	lex->nextToken(); // skip ~
	lex->nextToken(); // skip classname
	isDestructor = true;
    } else if( lex->lookAhead(0) == Token_operator ){
	return parseOperatorFunctionId( node );
    } else
	return false;
    
    if( !isDestructor ){
	
	int index = lex->index();
	
	if( lex->lookAhead(0) == '<' ){
	    lex->nextToken();
	    
	    // optional template arguments
	    AST::Ptr args;
	    parseTemplateArgumentList( args );
	    
	    if( lex->lookAhead(0) != '>' ){
		lex->setIndex( index );
	    } else
		lex->nextToken();
	}
    }
    
    return true;
}

void Parser::dump()
{
}

bool Parser::parseStringLiteral( AST::Ptr& /*node*/ )
{
    while( !lex->lookAhead(0).isNull() ) {
	if( lex->lookAhead(0) == Token_identifier &&
	    lex->lookAhead(0).toString() == "L" && lex->lookAhead(1) == Token_string_literal ) {
	    
	    lex->nextToken();
	    lex->nextToken();
	} else if( lex->lookAhead(0) == Token_string_literal ) {
	    lex->nextToken();
	} else
	    return false;
    }
    return true;
}

bool Parser::skipExpression()
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipExpression()" << endl;
    
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


bool Parser::skipExpressionStatement()
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipExpressionStatement()" << endl;
    skipCommaExpression();
    
    ADVANCE( ';', ";" );
    
    return true;
}

bool Parser::parseStatement(  AST::Ptr& node ) // thanks to fiore@8080.it ;)
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseStatement()" << endl;
    switch( lex->lookAhead(0) ){
	
    case Token_while:
	return parseWhileStatement( node );
	
    case Token_do:
	return parseDoStatement( node );
	
    case Token_for:
	return parseForStatement( node );
	
    case Token_if:
	return parseIfStatement( node );
	
    case Token_switch:
	return parseSwitchStatement( node );
	
    case Token_try:
	return parseTryBlockStatement( node );
	
    case Token_case:
    case Token_default:
	return parseLabeledStatement( node );
	
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
	skipCommaExpression();
	ADVANCE( ';', ";" );
	return true;
	
    case '{':
	return parseCompoundStatement( node );
	
    case Token_identifier:
	if( parseLabeledStatement(node) )
	    return true;
	break;
    }
    
    if ( parseDeclarationStatement(node) )
	return true;
    
    return skipExpressionStatement();
}

bool Parser::parseCondition( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCondition()" << endl;
    
    int index = lex->index();
    
    AST::Ptr spec;
    if( parseTypeSpecifier(spec) ){
	
    	AST::Ptr decl;
	if( parseDeclarator(decl) && lex->lookAhead(0) == '=' ) {
	    lex->nextToken();
	    
	    if( skipAssignmentExpression() )
		return true;
	}
    }
    
    lex->setIndex( index );
    return skipCommaExpression();
}


bool Parser::parseWhileStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseWhileStatement()" << endl;
    ADVANCE( Token_while, "while" );
    ADVANCE( '(' , "(" );
    
    AST::Ptr cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    AST::Ptr body;
    if( !parseStatement(body) ){
	reportError( i18n("statement expected") );
	return false;
    }
    
    return true;
}

bool Parser::parseDoStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDoStatement()" << endl;
    ADVANCE( Token_do, "do" );
    
    AST::Ptr body;
    if( !parseStatement(body) ){
	reportError( i18n("statement expected") );
	return false;
    }
    
    ADVANCE( Token_while, "while" );
    ADVANCE( '(' , "(" );
    
    if( !skipCommaExpression() ){
	reportError( i18n("expression expected") );
	return false;
    }
    
    ADVANCE( ')', ")" );
    ADVANCE( ';', ";" );
    
    return true;
}

bool Parser::parseForStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseForStatement()" << endl;
    ADVANCE( Token_for, "for" );
    ADVANCE( '(', "(" );
        
    AST::Ptr init;
    if( !parseForInitStatement(init) ){
	reportError( i18n("for initialization expected") );
	return false;
    }
    
    AST::Ptr cond;
    parseCondition( cond );
    ADVANCE( ';', ";" );
    
    skipCommaExpression();
    ADVANCE( ')', ")" );
    
    AST::Ptr body;
    if( parseStatement(body) ){
        return true;
    }
    
    return false;
}

bool Parser::parseForInitStatement( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseForInitStatement()" << endl;
    
    if ( parseDeclarationStatement(node) )
	return true;
	
    return skipExpressionStatement();
}

bool Parser::parseCompoundStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCompoundStatement()" << endl;
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
        
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;
	
	AST::Ptr stmt;
	if( !parseStatement(stmt) ){
	    skipUntilStatement();
	}
    }
    
    ADVANCE( '}', "}" );
    return true;
}

bool Parser::parseIfStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseIfStatement()" << endl;
    
    ADVANCE( Token_if, "if" );
    
    ADVANCE( '(' , "(" );
    
    AST::Ptr cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    AST::Ptr stmt;
    if( !parseStatement(stmt) ){
	reportError( i18n("statement expected") );
	return false;
    }
    
    if( lex->lookAhead(0) == Token_else ){
	lex->nextToken();
	AST::Ptr elseStmt;
	if( !parseStatement(elseStmt) ) {
	    reportError( i18n("statement expected") );
	    return false;
	}
    }
    
    return true;
}

bool Parser::parseSwitchStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseSwitchStatement()" << endl;
    ADVANCE( Token_switch, "switch" );
    
    ADVANCE( '(' , "(" );
        
    AST::Ptr cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    AST::Ptr stmt;
    if( !parseCompoundStatement(stmt) ){
	syntaxError();
	return false;
    }
    return true;
}

bool Parser::parseLabeledStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLabeledStatement()" << endl;
    switch( lex->lookAhead(0) ){
    case Token_identifier:
    case Token_default:
	if( lex->lookAhead(1) == ':' ){
	    lex->nextToken();
	    lex->nextToken();
	    
	    AST::Ptr stmt;
	    if( parseStatement(stmt) ){
	        return true;
	    }
	}
	break;
	
    case Token_case:
	lex->nextToken();
	if( !skipConstantExpression() ){
	    reportError( i18n("expression expected") );
	}
	ADVANCE( ':', ":" );
	
	AST::Ptr stmt;
	if( parseStatement(stmt) ){
	    return true;
	}
	break;
	
    }
    
    return false;
}

bool Parser::parseBlockDeclaration( AST::Ptr& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseBlockDeclaration()" << endl;
    switch( lex->lookAhead(0) ) {
    case Token_using:
	return parseUsing( node );
    case Token_asm:
	return parseAsmDefinition( node );
    case Token_namespace:
	return parseNamespaceAliasDefinition( node );
    }
    
    int index = lex->index();
    
    AST::Ptr storageSpec;
    while( parseStorageClassSpecifier(storageSpec) )
	;
    
    AST::Ptr cv;
    parseCvQualify( cv );
    
    AST::Ptr spec;
    if ( !parseTypeSpecifierOrClassSpec(spec) ) { // replace with simpleTypeSpecifier?!?!
	lex->setIndex( index );
	return false;
    }
    
    AST::Ptr declarators;
    parseInitDeclaratorList( declarators );
    
    return true;
}

bool Parser::parseNamespaceAliasDefinition( AST::Ptr& /*node*/ )
{
    if ( lex->lookAhead(0) != Token_namespace ) {
	return false;
    }
    lex->nextToken();

    ADVANCE( Token_identifier,  "identifier" );
    ADVANCE( '=', "=" );
    
    AST::Ptr name;
    if( !parseName(name) ){
	reportError( i18n("Namespace name expected") );
    }
    
    ADVANCE( ';', ";" );
        
    return true;
    
}

bool Parser::parseDeclarationStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclarationStatement()" << endl;
    
    int index = lex->index();
    
    AST::Ptr decl;
    if ( !parseBlockDeclaration(decl) )
	return false;
    
    if ( lex->lookAhead(0) != ';' ) {
	lex->setIndex( index );
	return false;
    }
    lex->nextToken();
    
    return true;
}

bool Parser::parseDeclaration( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclaration()" << endl;

#warning "TODO: Parser::parseDeclaration() -- fill abstract syntax tree"


    AST::Ptr funSpec;
    while( parseFunctionSpecifier(funSpec) )
        ;
    
    AST::Ptr storageSpec;
    while(  parseStorageClassSpecifier(storageSpec) )
        ;
    
    AST::Ptr cv;
    parseCvQualify( cv );
        
    int index = lex->index();

    AST::Ptr name;
    if( parseName(name) && lex->lookAhead(0) == '(' ){
	// no type specifier, maybe a constructor or a cast operator??
	
	lex->setIndex( index );
	
	AST::Ptr nestedName;
	parseNestedNameSpecifier( nestedName );
	QString nestedNameText = toString( index, lex->index() );
	
	AST::Ptr decl;
	if( parseInitDeclarator(decl) ){
	    switch( lex->lookAhead(0) ){
	    case ';':
		if( !nestedNameText ){
		    lex->nextToken();
		    return true;
		}
		break;
		
	    case ':':
	        {
		    AST::Ptr ctorInit, funBody;
		    if( parseCtorInitializer(ctorInit) && parseFunctionBody(funBody) ){
		        return true;
		    }
		}
		break;
		
	    case '{':
	        {
		    AST::Ptr funBody;
		    if( parseFunctionBody(funBody) ){
		        return true;
		    }
		}
		break;
	    }
	}
	
	syntaxError();
	return false;
    }
    
    lex->setIndex( index );
    
    if( lex->lookAhead(0) == Token_const && lex->lookAhead(1) == Token_identifier && lex->lookAhead(2) == '=' ){
	// constant definition
	lex->nextToken();
	AST::Ptr declarators;
	if( parseInitDeclaratorList(declarators) ){
	    ADVANCE( ';', ";" );
	    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "found constant definition" << endl;
	    return true;
	}
	syntaxError();
	return false;
    }
        
    AST::Ptr spec;
    if( parseTypeSpecifier(spec) ){
	
	if( lex->lookAhead(0) == ';' ){
	    // type definition
	    lex->nextToken();
	    return true;
	}
	
	AST::Ptr nestedName, declarators;
	if( parseNestedNameSpecifier(nestedName) ) {
	    // maybe a method declaration/definition
	    
	    AST::Ptr decl;
	    if ( !parseInitDeclarator(decl) ) {
		syntaxError();
		return false;
	    }
	} else if ( !parseInitDeclaratorList(declarators) ) {
	    syntaxError();
	    return false;
	}
	
	switch( lex->lookAhead(0) ){
	case ';':
	    lex->nextToken();
	    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "found function/field declaration" << endl;
	    return true;

	case '=':
	    {
	        AST::Ptr init;
	        if( parseInitializer(init) ){
		    return true;
		}
	    }
	    break;

	case '{':
	    {
	        AST::Ptr funBody;
	        if ( parseFunctionBody(funBody) ) {
		    return true;
	        }
	    }
	    break;
	    
	}
    }
    
    syntaxError();
    return false;
}

bool Parser::parseFunctionBody( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseFunctionBody()" << endl;
    
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
    
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;

	AST::Ptr stmt;	
	if( !parseStatement(stmt) ){
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

bool Parser::parseTypeSpecifierOrClassSpec( AST::Ptr& node )
{
    if( parseClassSpecifier(node) )
	return true;
    else if( parseEnumSpecifier(node) )
	return true;
    else if( parseTypeSpecifier(node) )
	return true;
	
    return false;
}

bool Parser::parseTryBlockStatement( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTryBlockStatement()" << endl;
    
    if( lex->lookAhead(0) != Token_try ){
	return false;
    }
    lex->nextToken();
    
    AST::Ptr stmt;
    if( !parseCompoundStatement(stmt) ){
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
	AST::Ptr cond;
	if( !parseCondition(cond) ){
	    reportError( i18n("condition expected") );
	    return false;
	}
	ADVANCE( ')', ")" );
	
	AST::Ptr body;
	if( !parseCompoundStatement(body) ){
	    syntaxError();
	    return false;
	}
    }
    
    return true;
}

#if 0

bool Parser::parsePrimaryExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parsePrimarExpression()" << endl;

    AST::Ptr lit;
    if( parseStringLiteral(lit) )
        return true;

    switch( lex->lookAhead(0) ){
        case Token_number_literal:
        case Token_char_literal:
        case Token_true:
        case Token_false:
            lex->nextToken();
            return true;

        case Token_this:
            lex->nextToken();
            return true;

        case '(':
            lex->nextToken();
            //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "token = " << lex->lookAhead(0).toString() << endl;
	    AST::Ptr expr;
            if( !parseExpression(expr) ){
                reportError( i18n("expression expected") );
            }
            if( lex->lookAhead(0) != ')' ){
                reportError( i18n(") expected") );
            }
            lex->nextToken();
            return true;
    }

    AST::Ptr name;
    if( parseName(name) ){
        return true;
    }
    
    return false;
}

bool Parser::parsePostfixExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parsePostfixExpression()" << endl;
    switch( lex->lookAhead(0) ){
        case Token_typename:
            lex->nextToken();
	    
	    AST::Ptr name;
            if( !parseName(name) ){
                reportError( i18n("name expected") );
                skipUntil( '(' );
            }
	    
            ADVANCE( '(', "(" );
	    AST::Ptr expr;
            parseCommaExpression(expr);
            ADVANCE( ')', ")" );
	    
            return true;

        case Token_dynamic_cast:
        case Token_static_cast:
        case Token_reinterpret_cast:
        case Token_const_cast:
	{
            lex->nextToken();
	    
            ADVANCE( '<', "<" );
	    AST::Ptr typeId;
            parseTypeId( typeId );
            ADVANCE( '>', ">" );

            ADVANCE( '(', "(" );
	    AST::Ptr expr;
            parseCommaExpression( expr );
            ADVANCE( ')', ")" );
	}
        return true;

        case Token_typeid:
	{
            lex->nextToken();
            ADVANCE( '(', "(" );
	    AST::Ptr expr;
            parseCommaExpression( expr );
            ADVANCE( ')', ")" );
	}
        return true;

        default:
	{
	    AST::Ptr expr, spec;
            if( parsePrimaryExpression(expr) )
                return true;
            else if( parseSimpleTypeSpecifier(spec) ){
                ADVANCE( '(', "(" );
                parseCommaExpression( expr );
                ADVANCE( ')', ")" );
		return true;
            }
	}
 	return false;
    }

    AST::Ptr expr;
    while( !lex->lookAhead(0).isNull() ){
        switch( lex->lookAhead(0) ){
            case '[':
                lex->nextToken();
                parseCommaExpression( expr );
                ADVANCE( ']', "]" );
                break;

            case '(':
                lex->nextToken();
                parseCommaExpression( expr );
                ADVANCE( ')', ")" );
                break;

            case '.':
            case Token_arrow:
                lex->nextToken();
                if( lex->lookAhead(0) == Token_template )
                    lex->nextToken();

		AST::Ptr name;
                if( !parseName(name) ){
                    reportError( i18n("name expected") );
                    return false;
                }
                break;

            case Token_incr:
            case Token_decr:
                lex->nextToken();
                break;

            default:
                return true;
        }

    }

    return true;
}

bool Parser::parseUnaryExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseUnaryExpression()" << endl;
    switch( lex->lookAhead(0) ){
        case Token_incr:
        case Token_decr:
        case '*':
        case '&':
        case '+':
        case '-':
        case '!':
        case '~':
            lex->nextToken();
            return parseCastExpression();

        case Token_sizeof:
            lex->nextToken();
            if( lex->lookAhead(0) == '(' ){
                lex->nextToken();
                parseTypeId();
                ADVANCE( ')', ")" );
            } else {
                return parseUnaryExpression();
            }
            return true;

        case Token_new:
            return parseNewExpression();

        case Token_delete:
            return parseDeleteExpression();
    }

    return parsePostfixExpression();
}

bool Parser::parseNewExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNewExpression()" << endl;
    if( lex->lookAhead(0) == Token_scope && lex->lookAhead(1) == Token_new )
        lex->nextToken();

    ADVANCE( Token_new, "new");

    if( lex->lookAhead(0) == '(' ){
        lex->nextToken();
        parseCommaExpression();
        ADVANCE( ')', ")" );
    }

    if( lex->lookAhead(0) == '(' ){
        lex->nextToken();
        parseTypeId();
        ADVANCE( ')', ")" );
    } else {
        parseNewTypeId();
    }

    parseNewInitializer();
    return true;
}

bool Parser::parseNewTypeId( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNewTypeId()" << endl;
    if( parseTypeSpecifier() ){
        parseNewDeclarator();
        return true;
    }

    return false;
}

bool Parser::parseNewDeclarator( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNewDeclarator()" << endl;
    if( parsePtrOperator() ){
        parseNewDeclarator();
        return true;
    }

    if( lex->lookAhead(0) == '[' ){
        while( lex->lookAhead(0) == '[' ){
            lex->nextToken();
            parseExpression();
            ADVANCE( ']', "]" );
        }
        return true;
    }

    return false;
}

bool Parser::parseNewInitializer( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNewInitializer()" << endl;
    if( lex->lookAhead(0) != '(' )
        return false;

    lex->nextToken();
    parseCommaExpression();
    ADVANCE( ')', ")" );

    return true;
}

bool Parser::parseDeleteExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeleteExpression()" << endl;
    if( lex->lookAhead(0) == Token_scope && lex->lookAhead(1) == Token_delete )
        lex->nextToken();

    ADVANCE( Token_delete, "delete" );

    if( lex->lookAhead(0) == '[' ){
        lex->nextToken();
        ADVANCE( ']', "]" );
    }

    return parseCastExpression();
}

bool Parser::parseCastExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCastExpression()" << endl;

#if 0
    int index = lex->lookAhead( 0 );

    if( lex->lookAhead(0) == '(' ){
        lex->nextToken();
        if ( parseTypeId() ) {
            if ( lex->lookAhead(0) == '(' ) {
                lex->nextToken();
                return parseCastExpression();
            }
        }
    }

    lex->setIndex( index );
#endif
    return parseUnaryExpression();
}

bool Parser::parsePmExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser:parsePmExpression()" << endl;
    if( !parseCastExpression() )
        return false;

    while( lex->lookAhead(0) == Token_ptrmem ){
        lex->nextToken();
        if( !parseCastExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseMultiplicativeExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMultiplicativeExpression()" << endl;
    if( !parsePmExpression() )
        return false;

    while( lex->lookAhead(0) == '*' || lex->lookAhead(0) == '/' || lex->lookAhead(0) == '/' ){
        lex->nextToken();

        if( !parsePmExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}


bool Parser::parseAdditiveExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAdditiveExpression()" << endl;
    if( !parseMultiplicativeExpression() )
        return false;

    while( lex->lookAhead(0) == '+' || lex->lookAhead(0) == '-' ){
        lex->nextToken();

        if( !parseMultiplicativeExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseShiftExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseShiftExpression()" << endl;
    if( !parseAdditiveExpression() )
        return false;

    while( lex->lookAhead(0) == Token_shift ){
        lex->nextToken();

        if( !parseAdditiveExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseRelationalExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseRelationalExpression()" << endl;
    if( !parseShiftExpression() )
        return false;

    while( lex->lookAhead(0) == '<' || lex->lookAhead(0) == '>' ||
           lex->lookAhead(0) == Token_leq || lex->lookAhead(0) == Token_geq ){
        lex->nextToken();

        if( !parseShiftExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseEqualityExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseEqualityExpression()" << endl;
    if( !parseRelationalExpression() )
        return false;

    while( lex->lookAhead(0) == Token_eq || lex->lookAhead(0) == Token_not_eq ){
        lex->nextToken();

        if( !parseRelationalExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseAndExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAndExpression()" << endl;
    if( !parseEqualityExpression() )
        return false;

    while( lex->lookAhead(0) == '&' ){
        lex->nextToken();

        if( !parseEqualityExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseExclusiveOrExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseExclusiveOrExpression()" << endl;
    if( !parseAndExpression() )
        return false;

    while( lex->lookAhead(0) == '^' ){
        lex->nextToken();

        if( !parseAndExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseInclusiveOrExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInclusiveOrExpression()" << endl;
    if( !parseExclusiveOrExpression() )
        return false;

    while( lex->lookAhead(0) == '|' ){
        lex->nextToken();

        if( !parseExclusiveOrExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseLogicalAndExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLogicalAndExpression()" << endl;
    if( !parseInclusiveOrExpression() )
        return false;

    while( lex->lookAhead(0) == Token_and ){
        lex->nextToken();

        if( !parseInclusiveOrExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseLogicalOrExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLogicalOrExpression()" << endl;
    if( !parseLogicalAndExpression() )
        return false;

    while( lex->lookAhead(0) == Token_or ){
        lex->nextToken();

        if( !parseLogicalAndExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseConditionalExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseConditionalExpression()" << endl;
    if( !parseLogicalOrExpression() )
        return false;

    if( lex->lookAhead(0) == '?' ){
        lex->nextToken();
        parseExpression();
        ADVANCE( ':', ":" );
        parseAssignmentExpression();
    }

    return true;
}

bool Parser::parseAssignmentExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAssignmentExpression()" << endl;
    if( lex->lookAhead(0) == Token_throw )
        parseThrowExpression();
    else if( !parseConditionalExpression() )
        return false;

    while( lex->lookAhead(0) == Token_assign || lex->lookAhead(0) == '=' ){
        lex->nextToken();

        if( !parseConditionalExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseConstantExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseConstantExpression()" << endl;
    return parseConditionalExpression();
}

bool Parser::parseExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseExpression()" << endl;

    return parseCommaExpression();
}

bool Parser::parseCommaExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCommaExpression()" << endl;
    if( !parseAssignmentExpression() )
        return false;

    while( lex->lookAhead(0) == ',' ){
        lex->nextToken();

        if( !parseAssignmentExpression() ){
            syntaxError();
            return false;
        }
    }

    return true;
}

bool Parser::parseThrowExpression( AST::Ptr& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseThrowExpression()" << endl;
    if( lex->lookAhead(0) != Token_throw )
        return false;

    ADVANCE( Token_throw, "throw" );
    parseAssignmentExpression();

    return true;
}


#endif
