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

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluestack.h>

class ParserPrivateData;
class ProblemReporter;
class QDomElement;

class ClassStore;
class ParsedClass;
class ParsedParent;
class ParsedClassContainer;
class ParsedScopeContainer;

class Driver;
class Lexer;
class Token;
class Error;

class Parser: public QObject{
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

    bool parseTranslationUnit( ClassStore* store=0 );

    bool parseDefinition( QDomElement& );
    bool parseBlockDeclaration();
    bool parseLinkageSpecification();
    bool parseLinkageBody();
    bool parseNamespace();
    bool parseNamespaceAliasDefinition();
    bool parseUsing();
    bool parseUsingDirective();
    bool parseTypedef();
    bool parseAsmDefinition();
    bool parseTemplateDeclaration();
    bool parseDeclaration();

    bool parseNestedNameSpecifier();
    bool parseUnqualiedName( QDomElement& );
    bool parseStringLiteral();
    bool parseName( QDomElement& );
    bool parseOperatorFunctionId( QDomElement& );
    bool parseTemplateArgumentList();
    bool parseOperator( QDomElement& );
    bool parseCvQualify();
    bool parseSimpleTypeSpecifier( QDomElement& );
    bool parsePtrOperator();
    bool parseTemplateArgument();
    bool parseTypeSpecifier( QDomElement& );
    bool parseTypeSpecifierOrClassSpec( QDomElement& );
    bool parseDeclarator( QDomElement& );
    bool parseTemplateParameterList();
    bool parseTemplateParameter();
    bool parseStorageClassSpecifier();
    bool parseFunctionSpecifier();
    bool parseInitDeclaratorList();
    bool parseInitDeclarator( QDomElement& );
    bool parseParameterDeclarationClause();
    bool parseCtorInitializer();
    bool parsePtrToMember();
    bool parseEnumSpecifier( QDomElement& );
    bool parseClassSpecifier( QDomElement& );
    bool parseElaboratedTypeSpecifier( QDomElement& );
    bool parseDeclaratorId( QDomElement& );
    bool parseExceptionSpecification();
    bool parseEnumeratorList();
    bool parseEnumerator();
    bool parseTypeParameter();
    bool parseParameterDeclaration();
    bool parseTypeId();
    bool parseAbstractDeclarator();
    bool parseParameterDeclarationList();
    bool parseMemberSpecification();
    bool parseAccessSpecifier( QString& );
    bool parseTypeIdList();
    bool parseMemInitializerList();
    bool parseMemInitializer();
    bool parseInitializer();
    bool parseBaseClause( QDomElement& parents );
    bool parseBaseSpecifierList( QDomElement& parents );
    bool parseBaseSpecifier( QDomElement& parent );
    bool parseInitializerClause();
    bool parseMemInitializerId();
    bool parseFunctionBody();

    // expression
    bool parseExpression();
    bool parseConstantExpression();
    bool parseCommaExpression();
    bool parseAssignmentExpression();

    // statement
    bool parseCondition();
    bool parseStatement();
    bool parseExpressionStatement();
    bool parseWhileStatement();
    bool parseDoStatement();
    bool parseForStatement();
    bool parseCompoundStatement();
    bool parseForInitStatement();
    bool parseIfStatement();
    bool parseSwitchStatement();
    bool parseLabeledStatement();
    bool parseDeclarationStatement();
    bool parseTryBlockStatement();

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
    ClassStore* m_store;
    QValueStack<ParsedScopeContainer*> m_scopeStack;
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
