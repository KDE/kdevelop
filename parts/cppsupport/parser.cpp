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

class Symbol{
public:
	Symbol( const QString& name, int kind )
		: m_name( name ), m_kind(kind), m_sourceStart( 0 ), m_sourceEnd( 0 )
		{}

	~Symbol()
		{}

	QString name() const { return m_name; }
	int kind() const { return m_kind; }

	int sourceStart() const { return m_sourceStart; }
	void setSourceStart( int start ) { m_sourceStart = start; }

	int sourceEnd() const { return m_sourceEnd; }
	void setSourceEnd( int end ) { m_sourceEnd = end; }

private:
	QString m_name;
	int m_kind;
	int m_sourceStart;
	int m_sourceEnd;
};

class SymbolTable{
public:
	SymbolTable( const QString& name, SymbolTable* parent=0 )
		: m_name( name ), m_parent( parent )
	{ 
		m_children.setAutoDelete( true );
		m_symbols.setAutoDelete( true );

		if( m_parent ){
			m_parent->addSymbolTable( this );
		}
	}

	~SymbolTable()
	{
		if( m_parent && m_parent->m_children.findRef(this) ){ 
			m_parent->m_children.take();
		}
	}

	QString name() const { return m_name; }

	QString fullName() const {
		QStringList l;
		const SymbolTable* tab = this;
		while( tab ){
			l.push_front( tab->name() );
			tab = tab->parent();
		}
		return l.join( "::" );
	}

	SymbolTable* parent() { return m_parent; }
	const SymbolTable* parent() const { return m_parent; }

	Symbol* bind( const QString& name, int kind ){
		Symbol* sym = new Symbol( name, kind );
		m_symbols.insert( name, sym ); 
		return sym;
	}

	Symbol* lookup( const QString& name ){
		Symbol* sym = m_symbols.find( name );
		if( sym )
			return sym;
		else if( m_parent )
			return m_parent->lookup( name );
		else
			return 0;
	}
	
	

	void addSymbolTable( SymbolTable* s ){
		m_children.append( s );
	}

	void dump() {
		//kdDebug(9007) << "symboltable: " << fullName() << endl;
		for( QAsciiDictIterator<Symbol> it(m_symbols); it.current(); ++it ){
			//kdDebug(9007) << "symbol = " << it.current()->name() << " - kind = " << it.current()->kind() << endl;
		}
	 	//kdDebug(9007) << endl << endl;

		for( QPtrListIterator<SymbolTable> it(m_children); it.current(); ++it ){
			it.current()->dump();
		}
	}

private:
	QString m_name;
	SymbolTable* m_parent;
	QPtrList<SymbolTable> m_children;
	QAsciiDict<Symbol> m_symbols;

private:
	SymbolTable( const SymbolTable& );
	void operator = ( const SymbolTable& );
};


Parser::Parser( ProblemReporter* pr, Driver* drv, Lexer* lexer )
    : m_problemReporter( pr ),
      driver( drv ),
      lex( lexer )
{
    m_fileName = "<stdin>";

    m_maxErrors = 5;
	m_globalSymbolTable = 0;
    resetErrors();
}

