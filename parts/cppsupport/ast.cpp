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

