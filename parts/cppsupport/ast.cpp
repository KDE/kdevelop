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

void NameAST::setNestedName( AST::Ptr& nestedName )
{
    m_nestedName = nestedName;
}

AST* NameAST::nestedName()
{
    return m_nestedName.get();
}
    
void NameAST::setUnqualifedName( AST::Ptr& unqualifiedName )
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

void LinkageBodyAST::addDeclaration( DeclarationAST::Ptr& ast )
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

void LinkageSpecificationAST::setLinkageBody( LinkageBodyAST::Ptr& linkageBody )
{
    m_linkageBody = linkageBody;
}

DeclarationAST* LinkageSpecificationAST::declaration()
{
    return m_declaration.get();
}

void LinkageSpecificationAST::setDeclaration( DeclarationAST::Ptr& decl )
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

void TranslationUnitAST::addDeclaration( DeclarationAST::Ptr& ast )
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

void NamespaceAST::setLinkageBody( LinkageBodyAST::Ptr& linkageBody )
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

void NamespaceAliasAST::setAliasName( NameAST::Ptr& name )
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

void UsingAST::setName( NameAST::Ptr& name )
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

void UsingDirectiveAST::setName( NameAST::Ptr& name )
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

void TypedefAST::setTypeSpec( AST::Ptr& typeSpec )
{
    m_typeSpec = typeSpec;
}

AST* TypedefAST::initDeclaratorList()
{
    return m_initDeclaratorList.get();
}

void TypedefAST::setInitDeclaratorList( AST::Ptr& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
}