Parser::~Parser()
{
	if( m_globalSymbolTable )
		delete( m_globalSymbolTable );
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

bool Parser::parseName()
{
    //kdDebug(9007) << "Parser::parseName()" << endl;
	
    if( lex->lookAhead(0) == Token_scope ){
        lex->nextToken();
    }
	
    parseNestedNameSpecifier();
    return parseUnqualiedName();
}

bool Parser::parseTranslationUnit()
{
    //kdDebug(9007) << "Parser::parseTranslationUnit()" << endl;

	if( m_globalSymbolTable )
		delete( m_globalSymbolTable );

	m_globalSymbolTable = new SymbolTable( "" );

	TranslationUnitAST translationUnit;
    while( !lex->lookAhead(0).isNull() ){
		DeclarationAST* decl = 0;
        if( parseDefinition(m_globalSymbolTable, decl) ){
			if( decl )
				translationUnit.addDeclaration( decl );
		} else {
            // error recovery
            skipUntilDeclaration();
        }
    }

    return m_errors == 0;
}

bool Parser::parseDefinition( SymbolTable* symtab, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseDefinition()" << endl;
	decl = 0;
    switch( lex->lookAhead(0) ){

    case ';':  /*ok*/
        lex->nextToken();
        return true;

    case Token_extern: /*ok*/
        return parseLinkageSpecification( symtab, decl ); 

    case Token_namespace: /*ok*/
        return parseNamespace( symtab, decl );

    case Token_using: /*ok*/
        return parseUsing( symtab, decl );

    case Token_typedef: /*ok*/
        return parseTypedef( symtab, decl );

    case Token_asm: /*ok*/
        return parseAsmDefinition( symtab, decl );

    case Token_template: /*ok*/
    case Token_export:
        return parseTemplateDeclaration( symtab, decl );

    default:
        if( parseEnumSpecifier(symtab, decl) || parseClassSpecifier(symtab, decl) ){
            parseInitDeclaratorList( symtab );
            ADVANCE( ';', ";" );
            return true;
        }

        return parseDeclaration( symtab, decl );

    } // end switch
}


bool Parser::parseLinkageSpecification( SymbolTable* symtab, DeclarationAST*& decl )
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
		BlockLinkageSpecificationAST* spec = new BlockLinkageSpecificationAST();		
		LinkageBodyAST* linkageBody = 0;
		
		parseLinkageBody( symtab, linkageBody );
		
		spec->setType( type );
		spec->setLinkageBody( linkageBody );
		decl = spec;
		
    } else {
		SimpleLinkageSpecificationAST* spec = new SimpleLinkageSpecificationAST();
		DeclarationAST* def = 0;
        if( !parseDefinition(symtab, def) ){
            reportError( i18n("Declaration syntax error") );
        }
		
		spec->setType( type );
		spec->setDeclaration( def );
		decl = spec;
    }

    return true;
}

