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

#ifndef __store_walker_h
#define __store_walker_h

#include "tree_parser.h"

#include <qstringlist.h>
#include <classstore.h>

class ParsedContainer;
class ParsedClassContainer;
class ParsedScopeContainer;
class ParsedClass;
class ParsedAttribute;
class ParsedMethod;

class StoreWalker: public TreeParser
{
public:
    StoreWalker( const QString& fileName, ClassStore* store );
    virtual ~StoreWalker();
    
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
    virtual void parseAccessDeclaration( AccessDeclarationAST* );

    // type-specifier
    virtual void parseTypeSpecifier( TypeSpecifierAST* );
    virtual void parseClassSpecifier( ClassSpecifierAST* );
    virtual void parseEnumSpecifier( EnumSpecifierAST* );
    virtual void parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* );

    virtual void parseTypeDeclaratation( TypeSpecifierAST* typeSpec );
    virtual void parseDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    virtual void parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    virtual void parseFunctionArguments( DeclaratorAST* declarator, ParsedMethod* method );
    virtual void parseBaseClause( BaseClauseAST* baseClause, ParsedClass* klass );

private:
    ParsedScopeContainer* findOrInsertScopeContainer( ParsedScopeContainer* scope, const QString& name );
    ParsedAttribute* findOrInsertAttribute( ParsedClassContainer* scope, const QString& name );
    QString typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator );
    ParsedClass* getClassByName( ParsedClassContainer* container, const QString& name );
    ParsedClass* getClassByName( ClassStore* container, const QString& name );

private:
    QString m_fileName;
    QStringList m_currentScope;
    ClassStore* m_store;
    ParsedScopeContainer* m_currentScopeContainer;
    ParsedClass* m_currentClass;
    PIAccess m_currentAccess;
    bool m_inSlots;
    bool m_inSignals;
    int m_anon;

private:
    StoreWalker( const StoreWalker& source );
    void operator = ( const StoreWalker& source );
};

#endif // __store_walker_h
