/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
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

#define UPDATE_POS(node, start, end) \
{ \
   int line, col; \
   (lex)->tokenAt(start).getStartPosition( &line, &col ); \
   (node)->setStartPosition( line, col ); \
   (lex)->tokenAt( end!=start ? end-1 : end ).getEndPosition( &line, &col ); \
   (node)->setEndPosition( line, col ); \
}

struct ParserPrivateData
{
    ParserPrivateData()
        {}
};


Parser::Parser( Driver* drv, Lexer* lexer )
    : driver( drv ),
      lex( lexer )
{
    d = new ParserPrivateData();
    m_fileName = "<stdin>";
    
    m_maxProblems = 5;
    m_problems.clear();
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
    if( (int)m_problems.size() < m_maxProblems ){
	int line=0, col=0;
	const Token& token = lex->lookAhead( 0 );
	lex->getTokenPosition( token, &line, &col );
	
	QString s = lex->lookAhead( 0 ).toString();
	s = s.left( 30 ).stripWhiteSpace();
	if( s.isEmpty() )
	    s = i18n( "<eof>" );
	
	m_problems << Problem( err.text.arg(s), line, col );
    }
        
    return true;
}

bool Parser::reportError( const QString& msg )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::reportError()" << endl;
    if( (int)m_problems.size() < m_maxProblems ){
	int line=0, col=0;
	const Token& token = lex->lookAhead( 0 );
	lex->getTokenPosition( token, &line, &col );

	m_problems << Problem( msg, line, col );
    }
    
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

bool Parser::parseName( NameAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseName()" << endl;

    int start = lex->index();

    NestedNameSpecifierAST::Node nestedName;
    AST::Node unqualifedName;
    bool isGlobal = false;
                
    if( lex->lookAhead(0) == Token_scope ){
        isGlobal = true;
	lex->nextToken();
    }
    
    bool hasNestedName = parseNestedNameSpecifier( nestedName );
    if( parseUnqualifiedName(unqualifedName) ){
        //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "----------> name parsed!!" << endl;
	
	NameAST::Node ast = CreateNode<NameAST>();
	node = ast;
	node->setGlobal( isGlobal );
	if( hasNestedName )
	    node->setNestedName( nestedName );
	node->setUnqualifedName( unqualifedName );
	node->setText( toString(start, lex->index()) );
	UPDATE_POS( node, start, lex->index() );
	
	return true;
    }
    
    return false;
}

bool Parser::parseTranslationUnit( TranslationUnitAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTranslationUnit()" << endl;
 
    int start = lex->index();
    
    m_problems.clear();
    
    TranslationUnitAST::Node tun = CreateNode<TranslationUnitAST>();
    node = tun;
    while( !lex->lookAhead(0).isNull() ){
        DeclarationAST::Node def;
        if( !parseDefinition(def) ){
	    // error recovery
	    skipUntilDeclaration();
	}
	node->addDeclaration( def );
    }
 
    UPDATE_POS( node, start, lex->index() );
    return m_problems.size() == 0;
}

bool Parser::parseDefinition( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDefinition()" << endl;
    
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
	    int start = lex->index();
	    TypeSpecifierAST::Node spec;
	    InitDeclaratorListAST::Node declarators;
	    AST::Node declarator;
	    
	    if( parseEnumSpecifier(spec) || parseClassSpecifier(spec) ){
	        parseInitDeclaratorList(declarators);
	        ADVANCE( ';', ";" );
		
		SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
		ast->setTypeSpec( spec );
		ast->setInitDeclaratorList( declarators );
		UPDATE_POS( ast, start, lex->index() );
		node = ast;
				
	        return true;
	    }
	
	    return parseDeclaration( node );
	}
	
    } // end switch
}

bool Parser::parseLinkageSpecification( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLinkageSpecification()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != Token_extern ){
	return false;
    }
    lex->nextToken();
    
    QString type;
    if( lex->lookAhead(0) == Token_string_literal ){
	type = lex->lookAhead( 0 ).toString();
	lex->nextToken();
    }
    
    LinkageSpecificationAST::Node ast = CreateNode<LinkageSpecificationAST>();
    
    ast->setExternType( type );
    
    if( lex->lookAhead(0) == '{' ){
        LinkageBodyAST::Node linkageBody;
	parseLinkageBody( linkageBody );	
	ast->setLinkageBody( linkageBody );
    } else {
        DeclarationAST::Node decl;
	if( !parseDefinition(decl) ){
	    reportError( i18n("Declaration syntax error") );
	}
	ast->setDeclaration( decl );
    }
    
    UPDATE_POS( ast, start, lex->index() );

    node = ast;
    
    return true;
}

bool Parser::parseLinkageBody( LinkageBodyAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLinkageBody()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
    
    LinkageBodyAST::Node lba = CreateNode<LinkageBodyAST>();
    node = lba;
        
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	
	if( tk == '}' )
	    break;
	
	DeclarationAST::Node def;
	if( parseDefinition(def) ){
	    node->addDeclaration( def );
	} else {
	    // error recovery
	    skipUntilDeclaration();
	}
    }
    
    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} expected") );
    } else
	lex->nextToken();
    
    UPDATE_POS( node, start, lex->index() );
    return true;
}

