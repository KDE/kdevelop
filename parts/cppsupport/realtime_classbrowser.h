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

#ifndef __realtime_classbrowser_h
#define __realtime_classbrowser_h

#include "tree_parser.h"
#include <qstringlist.h>

class QListView;
class QListViewItem;

class RTClassBrowser: public TreeParser
{
public:
    RTClassBrowser( const QString& fileName, QListView* lv );
    virtual ~RTClassBrowser();

    // translation-unit
    virtual void parseTranslationUnit( TranslationUnitAST* );

    // declarations
    virtual void parseDeclaration( DeclarationAST* );
    virtual void parseLinkageSpecification( LinkageSpecificationAST* );
    virtual void parseNamespace( NamespaceAST* );
    virtual void parseNamespaceAlias( NamespaceAliasAST* );
    virtual void parseUsing( UsingAST* );
    virtual void parseUsingDirective( UsingDirectiveAST* );
    virtual void parseTypedef( TypedefAST* );
    virtual void parseTemplateDeclaration( TemplateDeclarationAST* );
    virtual void parseSimpleDeclaration( SimpleDeclarationAST* );
    virtual void parseFunctionDefinition( FunctionDefinitionAST* );
    virtual void parseLinkageBody( LinkageBodyAST* );

    // type-specifier
    virtual void parseTypeSpecifier( TypeSpecifierAST* );
    virtual void parseClassSpecifier( ClassSpecifierAST* );
    virtual void parseEnumSpecifier( EnumSpecifierAST* );
    virtual void parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* );

    virtual void parseTypeDeclaratation( TypeSpecifierAST* typeSpec );
    virtual void parseDeclaration( TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    virtual void parseEnumerator( EnumeratorAST* );
    
private:
    QString m_fileName;
    QListView* m_listView;
    QListViewItem* m_currentItem;
    QStringList m_currentScope;

private:
    RTClassBrowser( const RTClassBrowser& source );
    void operator = ( const RTClassBrowser& source );
};

#endif // __realtime_classbrowser_h
