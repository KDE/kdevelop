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

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qvaluestack.h>

class ParserPrivateData;

class Driver;
class Lexer;
class Token;
class Error;

class Problem
{
public:
    Problem() {}
    Problem( const Problem& source )
	: m_text( source.m_text ), m_line( source.m_line ), m_column( source.m_column ) {}
    Problem( const QString& text, int line, int column )
	: m_text( text ), m_line( line ), m_column( column ) {}
    
    Problem& operator = ( const Problem& source )
    {
	m_text = source.m_text;
	m_line = source.m_line;
	m_column = source.m_column;
	return( *this );
    }
    
    bool operator == ( const Problem& p ) const
    {
	return m_text == p.m_text && m_line == p.m_line && m_column == p.m_column;
    }
    
    QString text() const { return m_text; }
    int line() const { return m_line; }
    int column() const { return m_column; }
    
private:
    QString m_text;
    int m_line;
    int m_column;
};

class Parser
{
public:
    Parser( Driver* drv, Lexer* lexer );
    virtual ~Parser();

    QString fileName() const;
    void setFileName( const QString& fileName );

    QValueList<Problem> problems() const { return m_problems; }
    
private:
    virtual bool reportError( const Error& err );
    /*TODO: remove*/ virtual bool reportError( const QString& msg );
    /*TODO: remove*/ virtual void syntaxError();
    /*TODO: remove*/ virtual void parseError();

public /*rules*/ :

    bool parseTranslationUnit( TranslationUnitAST::Node& node );

    bool parseDefinition( DeclarationAST::Node& node );
    bool parseBlockDeclaration( DeclarationAST::Node& node );
    bool parseLinkageSpecification( DeclarationAST::Node& node );
    bool parseLinkageBody( LinkageBodyAST::Node& node );
    bool parseNamespace( DeclarationAST::Node& node );
    bool parseNamespaceAliasDefinition( DeclarationAST::Node& node );
    bool parseUsing( DeclarationAST::Node& node );
    bool parseUsingDirective( DeclarationAST::Node& node );
    bool parseTypedef( DeclarationAST::Node& node );
    bool parseAsmDefinition( DeclarationAST::Node& node );
    bool parseTemplateDeclaration( DeclarationAST::Node& node );
    bool parseDeclaration( DeclarationAST::Node& node );
    
    bool parseNestedNameSpecifier( NestedNameSpecifierAST::Node& node );
    bool parseUnqualifiedName( AST::Node& node );
    bool parseStringLiteral( AST::Node& node );
    bool parseName( NameAST::Node& node );
    bool parseOperatorFunctionId( AST::Node& node );
    bool parseTemplateArgumentList( TemplateArgumentListAST::Node& node );
    bool parseOperator( AST::Node& node );
    bool parseCvQualify( AST::Node& node );
    bool parseSimpleTypeSpecifier( TypeSpecifierAST::Node& node );
    bool parsePtrOperator( AST::Node& node );
    bool parseTemplateArgument( AST::Node& node );
    bool parseTypeSpecifier( TypeSpecifierAST::Node& node );
    bool parseTypeSpecifierOrClassSpec( TypeSpecifierAST::Node& node );
    bool parseDeclarator( AST::Node& node );
    bool parseTemplateParameterList( AST::Node& node );
    bool parseTemplateParameter( AST::Node& node );
    bool parseStorageClassSpecifier( AST::Node& node );
    bool parseFunctionSpecifier( AST::Node& node );
    bool parseInitDeclaratorList( InitDeclaratorListAST::Node& node );
    bool parseInitDeclarator( AST::Node& node );
    bool parseParameterDeclarationClause( AST::Node& node );
    bool parseCtorInitializer( AST::Node& node );
    bool parsePtrToMember( AST::Node& node );
    bool parseEnumSpecifier( TypeSpecifierAST::Node& node );
    bool parseClassSpecifier( TypeSpecifierAST::Node& node );
    bool parseElaboratedTypeSpecifier( TypeSpecifierAST::Node& node );
    bool parseDeclaratorId( NameAST::Node& node );
    bool parseExceptionSpecification( AST::Node& node );
    bool parseEnumerator( EnumeratorAST::Node& node );
    bool parseTypeParameter( AST::Node& node );
    bool parseParameterDeclaration( AST::Node& node );
    bool parseTypeId( AST::Node& node );
    bool parseAbstractDeclarator( AST::Node& node );
    bool parseParameterDeclarationList( AST::Node& node );
    bool parseMemberSpecification( DeclarationAST::Node& node );
    bool parseAccessSpecifier( AST::Node& node );
    bool parseTypeIdList( AST::Node& node );
    bool parseMemInitializerList( AST::Node& node );
    bool parseMemInitializer( AST::Node& node );
    bool parseInitializer( AST::Node& node );
    bool parseBaseClause( BaseClauseAST::Node& node );
    bool parseBaseSpecifier( BaseSpecifierAST::Node& node );
    bool parseInitializerClause( AST::Node& node );
    bool parseMemInitializerId( NameAST::Node& node );
    bool parseFunctionBody( AST::Node& node );

