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

#include "ast.h"

// ------------------------------------------------------------------------
AST::AST()
    : m_start( 0 ), m_end( 0 )
{
}

AST::~AST()
{
}

// ------------------------------------------------------------------------
NameAST::NameAST()
    : m_global( false )
{
}

NameAST::~NameAST()
{
}

bool NameAST::isGlobal() const
{
    return m_global;
}

void NameAST::setGlobal( bool b )
{
    m_global = b;
}

void NameAST::setNestedName( NestedNameSpecifierAST::Node& nestedName )
{
    m_nestedName = nestedName;
}

NestedNameSpecifierAST* NameAST::nestedName()
{
    return m_nestedName.get();
}
    
void NameAST::setUnqualifedName( AST::Node& unqualifiedName )
{
    m_unqualifiedName = unqualifiedName;
}

AST* NameAST::unqualifiedName()
{
    return m_unqualifiedName.get();
}

// ------------------------------------------------------------------------
DeclarationAST::DeclarationAST()
{
}

DeclarationAST::~DeclarationAST()
{
}

// ------------------------------------------------------------------------
LinkageBodyAST::LinkageBodyAST()
{
    m_declarations.setAutoDelete( true );
}

LinkageBodyAST::~LinkageBodyAST()
{
}

void LinkageBodyAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;
	
    m_declarations.append( ast.release() );
}

// ------------------------------------------------------------------------
LinkageSpecificationAST::LinkageSpecificationAST()
{
}

LinkageSpecificationAST::~LinkageSpecificationAST()
{
}

QString LinkageSpecificationAST::externType() const
{
    return m_externType;
}

void LinkageSpecificationAST::setExternType( const QString& type )
{
    m_externType = type;
}

LinkageBodyAST* LinkageSpecificationAST::linkageBody()
{
    return m_linkageBody.get();
}

void LinkageSpecificationAST::setLinkageBody( LinkageBodyAST::Node& linkageBody )
{
    m_linkageBody = linkageBody;
}

DeclarationAST* LinkageSpecificationAST::declaration()
{
    return m_declaration.get();
}

void LinkageSpecificationAST::setDeclaration( DeclarationAST::Node& decl )
{
    m_declaration = decl;
}
    
// ------------------------------------------------------------------------
TranslationUnitAST::TranslationUnitAST()
{
    m_declarations.setAutoDelete( true );
}

TranslationUnitAST::~TranslationUnitAST()
{
}

void TranslationUnitAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;
	
    m_declarations.append( ast.release() );
}

// ------------------------------------------------------------------------
NamespaceAST::NamespaceAST()
{
}

NamespaceAST::~NamespaceAST()
{
}

QString NamespaceAST::namespaceName() const
{
    return m_namespaceName;
}

void NamespaceAST::setNamespaceName( const QString& name )
{
    m_namespaceName = name;
}

LinkageBodyAST* NamespaceAST::linkageBody()
{
    return m_linkageBody.get();
}

void NamespaceAST::setLinkageBody( LinkageBodyAST::Node& linkageBody )
{
    m_linkageBody = linkageBody;
}


// ------------------------------------------------------------------------
NamespaceAliasAST::NamespaceAliasAST()
{
}

NamespaceAliasAST::~NamespaceAliasAST()
{
}

QString NamespaceAliasAST::namespaceName() const
{
    return m_namespaceName;
}

void NamespaceAliasAST::setNamespaceName( const QString& name )
{
    m_namespaceName = name;
}

NameAST* NamespaceAliasAST::aliasName()
{
    return m_aliasName.get();
}

void NamespaceAliasAST::setAliasName( NameAST::Node& name )
{
    m_aliasName = name;
}

// ------------------------------------------------------------------------
UsingAST::UsingAST()
    : m_typename( false )
{
}

UsingAST::~UsingAST()
{
}

bool UsingAST::isTypename() const
{
    return m_typename;
}

void UsingAST::setTypename( bool b )
{
    m_typename = b;
}

NameAST* UsingAST::name()
{
    return m_name.get();
}

void UsingAST::setName( NameAST::Node& name )
{
    m_name = name;
}

// ------------------------------------------------------------------------
UsingDirectiveAST::UsingDirectiveAST()
{
}

UsingDirectiveAST::~UsingDirectiveAST()
{
}

NameAST* UsingDirectiveAST::name()
{
    return m_name.get();
}

void UsingDirectiveAST::setName( NameAST::Node& name )
{
    m_name = name;
}

TypedefAST::TypedefAST()
{
}

TypedefAST::~TypedefAST()
{
}

AST* TypedefAST::typeSpec()
{
    return m_typeSpec.get();
}

void TypedefAST::setTypeSpec( AST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
}

AST* TypedefAST::initDeclaratorList()
{
    return m_initDeclaratorList.get();
}

void TypedefAST::setInitDeclaratorList( AST::Node& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
}

// ------------------------------------------------------------------------
TemplateArgumentListAST::TemplateArgumentListAST()
{
}

TemplateArgumentListAST::~TemplateArgumentListAST()
{
}

void TemplateArgumentListAST::addArgument( AST::Node& arg )
{
    if( !arg.get() )
        return;
	
    m_arguments.append( arg.release() );
}


// ------------------------------------------------------------------------
TemplateDeclarationAST::TemplateDeclarationAST()
    : m_export( false )
{
}

TemplateDeclarationAST::~TemplateDeclarationAST()
{
}

bool TemplateDeclarationAST::isExport() const
{
    return m_export;
}

void TemplateDeclarationAST::setExport( bool b )
{
    m_export = b;
}

AST* TemplateDeclarationAST::templateParameterList()
{
    return m_templateParameterList.get();
}

void TemplateDeclarationAST::setTemplateParameterList( AST::Node& templateParameterList )
{
    m_templateParameterList = templateParameterList;
}

DeclarationAST* TemplateDeclarationAST::declaration()
{
    return m_declaration.get();
}

void TemplateDeclarationAST::setDeclaration( DeclarationAST::Node& declaration )
{
    m_declaration = declaration;
}

// ------------------------------------------------------------------------
ClassOrNamespaceNameAST::ClassOrNamespaceNameAST()
{
}

ClassOrNamespaceNameAST::~ClassOrNamespaceNameAST()
{
}

AST* ClassOrNamespaceNameAST::name()
{
    return m_name.get();
}

void ClassOrNamespaceNameAST::setName( AST::Node& name )
{
    m_name = name;
}

TemplateArgumentListAST* ClassOrNamespaceNameAST::templateArgumentList()
{
    return m_templateArgumentList.get();
}

void ClassOrNamespaceNameAST::setTemplateArgumentList( TemplateArgumentListAST::Node& templateArgumentList )
{
    m_templateArgumentList = templateArgumentList;
}

// ------------------------------------------------------------------------
NestedNameSpecifierAST::NestedNameSpecifierAST()
{
    m_classOrNamespaceNameList.setAutoDelete( true );
}

NestedNameSpecifierAST::~NestedNameSpecifierAST()
{
}

void NestedNameSpecifierAST::addClassOrNamespaceName( ClassOrNamespaceNameAST::Node& classOrNamespaceName )
{
    if( !classOrNamespaceName.get() )
        return;
	
    m_classOrNamespaceNameList.append( classOrNamespaceName.release() );
}

