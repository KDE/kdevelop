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

#ifndef __tag_creator_h
#define __tag_creator_h

#include "tree_parser.h"

#include <qstringlist.h>

class Catalog;
class Tag;

class TagCreator: public TreeParser
{
public:
    TagCreator( const QString& fileName, Catalog* c );
    virtual ~TagCreator();

    // translation-unit
    virtual void parseTranslationUnit( TranslationUnitAST* );

    // declarations
    virtual void parseNamespace( NamespaceAST* );
    virtual void parseUsingDirective( UsingDirectiveAST* );
    virtual void parseTypedef( TypedefAST* );
    virtual void parseTemplateDeclaration( TemplateDeclarationAST* );
    virtual void parseSimpleDeclaration( SimpleDeclarationAST* );
    virtual void parseFunctionDefinition( FunctionDefinitionAST* );
    virtual void parseLinkageBody( LinkageBodyAST* );
    virtual void parseAccessDeclaration( AccessDeclarationAST* );

    // type-specifier
    virtual void parseClassSpecifier( ClassSpecifierAST* );
    virtual void parseEnumSpecifier( EnumSpecifierAST* );

    virtual void parseMyDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    virtual void parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    virtual void parseFunctionArguments( Tag& tag, DeclaratorAST* declarator );
    virtual void parseBaseClause( const QString& className, BaseClauseAST* baseClause );

private:
    QString scopeOfDeclarator( DeclaratorAST* d );
    QString typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator );

private:
    Catalog* m_catalog;
    QString m_fileName;
    QStringList m_currentScope;
    QValueList<QStringList> m_imports;
    QString m_currentAccess;
    bool m_inClass;
    bool m_inSlots;
    bool m_inSignals;
    int m_anon;

private:
    TagCreator( const TagCreator& source );
    void operator = ( const TagCreator& source );
};

#endif // __tag_creator_h