    // expression
    bool skipExpression();
    bool skipConstantExpression();
    bool skipCommaExpression();
    bool skipAssignmentExpression();
    bool skipExpressionStatement();
    
#if 0
    bool parseExpression( AST::Node& node );
    bool parsePrimaryExpression( AST::Node& node );
    bool parsePostfixExpression( AST::Node& node );
    bool parseUnaryExpression( AST::Node& node );
    bool parseNewExpression( AST::Node& node );
    bool parseNewTypeId( AST::Node& node );
    bool parseNewDeclarator( AST::Node& node );
    bool parseNewInitializer( AST::Node& node );
    bool parseDeleteExpression( AST::Node& node );
    bool parseCastExpression( AST::Node& node );
    bool parsePmExpression( AST::Node& node );
    bool parseMultiplicativeExpression( AST::Node& node );
    bool parseAdditiveExpression( AST::Node& node );
    bool parseShiftExpression( AST::Node& node );
    bool parseRelationalExpression( AST::Node& node );
    bool parseEqualityExpression( AST::Node& node );
    bool parseAndExpression( AST::Node& node );
    bool parseExclusiveOrExpression( AST::Node& node );
    bool parseInclusiveOrExpression( AST::Node& node );
    bool parseLogicalAndExpression( AST::Node& node );
    bool parseLogicalOrExpression( AST::Node& node );
    bool parseConditionalExpression( AST::Node& node );
    bool parseAssignmentExpression( AST::Node& node );
    bool parseConstantExpression( AST::Node& node );
    bool parseCommaExpression( AST::Node& node );
    bool parseThrowExpression( AST::Node& node );
#endif

    // statement
    bool parseCondition( AST::Node& node );
    bool parseStatement( StatementAST::Node& node );
    bool parseWhileStatement( StatementAST::Node& node );
    bool parseDoStatement( StatementAST::Node& node );
    bool parseForStatement( StatementAST::Node& node );
    bool parseCompoundStatement( StatementAST::Node& node );
    bool parseForInitStatement( StatementAST::Node& node );
    bool parseIfStatement( StatementAST::Node& node );
    bool parseSwitchStatement( StatementAST::Node& node );
    bool parseLabeledStatement( StatementAST::Node& node );
    bool parseDeclarationStatement( StatementAST::Node& node );
    bool parseTryBlockStatement( StatementAST::Node& node );

    bool skipUntil( int token );
    bool skipUntilDeclaration();
    bool skipUntilStatement();
    bool skip( int l, int r );
    QString toString( int start, int end, const QString& sep="" ) const;

private:
    ParserPrivateData* d;
    Driver* driver;
    Lexer* lex;
    QString m_fileName;
    QValueList<Problem> m_problems;
    int m_maxProblems;
};

inline QString Parser::fileName() const
{
    return m_fileName;
}


#endif
