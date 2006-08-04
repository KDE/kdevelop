/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// c++ support
#include "driver.h"
#include "lexer.h"
#include "parser.h"
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
  const Token& token = lex->lookAhead( 0 ); \
  if( token != tk ){ \
      reportError( i18n("'%1' expected found '%2'").arg(descr).arg(token.text()) ); \
      return false; \
  } \
  nextToken(); \
}

#define ADVANCE_NR(tk, descr) \
{ \
  const Token& token = lex->lookAhead( 0 ); \
  if( token != tk ){ \
      reportError( i18n("'%1' expected found '%2'").arg(descr).arg(token.text()) ); \
  } \
  else \
      nextToken(); \
}

#define CHECK(tk, descr) \
{ \
  const Token& token = lex->lookAhead( 0 ); \
  if( token != tk ){ \
      return false; \
  } \
  nextToken(); \
}

#define MATCH(tk, descr) \
{ \
  const Token& token = lex->lookAhead( 0 ); \
  if( token != tk ){ \
      reportError( Errors::SyntaxError ); \
      return false; \
  } \
}



#define UPDATE_POS(node, start, end) \
{ \
   int line, col; \
   const Token &a = lex->tokenAt(start); \
   const Token &b = lex->tokenAt( end!=start ? end-1 : end ); \
   a.getStartPosition( &line, &col ); \
   (node)->setStartPosition( line, col ); \
   b.getEndPosition( &line, &col ); \
   (node)->setEndPosition( line, col ); \
   if( (node)->nodeType() == NodeType_Generic ) { \
       if ((start) == (end) || (end) == (start)+1) \
           (node)->setSlice(lex->source(), a.position(), a.length()); \
       else \
           (node)->setText( toString((start),(end)) ); \
   } \
}

#define AST_FROM_TOKEN(node, tk) \
    AST::Node node = CreateNode<AST>(); \
    UPDATE_POS( node, (tk), (tk)+1 );


//@todo remove me
enum
{
    OBJC_CLASS,
    OBJC_PROTOCOL,
    OBJC_ALIAS
};

struct ParserPrivateData
{
    ParserPrivateData()
        {}
};

Parser::Parser( Driver* driver, Lexer* lexer )
    : m_driver( driver ),
      lex( lexer )
{
    d = new ParserPrivateData();

    m_maxProblems = 5;
    objcp = false;
}

Parser::~Parser()
{
    delete d;
    d = 0;
}

bool Parser::reportError( const Error& err )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::reportError()" << endl;
    if( m_problems < m_maxProblems ){
	++m_problems;
	int line=0, col=0;
	const Token& token = lex->lookAhead( 0 );
	lex->getTokenPosition( token, &line, &col );

	QString s = lex->lookAhead(0).text();
        s = s.left( 30 ).stripWhiteSpace();
	if( s.isEmpty() )
	    s = i18n( "<eof>" );

	m_driver->addProblem( m_driver->currentFileName(), Problem(err.text.arg(s), line, col) );
    }

    return true;
}

bool Parser::reportError( const QString& msg )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::reportError()" << endl;
    if( m_problems < m_maxProblems ){
	++m_problems;
	int line=0, col=0;
	const Token& token = lex->lookAhead( 0 );
	lex->getTokenPosition( token, &line, &col );

	m_driver->addProblem( m_driver->currentFileName(), Problem(msg, line, col) );
    }

    return true;
}

void Parser::syntaxError()
{
    (void) reportError( Errors::SyntaxError );
}

bool Parser::skipUntil( int token )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::skipUntil()" << endl;
    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == token )
	    return true;

	nextToken();
    }

    return false;
}

bool Parser::skipUntilDeclaration()
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::skipUntilDeclaration()" << endl;
    clearComment();

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
	    nextToken();
	}
    }

    return false;
}

bool Parser::skipUntilStatement()
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::skipUntilStatement() -- token = " << lex->lookAhead(0).text() << endl;

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
  	        nextToken();
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
	else if( l != '{' && (tk == '{' || tk == '}' || tk == ';') )
	    return false;

	if( count == 0 )
	    return true;

	nextToken();
    }

    return false;
}

bool Parser::skipCommaExpression( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::skipCommaExpression()" << endl;

    int start = lex->index();

    AST::Node expr;
    if( !skipExpression(expr) )
	return false;

    while( lex->lookAhead(0) == ',' ){
	nextToken();

	if( !skipExpression(expr) ){
	    reportError( i18n("expression expected") );
	    return false;
	}
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::skipExpression( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::skipExpression()" << endl;

    int start = lex->index();

    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );

	switch( tk ){
	case '(':
	    skip( '(', ')' );
	    nextToken();
	    break;

	case '[':
	    skip( '[', ']' );
	    nextToken();
	    break;

#if 0
	case Token_identifier:
	    nextToken();
	    if( lex->lookAhead( 0 ) == Token_identifier )
		return true;
	    break;
#endif

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
	{
	    AST::Node ast = CreateNode<AST>();
	    UPDATE_POS( ast, start, lex->index() );
	    node = ast;
	}
	return true;

	default:
	    nextToken();
	}
    }

    return false;
}

bool Parser::parseName( NameAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseName()" << endl;

    GroupAST::Node winDeclSpec;
    parseWinDeclSpec( winDeclSpec );

    int start = lex->index();

    NameAST::Node ast = CreateNode<NameAST>();

    if( lex->lookAhead(0) == Token_scope ){
        ast->setGlobal( true );
	nextToken();
    }

    int idx = lex->index();

    while( true ){
        ClassOrNamespaceNameAST::Node n;
        if( !parseUnqualifiedName(n) ) {
	    return false;
	}

	if( lex->lookAhead(0) == Token_scope ){
	    nextToken();
	    ast->addClassOrNamespaceName( n );
	    if( lex->lookAhead(0) == Token_template )
	        nextToken(); /// skip optional template     #### @todo CHECK
	} else {
	    ast->setUnqualifiedName( n );
	    break;
	}
    }

    if( idx == lex->index() )
        return false;

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseTranslationUnit( TranslationUnitAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTranslationUnit()" << endl;

    int start = lex->index();

    m_problems = 0;
    TranslationUnitAST::Node tun = CreateNode<TranslationUnitAST>();
    node = tun;
    if( lex->lookAhead(0) == Token_comment ) {
        node->setComment( lex->lookAhead(0).text() );
        nextToken();
    }
    
    while( !lex->lookAhead(0).isNull() ){
        DeclarationAST::Node def;
	int startDecl = lex->index();
        if( !parseDeclaration(def) ){
	    // error recovery
	    if( startDecl == lex->index() )
	        nextToken(); // skip at least one token
	    skipUntilDeclaration();
	}
	node->addDeclaration( def );
    }

    UPDATE_POS( node, start, lex->index() );

    // force (0,0) as start position
    node->setStartPosition( 0, 0 );

    return m_problems == 0;
}

bool Parser::parseDeclaration( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDeclaration()" << endl;
    
    int start = lex->index();

    switch( lex->lookAhead(0) ){

    case ';':
	nextToken();
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
	    // lex->setIndex( start );

	    if( objcp && parseObjcDef(node) )
		return true;

	    lex->setIndex( start );

	    GroupAST::Node storageSpec;
	    parseStorageClassSpecifier( storageSpec );

	    GroupAST::Node cv;
	    parseCvQualify( cv );

	    TypeSpecifierAST::Node spec;
	    AST::Node declarator;
	    if( parseEnumSpecifier(spec) || parseClassSpecifier(spec) ) {
                int line, c;
                spec->getEndPosition( &line, &c );
	        spec->setCvQualify( cv );

	        GroupAST::Node cv2;
	        parseCvQualify( cv2 );
		spec->setCv2Qualify( cv2 );

		InitDeclaratorListAST::Node declarators;
		parseInitDeclaratorList(declarators);
                
                SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
	        
                ADVANCE( ';', ";" );
                
                preparseLineComments( line );
                
                ast->setComment( m_commentStore.getCommentInRange( line ) );
                
                ast->setStorageSpecifier( storageSpec );
		ast->setTypeSpec( spec );
		ast->setInitDeclaratorList( declarators );
		UPDATE_POS( ast, start, lex->index() );
		node = ast;

	        return true;
	    }

	    lex->setIndex( start );
	    return parseDeclarationInternal( node );
	}

    } // end switch
}

bool Parser::parseLinkageSpecification( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseLinkageSpecification()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_extern ){
	return false;
    }
    nextToken();

    LinkageSpecificationAST::Node ast = CreateNode<LinkageSpecificationAST>();

    int startExternType = lex->index();
    if( lex->lookAhead(0) == Token_string_literal ){
	nextToken();
        AST::Node externType = CreateNode<AST>();
        UPDATE_POS( externType, startExternType, lex->index() );

        ast->setExternType( externType );
    }

    if( lex->lookAhead(0) == '{' ){
        LinkageBodyAST::Node linkageBody;
	parseLinkageBody( linkageBody );
	ast->setLinkageBody( linkageBody );
    } else {
        DeclarationAST::Node decl;
	if( !parseDeclaration(decl) ){
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
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseLinkageBody()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    nextToken();

    LinkageBodyAST::Node lba = CreateNode<LinkageBodyAST>();
    node = lba;

    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );

	if( tk == '}' )
	    break;

	DeclarationAST::Node def;
	int startDecl = lex->index();
	if( parseDeclaration(def) ){
	    node->addDeclaration( def );
	} else {
	    // error recovery
	    if( startDecl == lex->index() )
	        nextToken(); // skip at least one token
	    skipUntilDeclaration();
	}
    }
    
    clearComment();

    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} expected") );
    } else
	nextToken();

    UPDATE_POS( node, start, lex->index() );
    return true;
}