bool Parser::parseNamespace( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNamespace()" << endl;
    
    int start = lex->index();
    
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
		
	NameAST::Node name;
	if( parseName(name) ){	    
	    ADVANCE( ';', ";" );	    
	
	    NamespaceAliasAST::Node ast = CreateNode<NamespaceAliasAST>();
	    ast->setNamespaceName( namespaceName );
	    ast->setAliasName( name );    
	    UPDATE_POS( ast, start, lex->index() );
	    node = ast;
	    return true;
	} else {
	    reportError( i18n("namespace expected") );
	    return false;
	}
    } else if( lex->lookAhead(0) != '{' ){
	reportError( i18n("{ expected") );
	return false;
    }

    NamespaceAST::Node ast = CreateNode<NamespaceAST>();
    ast->setNamespaceName( namespaceName );
        
    LinkageBodyAST::Node linkageBody;
    parseLinkageBody( linkageBody );
    
    ast->setLinkageBody( linkageBody );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseUsing( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseUsing()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != Token_using ){
	return false;
    }
    lex->nextToken();
        
    if( lex->lookAhead(0) == Token_namespace ){
	if( !parseUsingDirective(node) ){
	    return false;
	}
	UPDATE_POS( node, start, lex->index() );
	return true;
    }
    
    bool isTypename = false;
    if( lex->lookAhead(0) == Token_typename ){
        isTypename = true;
	lex->nextToken();
    }
    
    NameAST::Node name;
    if( !parseName(name) )
	return false;
	
    UsingAST::Node ast = CreateNode<UsingAST>();
    ast->setTypename( true );
    ast->setName( name );
        
    ADVANCE( ';', ";" );
    
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseUsingDirective( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseUsingDirective()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != Token_namespace ){
	return false;
    }
    lex->nextToken();
    
    NameAST::Node name;
    if( !parseName(name) ){
	reportError( i18n("Namespace name expected") );
	return false;
    }
    
    ADVANCE( ';', ";" );
    
    UsingDirectiveAST::Node ast = CreateNode<UsingDirectiveAST>();
    ast->setName( name );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
        
    return true;
}


bool Parser::parseOperatorFunctionId( AST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseOperatorFunctionId()" << endl;
    
    int start = lex->index();

    if( lex->lookAhead(0) != Token_operator ){
	return false;
    }
    lex->nextToken();    

    AST::Node op;    
    if( parseOperator(op) ){
	AST::Node asn = CreateNode<AST>();
	node = asn;
	UPDATE_POS( node, start, lex->index() );
	return true;
    } else {
	// parse cast operator
	AST::Node cv;
        parseCvQualify(cv);       
	
	TypeSpecifierAST::Node spec;
	if( !parseSimpleTypeSpecifier(spec) ){
	    parseError();
	}
	
	AST::Node cv2;
	parseCvQualify(cv2);
	
	AST::Node ptrOp;
	while( parsePtrOperator(ptrOp) )
  	    ;	    
	
	AST::Node asn = CreateNode<AST>();
	node = asn;
	UPDATE_POS( node, start, lex->index() );
	return true;
    }
}

bool Parser::parseTemplateArgumentList( TemplateArgumentListAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateArgumentList()" << endl;
    
    int start = lex->index();
    
    TemplateArgumentListAST::Node taln = CreateNode<TemplateArgumentListAST>();
    node = taln;
    
    AST::Node templArg;
    if( !parseTemplateArgument(templArg) )
	return false;
    node->addArgument( templArg );
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseTemplateArgument(templArg) ){
	    parseError();
	    break;
	}
	node->addArgument( templArg );
    }
    
    UPDATE_POS( node, start, lex->index() );
    
    return true;
}

bool Parser::parseTypedef( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypedef()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != Token_typedef ){
	return false;
    }
    lex->nextToken();
    
    TypeSpecifierAST::Node spec;
    if( !parseTypeSpecifierOrClassSpec(spec) ){
	reportError( i18n("Need a type specifier to declare") );
	return false;
    }
    
    InitDeclaratorListAST::Node declarators;
    if( !parseInitDeclaratorList(declarators) ){
	reportError( i18n("Need an identifier to declare") );
	return false;
    }
    
    ADVANCE( ';', ";" );
    
    TypedefAST::Node ast = CreateNode<TypedefAST>();
    ast->setTypeSpec( spec );
    ast->setInitDeclaratorList( declarators );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
        
    return true;
}

bool Parser::parseAsmDefinition( DeclarationAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAsmDefinition()" << endl;
    
    ADVANCE( Token_asm, "asm" );
    ADVANCE( '(', '(' );
    
    AST::Node lit;
    parseStringLiteral( lit );
    
    ADVANCE( ')', ')' );
    ADVANCE( ';', ';' );
    
    return true;
}

bool Parser::parseTemplateDeclaration( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateDeclaration()" << endl;
    
    int start = lex->index();
    
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

    AST::Node params;
    if( lex->lookAhead(0) == '<' ){
	lex->nextToken();
	parseTemplateParameterList( params );
	
	ADVANCE( '>', ">" );
    }
    
    DeclarationAST::Node def;
    if( !parseDefinition(def) ){
	reportError( i18n("expected a declaration") );
    }
    
    TemplateDeclarationAST::Node ast = CreateNode<TemplateDeclarationAST>();
    ast->setExport( _export );
    ast->setTemplateParameterList( params );
    ast->setDeclaration( def );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
        
    return true;
}

bool Parser::parseOperator( AST::Node& /*node*/ )
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

bool Parser::parseCvQualify( AST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCvQualify()" << endl;
    
    int start = lex->index();
        
    int n = 0;
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	if( tk == Token_const || tk == Token_volatile ){
	    ++n;
	    lex->nextToken();
	} else
	    break;
    }
    
    AST::Node asn = CreateNode<AST>();
    node = asn;
    UPDATE_POS( node, start, lex->index() );
    
    return n != 0;
}

bool Parser::parseSimpleTypeSpecifier( TypeSpecifierAST::Node& /*node*/ )
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
    NameAST::Node name;
    return parseName( name );
}

bool Parser::parsePtrOperator( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parsePtrOperator()" << endl;
    
    if( lex->lookAhead(0) == '&' ){
	lex->nextToken();
    } else if( lex->lookAhead(0) == '*' ){
	lex->nextToken();
    } else {
	int index = lex->index();
	AST::Node memPtr;
	if( !parsePtrToMember(memPtr) ){
	    lex->setIndex( index );
	    return false;
	}
    }
    
    AST::Node cv;
    parseCvQualify( cv );
    
    return true;
}


