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

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluestack.h>

class ParserPrivateData;

class Driver;
class Lexer;
class Token;
class Error;
class ProblemReporter;

class Parser
{
public:
    Parser( ProblemReporter* pr, Driver* drv, Lexer* lexer );
    virtual ~Parser();

    QString fileName() const;
    void setFileName( const QString& fileName );

    int errors() const;

    virtual void resetErrors();
    virtual bool reportError( const Error& err );
    /*TODO: remove*/ virtual bool reportError( const QString& msg );
    /*TODO: remove*/ virtual void syntaxError();
    /*TODO: remove*/ virtual void parseError();

public /*rules*/ :

    bool parseTranslationUnit( TranslationUnitAST::Ptr& node );

    bool parseDefinition( DeclarationAST::Ptr& node );
    bool parseBlockDeclaration( DeclarationAST::Ptr& node );
    bool parseLinkageSpecification( DeclarationAST::Ptr& node );
    bool parseLinkageBody( LinkageBodyAST::Ptr& node );
    bool parseNamespace( DeclarationAST::Ptr& node );
    bool parseNamespaceAliasDefinition( DeclarationAST::Ptr& node );
    bool parseUsing( DeclarationAST::Ptr& node );
    bool parseUsingDirective( DeclarationAST::Ptr& node );
    bool parseTypedef( DeclarationAST::Ptr& node );
    bool parseAsmDefinition( DeclarationAST::Ptr& node );
    bool parseTemplateDeclaration( DeclarationAST::Ptr& node );
    bool parseDeclaration( DeclarationAST::Ptr& node );
    
    bool parseNestedNameSpecifier( AST::Ptr& node );
    bool parseUnqualifiedName( AST::Ptr& node );
    bool parseStringLiteral( AST::Ptr& node );
    bool parseName( NameAST::Ptr& node );
    bool parseOperatorFunctionId( AST::Ptr& node );
    bool parseTemplateArgumentList( AST::Ptr& node );
    bool parseOperator( AST::Ptr& node );
    bool parseCvQualify( AST::Ptr& node );
    bool parseSimpleTypeSpecifier( AST::Ptr& node );
    bool parsePtrOperator( AST::Ptr& node );
    bool parseTemplateArgument( AST::Ptr& node );
    bool parseTypeSpecifier( AST::Ptr& node );
    bool parseTypeSpecifierOrClassSpec( AST::Ptr& node );
    bool parseDeclarator( AST::Ptr& node );
    bool parseTemplateParameterList( AST::Ptr& node );
    bool parseTemplateParameter( AST::Ptr& node );
    bool parseStorageClassSpecifier( AST::Ptr& node );
    bool parseFunctionSpecifier( AST::Ptr& node );
    bool parseInitDeclaratorList( AST::Ptr& node );
    bool parseInitDeclarator( AST::Ptr& node );
    bool parseParameterDeclarationClause( AST::Ptr& node );
    bool parseCtorInitializer( AST::Ptr& node );
    bool parsePtrToMember( AST::Ptr& node );
    bool parseEnumSpecifier( AST::Ptr& node );
    bool parseClassSpecifier( AST::Ptr& node );
    bool parseElaboratedTypeSpecifier( AST::Ptr& node );
    bool parseDeclaratorId( NameAST::Ptr& node );
    bool parseExceptionSpecification( AST::Ptr& node );
    bool parseEnumeratorList( AST::Ptr& node );
    bool parseEnumerator( AST::Ptr& node );
    bool parseTypeParameter( AST::Ptr& node );
    bool parseParameterDeclaration( AST::Ptr& node );
    bool parseTypeId( AST::Ptr& node );
    bool parseAbstractDeclarator( AST::Ptr& node );
    bool parseParameterDeclarationList( AST::Ptr& node );
    bool parseMemberSpecification( DeclarationAST::Ptr& node );
    bool parseAccessSpecifier( AST::Ptr& node );
    bool parseTypeIdList( AST::Ptr& node );
    bool parseMemInitializerList( AST::Ptr& node );
    bool parseMemInitializer( AST::Ptr& node );
    bool parseInitializer( AST::Ptr& node );
    bool parseBaseClause( AST::Ptr& node );
    bool parseBaseSpecifierList( AST::Ptr& node );
    bool parseBaseSpecifier( AST::Ptr& node );
    bool parseInitializerClause( AST::Ptr& node );
    bool parseMemInitializerId( NameAST::Ptr& node );
    bool parseFunctionBody( AST::Ptr& node );

    // expression
    bool skipExpression();
    bool skipConstantExpression();
    bool skipCommaExpression();
    bool skipAssignmentExpression();
    bool skipExpressionStatement();
    
#if 0
    bool parseExpression( AST::Ptr& node );
    bool parsePrimaryExpression( AST::Ptr& node );
    bool parsePostfixExpression( AST::Ptr& node );
    bool parseUnaryExpression( AST::Ptr& node );
    bool parseNewExpression( AST::Ptr& node );
    bool parseNewTypeId( AST::Ptr& node );
    bool parseNewDeclarator( AST::Ptr& node );
    bool parseNewInitializer( AST::Ptr& node );
    bool parseDeleteExpression( AST::Ptr& node );
    bool parseCastExpression( AST::Ptr& node );
    bool parsePmExpression( AST::Ptr& node );
    bool parseMultiplicativeExpression( AST::Ptr& node );
    bool parseAdditiveExpression( AST::Ptr& node );
    bool parseShiftExpression( AST::Ptr& node );
    bool parseRelationalExpression( AST::Ptr& node );
    bool parseEqualityExpression( AST::Ptr& node );
    bool parseAndExpression( AST::Ptr& node );
    bool parseExclusiveOrExpression( AST::Ptr& node );
    bool parseInclusiveOrExpression( AST::Ptr& node );
    bool parseLogicalAndExpression( AST::Ptr& node );
    bool parseLogicalOrExpression( AST::Ptr& node );
    bool parseConditionalExpression( AST::Ptr& node );
    bool parseAssignmentExpression( AST::Ptr& node );
    bool parseConstantExpression( AST::Ptr& node );
    bool parseCommaExpression( AST::Ptr& node );
    bool parseThrowExpression( AST::Ptr& node );
#endif

    // statement
    bool parseCondition( AST::Ptr& node );
    bool parseStatement( AST::Ptr& node );
    bool parseWhileStatement( AST::Ptr& node );
    bool parseDoStatement( AST::Ptr& node );
    bool parseForStatement( AST::Ptr& node );
    bool parseCompoundStatement( AST::Ptr& node );
    bool parseForInitStatement( AST::Ptr& node );
    bool parseIfStatement( AST::Ptr& node );
    bool parseSwitchStatement( AST::Ptr& node );
    bool parseLabeledStatement( AST::Ptr& node );
    bool parseDeclarationStatement( AST::Ptr& node );
    bool parseTryBlockStatement( AST::Ptr& node );

    bool skipUntil( int token );
    bool skipUntilDeclaration();
    bool skipUntilStatement();
    bool skip( int l, int r );
    QString toString( int start, int end, const QString& sep="" ) const;

private:
    ParserPrivateData* d;
    ProblemReporter* m_problemReporter;
    Driver* driver;
    Lexer* lex;
    QString m_fileName;
    int m_errors;
    int m_maxErrors;
};

inline QString Parser::fileName() const
{
    return m_fileName;
}

inline int Parser::errors() const
{
    return m_errors;
}

inline void Parser::resetErrors()
{
    m_errors = 0;
}


#endif
