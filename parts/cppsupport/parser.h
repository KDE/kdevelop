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

class ProblemReporter;

class Driver;
class Lexer;
class Token;
class Error;
class SymbolTable;

class Parser{
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

    void dump();

public /*rules*/ :

    bool parseTranslationUnit();
	
    bool parseDefinition( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseBlockDeclaration( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseLinkageSpecification( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseLinkageBody( SymbolTable* symtab, LinkageBodyAST*& decl );
    bool parseNamespace( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseNamespaceAliasDefinition( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseUsing( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseUsingDirective( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseTypedef( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseAsmDefinition( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseTemplateDeclaration( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseDeclaration( SymbolTable* symtab, DeclarationAST*& decl );


    bool parseNestedNameSpecifier();
    bool parseUnqualiedName();
    bool parseStringLiteral();
    bool parseName();
    bool parseOperatorFunctionId();
    bool parseTemplateArgumentList();
    bool parseOperator();
    bool parseCvQualify( QStringList& cv );
    bool parseSimpleTypeSpecifier();
    bool parsePtrOperator();
    bool parseTemplateArgument();
    bool parseTypeSpecifier();
    bool parseDeclarator( QString& name, int& start, int& end );
    bool parseTemplateParameterList();
    bool parseTemplateParameter();
    bool parseStorageClassSpecifier( QString& name );
    bool parseFunctionSpecifier( QString& name );
    bool parseConstantExpression();
    bool parseInitDeclaratorList( SymbolTable* symtab );
    bool parseInitDeclarator( SymbolTable* symtab );
    bool parseParameterDeclarationClause();
    bool parseCtorInitializer();
    bool parsePtrToMember();
    bool parseEnumSpecifier( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseClassSpecifier( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseElaboratedTypeSpecifier();
    bool parseDeclaratorId();
    bool parseExceptionSpecification();
    bool parseCommaExpression();
    bool parseEnumeratorList();
    bool parseEnumerator();
    bool parseTypeParameter();
    bool parseParameterDeclaration();
    bool parseTypeId();
    bool parseAbstractDeclarator();
    bool parseParameterDeclarationList();
    bool parseAssignmentExpression();
    bool parseMemberSpecification( SymbolTable* symtab, DeclarationAST*& decl );
    bool parseAccessSpecifier();
    bool parseBaseClause();
    bool parseTypeIdList();
    bool parseMemInitializerList();
    bool parseMemInitializer();
    bool parseInitializer();
    bool parseBaseSpecifierList();
    bool parseBaseSpecifier();
    bool parseInitializerClause();
    bool parseMemInitializerId();
    bool parseFunctionBody( SymbolTable* symtab );

    // expression
    bool parseExpression();

    // statement
    bool parseCondition( SymbolTable* symtab );
    bool parseStatement( SymbolTable* symtab );
    bool parseExpressionStatement( SymbolTable* symtab );
    bool parseWhileStatement( SymbolTable* symtab );
    bool parseDoStatement( SymbolTable* symtab );
    bool parseForStatement( SymbolTable* symtab );
    bool parseCompoundStatement( SymbolTable* symtab );
    bool parseForInitStatement( SymbolTable* symtab );
    bool parseIfStatement( SymbolTable* symtab );
    bool parseSwitchStatement( SymbolTable* symtab );
    bool parseLabeledStatement( SymbolTable* symtab );
    bool parseDeclarationStatement( SymbolTable* symtab );

    bool skipUntil( int token );
    bool skipUntilDeclaration();
	bool skipUntilStatement();
    bool skip( int l, int r );
	QString toString( int start, int end, const QString& sep="" );

private:
    ProblemReporter* m_problemReporter;
    Driver* driver;
    Lexer* lex;
    QString m_fileName;
    int m_errors;
    int m_maxErrors;
	SymbolTable* m_globalSymbolTable;
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