bool Parser::parseTemplateArgument( AST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateArgument()" << endl;
    
#if 0
    if( parseTypeId() ){
	qWarning( "token = %s", lex->lookAhead(0).toString().latin1() );
	return true;
    }
#endif
    int start = lex->index();
        
    if( !skipAssignmentExpression() ){
        return false;
    }
    
    AST::Node asn = CreateNode<AST>();
    node = asn;
    UPDATE_POS( node, start, lex->index() );
    
    return true;
}

bool Parser::parseTypeSpecifier( TypeSpecifierAST::Node& spec )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypeSpecifier()" << endl;

    AST::Node cv;    
    parseCvQualify( cv );
    
    if( parseElaboratedTypeSpecifier(spec) || parseSimpleTypeSpecifier(spec) ){
        AST::Node cv2;
	parseCvQualify( cv2 );
	return true;
    } 
    
    return false;
}

bool Parser::parseDeclarator( DeclaratorAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclarator()" << endl;

    int start = lex->index();
    
    DeclaratorAST::Node ast = CreateNode<DeclaratorAST>();
    
    DeclaratorAST::Node decl;
    NameAST::Node declId;
    
    AST::Node ptrOp;
    while( parsePtrOperator(ptrOp) ){
	ast->addPtrOp( ptrOp );
    }
    
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();

	if( !parseDeclarator(decl) ){
	    return false;
	}	
	ast->setSubDeclarator( decl );
	
	if( lex->lookAhead(0) != ')'){
	    return false;
	}
	lex->nextToken();
    } else {
	
	if( !parseDeclaratorId(declId) ){
	    return false;
	}	
	ast->setDeclaratorId( declId );
	
	if( lex->lookAhead(0) == ':' ){
	    lex->nextToken();
	    if( !skipConstantExpression() ){
		reportError( i18n("Constant expression expected") );
	    }
	    goto update_node;
	}
    }
    
    {
    
    while( lex->lookAhead(0) == '[' ){
        int startArray = lex->index();
	lex->nextToken();
	skipCommaExpression();
	
	ADVANCE( ']', "]" );
	AST::Node array = CreateNode<AST>();
	UPDATE_POS( array, startArray, lex->index() );
	ast->addArrayDimension( array );
    }
        
    bool skipParen = false;
    if( lex->lookAhead(0) == Token_identifier && lex->lookAhead(1) == '(' && lex->lookAhead(2) == '(' ){
    	lex->nextToken();
	lex->nextToken();
	skipParen = true;
    }
    
    int index = lex->index();
    if( lex->lookAhead(0) == '(' ){     
	lex->nextToken();
	
	AST::Node params;
	if( !parseParameterDeclarationClause(params) ){
	    lex->setIndex( index );
	    goto update_node;
	}
	ast->setParameterDeclarationClause( params );
	
	if( lex->lookAhead(0) != ')' ){
	    lex->setIndex( index );
	    goto update_node;
	} else
	    lex->nextToken();
	
	AST::Node cv;
	parseCvQualify( cv );
	
	AST::Node except;
	if( parseExceptionSpecification(except) ){
	    ast->setExceptionSpecification( except );
	}
    }
    
    if( skipParen ){
    	if( lex->lookAhead(0) != ')' ){
	    reportError( i18n("')' expected") );
	} else
	    lex->nextToken();
    }
       
    } 
    