bool Parser::parseLinkageBody( SymbolTable* symtab, LinkageBodyAST*& decl )
{
    //kdDebug(9007) << "Parser::parseLinkageBody()" << endl;
    if( lex->lookAhead(0) != '{' ){
        return false;
    }
    lex->nextToken();
	
	decl = new LinkageBodyAST();

    while( !lex->lookAhead(0).isNull() ){
        int tk = lex->lookAhead( 0 );

        if( tk == '}' )
            break;

		DeclarationAST* def = 0;			
        if( parseDefinition(symtab, def) ){
			if( def )
				decl->addDeclaration( def );
		} else {
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

bool Parser::parseNamespace( SymbolTable* symtab, DeclarationAST*& decl )
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

	if( namespaceName.isEmpty() )
		namespaceName = "$anon$";
    
	if ( lex->lookAhead(0) == '=' ) {
        // namespace alias
        lex->nextToken();

		int startName = lex->index();
        if( parseName() ){
			QString namespaceAlias = toString( startName, lex->index() );
			
			ADVANCE( ';', ";" );
			
			NamespaceAliasDefinitionAST* ast = new NamespaceAliasDefinitionAST();
			ast->setName( namespaceName );
			ast->setAlias( namespaceAlias );
			decl = ast;
            return true;
        } else {
            reportError( i18n("namespace expected") );
            return false;
        }
    } else if( lex->lookAhead(0) != '{' ){
        reportError( i18n("{ expected") );
        return false;
    }

	NamespaceDeclarationAST* ast = new NamespaceDeclarationAST();
	LinkageBodyAST* linkageBody = 0;
    parseLinkageBody( new SymbolTable(namespaceName, symtab), linkageBody );

	ast->setName( namespaceName );
	ast->setLinkageBody( linkageBody );
	decl = ast;

    return true;
}

bool Parser::parseUsing( SymbolTable* symtab, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseUsing()" << endl;

    if( lex->lookAhead(0) != Token_using ){
        return false;
    }
    lex->nextToken();

    if( lex->lookAhead(0) == Token_namespace ){
        return parseUsingDirective( symtab, decl );
    }

    if( lex->lookAhead(0) == Token_typename )
        lex->nextToken();

	int startName = lex->index();
    if( !parseName() )
        return false;
		
	int endName = lex->index();

    ADVANCE( ';', ";" )
	
	UsingDeclarationAST* ast = new UsingDeclarationAST();
	ast->setName( toString(startName, endName) );
	decl = ast;

    return true;
}

bool Parser::parseUsingDirective( SymbolTable* /*symtab*/, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseUsingDirective()" << endl;

    if( lex->lookAhead(0) != Token_namespace ){
        return false;
    }
    lex->nextToken();

	int startName = lex->index();
    if( !parseName() ){
        reportError( i18n("Namespace name expected") );
        return false;
    }
	int endName = lex->index();

    ADVANCE( ';', ";" );
	
	UsingDirectiveAST* ast = new UsingDirectiveAST();
	ast->setName( toString(startName, endName) );
	decl = ast;

    return true;
}


bool Parser::parseOperatorFunctionId()
{
    //kdDebug(9007) << "Parser::parseOperatorFunctionId()" << endl;
    if( lex->lookAhead(0) != Token_operator ){
        return false;
    }
    lex->nextToken();


    if( parseOperator() )
        return true;
    else {
        // parse cast operator
        QStringList cv;
        if( parseCvQualify(cv) ){
        }

        if( !parseSimpleTypeSpecifier() ){
            parseError();
        }

        if( parseCvQualify(cv) ){
        }

        while( parsePtrOperator() ){
        }

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

        if( parseTemplateArgument() ){
        } else {
            parseError();
            break;
        }
    }

    return true;
}

bool Parser::parseTypedef( SymbolTable* /*symtab*/, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseTypedef()" << endl;

    if( lex->lookAhead(0) != Token_typedef ){
        return false;
    }
    lex->nextToken();

    //kdDebug(9007) << "token = " << lex->lookAhead(0).toString() << endl;
    if( !parseTypeSpecifier() ){
        reportError( i18n("Need a type specifier to declare") );
    }

    //kdDebug(9007) << "token = " << lex->lookAhead(0).toString() << endl;
	QString name;
	int nameStart, nameEnd;
    if( !parseDeclarator(name, nameStart, nameEnd) ){
        reportError( i18n("Need an identifier to declare") );
    }

	ADVANCE( ';', ";" );
	
	TypedefDeclarationAST* ast = new TypedefDeclarationAST();
	ast->setName( toString(nameStart, nameEnd) );
	decl = ast;

    return true;
}

bool Parser::parseAsmDefinition( SymbolTable* /*symtab*/, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseAsmDefinition()" << endl;

    ADVANCE( Token_asm, "asm" );
    ADVANCE( '(', '(' );

    parseStringLiteral();

    ADVANCE( ')', ')' );
    ADVANCE( ';', ';' );

	decl = new AsmDefinitionAST();
    return true;
}

bool Parser::parseTemplateDeclaration( SymbolTable* symtab, DeclarationAST*& decl )
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
	
	int parametersStart = lex->index();
	ADVANCE( '<', "<" );        
	parseTemplateParameterList();
	
	ADVANCE( '>', ">" );
	int parametersEnd = lex->index();

	DeclarationAST* def = 0;
    if( !parseDefinition(symtab, def) ){
        reportError( i18n("expected a declaration") );
    }

	TemplateDeclarationAST* ast = new TemplateDeclarationAST();
	ast->setParameters( toString(parametersEnd, parametersStart, " ") );
	ast->setDeclaration( def );
	decl = ast;
	
    return true;
}

bool Parser::parseOperator()
{
    //kdDebug(9007) << "Parser::parseOperator()" << endl;

    switch( lex->lookAhead(0) ){
    case Token_new:
    case Token_delete:
        lex->nextToken();
        if( lex->lookAhead(0) == '[' && lex->lookAhead(1) == ']' ){
            lex->nextToken();
            lex->nextToken();
        } else {
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

bool Parser::parseCvQualify( QStringList& l )
{
    //kdDebug(9007) << "Parser::parseCvQualify()" << endl;

    int n = 0;
    while( !lex->lookAhead(0).isNull() ){
        int tk = lex->lookAhead( 0 );
        if( tk == Token_const || tk == Token_volatile ){
            ++n;
            l << lex->lookAhead( 0 ).toString();
            lex->nextToken();
        } else
            break;
    }
    return n != 0;
}

bool Parser::parseSimpleTypeSpecifier()
{
    //kdDebug(9007) << "Parser::parseSimpleTypeSpecifier()" << endl;

    bool isIntegral = false;

    while( !lex->lookAhead(0).isNull() ){
        int tk = lex->lookAhead( 0 );

        if( tk == Token_char || tk == Token_wchar_t || tk == Token_bool || tk == Token_short ||
            tk == Token_int || tk == Token_long || tk == Token_signed || tk == Token_unsigned ||
            tk == Token_float || tk == Token_double || tk == Token_void ){

            isIntegral = true;
            lex->nextToken();
        } else if( isIntegral ){
            return true;
        } else
            break;
    }

    //kdDebug(9007) << "!! token = " << lex->lookAhead(0).toString() << endl;
    return parseName();
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

    QStringList cv;
    if( parseCvQualify(cv) ){
    }

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

bool Parser::parseTypeSpecifier()
{
    //kdDebug(9007) << "Parser::parseTypeSpecifier()" << endl;

    QStringList cv;
    parseCvQualify(cv);

    if( parseElaboratedTypeSpecifier() || parseSimpleTypeSpecifier() ){
        parseCvQualify( cv );
    } else
        return false;

    return true;
}

bool Parser::parseDeclarator( QString& name, int& start, int& end )
{
    //kdDebug(9007) << "Parser::parseDeclarator()" << endl;

    while( parsePtrOperator() )
        ;

    if( lex->lookAhead(0) == '(' ){
        lex->nextToken();

        if( !parseDeclarator(name, start, end) ){
            return false;
        }

        if( lex->lookAhead(0) != ')'){
            return false;
        }
        lex->nextToken();

    } else {

		start = lex->index();
        if( !parseDeclaratorId() ){
            return false;
        }
		end = lex->index();
		name = toString( start, end );


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

        QStringList cv;
        parseCvQualify( cv );

        parseExceptionSpecification();
    }

    return true;
}

bool Parser::parseEnumSpecifier( SymbolTable* /*symtab*/, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseEnumSpecifier()" << endl;

    int index = lex->index();

	QString s;
	while( parseStorageClassSpecifier(s) )
		;

    if( lex->lookAhead(0) != Token_enum ){
        return false;
    }

    lex->nextToken();

	QString name;
    if( lex->lookAhead(0) == Token_identifier ){
		name = lex->lookAhead( 0 ).toString();
        lex->nextToken();
    }

	if( lex->lookAhead(0) != '{' ){
		lex->setIndex( index );
		return true;
	}
	lex->nextToken();

	EnumDeclarationAST* ast = new EnumDeclarationAST();
	ast->setName( name );
	decl = ast;
	
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

        if( parseTemplateParameter() ){
        } else {
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

bool Parser::parseStorageClassSpecifier( QString& name )
{
    //kdDebug(9007) << "Parser::parseStorageClassSpecifier()" << endl;

    switch( lex->lookAhead(0) ){
    case Token_friend:
    case Token_auto:
    case Token_register:
    case Token_static:
    case Token_extern:
    case Token_mutable:
		name = lex->lookAhead(0).toString();
        lex->nextToken();
        return true;
    }

    return false;
}

bool Parser::parseFunctionSpecifier( QString& name )
{
    //kdDebug(9007) << "Parser::parseFunctionSpecifier()" << endl;

    switch( lex->lookAhead(0) ){
    case Token_inline:
    case Token_virtual:
    case Token_explicit:
        lex->nextToken();
		name = lex->lookAhead(0).toString();
        return true;
    }

    return false;
}

bool Parser::parseTypeId()
{
    //kdDebug(9007) << "Parser::parseTypeId()" << endl;

    if( !parseTypeSpecifier() ){
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

		QString name;
		int nameStart, nameEnd;
        if( !parseDeclarator(name, nameStart, nameEnd) ){
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

        QStringList cv;
        parseCvQualify( cv );

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


bool Parser::parseInitDeclaratorList( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseInitDeclaratorList()" << endl;

    if( !parseInitDeclarator(symtab) ){
        return false;
    }

    while( lex->lookAhead(0) == ',' ){
        lex->nextToken();

        if( !parseInitDeclarator(symtab) ){
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
    if( !parseTypeSpecifier() ){
        lex->setIndex( index );
        return false;
    }

    index = lex->index();

	QString name;
	int nameStart, nameEnd;
    if( !parseDeclarator(name, nameStart, nameEnd) ){
        lex->setIndex( index );
        parseAbstractDeclarator();
    }

    if( lex->lookAhead(0) == '=' ){
        lex->nextToken();
        if( !parseAssignmentExpression() ){
            reportError( i18n("Expression expected") );
        }
    }

    return true;
}

bool Parser::parseClassSpecifier( SymbolTable* symtab, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseClassSpecifier()" << endl;

    int index = lex->index();

	QString s;
	while( parseStorageClassSpecifier(s) )
		;
		

    int kind = lex->lookAhead( 0 );
    if( kind == Token_class || kind == Token_struct || kind == Token_union ){
        lex->nextToken();
    } else {
        return false;
    }

	int nameStart = lex->index();
	parseUnqualiedName();
	int nameEnd = lex->index();
	
	QString name = toString( nameStart, nameEnd );

    parseBaseClause();

    if( lex->lookAhead(0) != '{' ){
        lex->setIndex( index );
        return false;
    }

    ADVANCE( '{', '{' );
	
	ClassDeclarationAST* klass = new ClassDeclarationAST();
	decl = klass;
	klass->setName( name );

	SymbolTable* my = new SymbolTable( name, symtab );
    
    while( !lex->lookAhead(0).isNull() ){
        if( lex->lookAhead(0) == '}' )
            break;

		DeclarationAST* def = 0;
        if( parseMemberSpecification(my, def) ){
			klass->addDeclaration( def );
		} else {
            skipUntilDeclaration();
        }
    }

	if( lex->lookAhead(0) != '}' ){
		reportError( i18n("} missing") );
	} else
		lex->nextToken();

    return true;
}

bool Parser::parseAccessSpecifier()
{
    //kdDebug(9007) << "Parser::parseAccessSpecifier()" << endl;

    switch( lex->lookAhead(0) ){
    case Token_public:
    case Token_protected:
    case Token_private:
        lex->nextToken();
        return true;
    }

    return false;
}

bool Parser::parseMemberSpecification( SymbolTable* symtab, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseMemberSpecification()" << endl;

    if( lex->lookAhead(0) == ';' ){
        lex->nextToken();
        return true;
    } else if( lex->lookAhead(0) == Token_Q_OBJECT || lex->lookAhead(0) == Token_K_DCOP ){
        lex->nextToken();
        return true;
    } else if( lex->lookAhead(0) == Token_signals || lex->lookAhead(0) == Token_k_dcop ){
        lex->nextToken();
		ADVANCE( ':', ":" );
        return true;
    } else if( parseTypedef(symtab, decl) ){
        return true;
    } else if( parseUsing(symtab, decl) ){
        return true;
    } else if( parseTemplateDeclaration(symtab, decl) ){
        return true;
    } else if( parseAccessSpecifier() ){
        if( lex->lookAhead(0) == Token_slots ){
            lex->nextToken();
        }
		ADVANCE( ':', ":" );
        return true;
    }

	if( parseEnumSpecifier(symtab, decl) || parseClassSpecifier(symtab, decl) ){
		parseInitDeclaratorList( symtab );
		ADVANCE( ';', ";" );
		return true;
	}
    return parseDeclaration( symtab, decl );
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

bool Parser::parseElaboratedTypeSpecifier()
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

        if( parseName() ){
            return true;
        }
    }

    lex->setIndex( index );
    return false;
}

bool Parser::parseDeclaratorId()
{
    //kdDebug(9007) << "Parser::parseDeclaratorId()" << endl;
    return parseName();
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
            reportError( i18n("Enumerator expected") );
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

        if( !parseConstantExpression() ){
            reportError( i18n("Constant expression expected") );
        }
    }

    return true;
}

bool Parser::parseInitDeclarator( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseInitDeclarator()" << endl;

	QString name;
	int nameStart, nameEnd;
    if( !parseDeclarator(name, nameStart, nameEnd) ){
        return false;
    }

    if( !name.isEmpty() )
    	symtab->bind( name, 10 );

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

bool Parser::parseBaseClause()
{
    //kdDebug(9007) << "Parser::parseBaseClause()" << endl;

    if( lex->lookAhead(0) != ':' ){
        return false;
    }
    lex->nextToken();


    if( !parseBaseSpecifierList() ){
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

bool Parser::parseBaseSpecifierList()
{
    //kdDebug(9007) << "Parser::parseBaseSpecifierList()" << endl;

    if( !parseBaseSpecifier() ){
        return false;
    }

    while( lex->lookAhead(0) == ',' ){
        lex->nextToken();

        if( parseBaseSpecifier() ){
        } else {
            reportError( i18n("Base class specifier expected") );
            break;
        }
    }

    return true;
}

bool Parser::parseBaseSpecifier()
{
    //kdDebug(9007) << "Parser::parseBaseSpecifier()" << endl;

    if( lex->lookAhead(0) == Token_virtual ){
        lex->nextToken();

        if( parseAccessSpecifier() ){
        }
    } else {

        if( parseAccessSpecifier() ){
        }

        if( lex->lookAhead(0) == Token_virtual ){
            lex->nextToken();
        }
    }

    if( lex->lookAhead(0) == Token_scope ){
        lex->nextToken();
    }

    if( !parseName() ){
        reportError( i18n("Identifier expected") );
    }

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
            reportError( i18n("Expression expected") );
        }
    }

    return true;
}

bool Parser::parseMemInitializerId()
{
    //kdDebug(9007) << "Parser::parseMemInitializerId()" << endl;

    return parseName();
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

bool Parser::parseUnqualiedName()
{
    //kdDebug(9007) << "Parser::parseUnqualiedName()" << endl;

    bool isDestructor = false;

    if( lex->lookAhead(0) == Token_identifier ){
        lex->nextToken();
    } else if( lex->lookAhead(0) == '~' && lex->lookAhead(1) == Token_identifier ){
        lex->nextToken(); // skip ~
        lex->nextToken(); // skip classname
        isDestructor = true;
    } else if( lex->lookAhead(0) == Token_operator ){
        return parseOperatorFunctionId();
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
        }
    }

    return true;
}

void Parser::dump()
{
	m_globalSymbolTable->dump();
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


bool Parser::parseExpressionStatement( SymbolTable* /*symtab*/ )
{
    //kdDebug(9007) << "Parser::parseExpressionStatement()" << endl;
    parseCommaExpression();
	
    ADVANCE( ';', ";" );
    
    return true;
}

bool Parser::parseStatement( SymbolTable* symtab ) // thanks to fiore@8080.it ;-)
{
    //kdDebug(9007) << "Parser::parseStatement()" << endl;
    switch( lex->lookAhead(0) ){

    case Token_while:
        return parseWhileStatement( symtab );

    case Token_do:
        return parseDoStatement( symtab );

    case Token_for:
        return parseForStatement( symtab );

    case Token_if:
        return parseIfStatement( symtab );

    case Token_switch:
        return parseSwitchStatement( symtab );

    case Token_case:
    case Token_default:
        return parseLabeledStatement( symtab );

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
        return parseCompoundStatement( symtab );

    case Token_identifier:
        if( parseLabeledStatement(symtab) )
            return true;
        break;
    }

    if ( parseDeclarationStatement(symtab) )
        return true;

    return parseExpressionStatement( symtab );
}

bool Parser::parseCondition( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseCondition()" << endl;

    int index = lex->index();

    if( parseTypeSpecifier() ){

		QString name;
		int nameStart, nameEnd;
        if( parseDeclarator(name, nameStart, nameEnd) && lex->lookAhead(0) == '=' ) {
            lex->nextToken();

			symtab->bind( name, 10 );

            if( parseAssignmentExpression() )
                return true;
        }
    }

    lex->setIndex( index );
    return parseCommaExpression();
}


bool Parser::parseWhileStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseWhileStatement()" << endl;
    ADVANCE( Token_while, "while" );
    ADVANCE( '(' , "(" );

	SymbolTable* my = new SymbolTable( "$anon$", symtab );
    if( !parseCondition(my) ){
        reportError( i18n("condition expected") );
		return false;
    }
    ADVANCE( ')', ")" );

    if( !parseStatement(my) ){
        reportError( i18n("statement expected") );
		return false;
    }

    return true;
}

bool Parser::parseDoStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseDoStatement()" << endl;
    ADVANCE( Token_do, "do" );
    if( !parseStatement(symtab) ){
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

bool Parser::parseForStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseForStatement()" << endl;
    ADVANCE( Token_for, "for" );
    ADVANCE( '(', "(" );

	SymbolTable* my = new SymbolTable( "$anon$", symtab );

    if( !parseForInitStatement(symtab) ){
        reportError( i18n("for initialization expected") );
		return false;
    }

    parseCondition( my );
    ADVANCE( ';', ";" );
    parseCommaExpression();
    ADVANCE( ')', ")" );

    return parseStatement( my );
}

bool Parser::parseForInitStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseForInitStatement()" << endl;

    if ( parseDeclarationStatement(symtab) )
        return true;
    return parseExpressionStatement( symtab );
}

bool Parser::parseCompoundStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseCompoundStatement()" << endl;
    if( lex->lookAhead(0) != '{' ){
        return false;
    }
    lex->nextToken();

	SymbolTable* my = new SymbolTable( "$anon$", symtab );
    
	while( !lex->lookAhead(0).isNull() ){
        if( lex->lookAhead(0) == '}' )
            break;

        if( !parseStatement(my) ){
			skipUntilStatement();
		} 
    }

	ADVANCE( '}', "}" );
    return true;
}

bool Parser::parseIfStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseIfStatement()" << endl;

    ADVANCE( Token_if, "if" );

    ADVANCE( '(' , "(" );

	SymbolTable* my = new SymbolTable( "$anon$", symtab );

    if( !parseCondition(my) ){
        reportError( i18n("condition expected") );
		return false;
    }
    ADVANCE( ')', ")" );
    
	if( !parseStatement(my) ){
        reportError( i18n("statement expected") );
		return false;
    }

    if( lex->lookAhead(0) == Token_else ){
        lex->nextToken();
        if( !parseStatement(symtab) ) {
            reportError( i18n("statement expected") );
            return false;
        }
    }

    return true;
}

bool Parser::parseSwitchStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseSwitchStatement()" << endl;
    ADVANCE( Token_switch, "switch" );

    ADVANCE( '(' , "(" );

	SymbolTable* my = new SymbolTable( "$anon$", symtab );

    if( !parseCondition(my) ){
        reportError( i18n("condition expected") );
		return false;
    }
    ADVANCE( ')', ")" );

    if( !parseCompoundStatement(my) ){
		syntaxError();
		return false;
	}
	return true;
}

bool Parser::parseLabeledStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseLabeledStatement()" << endl;
    switch( lex->lookAhead(0) ){
    case Token_identifier:
    case Token_default:
        if( lex->lookAhead(1) == ':' ){
            lex->nextToken();
            lex->nextToken();
            return parseStatement( symtab );
        }
        break;

    case Token_case:
        lex->nextToken();
        if( !parseConstantExpression() ){
            reportError( i18n("expression expected") );
        }
        ADVANCE( ':', ":" );
        return parseStatement( symtab );
    }
    return false;
}

bool Parser::parseBlockDeclaration( SymbolTable* symtab, DeclarationAST*& decl )
{
    //kdDebug(9007) << "Parser::parseBlockDeclaration()" << endl;
    switch( lex->lookAhead(0) ) {
    case Token_using:
        return parseUsing( symtab, decl );
    case Token_asm:
        return parseAsmDefinition( symtab, decl );
    case Token_namespace:
        return parseNamespaceAliasDefinition( symtab, decl );
    }

    int index = lex->index();

    QStringList cv;
    parseCvQualify( cv );

    if ( !parseTypeSpecifier() ) { // replace with simpleTypeSpecifier?!?!
        lex->setIndex( index );
        return false;
    }

    parseInitDeclaratorList( symtab );

    return true;
}

bool Parser::parseNamespaceAliasDefinition( SymbolTable* /*symtab*/, DeclarationAST*& decl )
{
    if ( lex->lookAhead(0) != Token_namespace ) {
        return false;
    }
    lex->nextToken();
	QString name = lex->lookAhead( 0 ).toString();
    ADVANCE( Token_identifier,  "identifier" );
    ADVANCE( '=', "=" );
	
	int aliasStart = lex->index();
    if( !parseName() ){
		reportError( i18n("Namespace name expected") );
	}
	int aliasEnd = lex->index();
	
	ADVANCE( ';', ";" );
	
 	NamespaceAliasDefinitionAST* ast = new NamespaceAliasDefinitionAST();
	ast->setName( name );
	ast->setAlias( toString(aliasStart, aliasEnd) );
	decl = ast;
	
	return true;
	
}

bool Parser::parseDeclarationStatement( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseDeclarationStatement()" << endl;

    int index = lex->index();

	DeclarationAST* decl = 0;
    if ( !parseBlockDeclaration(symtab, decl) )
        return false;

    if ( lex->lookAhead(0) != ';' ) {
        lex->setIndex( index );
        return false;
    }
    lex->nextToken();

    return true;
}

bool Parser::parseDeclaration( SymbolTable* symtab, DeclarationAST*& decl )
{
	decl = 0;
#warning "TODO: Parser::parseDeclaration() -- fill abstract syntax tree"
	QStringList functionSpec;
	QString s;
	while( parseFunctionSpecifier(s) )
		functionSpec << s;
		
	QStringList storageSpec;
	while(  parseStorageClassSpecifier(s) )
		storageSpec << s;

    int index = lex->index();

    if( parseName() && lex->lookAhead(0) == '(' ){
        // no type specifier, maybe a constructor or a cast operator!?!?!

        lex->setIndex( index );

        parseNestedNameSpecifier();
		QString nestedName = toString( index, lex->index() );

		QString name;
		int nameStart, nameEnd;
        if( parseDeclarator(name, nameStart, nameEnd) ){
            switch( lex->lookAhead(0) ){
            case ';':
				if( !nestedName ){
                	lex->nextToken();
                	return true;
				}
				syntaxError();
				return false;
            case ':':
                if( parseCtorInitializer() && parseFunctionBody(symtab) )
                    return true;
                syntaxError();
                return false;
            case '{':
                if( !parseFunctionBody(symtab) ){
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
        if( parseInitDeclaratorList(symtab) ){
            ADVANCE( ';', ";" );
            //kdDebug(9007) << "found constant definition" << endl;
            return true;
        }
        syntaxError();
        return false;
    }

    QStringList cv;
    parseCvQualify( cv );

    if( parseTypeSpecifier() ){

        if( lex->lookAhead(0) == ';' ){
            // type definition
            lex->nextToken();
            return true;
        }

        if( parseNestedNameSpecifier() ) {
            // maybe a method declaration/definition

			QString name;
			int nameStart, nameEnd;
            if ( !parseDeclarator(name, nameStart, nameEnd) ) {
                syntaxError();
                return false;
            }
        } else if ( !parseInitDeclaratorList(symtab) ) {
            syntaxError();
            return false;
        }

        switch( lex->lookAhead(0) ){
        case ';':
            lex->nextToken();
            //kdDebug(9007) << "found function/field declaration" << endl;
            return true;

        case '{':
            if ( parseFunctionBody(symtab) ) {
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

bool Parser::parseFunctionBody( SymbolTable* symtab )
{
    //kdDebug(9007) << "Parser::parseFunctionBody()" << endl;

    if( lex->lookAhead(0) != '{' ){
        return false;
    }
    lex->nextToken();

	SymbolTable* my = new SymbolTable( "$funbody$", symtab );
    while( !lex->lookAhead(0).isNull() ){
        if( lex->lookAhead(0) == '}' )
            break;
			
		if( !parseStatement(my) ){
			skipUntilStatement();
		}
    }

	ADVANCE( '}', "}" );

    return true;
}

QString Parser::toString( int start, int end, const QString& sep )
{
	QStringList l;

	for( int i=start; i<end; ++i ){
		l << lex->tokenAt( i ).toString();
	}

	return l.join( sep ).stripWhiteSpace();
}
