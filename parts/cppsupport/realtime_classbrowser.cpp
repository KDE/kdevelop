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

#include "realtime_classbrowser.h"
#include "ast_utils.h"

#include <kdebug.h>
#include <klistview.h>
#include <qfileinfo.h>

RTClassBrowser::RTClassBrowser( const QString& fileName, QListView* lv )
    : m_fileName( fileName ), m_listView( lv ), m_currentItem( 0 )
{
}

RTClassBrowser::~RTClassBrowser()
{
}

void RTClassBrowser::parseTranslationUnit( TranslationUnitAST* ast )
{
    m_listView->clear();
    m_currentScope.clear();

    QFileInfo fileInfo( m_fileName );

    m_currentItem = new QListViewItem( m_listView, fileInfo.fileName() );
    TreeParser::parseTranslationUnit( ast );
    m_currentItem->setOpen( true );
}

void RTClassBrowser::parseDeclaration( DeclarationAST* ast )
{
    TreeParser::parseDeclaration( ast );
}

void RTClassBrowser::parseLinkageSpecification( LinkageSpecificationAST* ast )
{
    TreeParser::parseLinkageSpecification( ast );
}

void RTClassBrowser::parseNamespace( NamespaceAST* ast )
{
    QString nsName;
    if( ast->namespaceName() )
        nsName = ast->namespaceName()->text();
    if( nsName.isEmpty() )
        nsName = QString::fromLatin1( "<noname>" );
    
    m_currentScope.push_back( nsName );
    QListViewItem* old = m_currentItem;    

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    (void) new QListViewItem( m_currentItem, nsName,
    		QString::number(startLine), QString::number(startColumn),
    		QString::number(endLine), QString::number(endColumn) );
    
    TreeParser::parseNamespace( ast );
    m_currentItem = old;
    m_currentScope.pop_back();
}

void RTClassBrowser::parseNamespaceAlias( NamespaceAliasAST* ast )
{
    
    TreeParser::parseNamespaceAlias( ast );
}

void RTClassBrowser::parseUsing( UsingAST* ast )
{
    TreeParser::parseUsing( ast );
}

void RTClassBrowser::parseUsingDirective( UsingDirectiveAST* ast )
{
    TreeParser::parseUsingDirective( ast );
}

void RTClassBrowser::parseTypedef( TypedefAST* ast )
{
    TreeParser::parseTypedef( ast );
}

void RTClassBrowser::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
    parseDeclaration( ast->declaration() );
    TreeParser::parseTemplateDeclaration( ast );
}

void RTClassBrowser::parseSimpleDeclaration( SimpleDeclarationAST* ast )
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

void RTClassBrowser::parseFunctionDefinition( FunctionDefinitionAST* ast )
{
    QString text;
    text += declaratorToString( ast->initDeclarator()->declarator(), QString::null, true );

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    (void) new QListViewItem( m_currentItem, text,
    		QString::number(startLine), QString::number(startColumn),
    		QString::number(endLine), QString::number(endColumn) );

    TreeParser::parseFunctionDefinition( ast );
}

void RTClassBrowser::parseLinkageBody( LinkageBodyAST* ast )
{
    TreeParser::parseLinkageBody( ast );
}

void RTClassBrowser::parseTypeSpecifier( TypeSpecifierAST* ast )
{
    TreeParser::parseTypeSpecifier( ast );
}

void RTClassBrowser::parseClassSpecifier( ClassSpecifierAST* ast )
{
    QString className;
    if( ast->name() )
        className = ast->name()->text();
    if( className.isEmpty() )
        className = QString::fromLatin1( "<noname>" );


    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    m_currentScope.push_back( className );

    QListViewItem* old = m_currentItem;

    m_currentItem = new QListViewItem( m_currentItem, className,
    		QString::number(startLine), QString::number(startColumn),
    		QString::number(endLine), QString::number(endColumn) );

    TreeParser::parseClassSpecifier( ast );

    m_currentItem->setOpen( true );
    m_currentItem = old;

    m_currentScope.pop_back();
}

void RTClassBrowser::parseEnumSpecifier( EnumSpecifierAST* ast )
{
    QString enumName;
    if( ast->name() )
        enumName = ast->name()->text();
    if( enumName.isEmpty() )
        enumName = QString::fromLatin1( "<noname>" );

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    QListViewItem* old = m_currentItem;
    m_currentItem = new QListViewItem( m_currentItem, enumName,
    		QString::number(startLine), QString::number(startColumn),
    		QString::number(endLine), QString::number(endColumn) );

    TreeParser::parseEnumSpecifier( ast );
    m_currentItem->setOpen( true );
    m_currentItem = old;
}

void RTClassBrowser::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* ast )
{
    TreeParser::parseElaboratedTypeSpecifier( ast );
}

void RTClassBrowser::parseTypeDeclaratation( TypeSpecifierAST* typeSpec )
{
    parseTypeSpecifier( typeSpec );
}

void RTClassBrowser::parseDeclaration( TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl )
{
    Q_UNUSED( typeSpec );

    QString text;
    text += declaratorToString( decl->declarator(), QString::null, true );
    int startLine, startColumn;
    int endLine, endColumn;

    decl->getStartPosition( &startLine, &startColumn );
    decl->getEndPosition( &endLine, &endColumn );

    (void) new QListViewItem( m_currentItem, text,
    		QString::number(startLine), QString::number(startColumn),
    		QString::number(endLine), QString::number(endColumn) );
}