update_node:
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseEnumSpecifier( TypeSpecifierAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseEnumSpecifier()" << endl;
    
    int start = lex->index();
    
    AST::Node storageSpec;
    while( parseStorageClassSpecifier(storageSpec) )
	;


    AST::Node cv;
    parseCvQualify( cv );
    
    if( lex->lookAhead(0) != Token_enum ){
	return false;
    }

    lex->nextToken();
    
    if( lex->lookAhead(0) == Token_identifier ){
	lex->nextToken();
    }
    
    if( lex->lookAhead(0) != '{' ){
	lex->setIndex( start );
	return false;
    }
    lex->nextToken();
    
    EnumSpecifierAST::Node ast = CreateNode<EnumSpecifierAST>();
            
    EnumeratorAST::Node enumerator;
    if( parseEnumerator(enumerator) ){
        ast->addEnumerator( enumerator );
    
        while( lex->lookAhead(0) == ',' ){
	    lex->nextToken();
	
	    if( !parseEnumerator(enumerator) ){
	        //reportError( i18n("Enumerator expected") );
	        break;
	    }
	
	    ast->addEnumerator( enumerator );
        }
    }
    
    if( lex->lookAhead(0) != '}' )
	reportError( i18n("} missing") );
    else
	lex->nextToken();
    
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseTemplateParameterList( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTemplateParameterList()" << endl;
    
    AST::Node param;
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

bool Parser::parseTemplateParameter( AST::Node& node )
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

bool Parser::parseTypeParameter( AST::Node& /*node*/ )
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
		    
		    AST::Node typeId;
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
		    
		    AST::Node typeId;
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
	    
	    AST::Node params;
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

bool Parser::parseStorageClassSpecifier( AST::Node& /*node*/ )
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

bool Parser::parseFunctionSpecifier( AST::Node& /*node*/ )
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

bool Parser::parseTypeId( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypeId()" << endl;
    
    TypeSpecifierAST::Node spec;
    if( !parseTypeSpecifier(spec) ){
	return false;
    }
    
    AST::Node decl;
    parseAbstractDeclarator( decl );
    
    return true;
}

bool Parser::parseAbstractDeclarator( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAbstractDeclarator()" << endl;
    
    AST::Node ptrOp;
    while( parsePtrOperator(ptrOp) )
       ;
    
    if( lex->lookAhead(0) == '(' ){
	lex->nextToken();
	
	DeclaratorAST::Node decl;
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
	
	AST::Node param;
	if( !parseParameterDeclarationClause(param) ){
	    lex->setIndex( index );
	    return true;
	}
	
	ADVANCE( ')', ")" );	
	
	AST::Node cv, except;
	
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


bool Parser::parseInitDeclaratorList( InitDeclaratorListAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitDeclaratorList()" << endl;
  
    int start = lex->index();
    
    InitDeclaratorListAST::Node ast = CreateNode<InitDeclaratorListAST>();
    InitDeclaratorAST::Node decl;
    
    if( !parseInitDeclarator(decl) ){
	return false;
    }
    ast->addInitDeclarator( decl );
    
    while( lex->lookAhead(0) == ',' ){
	lex->nextToken();
	
	if( !parseInitDeclarator(decl) ){
	    parseError();
	    break;
	}
	ast->addInitDeclarator( decl );
    }
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitDeclaratorList() -- end" << endl;
    
    UPDATE_POS( ast, start, lex->index() );
    ast->setText( toString(start, lex->index()) );
    node = ast;
    
    return true;
}

bool Parser::parseParameterDeclarationClause( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseParameterDeclarationClause()" << endl;
    
    AST::Node params;
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

bool Parser::parseParameterDeclarationList( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseParameterDeclarationList()" << endl;
    
    int index = lex->index();
    
    AST::Node param;
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

bool Parser::parseParameterDeclaration( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseParameterDeclaration()" << endl;
    
    int index = lex->index();
    
    // parse decl spec
    
    TypeSpecifierAST::Node spec;
    if( !parseTypeSpecifier(spec) ){
	lex->setIndex( index );
	return false;
    }
    
    index = lex->index();
    
    DeclaratorAST::Node decl;
    if( !parseDeclarator(decl) ){
	lex->setIndex( index );
	//removed parseAbstractDeclarator(decl);
    }
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	if( !skipAssignmentExpression() ){
	    //reportError( i18n("Expression expected") );
	}
    }
    
    return true;
}

bool Parser::parseClassSpecifier( TypeSpecifierAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseClassSpecifier()" << endl;
    
    int start = lex->index();

    AST::Node storageSpec;    
    while( parseStorageClassSpecifier(storageSpec) )
	;   

    AST::Node cv;
    parseCvQualify( cv );

    AST::Node classKey;
    int classKeyStart = lex->index();
    
    int kind = lex->lookAhead( 0 );
    if( kind == Token_class || kind == Token_struct || kind == Token_union ){
	AST::Node asn = CreateNode<AST>();
        classKey = asn;
	lex->nextToken();
	UPDATE_POS( classKey, classKeyStart, lex->index() );
    } else {
	return false;
    }

    while( lex->lookAhead(0) == Token_identifier && lex->lookAhead(1) == Token_identifier )
    	lex->nextToken();
	
    NameAST::Node name;
    parseName( name );
    
    BaseClauseAST::Node bases;
    if( lex->lookAhead(0) == ':' ){      
	if( !parseBaseClause(bases) ){
	    skipUntil( '{' );
	}
    }

    if( lex->lookAhead(0) != '{' ){
	lex->setIndex( start );
	return false;
    }

    ADVANCE( '{', '{' );

    ClassSpecifierAST::Node ast = CreateNode<ClassSpecifierAST>();
    ast->setClassKey( classKey );
    ast->setName( name );
    //ast->setBaseClause( bases );

    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;
	
	DeclarationAST::Node memSpec;
	if( !parseMemberSpecification(memSpec) ){
	    skipUntilDeclaration();
	} else
	    ast->addDeclaration( memSpec );
    }
    
    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} missing") );
    } else
	lex->nextToken();
    	
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseAccessSpecifier( AST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseAccessSpecifier()" << endl;

    int start = lex->index();    
    
    switch( lex->lookAhead(0) ){
    case Token_public:
    case Token_protected:
    case Token_private: {
        AST::Node asn = CreateNode<AST>();
	node = asn;
	lex->nextToken();
	UPDATE_POS( node, start, lex->index() );
	return true;
        }
    }
    
    return false;
}

bool Parser::parseMemberSpecification( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMemberSpecification()" << endl;

    AST::Node access;
        
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
    } else if( parseAccessSpecifier(access) ){
	if( lex->lookAhead(0) == Token_slots ){
	    lex->nextToken();
	}
	ADVANCE( ':', ":" );
	return true;
    }

    int start = lex->index();
    
    TypeSpecifierAST::Node spec;
    if( parseEnumSpecifier(spec) || parseClassSpecifier(spec) ){
    	InitDeclaratorListAST::Node declarators;
	parseInitDeclaratorList( declarators );
	ADVANCE( ';', ";" );
	
	SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
	ast->setTypeSpec( spec );
	ast->setInitDeclaratorList( declarators );
	UPDATE_POS( ast, start, lex->index() );
	node = ast;
	
	return true;
    }
    
    return parseDeclaration( node );
}

bool Parser::parseCtorInitializer( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCtorInitializer()" << endl;
    
    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    lex->nextToken();
    
    AST::Node inits;
    if( !parseMemInitializerList(inits) ){
	reportError( i18n("Member initializers expected") );
    }
    
    return true;
}

bool Parser::parseElaboratedTypeSpecifier( TypeSpecifierAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseElaboratedTypeSpecifier()" << endl;
    
    int start = lex->index();
    
    int tk = lex->lookAhead( 0 );
    if( tk == Token_class  ||
	tk == Token_struct ||
	tk == Token_union  ||
	tk == Token_enum   ||
	tk == Token_typename )
    {
        AST::Node kind = CreateNode<AST>();	
	lex->nextToken();
	UPDATE_POS( kind, start, lex->index() );	
	
	NameAST::Node name;
	
	if( parseName(name) ){
	    ElaboratedTypeSpecifierAST::Node ast = CreateNode<ElaboratedTypeSpecifierAST>();
	    ast->setKind( kind );
	    ast->setName( name );
	    UPDATE_POS( ast, start, lex->index() );
	    node = ast;
	    
	    return true;
	}
    }
    
    lex->setIndex( start );
    return false;
}

bool Parser::parseDeclaratorId( NameAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclaratorId()" << endl;
    return parseName( node );
}

bool Parser::parseExceptionSpecification( AST::Node& node )
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

bool Parser::parseEnumerator( EnumeratorAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseEnumerator()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != Token_identifier ){
	return false;
    }
    lex->nextToken();
    
    EnumeratorAST::Node ena = CreateNode<EnumeratorAST>();
    node = ena;
    
    AST::Node id = CreateNode<AST>();
    UPDATE_POS( id, start, lex->index() );
    id->setText( toString(start, lex->index()) );
    node->setId( id );
            
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();

	AST::Node expr = CreateNode<AST>();	
	int startExpr = lex->index();
	
	if( !skipExpression() ){
	    reportError( i18n("Constant expression expected") );
	}
	UPDATE_POS( expr, startExpr, lex->index() );
	expr->setText( toString(startExpr, lex->index()) );
	node->setExpr( expr );
    }
    
    UPDATE_POS( node, start, lex->index() );
    
    return true;
}

