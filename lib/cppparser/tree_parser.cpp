/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "tree_parser.h"
#include <kdebug.h>
//Added by qt3to4:
#include <Q3PtrList>

TreeParser::TreeParser()
{
}

TreeParser::~TreeParser()
{
}

void TreeParser::parseTranslationUnit( TranslationUnitAST* translationUnit )
{
    //kdDebug(9007) << "TreeParser::parseTranslationUnit()" << endl;

    Q3PtrList<DeclarationAST> declarations = translationUnit->declarationList();
    Q3PtrListIterator<DeclarationAST> it( declarations );
    while( it.current() ){
	parseDeclaration( it.current() );
	++it;
    }
}

void TreeParser::parseDeclaration( DeclarationAST* declaration )
{
    //kdDebug(9007) << "TreeParser::parseDeclaration()" << endl;

    if( !declaration )
        return;

    switch( declaration->nodeType() )
    {
    case NodeType_LinkageSpecification:
	parseLinkageSpecification( static_cast<LinkageSpecificationAST*>(declaration) );
	break;

    case NodeType_Namespace:
	parseNamespace( static_cast<NamespaceAST*>(declaration) );
	break;

    case NodeType_NamespaceAlias:
	parseNamespaceAlias( static_cast<NamespaceAliasAST*>(declaration) );
	break;

    case NodeType_Using:
	parseUsing( static_cast<UsingAST*>(declaration) );
	break;

    case NodeType_UsingDirective:
	parseUsingDirective( static_cast<UsingDirectiveAST*>(declaration) );
	break;

    case NodeType_Typedef:
	parseTypedef( static_cast<TypedefAST*>(declaration) );
	break;

    case NodeType_TemplateDeclaration:
	parseTemplateDeclaration( static_cast<TemplateDeclarationAST*>(declaration) );
	break;

    case NodeType_SimpleDeclaration:
	parseSimpleDeclaration( static_cast<SimpleDeclarationAST*>(declaration) );
	break;

    case NodeType_FunctionDefinition:
	parseFunctionDefinition( static_cast<FunctionDefinitionAST*>(declaration) );
	break;

    case NodeType_AccessDeclaration:
        parseAccessDeclaration( static_cast<AccessDeclarationAST*>(declaration) );
	break;
   }
}

void TreeParser::parseLinkageSpecification( LinkageSpecificationAST* ast )
{
    //kdDebug(9007) << "TreeParser::parseLinkageSpecification()" << endl;
    if( ast->linkageBody() )
	parseLinkageBody( ast->linkageBody() );
    else if( ast->declaration() )
	parseDeclaration( ast->declaration() );
}

void TreeParser::parseNamespace( NamespaceAST* decl )
{
    //kdDebug(9007) << "TreeParser::parseNamespace()" << endl;
    if( decl->linkageBody() )
	parseLinkageBody( decl->linkageBody() );
}

void TreeParser::parseNamespaceAlias( NamespaceAliasAST* decl )
{
    //kdDebug(9007) << "TreeParser::parseNamespaceAlias()" << endl;
    Q_UNUSED( decl );
}

void TreeParser::parseUsing( UsingAST* decl )
{
    //kdDebug(9007) << "TreeParser::parseUsing()" << endl;
    Q_UNUSED( decl );
}

void TreeParser::parseUsingDirective( UsingDirectiveAST* decl )
{
    //kdDebug(9007) << "TreeParser::parseUsingDirective()" << endl;
    Q_UNUSED( decl );
}

void TreeParser::parseTypedef( TypedefAST* decl )
{
    //kdDebug(9007) << "TreeParser::parseTypedef()" << endl;
    if( decl->typeSpec() )
	parseTypeSpecifier( decl->typeSpec() );
}

void TreeParser::parseTemplateDeclaration( TemplateDeclarationAST* decl )
{
    //kdDebug(9007) << "TreeParser::parseTemplateDeclaration()" << endl;
    Q_UNUSED( decl );
}

void TreeParser::parseSimpleDeclaration( SimpleDeclarationAST* decl )
{
    //kdDebug(9007) << "TreeParser::parseSimpleDeclaration()" << endl;
    Q_UNUSED( decl );
}

void TreeParser::parseFunctionDefinition( FunctionDefinitionAST* def )
{
    //kdDebug(9007) << "TreeParser::parseFunctionDefinition()" << endl;
    Q_UNUSED( def );
}

void TreeParser::parseLinkageBody( LinkageBodyAST* linkageBody )
{
    //kdDebug(9007) << "TreeParser::parseLinkageBody()" << endl;
    Q3PtrList<DeclarationAST> declarations = linkageBody->declarationList();
    for( Q3PtrListIterator<DeclarationAST> it(declarations); it.current(); ++it ){
	parseDeclaration( it.current() );
    }
}

void TreeParser::parseTypeSpecifier( TypeSpecifierAST* typeSpec )
{
    //kdDebug(9007) << "TreeParser::parseTypeSpecifier()" << endl;
    switch( typeSpec->nodeType() )
    {
    case NodeType_ClassSpecifier:
	parseClassSpecifier( static_cast<ClassSpecifierAST*>(typeSpec) );
	break;

    case NodeType_EnumSpecifier:
	parseEnumSpecifier( static_cast<EnumSpecifierAST*>(typeSpec) );
	break;

    case NodeType_ElaboratedTypeSpecifier:
	parseElaboratedTypeSpecifier( static_cast<ElaboratedTypeSpecifierAST*>(typeSpec) );
	break;
    }
}

void TreeParser::parseClassSpecifier( ClassSpecifierAST* classSpec )
{
    //kdDebug(9007) << "TreeParser::parseClassSpecifier()" << endl;
    Q3PtrList<DeclarationAST> declarations = classSpec->declarationList();
    for( Q3PtrListIterator<DeclarationAST> it(declarations); it.current(); ++it ){
	parseDeclaration( it.current() );
    }
}

void TreeParser::parseEnumSpecifier( EnumSpecifierAST* enumSpec )
{
    //kdDebug(9007) << "TreeParser::parseEnumSpecifier()" << endl;
    Q_UNUSED( enumSpec );
}

void TreeParser::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* typeSpec )
{
    //kdDebug(9007) << "TreeParser::parseElaboratedTypeSpecifier()" << endl;
    Q_UNUSED( typeSpec );
}

void TreeParser::parseAccessDeclaration ( AccessDeclarationAST * access )
{
    //kdDebug(9007) << "TreeParser::parseAccessDeclaration()" << endl;
    Q_UNUSED( access );
}

