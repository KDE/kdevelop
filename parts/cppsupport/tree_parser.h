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

#ifndef __tree_parser_h
#define __tree_parser_h

#include "ast.h"

class TreeParser
{
public:
    TreeParser();
    virtual ~TreeParser();
    
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

private:
    TreeParser( const TreeParser& source );
    void operator = ( const TreeParser& source );
};

#endif // __tree_parser_h