bool Parser::parseInitDeclarator( InitDeclaratorAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitDeclarator()" << endl;
    
    DeclaratorAST::Node decl;
    AST::Node init;
    if( !parseDeclarator(decl) ){
	return false;
    }
        
    parseInitializer( init );
    
    return true;
}

bool Parser::skipAssignmentExpression()
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::skipAssignmentExpression()" << endl;
        
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

bool Parser::parseBaseClause( BaseClauseAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseBaseClause()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    lex->nextToken();
    
    BaseClauseAST::Node bca = CreateNode<BaseClauseAST>();
    node = bca;
    
    BaseSpecifierAST::Node baseSpec;
    if( parseBaseSpecifier(baseSpec) ){
        node->addBaseSpecifier( baseSpec );
	
        while( lex->lookAhead(0) == ',' ){
	    lex->nextToken();
	
	    if( !parseBaseSpecifier(baseSpec) ){
	        reportError( i18n("Base class specifier expected") );
	        return false;
	    }
	    node->addBaseSpecifier( baseSpec );
        }
    } else
        return false;
    
    UPDATE_POS( node, start, lex->index() );
    
    return true;
}

bool Parser::parseInitializer( AST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitializer()" << endl;
    
    if( lex->lookAhead(0) == '=' ){
	lex->nextToken();
	
	AST::Node init;
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

bool Parser::parseMemInitializerList( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMemInitializerList()" << endl;
    
    AST::Node init;
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

bool Parser::parseMemInitializer( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseMemInitializer()" << endl;
    
    NameAST::Node initId;
    if( !parseMemInitializerId(initId) ){
	reportError( i18n("Identifier expected") );
	return false;
    }
    ADVANCE( '(', '(' );
    skipCommaExpression();
    ADVANCE( ')', ')' );
    
    return true;
}

bool Parser::parseTypeIdList( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTypeIdList()" << endl;
    
    AST::Node typeId;
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

bool Parser::parseBaseSpecifier( BaseSpecifierAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseBaseSpecifier()" << endl;
    AST::Node access;
    
    if( lex->lookAhead(0) == Token_virtual ){
	lex->nextToken();
	
	parseAccessSpecifier( access );
    } else {	
        parseAccessSpecifier( access );
	
	if( lex->lookAhead(0) == Token_virtual ){
	    lex->nextToken();
	}
    }
        
    NameAST::Node name;
    if( !parseName(name) ){
	reportError( i18n("Identifier expected") );
    }

    return true;
}


bool Parser::parseInitializerClause( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseInitializerClause()" << endl;
        
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

bool Parser::parseMemInitializerId( NameAST::Node& node )
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

// nested-name-specifier
//   class-or-namespace-name "::" nested-name-specifier-opt
//   class-or-namespace-name "::" "template"-opt unqualified-id
bool Parser::parseNestedNameSpecifier( NestedNameSpecifierAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNestedNameSpecifier()" << endl;

    int start = lex->index();
    bool ok = false;
    
    NestedNameSpecifierAST::Node nns = CreateNode<NestedNameSpecifierAST>();
    
    int startId = start;
    while( lex->lookAhead(0) == Token_identifier ){
	
	startId = lex->index();
	
	ClassOrNamespaceNameAST::Node classOrNamespaceName = CreateNode<ClassOrNamespaceNameAST>();
	
	if( lex->lookAhead(1) == '<' ){
	    lex->nextToken(); // skip template name
	    lex->nextToken(); // skip <
	    
	    TemplateArgumentListAST::Node args;
	    if( !parseTemplateArgumentList(args) ){
		lex->setIndex( startId );
		return false;
	    }
	    	    
	    if( lex->lookAhead(0) != '>' ){
		lex->setIndex( startId );
		return false;
	    }
	    
	    lex->nextToken(); // skip >
	    
	    args->setText( toString(startId, lex->index()) );
	    UPDATE_POS( args, startId, lex->index() );
	    
	    classOrNamespaceName->setTemplateArgumentList( args );
	    
	    if ( lex->lookAhead(0) == Token_scope ) {
	        
	    	UPDATE_POS( classOrNamespaceName, startId, lex->index() );	
		
		lex->nextToken();
		ok = true;
		
		nns->addClassOrNamespaceName( classOrNamespaceName );
		
	    } else {
		lex->setIndex( startId );
		break;
	    }
	    
	} else if( lex->lookAhead(1) == Token_scope ){
	    lex->nextToken(); // skip name
	    
	    classOrNamespaceName->setText( toString(startId, lex->index()) );
	    UPDATE_POS( classOrNamespaceName, startId, lex->index() );	
	    nns->addClassOrNamespaceName( classOrNamespaceName );
	    
	    lex->nextToken(); // skip ::
	    if( lex->lookAhead(0) == Token_template && lex->lookAhead(1) == Token_identifier ){
		lex->nextToken(); // skip optional template keyword
	    }
	    ok = true;
	    
	} else
	    break;	    
    }
    
    if ( !ok ) {
	lex->setIndex( startId );
	return false;
    }
    
    node = nns;
    UPDATE_POS( node, start, lex->index() );
    node->setText( toString(start,lex->index()-1) );
    
    return true;
}

bool Parser::parsePtrToMember( AST::Node& /*node*/ )
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

bool Parser::parseUnqualifiedName( AST::Node& node )
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
	    TemplateArgumentListAST::Node args;
	    parseTemplateArgumentList( args );
	    
	    if( lex->lookAhead(0) != '>' ){
		lex->setIndex( index );
	    } else
		lex->nextToken();
	}
    }
    
    return true;
}

bool Parser::parseStringLiteral( AST::Node& /*node*/ )
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

bool Parser::parseStatement( StatementAST::Node& node ) // thanks to fiore@8080.it ;)
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

bool Parser::parseCondition( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCondition()" << endl;
    
    int index = lex->index();
    
    TypeSpecifierAST::Node spec;
    if( parseTypeSpecifier(spec) ){
	
    	DeclaratorAST::Node decl;
	if( parseDeclarator(decl) && lex->lookAhead(0) == '=' ) {
	    lex->nextToken();
	    
	    if( skipAssignmentExpression() )
		return true;
	}
    }
    
    lex->setIndex( index );
    return skipCommaExpression();
}


bool Parser::parseWhileStatement( StatementAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseWhileStatement()" << endl;
    int start = lex->index();
    
    ADVANCE( Token_while, "while" );
    ADVANCE( '(' , "(" );
    
    AST::Node cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    StatementAST::Node body;
    if( !parseStatement(body) ){
	reportError( i18n("statement expected") );
	return false;
    }
    
    WhileStatementAST::Node ast = CreateNode<WhileStatementAST>();
    ast->setCondition( cond );
    ast->setStatement( body );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseDoStatement( StatementAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDoStatement()" << endl;
    int start = lex->index();
    
    ADVANCE( Token_do, "do" );
    
    StatementAST::Node body;
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
    
    DoStatementAST::Node ast = CreateNode<DoStatementAST>();
    ast->setStatement( body );
    //ast->setCondition( condition );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseForStatement( StatementAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseForStatement()" << endl;
    ADVANCE( Token_for, "for" );
    ADVANCE( '(', "(" );
        
    StatementAST::Node init;
    if( !parseForInitStatement(init) ){
	reportError( i18n("for initialization expected") );
	return false;
    }
    
    AST::Node cond;
    parseCondition( cond );
    ADVANCE( ';', ";" );
    
    skipCommaExpression();
    ADVANCE( ')', ")" );
    
    StatementAST::Node body;
    if( parseStatement(body) ){
        return true;
    }
    
    return false;
}

bool Parser::parseForInitStatement( StatementAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseForInitStatement()" << endl;
    
    if ( parseDeclarationStatement(node) )
	return true;
	
    return skipExpressionStatement();
}

bool Parser::parseCompoundStatement( StatementAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseCompoundStatement()" << endl;
    int start = lex->index();
    
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
    
    StatementListAST::Node ast = CreateNode<StatementListAST>();
        
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;
	
	StatementAST::Node stmt;
	if( !parseStatement(stmt) ){
	    skipUntilStatement();
	} else {
	    ast->addStatement( stmt );
	}
    }
    
    ADVANCE( '}', "}" );
    
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseIfStatement( StatementAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseIfStatement()" << endl;
    
    int start = lex->index();
    
    ADVANCE( Token_if, "if" );
    
    ADVANCE( '(' , "(" );
 
    IfStatementAST::Node ast = CreateNode<IfStatementAST>();
    
    AST::Node cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    StatementAST::Node stmt;
    if( !parseStatement(stmt) ){
	reportError( i18n("statement expected") );
	return false;
    }
  
    ast->setCondition( cond );
    ast->setStatement( stmt );
    
    if( lex->lookAhead(0) == Token_else ){
	lex->nextToken();
	StatementAST::Node elseStmt;
	if( !parseStatement(elseStmt) ) {
	    reportError( i18n("statement expected") );
	    return false;
	}
	ast->setElseStatement( elseStmt );
    }
        
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseSwitchStatement( StatementAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseSwitchStatement()" << endl;
    ADVANCE( Token_switch, "switch" );
    
    ADVANCE( '(' , "(" );
        
    AST::Node cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );
    
    StatementAST::Node stmt;
    if( !parseCompoundStatement(stmt) ){
	syntaxError();
	return false;
    }
    return true;
}

bool Parser::parseLabeledStatement( StatementAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseLabeledStatement()" << endl;
    switch( lex->lookAhead(0) ){
    case Token_identifier:
    case Token_default:
	if( lex->lookAhead(1) == ':' ){
	    lex->nextToken();
	    lex->nextToken();
	    
	    StatementAST::Node stmt;
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
	
	StatementAST::Node stmt;
	if( parseStatement(stmt) ){
	    return true;
	}
	break;
	
    }
    
    return false;
}

bool Parser::parseBlockDeclaration( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseBlockDeclaration()" << endl;
    switch( lex->lookAhead(0) ) {
    case Token_typedef:
	return parseTypedef( node );
    case Token_using:
	return parseUsing( node );
    case Token_asm:
	return parseAsmDefinition( node );
    case Token_namespace:
	return parseNamespaceAliasDefinition( node );
    }
    
    int start = lex->index();
    
    AST::Node storageSpec;
    while( parseStorageClassSpecifier(storageSpec) )
	;
        
    TypeSpecifierAST::Node spec;
    if ( !parseTypeSpecifierOrClassSpec(spec) ) { // replace with simpleTypeSpecifier?!?!
	lex->setIndex( start );
	return false;
    }
    
    InitDeclaratorListAST::Node declarators;
    parseInitDeclaratorList( declarators );
    
    SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
    ast->setTypeSpec( spec );
    ast->setInitDeclaratorList( declarators );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}

bool Parser::parseNamespaceAliasDefinition( DeclarationAST::Node& /*node*/ )
{
    if ( lex->lookAhead(0) != Token_namespace ) {
	return false;
    }
    lex->nextToken();

    ADVANCE( Token_identifier,  "identifier" );
    ADVANCE( '=', "=" );
    
    NameAST::Node name;
    if( !parseName(name) ){
	reportError( i18n("Namespace name expected") );
    }
    
    ADVANCE( ';', ";" );
        
    return true;
    
}

bool Parser::parseDeclarationStatement( StatementAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclarationStatement()" << endl;
    
    int index = lex->index();
    
    DeclarationAST::Node decl;
    if ( !parseBlockDeclaration(decl) )
	return false;
    
    if ( lex->lookAhead(0) != ';' ) {
	lex->setIndex( index );
	return false;
    }
    lex->nextToken();
    
    return true;
}

bool Parser::parseDeclaration( DeclarationAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseDeclaration()" << endl;

    int start = lex->index();
    
    AST::Node funSpec;
    while( parseFunctionSpecifier(funSpec) )
        ;
    
    AST::Node storageSpec;
    while(  parseStorageClassSpecifier(storageSpec) )
        ;
    
    AST::Node cv;
    parseCvQualify( cv );
        
    int index = lex->index();

    NameAST::Node name;
    if( parseName(name) && lex->lookAhead(0) == '(' ){
	// no type specifier, maybe a constructor or a cast operator??
	
	lex->setIndex( index );
	
	NestedNameSpecifierAST::Node nestedName;
	parseNestedNameSpecifier( nestedName );
	QString nestedNameText = toString( index, lex->index() );
	
	InitDeclaratorAST::Node decl;
	if( parseInitDeclarator(decl) ){
	    
	    int endSignature = lex->index();
	    
	    switch( lex->lookAhead(0) ){
	    case ';':
		if( !nestedNameText ){
		    lex->nextToken();
		    
		    FunctionDeclarationAST::Node ast = CreateNode<FunctionDeclarationAST>();
		    ast->setNestedName( nestedName );
		    ast->setText( toString(start, endSignature) );
		    node = ast;
		    UPDATE_POS( node, start, lex->index() );
		    return true;
		}
		break;
		
	    case ':':
	        {
		    AST::Node ctorInit;
		    StatementListAST::Node funBody;
		    if( parseCtorInitializer(ctorInit) && parseFunctionBody(funBody) ){
			FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
			ast->setNestedName( nestedName );
			ast->setInitDeclarator( decl );
			ast->setFunctionBody( funBody );
			ast->setText( toString(start, endSignature) );
			node = ast;
			UPDATE_POS( node, start, lex->index() );
		        return true;
		    }
		}
		break;
		
	    case '{':
	        {
		    StatementListAST::Node funBody;
		    if( parseFunctionBody(funBody) ){
			FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
			ast->setNestedName( nestedName );
			ast->setInitDeclarator( decl );
			ast->setText( toString(start, endSignature) );
			ast->setFunctionBody( funBody );
			node = ast;
			UPDATE_POS( node, start, lex->index() );
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
	InitDeclaratorListAST::Node declarators;
	if( parseInitDeclaratorList(declarators) ){
	    ADVANCE( ';', ";" );
	    DeclarationAST::Node ast = CreateNode<DeclarationAST>();
	    node = ast;
	    UPDATE_POS( node, start, lex->index() );
	    return true;
	}
	syntaxError();
	return false;
    }
        
    TypeSpecifierAST::Node spec;
    if( parseTypeSpecifier(spec) ){
	
	if( lex->lookAhead(0) == ';' ){
	    // type definition
	    lex->nextToken();
	    DeclarationAST::Node ast = CreateNode<DeclarationAST>();
	    node = ast;
	    UPDATE_POS( node, start, lex->index() );
	    return true;
	}
	
	NestedNameSpecifierAST::Node nestedName;
	InitDeclaratorListAST::Node declarators;
	
	if( parseNestedNameSpecifier(nestedName) ) {
	    // maybe a method declaration/definition
	    
	    InitDeclaratorAST::Node decl;
	    if ( !parseInitDeclarator(decl) ) {
		syntaxError();
		return false;
	    }
	} else if ( !parseInitDeclaratorList(declarators) ) {
	    syntaxError();
	    return false;
	}
	
	int endSignature = lex->index();
	switch( lex->lookAhead(0) ){
	case ';':
	    {
		lex->nextToken();
		FunctionDeclarationAST::Node ast = CreateNode<FunctionDeclarationAST>();
		ast->setNestedName( nestedName );
		ast->setText( toString(start, endSignature) );
		ast->setTypeSpec( spec );
		node = ast;
		UPDATE_POS( node, start, lex->index() );
	    }
	    return true;

	case '=':
	    {
	        AST::Node init;
	        if( parseInitializer(init) ){
		    FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
		    ast->setNestedName( nestedName );
		    ast->setText( toString(start, endSignature) );
		    ast->setTypeSpec( spec );
		    node = ast;
		    UPDATE_POS( node, start, lex->index() );
		    return true;
		}
	    }
	    break;

	case '{':
	    {
	        StatementListAST::Node funBody;
	        if ( parseFunctionBody(funBody) ) {
		    FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
		    ast->setNestedName( nestedName );
		    ast->setText( toString(start, endSignature) );
		    ast->setTypeSpec( spec );
		    node = ast;
		    UPDATE_POS( node, start, lex->index() );
		    return true;
	        }
	    }
	    break;
	    
	}
    }
    
    syntaxError();
    return false;
}

bool Parser::parseFunctionBody( StatementListAST::Node& node )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseFunctionBody()" << endl;
    
    int start = lex->index();
    
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    lex->nextToken();
    
    StatementListAST::Node ast = CreateNode<StatementListAST>();
    
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;

	StatementAST::Node stmt;	
	if( !parseStatement(stmt) ){
	    skipUntilStatement();
	} else
	    ast->addStatement( stmt );
    }
    
    ADVANCE( '}', "}" );
    
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
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

bool Parser::parseTypeSpecifierOrClassSpec( TypeSpecifierAST::Node& node )
{
    if( parseClassSpecifier(node) )
	return true;
    else if( parseEnumSpecifier(node) )
	return true;
    else if( parseTypeSpecifier(node) )
	return true;
	
    return false;
}

bool Parser::parseTryBlockStatement( StatementAST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseTryBlockStatement()" << endl;
    
    if( lex->lookAhead(0) != Token_try ){
	return false;
    }
    lex->nextToken();
    
    StatementAST::Node stmt;
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
	AST::Node cond;
	if( !parseCondition(cond) ){
	    reportError( i18n("condition expected") );
	    return false;
	}
	ADVANCE( ')', ")" );
	
	StatementAST::Node body;
	if( !parseCompoundStatement(body) ){
	    syntaxError();
	    return false;
	}
    }
    
    return true;
}

#if 0

bool Parser::parsePrimaryExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parsePrimarExpression()" << endl;

    AST::Node lit;
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
	    AST::Node expr;
            if( !parseExpression(expr) ){
                reportError( i18n("expression expected") );
            }
            if( lex->lookAhead(0) != ')' ){
                reportError( i18n(") expected") );
            }
            lex->nextToken();
            return true;
    }

    AST::Node name;
    if( parseName(name) ){
        return true;
    }
    
    return false;
}

bool Parser::parsePostfixExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parsePostfixExpression()" << endl;
    switch( lex->lookAhead(0) ){
        case Token_typename:
            lex->nextToken();
	    
	    AST::Node name;
            if( !parseName(name) ){
                reportError( i18n("name expected") );
                skipUntil( '(' );
            }
	    
            ADVANCE( '(', "(" );
	    AST::Node expr;
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
	    AST::Node typeId;
            parseTypeId( typeId );
            ADVANCE( '>', ">" );

            ADVANCE( '(', "(" );
	    AST::Node expr;
            parseCommaExpression( expr );
            ADVANCE( ')', ")" );
	}
        return true;

        case Token_typeid:
	{
            lex->nextToken();
            ADVANCE( '(', "(" );
	    AST::Node expr;
            parseCommaExpression( expr );
            ADVANCE( ')', ")" );
	}
        return true;

        default:
	{
	    AST::Node expr, spec;
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

    AST::Node expr;
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

		AST::Node name;
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

bool Parser::parseUnaryExpression( AST::Node& /*node*/ )
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

bool Parser::parseNewExpression( AST::Node& /*node*/ )
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

bool Parser::parseNewTypeId( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNewTypeId()" << endl;
    if( parseTypeSpecifier() ){
        parseNewDeclarator();
        return true;
    }

    return false;
}

bool Parser::parseNewDeclarator( AST::Node& /*node*/ )
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

bool Parser::parseNewInitializer( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseNewInitializer()" << endl;
    if( lex->lookAhead(0) != '(' )
        return false;

    lex->nextToken();
    parseCommaExpression();
    ADVANCE( ')', ")" );

    return true;
}

bool Parser::parseDeleteExpression( AST::Node& /*node*/ )
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

bool Parser::parseCastExpression( AST::Node& /*node*/ )
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

bool Parser::parsePmExpression( AST::Node& /*node*/ )
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

bool Parser::parseMultiplicativeExpression( AST::Node& /*node*/ )
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


bool Parser::parseAdditiveExpression( AST::Node& /*node*/ )
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

bool Parser::parseShiftExpression( AST::Node& /*node*/ )
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

bool Parser::parseRelationalExpression( AST::Node& /*node*/ )
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

bool Parser::parseEqualityExpression( AST::Node& /*node*/ )
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

bool Parser::parseAndExpression( AST::Node& /*node*/ )
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

bool Parser::parseExclusiveOrExpression( AST::Node& /*node*/ )
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

bool Parser::parseInclusiveOrExpression( AST::Node& /*node*/ )
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

bool Parser::parseLogicalAndExpression( AST::Node& /*node*/ )
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

bool Parser::parseLogicalOrExpression( AST::Node& /*node*/ )
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

bool Parser::parseConditionalExpression( AST::Node& /*node*/ )
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

bool Parser::parseAssignmentExpression( AST::Node& /*node*/ )
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

bool Parser::parseConstantExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseConstantExpression()" << endl;
    return parseConditionalExpression();
}

bool Parser::parseExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseExpression()" << endl;

    return parseCommaExpression();
}

bool Parser::parseCommaExpression( AST::Node& /*node*/ )
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

bool Parser::parseThrowExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007) << "--- tok = " << lex->lookAhead(0).toString() << " -- "  << "Parser::parseThrowExpression()" << endl;
    if( lex->lookAhead(0) != Token_throw )
        return false;

    ADVANCE( Token_throw, "throw" );
    parseAssignmentExpression();

    return true;
}


#endif