bool Parser::parseNamespace( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseNamespace()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_namespace ){
	return false;
    }
    nextToken();

    int startNamespaceName = lex->index();
    if( lex->lookAhead(0) == Token_identifier ){
	nextToken();
    }
    AST::Node namespaceName = CreateNode<AST>();
    UPDATE_POS( namespaceName, startNamespaceName, lex->index() );

    if ( lex->lookAhead(0) == '=' ) {
	// namespace alias
	nextToken();

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
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseUsing()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_using ){
	return false;
    }
    nextToken();

    if( lex->lookAhead(0) == Token_namespace ){
	if( !parseUsingDirective(node) ){
	    return false;
	}
	UPDATE_POS( node, start, lex->index() );
	return true;
    }

    UsingAST::Node ast = CreateNode<UsingAST>();

    int startTypeName = lex->index();
    if( lex->lookAhead(0) == Token_typename ){
	nextToken();
	AST::Node tn = CreateNode<AST>();
	UPDATE_POS( tn, startTypeName, lex->index() );
	ast->setTypeName( tn );
    }

    NameAST::Node name;
    if( !parseName(name) )
	return false;

    ast->setName( name );

    ADVANCE( ';', ";" );

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseUsingDirective( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseUsingDirective()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_namespace ){
	return false;
    }
    nextToken();

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
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseOperatorFunctionId()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_operator ){
	return false;
    }
    nextToken();

    AST::Node op;
    if( parseOperator(op) ){
	AST::Node asn = CreateNode<AST>();
	node = asn;
	UPDATE_POS( node, start, lex->index() );
	return true;
    } else {
	// parse cast operator
	GroupAST::Node cv;
        parseCvQualify(cv);

	TypeSpecifierAST::Node spec;
	if( !parseSimpleTypeSpecifier(spec) ){
	    syntaxError();
	    return false;
	}
        spec->setCvQualify( cv );

	GroupAST::Node cv2;
	parseCvQualify(cv2);
        spec->setCv2Qualify( cv2 );

	AST::Node ptrOp;
	while( parsePtrOperator(ptrOp) )
  	    ;

	AST::Node asn = CreateNode<AST>();
	node = asn;
	UPDATE_POS( node, start, lex->index() );
	return true;
    }
}

