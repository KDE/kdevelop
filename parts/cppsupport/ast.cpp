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

AST::AST()
    : m_start( 0 ),
      m_end( 0 )
{
}

AST::~AST()
{

}

int AST::start() const
{
    return m_start;
}

void AST::setStart( int start )
{
    m_start = start;
}

int AST::end() const
{
    return m_end;
}

void AST::setEnd( int end )
{
    m_end = end;
}


// ----------------------------------------------------------------------
DeclarationAST::DeclarationAST()
    : m_nameStart( 0 ),
      m_nameEnd( 0 )
{

}

DeclarationAST::~DeclarationAST()
{

}

int DeclarationAST::nameStart() const
{
    return m_nameStart;
}

void DeclarationAST::setNameStart( int start )
{
    m_nameStart = start;
}

int DeclarationAST::nameEnd() const
{
    return m_nameEnd;
}

void DeclarationAST::setNameEnd( int end )
{
    m_nameEnd = end;
}

// ----------------------------------------------------------------------
ClassDeclarationAST::ClassDeclarationAST()
{

}

ClassDeclarationAST::~ClassDeclarationAST()
{

}

// ----------------------------------------------------------------------
MethodDeclarationAST::MethodDeclarationAST()
{

}

MethodDeclarationAST::~MethodDeclarationAST()
{

}

// ----------------------------------------------------------------------
FieldDeclarationAST::FieldDeclarationAST()
{

}

FieldDeclarationAST::~FieldDeclarationAST()
{

}

// ----------------------------------------------------------------------
NamespaceDeclarationAST::NamespaceDeclarationAST()
{

}

NamespaceDeclarationAST::~NamespaceDeclarationAST()
{

}

// ----------------------------------------------------------------------
TranslationUnitAST::TranslationUnitAST()
{
    m_declarations.setAutoDelete( true );
}

TranslationUnitAST::~TranslationUnitAST()
{

}

void TranslationUnitAST::addDeclaration( DeclarationAST* decl )
{
    m_declarations.append( decl );
}

// ----------------------------------------------------------------------
UsingDeclarationAST::UsingDeclarationAST()
{
}

UsingDeclarationAST::~UsingDeclarationAST()
{
}

// ----------------------------------------------------------------------
TypedefDeclarationAST::TypedefDeclarationAST()
    : m_declarator( 0 )
{

}

TypedefDeclarationAST::~TypedefDeclarationAST()
{
    delete( m_declarator );
    m_declarator = 0;
}

// ----------------------------------------------------------------------
DeclaratorAST::DeclaratorAST()
    : m_nameStart( 0 ),
      m_nameEnd( 0 ),
      m_sub( 0 ),
      m_array( false ),
      m_function( false )
{

}

DeclaratorAST::~DeclaratorAST()
{
    delete( m_sub );
    m_sub = 0;
}

int DeclaratorAST::nameStart() const
{
    if( m_sub )
        return m_sub->nameStart();

    return m_nameStart;
}

void DeclaratorAST::setNameStart( int start )
{
    m_nameStart = start;
}

int DeclaratorAST::nameEnd() const
{
    if( m_sub )
        return m_sub->nameEnd();

    return m_nameEnd;
}

void DeclaratorAST::setNameEnd( int end )
{
    m_nameEnd = end;
}

DeclaratorAST* DeclaratorAST::subDeclarator() const
{
    return m_sub;
}

void DeclaratorAST::setSubDeclarator( DeclaratorAST* sub )
{
    m_sub = sub;
}

bool DeclaratorAST::isArray() const
{
    return m_array;
}

void DeclaratorAST::setIsArray( bool b )
{
    m_array = b;
}

bool DeclaratorAST::isFunction() const
{
    return m_function;
}

void DeclaratorAST::setIsFunction( bool b )
{
    m_function = b;
}

void TypedefDeclarationAST::setDeclarator( DeclaratorAST* declarator )
{
    m_declarator = declarator;
}

// ----------------------------------------------------------------------
AsmDeclaratationAST::AsmDeclaratationAST()
{

}

AsmDeclaratationAST::~AsmDeclaratationAST()
{

}

// ----------------------------------------------------------------------
TemplateDeclarationAST::TemplateDeclarationAST()
    : m_declaration( 0 )
{

}

TemplateDeclarationAST::~TemplateDeclarationAST()
{
    delete( m_declaration );
    m_declaration = 0;
}

int TemplateDeclarationAST::nameStart() const
{
    if( m_declaration )
        return m_declaration->nameStart();
    return 0;
}

int TemplateDeclarationAST::nameEnd() const
{
    if( m_declaration )
        return m_declaration->nameEnd();
    return 0;
}

void TemplateDeclarationAST::setDeclaration( DeclarationAST* decl )
{
    m_declaration = decl;
}

// ----------------------------------------------------------------------
LinkageBodyAST::LinkageBodyAST()
{
    m_declarations.setAutoDelete( true );
}

LinkageBodyAST::~LinkageBodyAST()
{

}

void LinkageBodyAST::addDeclaration( DeclarationAST* decl )
{
    m_declarations.append( decl );
}

// ----------------------------------------------------------------------
NullDeclarationAST::NullDeclarationAST()
{

}

NullDeclarationAST::~NullDeclarationAST()
{

}

// ----------------------------------------------------------------------
FriendDeclarationAST::FriendDeclarationAST()
    : m_declaration( 0 )
{

}

FriendDeclarationAST::~FriendDeclarationAST()
{
    delete( m_declaration );
    m_declaration = 0;
}

int FriendDeclarationAST::nameStart() const
{
    if( m_declaration )
        return m_declaration->nameStart();
    return 0;
}

int FriendDeclarationAST::nameEnd() const
{
    if( m_declaration )
        return m_declaration->nameEnd();
    return 0;
}

void FriendDeclarationAST::setDeclaration( DeclarationAST* decl )
{
    m_declaration = decl;
}

// ----------------------------------------------------------------------
DeclaratorListAST::DeclaratorListAST()
{
    m_declarators.setAutoDelete( true );
}

DeclaratorListAST::~DeclaratorListAST()
{

}

void DeclaratorListAST::addDeclarator( DeclaratorAST* declarator )
{
    m_declarators.append( declarator );
}
