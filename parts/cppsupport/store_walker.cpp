/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "store_walker.h"
#include <classstore.h>
#include <parsedscopecontainer.h>
#include <kdebug.h>
#include <qfileinfo.h>

StoreWalker::StoreWalker( const QString& fileName, ClassStore* store )
    : m_fileName( fileName ), m_store( store ), m_currentScope( 0 ), m_currentClass( 0 )
{
}

StoreWalker::~StoreWalker()
{
}

void StoreWalker::parseTranslationUnit( TranslationUnitAST* ast )
{
    m_currentScope = m_store->globalScope();
    m_currentClass = 0;

    TreeParser::parseTranslationUnit( ast );
}

void StoreWalker::parseDeclaration( DeclarationAST* ast )
{
    TreeParser::parseDeclaration( ast );
}

void StoreWalker::parseLinkageSpecification( LinkageSpecificationAST* ast )
{
    TreeParser::parseLinkageSpecification( ast );
}

void StoreWalker::parseNamespace( NamespaceAST* ast )
{
    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    ParsedScopeContainer* ns = new ParsedScopeContainer;
    ns->setDeclaredOnLine( startLine );
    ns->setDeclaredInFile( m_fileName );
    ns->setDefinedInFile( m_fileName );
    ns->setDeclarationEndsOnLine( endLine );

    if( !ast->namespaceName() ){
	QFileInfo fileInfo( m_fileName );
	QString shortFileName = fileInfo.baseName();
	ns->setName( QString::fromLatin1("(") + shortFileName + QString::fromLatin1(")") );
    } else {
	ns->setName( ast->namespaceName()->text() );
    }

    ParsedScopeContainer* ns2 = m_currentScope->getScopeByName( ns->name() );
    if( ns2 ){
	delete( ns );
	ns = ns2;
    } else {
	m_currentScope->addScope( ns );
	m_store->addScope( ns );
    }

    ParsedScopeContainer* old_scope = m_currentScope;
    m_currentScope = ns;
    TreeParser::parseNamespace( ast );
    m_currentScope = old_scope;
}

void StoreWalker::parseNamespaceAlias( NamespaceAliasAST* ast )
{
    TreeParser::parseNamespaceAlias( ast );
}

void StoreWalker::parseUsing( UsingAST* ast )
{
    TreeParser::parseUsing( ast );
}

void StoreWalker::parseUsingDirective( UsingDirectiveAST* ast )
{
    TreeParser::parseUsingDirective( ast );
}

void StoreWalker::parseTypedef( TypedefAST* ast )
{
    TreeParser::parseTypedef( ast );
}

void StoreWalker::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
    TreeParser::parseTemplateDeclaration( ast );
}

void StoreWalker::parseSimpleDeclaration( SimpleDeclarationAST* ast )
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    InitDeclaratorListAST* declarators = ast->initDeclaratorList();

    if( typeSpec )
	parseTypeSpecifier( typeSpec );

    if( declarators ){
	QPtrList<InitDeclaratorAST> l = declarators->initDeclaratorList();

	QPtrListIterator<InitDeclaratorAST> it( l );
	while( it.current() ){
	    parseDeclaration( typeSpec, it.current() );
	    ++it;
	}
    }

    TreeParser::parseSimpleDeclaration( ast );
}

void StoreWalker::parseFunctionDefinition( FunctionDefinitionAST* ast )
{
    TreeParser::parseFunctionDefinition( ast );
}

void StoreWalker::parseLinkageBody( LinkageBodyAST* ast )
{
    TreeParser::parseLinkageBody( ast );
}

void StoreWalker::parseTypeSpecifier( TypeSpecifierAST* ast )
{
    TreeParser::parseTypeSpecifier( ast );
}

void StoreWalker::parseClassSpecifier( ClassSpecifierAST* ast )
{
    TreeParser::parseClassSpecifier( ast );
}

void StoreWalker::parseEnumSpecifier( EnumSpecifierAST* ast )
{
    TreeParser::parseEnumSpecifier( ast );
}

void StoreWalker::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* ast )
{
    TreeParser::parseElaboratedTypeSpecifier( ast );
}

void StoreWalker::parseTypeDeclaratation( TypeSpecifierAST* typeSpec )
{
    parseTypeSpecifier( typeSpec );
}

void StoreWalker::parseDeclaration( TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl )
{
}