bool Parser::parseTemplateArgumentList( TemplateArgumentListAST::Node& node, bool reportError )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTemplateArgumentList()" << endl;

    int start = lex->index();

    TemplateArgumentListAST::Node ast = CreateNode<TemplateArgumentListAST>();

    AST::Node templArg;
    if( !parseTemplateArgument(templArg) )
	return false;
    ast->addArgument( templArg );

    while( lex->lookAhead(0) == ',' ){
	nextToken();

	if( !parseTemplateArgument(templArg) ){
	    if( reportError ){
	       syntaxError();
	       break;
	    } else
	       return false;
	}
	ast->addArgument( templArg );
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseTypedef( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTypedef()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_typedef ){
	return false;
    }
    nextToken();

    TypeSpecifierAST::Node spec;
    if( !parseTypeSpecifierOrClassSpec(spec) ){
	reportError( i18n("Need a type specifier to declare") );
	return false;
    }

    InitDeclaratorListAST::Node declarators;
    if( !parseInitDeclaratorList(declarators) ){
	//reportError( i18n("Need an identifier to declare") );
	//return false;
    }

    TypedefAST::Node ast = CreateNode<TypedefAST>();
    
    
    if( comment() ) {
        ast->setComment( comment() );
        clearComment();
        
        preparseLineComments( currentLine() );
        
        if( comment() ) {
            ast->addComment( comment() );
            clearComment();
        }
    }
    
    ADVANCE( ';', ";" );

    ast->setTypeSpec( spec );
    ast->setInitDeclaratorList( declarators );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseAsmDefinition( DeclarationAST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseAsmDefinition()" << endl;

    ADVANCE( Token_asm, "asm" );

    GroupAST::Node cv;
    parseCvQualify( cv );

    skip( '(', ')' );
    ADVANCE( ')', ")" );
    ADVANCE( ';', ';' );

    return true;
}

bool Parser::parseTemplateDeclaration( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTemplateDeclaration()" << endl;

    int start = lex->index();

    AST::Node exp;

    int startExport = lex->index();
    if( lex->lookAhead(0) == Token_export ){
	nextToken();
	AST::Node n = CreateNode<AST>();
	UPDATE_POS( n, startExport, lex->index() );
	exp = n;
    }

    if( lex->lookAhead(0) != Token_template ){
	return false;
    }
    nextToken();

    TemplateParameterListAST::Node params;
    if( lex->lookAhead(0) == '<' ){
	nextToken();
	parseTemplateParameterList( params );

	ADVANCE( '>', ">" );
    }

    DeclarationAST::Node def;
    if( !parseDeclaration(def) ){
	reportError( i18n("expected a declaration") );
    }

    TemplateDeclarationAST::Node ast = CreateNode<TemplateDeclarationAST>();
    ast->setExported( exp );
    ast->setTemplateParameterList( params );
    ast->setDeclaration( def );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseOperator( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseOperator()" << endl;
    QString text = lex->lookAhead(0).text();

    switch( lex->lookAhead(0) ){
    case Token_new:
    case Token_delete:
	nextToken();
	if( lex->lookAhead(0) == '[' && lex->lookAhead(1) == ']' ){
	    nextToken();
	    nextToken();
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
	nextToken();
	return true;

    default:
	if( lex->lookAhead(0) == '(' && lex->lookAhead(1) == ')' ){
	    nextToken();
	    nextToken();
	    return true;
	} else if( lex->lookAhead(0) == '[' && lex->lookAhead(1) == ']' ){
	    nextToken();
	    nextToken();
	    return true;
	}
    }

    return false;
}

bool Parser::parseCvQualify( GroupAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseCvQualify()" << endl;

    int start = lex->index();

    GroupAST::Node ast = CreateNode<GroupAST>();

    int n = 0;
    while( !lex->lookAhead(0).isNull() ){
	int tk = lex->lookAhead( 0 );
	if( tk == Token_const || tk == Token_volatile ){
	    ++n;
	    int startWord = lex->index();
	    nextToken();
            AST::Node word = CreateNode<AST>();
            UPDATE_POS( word, startWord, lex->index() );
	    ast->addNode( word );
	} else
	    break;
    }

    if( n == 0 )
        return false;


    //kdDebug(9007)<< "-----------------> token = " << lex->lookAhead(0).text() << endl;
    UPDATE_POS( ast, start, lex->index() );

    node = ast;
    return true;
}

bool Parser::parseSimpleTypeSpecifier( TypeSpecifierAST::Node& node )
{
    int start = lex->index();
    bool isIntegral = false;
    bool done = false;

    while( !done ){

        switch( lex->lookAhead(0) ){
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
	        isIntegral = true;
	        nextToken();
	        break;

	    default:
	        done = true;
	}
    }

    TypeSpecifierAST::Node ast = CreateNode<TypeSpecifierAST>();
    if( isIntegral ){
        ClassOrNamespaceNameAST::Node cl = CreateNode<ClassOrNamespaceNameAST>();

	AST::Node n = CreateNode<AST>();
	UPDATE_POS( n, start, lex->index() );
	cl->setName( n );
	UPDATE_POS( cl, start, lex->index() );

        NameAST::Node name = CreateNode<NameAST>();
	name->setUnqualifiedName( cl );
	UPDATE_POS( name, start, lex->index() );
	ast->setName( name );

    } else {
        NameAST::Node name;
        if( !parseName(name) ){
            lex->setIndex( start );
	    return false;
        }
	ast->setName( name );
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parsePtrOperator( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parsePtrOperator()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) == '&' ){
	nextToken();
    } else if( lex->lookAhead(0) == '*' ){
	nextToken();
    } else {
	int index = lex->index();
	AST::Node memPtr;
	if( !parsePtrToMember(memPtr) ){
	    lex->setIndex( index );
	    return false;
	}
    }

    GroupAST::Node cv;
    parseCvQualify( cv );

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}


bool Parser::parseTemplateArgument( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTemplateArgument()" << endl;

    int start = lex->index();
    if( parseTypeId(node) ){
        if( lex->lookAhead(0) == ',' || lex->lookAhead(0) == '>' )
	    return true;
    }

    lex->setIndex( start );
    if( !parseLogicalOrExpression(node, true) ){
        return false;
    }

    return true;
}

bool Parser::parseTypeSpecifier( TypeSpecifierAST::Node& spec )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTypeSpecifier()" << endl;

    GroupAST::Node cv;
    parseCvQualify( cv );

    if( parseElaboratedTypeSpecifier(spec) || parseSimpleTypeSpecifier(spec) ){
        spec->setCvQualify( cv );

        GroupAST::Node cv2;
	parseCvQualify( cv2 );
        spec->setCv2Qualify( cv2 );

	return true;
    }

    return false;
}

bool Parser::parseDeclarator( DeclaratorAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDeclarator()" << endl;

    int start = lex->index();

    DeclaratorAST::Node ast = CreateNode<DeclaratorAST>();

    DeclaratorAST::Node decl;
    NameAST::Node declId;

    AST::Node ptrOp;
    while( parsePtrOperator(ptrOp) ){
	ast->addPtrOp( ptrOp );
    }

    if( lex->lookAhead(0) == '(' ){
	nextToken();

	if( !parseDeclarator(decl) ){
	    return false;
	}
	ast->setSubDeclarator( decl );

	if( lex->lookAhead(0) != ')'){
	    return false;
	}
	nextToken();
    } else {

        if( lex->lookAhead(0) == ':' ){
             // unnamed bitfield
        } else if( parseDeclaratorId(declId) ){
	    ast->setDeclaratorId( declId );
	} else {
	    lex->setIndex( start );
	    return false;
	}

	if( lex->lookAhead(0) == ':' ){
	    nextToken();
	    AST::Node expr;
	    if( !parseConstantExpression(expr) ){
		reportError( i18n("Constant expression expected") );
	    }
	    goto update_pos;
	}
    }

    {
	bool isVector = true;

	while( lex->lookAhead(0) == '[' ){
	    int startArray = lex->index();
	    nextToken();
	    AST::Node expr;
	    parseCommaExpression( expr );

	    ADVANCE( ']', "]" );
	    AST::Node array = CreateNode<AST>();
	    UPDATE_POS( array, startArray, lex->index() );
	    ast->addArrayDimension( array );
	    isVector = true;
	}

	bool skipParen = false;
	if( lex->lookAhead(0) == Token_identifier && lex->lookAhead(1) == '(' && lex->lookAhead(2) == '(' ){
	    nextToken();
	    nextToken();
	    skipParen = true;
	}

	if( ast->subDeclarator() && (!isVector || lex->lookAhead(0) != '(') ){
	    lex->setIndex( start );
	    return false;
	}

	int index = lex->index();
	if( lex->lookAhead(0) == '(' ){
	    nextToken();

	    ParameterDeclarationClauseAST::Node params;
	    if( !parseParameterDeclarationClause(params) ){
		//kdDebug(9007)<< "----------------------> not a parameter declaration, maybe an initializer!?" << endl;
		lex->setIndex( index );
		goto update_pos;
	    }
	    ast->setParameterDeclarationClause( params );

	    if( lex->lookAhead(0) != ')' ){
		lex->setIndex( index );
		goto update_pos;
	    }

	    nextToken();  // skip ')'

	    int startConstant = lex->index();
	    if( lex->lookAhead(0) == Token_const ){
		nextToken();
		AST::Node constant = CreateNode<AST>();
		UPDATE_POS( constant, startConstant, lex->index() );
		ast->setConstant( constant );
	    }

	    GroupAST::Node except;
	    if( parseExceptionSpecification(except) ){
		ast->setExceptionSpecification( except );
	    }
	}

	if( skipParen ){
	    if( lex->lookAhead(0) != ')' ){
		reportError( i18n("')' expected") );
	    } else
		nextToken();
	}

    }

update_pos:
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseAbstractDeclarator( DeclaratorAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDeclarator()" << endl;
    int start = lex->index();

    DeclaratorAST::Node ast = CreateNode<DeclaratorAST>();

    DeclaratorAST::Node decl;
    NameAST::Node declId;

    AST::Node ptrOp;
    while( parsePtrOperator(ptrOp) ){
	ast->addPtrOp( ptrOp );
    }

    if( lex->lookAhead(0) == '(' ){
	nextToken();

	if( !parseAbstractDeclarator(decl) ){
	    return false;
	}
	ast->setSubDeclarator( decl );

	if( lex->lookAhead(0) != ')'){
	    return false;
	}
	nextToken();
    }

    {

    while( lex->lookAhead(0) == '[' ){
        int startArray = lex->index();
	nextToken();
	AST::Node expr;
	skipCommaExpression( expr );

	ADVANCE( ']', "]" );
	AST::Node array = CreateNode<AST>();
	UPDATE_POS( array, startArray, lex->index() );
	ast->addArrayDimension( array );
    }

    bool skipParen = false;
    if( lex->lookAhead(0) == Token_identifier && lex->lookAhead(1) == '(' && lex->lookAhead(2) == '(' ){
    	nextToken();
	nextToken();
	skipParen = true;
    }

    int index = lex->index();
    if( lex->lookAhead(0) == '(' ){
	nextToken();

	ParameterDeclarationClauseAST::Node params;
	if( !parseParameterDeclarationClause(params) ){
	    lex->setIndex( index );
	    goto UPDATE_POS;
	}
	ast->setParameterDeclarationClause( params );

	if( lex->lookAhead(0) != ')' ){
	    lex->setIndex( index );
	    goto UPDATE_POS;
	} else
	    nextToken();

	int startConstant = lex->index();
	if( lex->lookAhead(0) == Token_const ){
	    nextToken();
	    AST::Node constant = CreateNode<AST>();
	    UPDATE_POS( constant, startConstant, lex->index() );
	    ast->setConstant( constant );
	}

	GroupAST::Node except;
	if( parseExceptionSpecification(except) ){
	    ast->setExceptionSpecification( except );
	}
    }

    if( skipParen ){
    	if( lex->lookAhead(0) != ')' ){
	    reportError( i18n("')' expected") );
	} else
	    nextToken();
    }

    }

UPDATE_POS:
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}


bool Parser::parseEnumSpecifier( TypeSpecifierAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseEnumSpecifier()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_enum ){
	return false;
    }

    nextToken();

    Comment c = comment();
    clearComment();
    NameAST::Node name;
    parseName( name );

    if( lex->lookAhead(0) != '{' ){
	lex->setIndex( start );
	return false;
    }
    nextToken();

    EnumSpecifierAST::Node ast = CreateNode<EnumSpecifierAST>();
    ast->setName( name );
    
    ast->setComment( c );

    EnumeratorAST::Node enumerator;
    if( parseEnumerator(enumerator) ){
        ast->addEnumerator( enumerator );

        while( lex->lookAhead(0) == ',' ){
	    nextToken();

	    if( !parseEnumerator(enumerator) ){
	        //reportError( i18n("Enumerator expected") );
	        break;
	    }

	    ast->addEnumerator( enumerator );
        }
    }

    clearComment( );
    
    if( lex->lookAhead(0) != '}' )
	reportError( i18n("} missing") );
    else
	nextToken();
    
    
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseTemplateParameterList( TemplateParameterListAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTemplateParameterList()" << endl;

    int start = lex->index();

    TemplateParameterListAST::Node ast = CreateNode<TemplateParameterListAST>();

    TemplateParameterAST::Node param;
    if( !parseTemplateParameter(param) ){
	return false;
    }
    ast->addTemplateParameter( param );

    while( lex->lookAhead(0) == ',' ){
	nextToken();

	if( !parseTemplateParameter(param) ){
	    syntaxError();
	    break;
	} else {
	    ast->addTemplateParameter( param );
	}
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseTemplateParameter( TemplateParameterAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTemplateParameter()" << endl;

    int start = lex->index();
    TemplateParameterAST::Node ast = CreateNode<TemplateParameterAST>();

    TypeParameterAST::Node typeParameter;
    ParameterDeclarationAST::Node param;

    int tk = lex->lookAhead( 0 );

    if( (tk == Token_class || tk == Token_typename || tk == Token_template) && parseTypeParameter(typeParameter) ){
	ast->setTypeParameter( typeParameter );
	goto ok;
    }

    if( !parseParameterDeclaration(param) )
        return false;
    ast->setTypeValueParameter( param );

ok:
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseTypeParameter( TypeParameterAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTypeParameter()" << endl;

    int start = lex->index();
    TypeParameterAST::Node ast = CreateNode<TypeParameterAST>();

    AST_FROM_TOKEN( kind, lex->index() );
    ast->setKind( kind );

    switch( lex->lookAhead(0) ){

    case Token_class:
    case Token_typename:
	{
	    nextToken(); // skip class

	    // parse optional name
	    NameAST::Node name;
	    if( parseName(name) ){
		ast->setName( name );
		if( lex->lookAhead(0) == '=' ){
		    nextToken();

		    AST::Node typeId;
		    if( !parseTypeId(typeId) ){
			syntaxError();
			return false;
		    }
		    ast->setTypeId( typeId );
		}
	    }
	}
	break;

    case Token_template:
	{
	    nextToken(); // skip template
	    ADVANCE( '<', '<' );

	    TemplateParameterListAST::Node params;
	    if( !parseTemplateParameterList(params) ){
		return false;
	    }
	    ast->setTemplateParameterList( params );

	    ADVANCE( '>', ">" );

	    if( lex->lookAhead(0) == Token_class )
		nextToken();

	    // parse optional name
	    NameAST::Node name;
	    if( parseName(name) ){
		ast->setName( name );
		if( lex->lookAhead(0) == '=' ){
		    nextToken();

		    AST::Node typeId;
		    if( !parseTypeId(typeId) ){
			syntaxError();
			return false;
		    }
		    ast->setTypeId( typeId );
		}
	    }

	    if( lex->lookAhead(0) == '=' ){
		nextToken();

		NameAST::Node templ_name;
		parseName( templ_name );
	    }
	}
	break;

    default:
	return false;

    } // end switch


    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseStorageClassSpecifier( GroupAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseStorageClassSpecifier()" << endl;

    int start = lex->index();
    GroupAST::Node ast = CreateNode<GroupAST>();

    while( !lex->lookAhead(0).isNull() ){
        int tk = lex->lookAhead( 0 );
        if( tk == Token_friend || tk == Token_auto || tk == Token_register || tk == Token_static ||
		tk == Token_extern || tk == Token_mutable ){
	    int startNode = lex->index();
	    nextToken();

	    AST::Node n = CreateNode<AST>();
	    UPDATE_POS( n, startNode, lex->index() );
	    ast->addNode( n );
	} else
	    break;
    }

    if( ast->nodeList().count() == 0 )
       return false;

    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseFunctionSpecifier( GroupAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseFunctionSpecifier()" << endl;

    int start = lex->index();
    GroupAST::Node ast = CreateNode<GroupAST>();

    while( !lex->lookAhead(0).isNull() ){
        int tk = lex->lookAhead( 0 );
        if( tk == Token_inline || tk == Token_virtual || tk == Token_explicit ){
	    int startNode = lex->index();
	    nextToken();

	    AST::Node n = CreateNode<AST>();
	    UPDATE_POS( n, startNode, lex->index() );
	    ast->addNode( n );
	} else {
	    break;
    }
    }

    if( ast->nodeList().count() == 0 )
       return false;

    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseTypeId( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTypeId()" << endl;

    /// @todo implement the AST for typeId
    int start = lex->index();
    AST::Node ast = CreateNode<AST>();

    TypeSpecifierAST::Node spec;
    if( !parseTypeSpecifier(spec) ){
	return false;
    }

    DeclaratorAST::Node decl;
    parseAbstractDeclarator( decl );

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseInitDeclaratorList( InitDeclaratorListAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseInitDeclaratorList()" << endl;

    int start = lex->index();

    InitDeclaratorListAST::Node ast = CreateNode<InitDeclaratorListAST>();
    InitDeclaratorAST::Node decl;

    if( !parseInitDeclarator(decl) ){
	return false;
    }
    ast->addInitDeclarator( decl );

    while( lex->lookAhead(0) == ',' ){
	nextToken();

	if( !parseInitDeclarator(decl) ){
	    syntaxError();
	    break;
	}
	ast->addInitDeclarator( decl );
    }
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseInitDeclaratorList() -- end" << endl;

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseParameterDeclarationClause( ParameterDeclarationClauseAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseParameterDeclarationClause()" << endl;

    int start = lex->index();

    ParameterDeclarationClauseAST::Node ast = CreateNode<ParameterDeclarationClauseAST>();

    ParameterDeclarationListAST::Node params;
    if( !parseParameterDeclarationList(params) ){

	if ( lex->lookAhead(0) == ')' )
	    goto good;

	if( lex->lookAhead(0) == Token_ellipsis && lex->lookAhead(1) == ')' ){
	    AST_FROM_TOKEN( ellipsis, lex->index() );
	    ast->setEllipsis( ellipsis );
	    nextToken();
	    goto good;
	}
	return false;
    }

    if( lex->lookAhead(0) == Token_ellipsis ){
	AST_FROM_TOKEN( ellipsis, lex->index() );
	ast->setEllipsis( ellipsis );
	nextToken();
    }

good:
    ast->setParameterDeclarationList( params );

    /// @todo add ellipsis
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    
    return true;
}


void Parser::nextToken( bool skipComm ) {
    lex->nextToken();
    if( skipComm ) {
        if( lex->lookAhead(0) == Token_comment ) {
            processComment();
            nextToken();
        }
    }
}


Comment Parser::comment() {
    return m_commentStore.latestComment();
}


void Parser::preparseLineComments( int l ) {
    for( int a = 0; a < 40; a++ ) {
        if( lex->lookAhead(a).isNull() ) break;
        int line, col;
        lex->lookAhead( a ).getStartPosition( &line, &col );
        if( line < l ) {
            continue;
        } else if( line == l ) {
            if( lex->lookAhead( a ) == Token_comment ) {
                processComment( a );
            }
        } else {
            break;
        }
    }
}


void Parser::processComment( int offset ) {
    int line, col;
    lex->lookAhead( offset ).getStartPosition( &line, &col );
    m_commentStore.addComment( Comment( lex->lookAhead(offset).text(), line ) );
}

template<class Type>
void Parser::eventuallyTakeComment( int startLn, int endLn, Type& ast ) {
    if( comment().line() >= startLn && comment().line() <= endLn ) {
        if( &(*ast) ) {
            if( comment() ) {
                ast->setComment( comment() );
            }
        }
        
        clearComment();
    }
}

template<class Type>
void Parser::eventuallyTakeComment( Type& ast ) {
    if( &(*ast) && comment() ) {
        ast->setComment( comment() );
    }

    clearComment();
}

void Parser::clearComment( ) {
    
    m_commentStore.clear();
    
}

int Parser::currentLine() {
    int ln, col;
    lex->lookAhead( 0 ).getStartPosition( &ln, &col );
    return ln;
}

bool Parser::parseParameterDeclarationList( ParameterDeclarationListAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseParameterDeclarationList()" << endl;

    int start = lex->index();

    ParameterDeclarationListAST::Node ast = CreateNode<ParameterDeclarationListAST>();

    ParameterDeclarationAST::Node param;
    if( !parseParameterDeclaration(param) ){
	lex->setIndex( start );
	return false;
    }
    ast->addParameter( param );

    while( lex->lookAhead(0) == ',' ){
	nextToken();

	if( lex->lookAhead(0) == Token_ellipsis )
	    break;

	if( !parseParameterDeclaration(param) ){
	    lex->setIndex( start );
	    return false;
	}
        ast->addParameter( param );
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseParameterDeclaration( ParameterDeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseParameterDeclaration()" << endl;

    int start = lex->index();

    // parse decl spec
    TypeSpecifierAST::Node spec;
    if( !parseTypeSpecifier(spec) ){
	lex->setIndex( start );
	return false;
    }

    int index = lex->index();

    DeclaratorAST::Node decl;
    if( !parseDeclarator(decl) ){
	lex->setIndex( index );

	// try with abstract declarator
	if( !parseAbstractDeclarator(decl) )
	    return false;
    }

    AST::Node expr;
    if( lex->lookAhead(0) == '=' ){
	nextToken();
	if( !parseLogicalOrExpression(expr,true) ){
	    //reportError( i18n("Expression expected") );
        }
    }

    ParameterDeclarationAST::Node ast = CreateNode<ParameterDeclarationAST>();
    ast->setTypeSpec( spec );
    ast->setDeclarator( decl );
    ast->setExpression( expr );

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}


bool Parser::parseClassSpecifier( TypeSpecifierAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseClassSpecifier()" << endl;

    int start = lex->index();

    AST::Node classKey;
    int classKeyStart = lex->index();

    int kind = lex->lookAhead( 0 );
    if( kind == Token_class || kind == Token_struct || kind == Token_union ){
	AST::Node asn = CreateNode<AST>();
        classKey = asn;
	nextToken();
	UPDATE_POS( classKey, classKeyStart, lex->index() );
    } else {
	return false;
    }

    GroupAST::Node winDeclSpec;
    parseWinDeclSpec( winDeclSpec );

    while( lex->lookAhead(0) == Token_identifier && lex->lookAhead(1) == Token_identifier )
    	nextToken();

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

    ClassSpecifierAST::Node ast = CreateNode<ClassSpecifierAST>();
    
    eventuallyTakeComment( ast );
    
    ADVANCE( '{', '{' );


    ast->setWinDeclSpec( winDeclSpec );
    ast->setClassKey( classKey );
    ast->setName( name );
    ast->setBaseClause( bases );


    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;

	DeclarationAST::Node memSpec;
	int startDecl = lex->index();
	if( !parseMemberSpecification(memSpec) ){
	    if( startDecl == lex->index() )
	        nextToken(); // skip at least one token
	    skipUntilDeclaration();
	} else
	    ast->addDeclaration( memSpec );
    }
    
    clearComment();

    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} missing") );
    } else
	nextToken();

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseAccessSpecifier( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseAccessSpecifier()" << endl;

    int start = lex->index();

    switch( lex->lookAhead(0) ){
    case Token_public:
    case Token_protected:
    case Token_private: {
        AST::Node asn = CreateNode<AST>();
	node = asn;
	nextToken();
	UPDATE_POS( node, start, lex->index() );
	return true;
        }
    }

    return false;
}

bool Parser::parseMemberSpecification( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseMemberSpecification()" << endl;

    int start = lex->index();

    AST::Node access;

    if( lex->lookAhead(0) == ';' ){
	nextToken();
	return true;
    } else if( lex->lookAhead(0) == Token_Q_OBJECT || lex->lookAhead(0) == Token_K_DCOP ){
	nextToken();
	return true;
    } else if( lex->lookAhead(0) == Token_signals || lex->lookAhead(0) == Token_k_dcop || lex->lookAhead(0) == Token_k_dcop_signals ){
        AccessDeclarationAST::Node ast = CreateNode<AccessDeclarationAST>();
	nextToken();
	AST::Node n = CreateNode<AST>();
	UPDATE_POS( n, start, lex->index() );
	ast->addAccess( n );
	ADVANCE( ':', ":" );
	UPDATE_POS( ast, start, lex->index() );
	node = ast;
	return true;
    } else if( parseTypedef(node) ){
	return true;
    } else if( parseUsing(node) ){
	return true;
    } else if( parseTemplateDeclaration(node) ){
	return true;
    } else if( parseAccessSpecifier(access) ){
        AccessDeclarationAST::Node ast = CreateNode<AccessDeclarationAST>();
	ast->addAccess( access );

        int startSlot = lex->index();
	if( lex->lookAhead(0) == Token_slots ){
	    nextToken();
	    AST::Node sl = CreateNode<AST>();
	    UPDATE_POS( sl, startSlot, lex->index() );
	    ast->addAccess( sl );
	}
	ADVANCE( ':', ":" );
	UPDATE_POS( ast, start, lex->index() );
	node = ast;
	return true;
    }

    lex->setIndex( start );

    GroupAST::Node storageSpec;
    parseStorageClassSpecifier( storageSpec );

    GroupAST::Node cv;
    parseCvQualify( cv );

    TypeSpecifierAST::Node spec;
    if( parseEnumSpecifier(spec) || parseClassSpecifier(spec) ){
        spec->setCvQualify( cv );

        GroupAST::Node cv2;
        parseCvQualify( cv2 );
        spec->setCv2Qualify( cv2 );

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

    lex->setIndex( start );
    return parseDeclarationInternal( node );
}

bool Parser::parseCtorInitializer( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseCtorInitializer()" << endl;

    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    nextToken();

    AST::Node inits;
    if( !parseMemInitializerList(inits) ){
	reportError( i18n("Member initializers expected") );
    }

    return true;
}

bool Parser::parseElaboratedTypeSpecifier( TypeSpecifierAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseElaboratedTypeSpecifier()" << endl;

    int start = lex->index();

    int tk = lex->lookAhead( 0 );
    if( tk == Token_class  ||
	tk == Token_struct ||
	tk == Token_union  ||
	tk == Token_enum   ||
	tk == Token_typename )
    {
        AST::Node kind = CreateNode<AST>();
	nextToken();
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
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDeclaratorId()" << endl;
    return parseName( node );
}

bool Parser::parseExceptionSpecification( GroupAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseExceptionSpecification()" << endl;

    if( lex->lookAhead(0) != Token_throw ){
	return false;
    }
    nextToken();

    ADVANCE( '(', "(" );
    if( lex->lookAhead(0) == Token_ellipsis ){
        // extension found in MSVC++ 7.x headers
        int start = lex->index();
        GroupAST::Node ast = CreateNode<GroupAST>();
        AST_FROM_TOKEN( ellipsis, lex->index() );
        ast->addNode( ellipsis );
        nextToken();
        UPDATE_POS( ast, start, lex->index() );
        node = ast;
    } else {
        parseTypeIdList( node );
    }
    ADVANCE( ')', ")" );

    return true;
}

bool Parser::parseEnumerator( EnumeratorAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseEnumerator()" << endl;

    int start = lex->index();

    if( lex->lookAhead(0) != Token_identifier ){
        return false;
    }
     
    nextToken();
    

    EnumeratorAST::Node ena = CreateNode<EnumeratorAST>();
    node = ena;

    AST::Node id = CreateNode<AST>();
    UPDATE_POS( id, start, lex->index() );
    node->setId( id );
    int line = currentLine();

    if( lex->lookAhead(0) == '=' ){
	nextToken();

	AST::Node expr;
        line = currentLine();
	if( !parseConstantExpression(expr) ){
	    reportError( i18n("Constant expression expected") );
	}
	node->setExpr( expr );
    }

    UPDATE_POS( node, start, lex->index() );

    preparseLineComments( line );
    
    node->setComment( m_commentStore.getCommentInRange( line ) );
    
    return true;
}

bool Parser::parseInitDeclarator( InitDeclaratorAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseInitDeclarator()" << endl;

    int start = lex->index();

    DeclaratorAST::Node decl;
    AST::Node init;
    if( !parseDeclarator(decl) ){
	return false;
    }

    parseInitializer( init );

    InitDeclaratorAST::Node ast = CreateNode<InitDeclaratorAST>();
    ast->setDeclarator( decl );
    ast->setInitializer( init );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}



bool Parser::parseBaseClause( BaseClauseAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseBaseClause()" << endl;

    int start = lex->index();
    if( lex->lookAhead(0) != ':' ){
	return false;
    }
    nextToken();

    BaseClauseAST::Node bca = CreateNode<BaseClauseAST>();

    BaseSpecifierAST::Node baseSpec;
    if( parseBaseSpecifier(baseSpec) ){
        bca->addBaseSpecifier( baseSpec );

        while( lex->lookAhead(0) == ',' ){
	    nextToken();

	    if( !parseBaseSpecifier(baseSpec) ){
	        reportError( i18n("Base class specifier expected") );
	        return false;
	    }
	    bca->addBaseSpecifier( baseSpec );
        }
    } else
        return false;

    UPDATE_POS( bca, start, lex->index() );
    node = bca;

    return true;
}

bool Parser::parseInitializer( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseInitializer()" << endl;

    if( lex->lookAhead(0) == '=' ){
	nextToken();

	AST::Node init;
	if( !parseInitializerClause(node) ){
	    reportError( i18n("Initializer clause expected") );
	    return false;
	}
    } else if( lex->lookAhead(0) == '(' ){
	nextToken();
	AST::Node expr;
	skipCommaExpression( expr );

	ADVANCE( ')', ")" );
    }

    return false;
}

bool Parser::parseMemInitializerList( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseMemInitializerList()" << endl;

    AST::Node init;
    if( !parseMemInitializer(init) ){
	return false;
    }

    while( lex->lookAhead(0) == ',' ){
	nextToken();

	if( parseMemInitializer(init) ){
	} else {
	    break;
	}
    }

    return true;
}

bool Parser::parseMemInitializer( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseMemInitializer()" << endl;

    NameAST::Node initId;
    if( !parseMemInitializerId(initId) ){
	reportError( i18n("Identifier expected") );
	return false;
    }
    ADVANCE( '(', '(' );
    AST::Node expr;
    skipCommaExpression( expr );
    ADVANCE( ')', ')' );

    return true;
}

bool Parser::parseTypeIdList( GroupAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTypeIdList()" << endl;

    int start = lex->index();

    AST::Node typeId;
    if( !parseTypeId(typeId) ){
	return false;
    }

    GroupAST::Node ast = CreateNode<GroupAST>();
    ast->addNode( typeId );

    while( lex->lookAhead(0) == ',' ){
	nextToken();
	if( parseTypeId(typeId) ){
	    ast->addNode( typeId );
	} else {
	    reportError( i18n("Type id expected") );
	    break;
	}
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseBaseSpecifier( BaseSpecifierAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseBaseSpecifier()" << endl;

    int start = lex->index();
    BaseSpecifierAST::Node ast = CreateNode<BaseSpecifierAST>();

    AST::Node access;
    if( lex->lookAhead(0) == Token_virtual ){
	AST_FROM_TOKEN( virt, lex->index() );
	ast->setIsVirtual( virt );

	nextToken();

	parseAccessSpecifier( access );
    } else {
        parseAccessSpecifier( access );

	if( lex->lookAhead(0) == Token_virtual ){
	    AST_FROM_TOKEN( virt, lex->index() );
	    ast->setIsVirtual( virt );
	    nextToken();
	}
    }

    NameAST::Node name;
    if( !parseName(name) ){
	reportError( i18n("Class name expected") );
    }

    ast->setAccess( access );
    ast->setName( name );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}


bool Parser::parseInitializerClause( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseInitializerClause()" << endl;

    if( lex->lookAhead(0) == '{' ){
	if( !skip('{','}') ){
	    reportError( i18n("} missing") );
        } else {
            clearComment();
	    nextToken();
        }
    } else {
	if( !parseAssignmentExpression(node) ){
	    //reportError( i18n("Expression expected") );
	}
    }

    return true;
}

bool Parser::parseMemInitializerId( NameAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseMemInitializerId()" << endl;

    return parseName( node );
}

bool Parser::parsePtrToMember( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parsePtrToMember()" << endl;

    if( lex->lookAhead(0) == Token_scope ){
	nextToken();
    }

    while( lex->lookAhead(0) == Token_identifier ){
	nextToken();

	if( lex->lookAhead(0) == Token_scope && lex->lookAhead(1) == '*' ){
	    nextToken(); // skip ::
	    nextToken(); // skip *
	    return true;
	} else
	    break;
    }

    return false;
}

bool Parser::parseUnqualifiedName( ClassOrNamespaceNameAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseUnqualifiedName()" << endl;

    int start = lex->index();
    bool isDestructor = false;

    ClassOrNamespaceNameAST::Node ast = CreateNode<ClassOrNamespaceNameAST>();

    if( lex->lookAhead(0) == Token_identifier ){
        int startName = lex->index();
        AST::Node n = CreateNode<AST>();
	nextToken();
	UPDATE_POS( n, startName, lex->index() );
	ast->setName( n );
    } else if( lex->lookAhead(0) == '~' && lex->lookAhead(1) == Token_identifier ){
        int startName = lex->index();
        AST::Node n = CreateNode<AST>();
	nextToken(); // skip ~
	nextToken(); // skip classname
	UPDATE_POS( n, startName, lex->index() );
	ast->setName( n );
	isDestructor = true;
    } else if( lex->lookAhead(0) == Token_operator ){
        AST::Node n;
	if( !parseOperatorFunctionId(n) )
	    return false;
	ast->setName( n );
    } else {
	return false;
    }

    if( !isDestructor ){

	int index = lex->index();

	if( lex->lookAhead(0) == '<' ){
	    nextToken();

	    // optional template arguments
	    TemplateArgumentListAST::Node args;
	    parseTemplateArgumentList( args );

	    if( lex->lookAhead(0) != '>' ){
		lex->setIndex( index );
	    } else {
		nextToken();
	        ast->setTemplateArgumentList( args );
	    }
	}
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseStringLiteral( AST::Node& /*node*/ )
{
    while( !lex->lookAhead(0).isNull() ) {
	if( lex->lookAhead(0) == Token_identifier &&
	    lex->lookAhead(0).text() == "L" && lex->lookAhead(1) == Token_string_literal ) {

	    nextToken();
	    nextToken();
	} else if( lex->lookAhead(0) == Token_string_literal ) {
	    nextToken();
	} else
	    return false;
    }
    return true;
}

bool Parser::skipExpressionStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::skipExpressionStatement()" << endl;

    int start = lex->index();

    AST::Node expr;
    skipCommaExpression( expr );

    ADVANCE( ';', ";" );

    ExpressionStatementAST::Node ast = CreateNode<ExpressionStatementAST>();
    ast->setExpression( expr );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseStatement( StatementAST::Node& node ) // thanks to fiore@8080.it ;)
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseStatement()" << endl;
    switch( lex->lookAhead(0) ){

    case Token_while:
	return parseWhileStatement( node );

    case Token_do:
	return parseDoStatement( node );

    case Token_for:
	return parseForStatement( node );

    case Token_foreach:
	return parseForEachStatement( node );
    
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
	nextToken();
	ADVANCE( ';', ";" );
	return true;

    case Token_goto:
	nextToken();
	ADVANCE( Token_identifier, "identifier" );
	ADVANCE( ';', ";" );
	return true;

    case Token_return:
    {
	nextToken();
	AST::Node expr;
	skipCommaExpression( expr );
	ADVANCE( ';', ";" );
    }
    return true;

    case '{':
	return parseCompoundStatement( node );

    case Token_identifier:
	if( parseLabeledStatement(node) )
	    return true;
	break;
    }

    //kdDebug(9007)<< "------------> try with declaration statement" << endl;
    if ( parseDeclarationStatement(node) )
	return true;

    return skipExpressionStatement( node );
}

bool Parser::parseCondition( ConditionAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseCondition()" << endl;

    int start = lex->index();

    ConditionAST::Node ast = CreateNode<ConditionAST>();

    TypeSpecifierAST::Node spec;
    if( parseTypeSpecifier(spec) ){
    	DeclaratorAST::Node decl;
		if( parseDeclarator(decl) ) {
			if( lex->lookAhead(0) == '=' ) {
			nextToken();
	
			AST::Node expr;
			if( skipExpression(expr) ){
					ast->setTypeSpec( spec );
					ast->setDeclarator( decl );
					ast->setExpression( expr );
	
					UPDATE_POS( ast, start, lex->index() );
					node = ast;
	
					return true;
				}
			} else {
				ast->setTypeSpec( spec );
				ast->setDeclarator( decl );
				
				UPDATE_POS( ast, start, lex->index() );
				node = ast;
				
				return true;
			}
		}
    }

    lex->setIndex( start );

    AST::Node expr;
    if( !skipCommaExpression(expr) )
        return false;

    ast->setExpression( expr );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}


bool Parser::parseWhileStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseWhileStatement()" << endl;
    int start = lex->index();

    ADVANCE( Token_while, "while" );
    ADVANCE( '(' , "(" );

    ConditionAST::Node cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );

    StatementAST::Node body;
    if( !parseStatement(body) ){
	reportError( i18n("statement expected") );
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
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDoStatement()" << endl;
    int start = lex->index();

    ADVANCE( Token_do, "do" );

    StatementAST::Node body;
    if( !parseStatement(body) ){
	reportError( i18n("statement expected") );
	//return false;
    }

    ADVANCE_NR( Token_while, "while" );
    ADVANCE_NR( '(' , "(" );

    AST::Node expr;
    if( !skipCommaExpression(expr) ){
	reportError( i18n("expression expected") );
	//return false;
    }

    ADVANCE_NR( ')', ")" );
    ADVANCE_NR( ';', ";" );

    DoStatementAST::Node ast = CreateNode<DoStatementAST>();
    ast->setStatement( body );
    //ast->setCondition( condition );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseForStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseForStatement()" << endl;
    int start = lex->index();

    ADVANCE( Token_for, "for" );
    ADVANCE( '(', "(" );

    StatementAST::Node init;
    if( !parseForInitStatement(init) ){
	reportError( i18n("for initialization expected") );
	return false;
    }

    ConditionAST::Node cond;
    parseCondition( cond );
    ADVANCE( ';', ";" );

    AST::Node expr;
    skipCommaExpression( expr );
    ADVANCE( ')', ")" );

    StatementAST::Node body;
    parseStatement(body);

    ForStatementAST::Node ast = CreateNode<ForStatementAST>();
    ast->setInitStatement( init );
    ast->setCondition( cond );
    // ast->setExpression( expression );
    ast->setStatement( body );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

// qt4 [erbsland]
///@todo add the right parsing for the foreach statement
bool Parser::parseForEachStatement( StatementAST::Node& node )
{
    int start = lex->index();

    ADVANCE( Token_foreach, "foreach" );
    ADVANCE( '(', "(" );
    
    AST::Node expr;
    // replace with the right parsing
    skipCommaExpression( expr );
    ADVANCE( ')', ")" );

    StatementAST::Node body;
    parseStatement(body);

    ForEachStatementAST::Node ast = CreateNode<ForEachStatementAST>();
    // add here the parser results
    ast->setStatement( body );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseForInitStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseForInitStatement()" << endl;

    if ( parseDeclarationStatement(node) )
	return true;

    return skipExpressionStatement( node );
}

bool Parser::parseCompoundStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseCompoundStatement()" << endl;
    int start = lex->index();

    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    nextToken();

    StatementListAST::Node ast = CreateNode<StatementListAST>();

    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;

	StatementAST::Node stmt;
	int startStmt = lex->index();
	if( !parseStatement(stmt) ){
	    if( startStmt == lex->index() )
	        nextToken();
	    skipUntilStatement();
	} else {
	    ast->addStatement( stmt );
	}
    }
    
    clearComment();

    if( lex->lookAhead(0) != '}' ){
        reportError( i18n("} expected") );
    } else {
        nextToken();
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseIfStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseIfStatement()" << endl;

    int start = lex->index();

    ADVANCE( Token_if, "if" );

    ADVANCE( '(' , "(" );

    IfStatementAST::Node ast = CreateNode<IfStatementAST>();

    ConditionAST::Node cond;
    if( !parseCondition(cond) ){
	reportError( i18n("condition expected") );
	return false;
    }
    ADVANCE( ')', ")" );

    StatementAST::Node stmt;
    if( !parseStatement(stmt) ){
	reportError( i18n("statement expected") );
    }

    ast->setCondition( cond );
    ast->setStatement( stmt );

    if( lex->lookAhead(0) == Token_else ){
	nextToken();
	StatementAST::Node elseStmt;
	if( !parseStatement(elseStmt) ) {
	reportError( i18n("statement expected") );
	}
	ast->setElseStatement( elseStmt );
    }

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseSwitchStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseSwitchStatement()" << endl;
    int start = lex->index();
    ADVANCE( Token_switch, "switch" );

    ADVANCE( '(' , "(" );

    ConditionAST::Node cond;
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

    SwitchStatementAST::Node ast = CreateNode<SwitchStatementAST>();
    ast->setCondition( cond );
    ast->setStatement( stmt );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parseLabeledStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseLabeledStatement()" << endl;
    switch( lex->lookAhead(0) ){
    case Token_identifier:
    case Token_default:
	if( lex->lookAhead(1) == ':' ){
	    nextToken();
	    nextToken();

	    StatementAST::Node stmt;
	    if( parseStatement(stmt) ){
                node = stmt;
	        return true;
	    }
	}
	break;

    case Token_case:
    {
	nextToken();
	AST::Node expr;
	if( !parseConstantExpression(expr) ){
	    reportError( i18n("expression expected") );
	} else if( lex->lookAhead(0) == Token_ellipsis ){
	    nextToken();
	    
	    AST::Node expr2;
	    if( !parseConstantExpression(expr2) ){
	        reportError( i18n("expression expected") );
	    }
	}
	ADVANCE( ':', ":" );

	StatementAST::Node stmt;
	if( parseStatement(stmt) ){
            node = stmt;
	    return true;
	}
    }
    break;

    }

    return false;
}

bool Parser::parseBlockDeclaration( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseBlockDeclaration()" << endl;
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

    GroupAST::Node storageSpec;
    parseStorageClassSpecifier( storageSpec );

    GroupAST::Node cv;
    parseCvQualify( cv );

    TypeSpecifierAST::Node spec;
    if ( !parseTypeSpecifierOrClassSpec(spec) ) { // replace with simpleTypeSpecifier?!?!
	lex->setIndex( start );
	return false;
    }
    spec->setCvQualify( cv );

    GroupAST::Node cv2;
    parseCvQualify( cv2 );
    spec->setCv2Qualify( cv2 );

    InitDeclaratorListAST::Node declarators;
    parseInitDeclaratorList( declarators );

    if( lex->lookAhead(0) != ';' ){
        lex->setIndex( start );
	return false;
    }
    nextToken();

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
    nextToken();

    ADVANCE( Token_identifier,  "identifier" );
    ADVANCE( '=', "=" );

    NameAST::Node name;
    if( !parseName(name) ){
	reportError( i18n("Namespace name expected") );
    }

    ADVANCE( ';', ";" );

    return true;

}

bool Parser::parseDeclarationStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDeclarationStatement()" << endl;

    int start = lex->index();

    DeclarationAST::Node decl;
    if ( !parseBlockDeclaration(decl) ){
	return false;
    }

    DeclarationStatementAST::Node ast = CreateNode<DeclarationStatementAST>();
    ast->setDeclaration( decl );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    //kdDebug(9007)<< "---------------------> found a block declaration" << endl;
    return true;
}

bool Parser::parseDeclarationInternal( DeclarationAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDeclarationInternal()" << endl;

    int start = lex->index();

    // that is for the case '__declspec(dllexport) int ...' or
    // '__declspec(dllexport) inline int ...', etc.
    GroupAST::Node winDeclSpec;
    parseWinDeclSpec( winDeclSpec );

    GroupAST::Node funSpec;
    bool hasFunSpec = parseFunctionSpecifier( funSpec );

    GroupAST::Node storageSpec;
    bool hasStorageSpec = parseStorageClassSpecifier( storageSpec );

    if( hasStorageSpec && !hasFunSpec )
        hasFunSpec = parseFunctionSpecifier( funSpec );

    // that is for the case 'friend __declspec(dllexport) ....'
    GroupAST::Node winDeclSpec2;
    parseWinDeclSpec( winDeclSpec2 );

    GroupAST::Node cv;
    parseCvQualify( cv );

    int index = lex->index();
    NameAST::Node name;
    if( parseName(name) && lex->lookAhead(0) == '(' ){
	// no type specifier, maybe a constructor or a cast operator??

	lex->setIndex( index );

	InitDeclaratorAST::Node declarator;
	if( parseInitDeclarator(declarator) ){
	    int endSignature = lex->index();

	    switch( lex->lookAhead(0) ){
	    case ';':
	        {
		    nextToken();

		    InitDeclaratorListAST::Node declarators = CreateNode<InitDeclaratorListAST>();

		    // update declarators position
		    int startLine, startColumn, endLine, endColumn;
		    if( declarator.get() ){
		        declarator->getStartPosition( &startLine, &startColumn );
		        declarator->getEndPosition( &endLine, &endColumn );
		        declarators->setStartPosition( startLine, startColumn );
		        declarators->setEndPosition( endLine, endColumn );
		    }
		    declarators->addInitDeclarator( declarator );

		    SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
		    ast->setInitDeclaratorList( declarators );
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
			ast->setStorageSpecifier( storageSpec );
			ast->setFunctionSpecifier( funSpec );
			ast->setInitDeclarator( declarator );
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
			ast->setStorageSpecifier( storageSpec );
			ast->setFunctionSpecifier( funSpec );
			ast->setInitDeclarator( declarator );
			ast->setText( toString(start, endSignature) );
			ast->setFunctionBody( funBody );
			node = ast;
			UPDATE_POS( node, start, lex->index() );
		        return true;
		    }
		}
		break;

	    case '(':
	    case '[':
	        // ops!! it seems a declarator
		goto start_decl;
		break;
	    }

	}

	syntaxError();
	return false;
    }

start_decl:
    lex->setIndex( index );

    if( lex->lookAhead(0) == Token_const && lex->lookAhead(1) == Token_identifier && lex->lookAhead(2) == '=' ){
	// constant definition
	nextToken();
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
    
    Comment mcomment = comment();
    clearComment();

    TypeSpecifierAST::Node spec;
    if( parseTypeSpecifier(spec) ){
	if ( !hasFunSpec )
	    parseFunctionSpecifier( funSpec ); 	// e.g. "void inline"
        spec->setCvQualify( cv );

	InitDeclaratorListAST::Node declarators;

	InitDeclaratorAST::Node decl;
	int startDeclarator = lex->index();
	bool maybeFunctionDefinition = false;

        if( lex->lookAhead(0) != ';' ){
            if( parseInitDeclarator(decl) && lex->lookAhead(0) == '{' ){
                // function definition
                maybeFunctionDefinition = true;
            } else {
                lex->setIndex( startDeclarator );
                if( !parseInitDeclaratorList(declarators) ){
                    syntaxError();
                    return false;
                }
            }
        }

	int endSignature = lex->index();
	switch( lex->lookAhead(0) ){
	case ';':
	    {
		nextToken();
		SimpleDeclarationAST::Node ast = CreateNode<SimpleDeclarationAST>();
                int line, col;
                ast->setComment( mcomment );
                if( &(*decl) ) {
                    decl->getEndPosition( &line, &col );
                
                    preparseLineComments( line );
                    Comment c = m_commentStore.getCommentInRange( line );
                    if( c ) {
                        ast->addComment( c );
                    }
                }
                
                ast->setStorageSpecifier( storageSpec );
		ast->setFunctionSpecifier( funSpec );
		ast->setText( toString(start, endSignature) );
		ast->setTypeSpec( spec );
		ast->setWinDeclSpec( winDeclSpec );
		ast->setInitDeclaratorList( declarators );
                
		node = ast;
		UPDATE_POS( node, start, lex->index() );
	    }
	    return true;

	case '{':
	    {
	        if( !maybeFunctionDefinition ){
		    syntaxError();
		    return false;
		}
	        StatementListAST::Node funBody;
	        if ( parseFunctionBody(funBody) ) {
		    FunctionDefinitionAST::Node ast = CreateNode<FunctionDefinitionAST>();
                    
                    ast->setComment( mcomment );
                    
		    ast->setWinDeclSpec( winDeclSpec );
		    ast->setStorageSpecifier( storageSpec );
		    ast->setFunctionSpecifier( funSpec );
		    ast->setText( toString(start, endSignature) );
		    ast->setTypeSpec( spec );
		    ast->setFunctionBody( funBody );
		    ast->setInitDeclarator( decl );
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
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseFunctionBody()" << endl;

    int start = lex->index();
    if( lex->lookAhead(0) != '{' ){
	return false;
    }
    nextToken();

    StatementListAST::Node ast = CreateNode<StatementListAST>();

    while( !lex->lookAhead(0).isNull() ){
	if( lex->lookAhead(0) == '}' )
	    break;

	StatementAST::Node stmt;
	int startStmt = lex->index();
	if( !parseStatement(stmt) ){
	    if( startStmt == lex->index() )
	        nextToken();
	    skipUntilStatement();
	} else
	    ast->addStatement( stmt );
    }
    
    clearComment();

    if( lex->lookAhead(0) != '}' ){
	reportError( i18n("} expected") );
    } else
	nextToken();

    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

QString Parser::toString( int start, int end, const QString& sep ) const
{
    QStringList l;

    for( int i=start; i<end; ++i ){
        const Token& t = lex->tokenAt(i);
        if( t != Token_comment )
            l << t.text();
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

bool Parser::parseTryBlockStatement( StatementAST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseTryBlockStatement()" << endl;

    int start = lex->index();
    if( lex->lookAhead(0) != Token_try ){
	return false;
    }
    nextToken();

    StatementAST::Node stmt;
    if( !parseCompoundStatement(stmt) ){
	syntaxError();
    }

    if( lex->lookAhead(0) != Token_catch ){
	reportError( i18n("catch expected") );
    }

    CatchStatementListAST::Node list = CreateNode<CatchStatementListAST>();

    while( lex->lookAhead(0) == Token_catch ){

	nextToken();
	ADVANCE( '(', "(" );
	ConditionAST::Node cond;
	if( !parseCondition(cond) ){
	    reportError( i18n("condition expected") );
	    return false;
	}
	ADVANCE( ')', ")" );

	StatementAST::Node body;
	if( !parseCompoundStatement(body) ){
	    syntaxError();
	}

	CatchStatementAST::Node cstmt = CreateNode<CatchStatementAST>();
	cstmt->setCondition( cond );
	cstmt->setStatement( body );
	int l=0, c=0;
	    if( cond.get() )
	    cond->getStartPosition( &l, &c );
	else if( body.get() )
		body->getStartPosition( &l, &c );
	    
	cstmt->setStartPosition( l, c );
	if( body.get() )
		body->getEndPosition( &l, &c );
		
	cstmt->setEndPosition( l, c ); 
	list->addStatement( cstmt );
    }

    TryBlockStatementAST::Node ast = CreateNode<TryBlockStatementAST>();
    ast->setStatement( stmt );
    ast->setCatchStatementList( list );
    UPDATE_POS( ast, start, lex->index() );
    node = ast;

    return true;
}

bool Parser::parsePrimaryExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parsePrimarExpression()" << endl;


    switch( lex->lookAhead(0) ){
        case Token_string_literal:
	{
	    AST::Node lit;
  	    parseStringLiteral( lit );
	}
	return true;

        case Token_number_literal:
        case Token_char_literal:
        case Token_true:
        case Token_false:
            nextToken();
            return true;

        case Token_this:
            nextToken();
            return true;

	case Token_dynamic_cast:
        case Token_static_cast:
        case Token_reinterpret_cast:
        case Token_const_cast:
	    {
		nextToken();

		CHECK( '<', "<" );
		AST::Node typeId;
		parseTypeId( typeId );
		CHECK( '>', ">" );

		CHECK( '(', "(" );
		AST::Node expr;
		parseCommaExpression( expr );
		CHECK( ')', ")" );
	    }
	    return true;

	case Token_typeid:
	    {
		nextToken();
		CHECK( '(', "(" );
		AST::Node expr;
		parseCommaExpression( expr );
		CHECK( ')', ")" );
	    }
	    return true;

	case '(':
	    {
		nextToken();
		//kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "token = " << lex->lookAhead(0).text() << endl;
		AST::Node expr;
		if( !parseExpression(expr) ){
		    return false;
		}
		CHECK( ')', ")" );
	    }
	    return true;

	default:
	    {
		int start = lex->index();
		TypeSpecifierAST::Node typeSpec;
		if( parseSimpleTypeSpecifier(typeSpec) && lex->lookAhead(0) == '(' ){
		    nextToken();
		    AST::Node expr;
		    parseCommaExpression( expr );
		    CHECK( ')', ")" );
		    return true;
		}

		lex->setIndex( start );
		NameAST::Node name;
		if( parseName(name) )
		    return true;
	    }
	}

    return false;
}

bool Parser::parsePostfixExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parsePostfixExpression()" << endl;

    AST::Node expr;
    if( !parsePrimaryExpression(expr) )
	return false;

    while( true ){
	switch(lex->lookAhead(0))
	{
	case '[':
	    {
		nextToken();
		AST::Node e;
		parseCommaExpression( e );
		CHECK( ']', "]" );
	    }
	    break;

	case '(':
	    {
		nextToken();
		AST::Node funArgs;
		parseCommaExpression( funArgs );
		CHECK( ')', ")" );
	    }
	    break;

	case Token_incr:
	case Token_decr:
	    nextToken();
	    break;

	case '.':
	case Token_arrow:
	    {
                nextToken();
                if( lex->lookAhead(0) == Token_template )
                    nextToken();

                NameAST::Node name;
                if( !parseName(name) ){
                    return false;
                }
            }
            break;

	case Token_typename:
	    {
		nextToken();

		NameAST::Node name;
		if( !parseName(name) ){
		    return false;
		}

		CHECK( '(', "(" );
		AST::Node expr;
		parseCommaExpression(expr);
		CHECK( ')', ")" );
	    }
	    return true;

	default:
	    return true;

	} // end switch

    } // end while

    return true;
}

bool Parser::parseUnaryExpression( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseUnaryExpression()" << endl;

    switch( lex->lookAhead(0) ){
        case Token_incr:
        case Token_decr:
        case '*':
        case '&':
        case '+':
        case '-':
        case '!':
        case '~':
	{
            nextToken();
	    AST::Node expr;
            return parseCastExpression( expr );
	}

        case Token_sizeof:
	{
            nextToken();
	    int index = lex->index();
            if( lex->lookAhead(0) == '(' ){
                nextToken();
		AST::Node typeId;
		if( parseTypeId(typeId) && lex->lookAhead(0) == ')' ){
		    nextToken();
		    return true;
		}
		lex->setIndex( index );
            }
	    AST::Node expr;
	    return parseUnaryExpression( expr );
	}

        case Token_new:
            return parseNewExpression( node );

        case Token_delete:
            return parseDeleteExpression( node );
    }

    return parsePostfixExpression( node );
}

bool Parser::parseNewExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseNewExpression()" << endl;
    if( lex->lookAhead(0) == Token_scope && lex->lookAhead(1) == Token_new )
        nextToken();

    CHECK( Token_new, "new");

    if( lex->lookAhead(0) == '(' ){
        nextToken();
	AST::Node expr;
        parseCommaExpression( expr );
        CHECK( ')', ")" );
    }

    if( lex->lookAhead(0) == '(' ){
        nextToken();
	AST::Node typeId;
        parseTypeId( typeId );
        CHECK( ')', ")" );
    } else {
	AST::Node typeId;
        parseNewTypeId( typeId );
    }

    AST::Node init;
    parseNewInitializer( init );
    return true;
}

bool Parser::parseNewTypeId( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseNewTypeId()" << endl;
    TypeSpecifierAST::Node typeSpec;
    if( parseTypeSpecifier(typeSpec) ){
	AST::Node declarator;
        parseNewDeclarator( declarator );
        return true;
    }

    return false;
}

bool Parser::parseNewDeclarator( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseNewDeclarator()" << endl;
    AST::Node ptrOp;
    if( parsePtrOperator(ptrOp) ){
	AST::Node declarator;
        parseNewDeclarator( declarator );
        return true;
    }

    if( lex->lookAhead(0) == '[' ){
        while( lex->lookAhead(0) == '[' ){
            nextToken();
	    AST::Node expr;
            parseExpression( expr );
            ADVANCE( ']', "]" );
        }
        return true;
    }

    return false;
}

bool Parser::parseNewInitializer( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseNewInitializer()" << endl;
    if( lex->lookAhead(0) != '(' )
        return false;

    nextToken();
    AST::Node expr;
    parseCommaExpression( expr );
    CHECK( ')', ")" );

    return true;
}

bool Parser::parseDeleteExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseDeleteExpression()" << endl;
    if( lex->lookAhead(0) == Token_scope && lex->lookAhead(1) == Token_delete )
        nextToken();

    CHECK( Token_delete, "delete" );

    if( lex->lookAhead(0) == '[' ){
        nextToken();
        CHECK( ']', "]" );
    }

    AST::Node expr;
    return parseCastExpression( expr );
}

bool Parser::parseCastExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseCastExpression()" << endl;

    int index = lex->index();

    if( lex->lookAhead(0) == '(' ){
        nextToken();
	AST::Node typeId;
        if ( parseTypeId(typeId) ) {
            if ( lex->lookAhead(0) == ')' ) {
                nextToken();
		AST::Node expr;
                if( parseCastExpression(expr) )
		    return true;
            }
        }
    }

    lex->setIndex( index );

    AST::Node expr;
    return parseUnaryExpression( expr );
}

bool Parser::parsePmExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser:parsePmExpression()" << endl;
    AST::Node expr;
    if( !parseCastExpression(expr) )
        return false;

    while( lex->lookAhead(0) == Token_ptrmem ){
        nextToken();

        if( !parseCastExpression(expr) )
            return false;
    }

    return true;
}

bool Parser::parseMultiplicativeExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseMultiplicativeExpression()" << endl;
    AST::Node expr;
    if( !parsePmExpression(expr) )
        return false;

    while( lex->lookAhead(0) == '*' || lex->lookAhead(0) == '/' || lex->lookAhead(0) == '%' ){
        nextToken();

        if( !parsePmExpression(expr) )
            return false;
    }

    return true;
}


bool Parser::parseAdditiveExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseAdditiveExpression()" << endl;
    AST::Node expr;
    if( !parseMultiplicativeExpression(expr) )
        return false;

    while( lex->lookAhead(0) == '+' || lex->lookAhead(0) == '-' ){
        nextToken();

        if( !parseMultiplicativeExpression(expr) )
            return false;
    }

    return true;
}

bool Parser::parseShiftExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseShiftExpression()" << endl;
    AST::Node expr;
    if( !parseAdditiveExpression(expr) )
        return false;

    while( lex->lookAhead(0) == Token_shift ){
        nextToken();

        if( !parseAdditiveExpression(expr) )
            return false;
    }

    return true;
}

bool Parser::parseRelationalExpression( AST::Node& /*node*/, bool templArgs )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseRelationalExpression()" << endl;
    AST::Node expr;
    if( !parseShiftExpression(expr) )
        return false;

    while( lex->lookAhead(0) == '<' || (lex->lookAhead(0) == '>' && !templArgs) ||
           lex->lookAhead(0) == Token_leq || lex->lookAhead(0) == Token_geq ){
        nextToken();

        if( !parseShiftExpression(expr) )
            return false;
    }

    return true;
}

bool Parser::parseEqualityExpression( AST::Node& /*node*/, bool templArgs )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseEqualityExpression()" << endl;
    AST::Node expr;
    if( !parseRelationalExpression(expr, templArgs) )
        return false;

    while( lex->lookAhead(0) == Token_eq || lex->lookAhead(0) == Token_not_eq ){
        nextToken();

        if( !parseRelationalExpression(expr, templArgs) )
            return false;
    }

    return true;
}

bool Parser::parseAndExpression( AST::Node& /*node*/, bool templArgs )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseAndExpression()" << endl;
    AST::Node expr;
    if( !parseEqualityExpression(expr, templArgs) )
        return false;

    while( lex->lookAhead(0) == '&' ){
        nextToken();

        if( !parseEqualityExpression(expr, templArgs) )
            return false;
    }

    return true;
}

bool Parser::parseExclusiveOrExpression( AST::Node& /*node*/, bool templArgs )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseExclusiveOrExpression()" << endl;
    AST::Node expr;
    if( !parseAndExpression(expr, templArgs) )
        return false;

    while( lex->lookAhead(0) == '^' ){
        nextToken();

        if( !parseAndExpression(expr, templArgs) )
            return false;
    }

    return true;
}

bool Parser::parseInclusiveOrExpression( AST::Node& /*node*/, bool templArgs )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseInclusiveOrExpression()" << endl;
    AST::Node expr;
    if( !parseExclusiveOrExpression(expr, templArgs) )
        return false;

    while( lex->lookAhead(0) == '|' ){
        nextToken();

        if( !parseExclusiveOrExpression(expr, templArgs) )
            return false;
    }

    return true;
}

bool Parser::parseLogicalAndExpression( AST::Node& /*node*/, bool templArgs )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseLogicalAndExpression()" << endl;

    AST::Node expr;
    if( !parseInclusiveOrExpression(expr, templArgs) )
        return false;

    while( lex->lookAhead(0) == Token_and ){
        nextToken();

        if( !parseInclusiveOrExpression(expr, templArgs) )
            return false;
    }

    return true;
}

bool Parser::parseLogicalOrExpression( AST::Node& node, bool templArgs )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseLogicalOrExpression()" << endl;

    int start = lex->index();

    AST::Node expr;
    if( !parseLogicalAndExpression(expr, templArgs) )
        return false;

    while( lex->lookAhead(0) == Token_or ){
        nextToken();

        if( !parseLogicalAndExpression(expr, templArgs) )
            return false;
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseConditionalExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseConditionalExpression()" << endl;
    AST::Node expr;
    if( !parseLogicalOrExpression(expr) )
        return false;

    if( lex->lookAhead(0) == '?' ){
        nextToken();

        if( !parseExpression(expr) )
	    return false;

        CHECK( ':', ":" );

        if( !parseAssignmentExpression(expr) )
	    return false;
    }

    return true;
}

bool Parser::parseAssignmentExpression( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseAssignmentExpression()" << endl;
    int start = lex->index();
    AST::Node expr;
    if( lex->lookAhead(0) == Token_throw && !parseThrowExpression(expr) )
        return false;
    else if( !parseConditionalExpression(expr) )
        return false;

    while( lex->lookAhead(0) == Token_assign || lex->lookAhead(0) == '=' ){
        nextToken();

        if( !parseConditionalExpression(expr) )
            return false;
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseConstantExpression( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseConstantExpression()" << endl;
    int start = lex->index();
    if( parseConditionalExpression(node) ){
	AST::Node ast = CreateNode<AST>();
	UPDATE_POS( ast, start, lex->index() );
	node = ast;
	return true;
    }
    return false;
}

bool Parser::parseExpression( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseExpression()" << endl;

    int start = lex->index();

    if( !parseCommaExpression(node) )
        return false;

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseCommaExpression( AST::Node& node )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseCommaExpression()" << endl;
    int start = lex->index();

    AST::Node expr;
    if( !parseAssignmentExpression(expr) )
        return false;

    while( lex->lookAhead(0) == ',' ){
        nextToken();

        if( !parseAssignmentExpression(expr) )
            return false;
    }

    AST::Node ast = CreateNode<AST>();
    UPDATE_POS( ast, start, lex->index() );
    node = ast;
    return true;
}

bool Parser::parseThrowExpression( AST::Node& /*node*/ )
{
    //kdDebug(9007)<< "--- tok = " << lex->lookAhead(0).text() << " -- "  << "Parser::parseThrowExpression()" << endl;
    if( lex->lookAhead(0) != Token_throw )
        return false;

    CHECK( Token_throw, "throw" );
    AST::Node expr;
    if( !parseAssignmentExpression(expr) )
        return false;

    return true;
}

bool Parser::parseIvarDeclList( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseIvarDecls( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseIvarDecl( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseIvars( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseIvarDeclarator( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseMethodDecl( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseUnarySelector( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseKeywordSelector( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseSelector( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseKeywordDecl( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseReceiver( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcMessageExpr( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseMessageArgs( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseKeywordExpr( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseKeywordArgList( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseKeywordArg( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseReservedWord( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseMyParms( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseMyParm( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseOptParmList( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcSelectorExpr( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseSelectorArg( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseKeywordNameList( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseKeywordName( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcEncodeExpr( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcString( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseProtocolRefs( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseIdentifierList( GroupAST::Node & node )
{
    int start = lex->index();

    if( lex->lookAhead(0) != Token_identifier )
	return false;

    GroupAST::Node ast = CreateNode<GroupAST>();

    AST_FROM_TOKEN( tk, lex->index() );
    ast->addNode( tk );
    nextToken();

    while( lex->lookAhead(0) == ',' ){
	nextToken();
	if( lex->lookAhead(0) == Token_identifier ){
	    AST_FROM_TOKEN( tk, lex->index() );
	    ast->addNode( tk );
	    nextToken();
	}
	ADVANCE( Token_identifier, "identifier" );
    }

    node = ast;
    UPDATE_POS( node, start, lex->index() );
    return true;
}

bool Parser::parseIdentifierColon( AST::Node & node )
{
    Q_UNUSED( node );

    if( lex->lookAhead(0) == Token_identifier && lex->lookAhead(1) == ':' ){
	nextToken();
	nextToken();
	return true;
    } // ### else if PTYPENAME -> return true ;

    return false;
}

bool Parser::parseObjcProtocolExpr( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcOpenBracketExpr( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcCloseBracket( AST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcDef( DeclarationAST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcClassDef( DeclarationAST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcClassDecl( DeclarationAST::Node & node )
{
    Q_UNUSED( node );

    ADVANCE( OBJC_CLASS, "@class" );

    GroupAST::Node idList;
    parseIdentifierList( idList );
    ADVANCE( ';', ";" );

    return true;
}

bool Parser::parseObjcProtocolDecl( DeclarationAST::Node & node )
{
    Q_UNUSED( node );

    ADVANCE( OBJC_PROTOCOL, "@protocol" );

    GroupAST::Node idList;
    parseIdentifierList( idList );
    ADVANCE( ';', ";" );

    return true;
}

bool Parser::parseObjcAliasDecl( DeclarationAST::Node & node )
{
    Q_UNUSED( node );

    ADVANCE( OBJC_ALIAS, "@alias" );

    GroupAST::Node idList;
    parseIdentifierList( idList );
    ADVANCE( ';', ";" );

    return true;
}

bool Parser::parseObjcProtocolDef( DeclarationAST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseObjcMethodDef( DeclarationAST::Node & node )
{
    Q_UNUSED( node );
    return false;
}

bool Parser::parseWinDeclSpec( GroupAST::Node & node )
{
    if( lex->lookAhead(0) == Token_identifier && lex->lookAhead(0).text() == "__declspec" && lex->lookAhead(1) == '(' && lex->lookAhead(2) != ')'){
	int start = lex->index();
	nextToken();
	nextToken(); // skip '('

	parseIdentifierList( node );
	ADVANCE( ')', ")" );

	UPDATE_POS( node, start, lex->index() );
	return true;
    }

    return false;
}

