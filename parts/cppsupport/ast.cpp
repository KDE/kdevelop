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
TranslationUnitAST::TranslationUnitAST()
{
    m_declarations.setAutoDelete( true );
}

TranslationUnitAST::~TranslationUnitAST()
{
}

void TranslationUnitAST::addDeclaration( AST::Ptr& ast )
{
    if( !ast.get() )
        return;
	
    m_declarations.append( ast.release() );
}

