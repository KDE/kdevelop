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


#ifndef  __ast_h
#define __ast_h

#include <memory>
#include <qptrlist.h>

class AST
{
public:
    typedef std::auto_ptr<AST> Ptr;
    
public:
    AST();
    virtual ~AST();
    
    void setStart( int start ) { m_start = start; }
    int start() const { return m_start; }
    
    void setEnd( int end ) { m_end = end; }
    int end() const { return m_end; }
    
private:
    int m_start;
    int m_end;
    
private:
    AST( const AST& source );
    void operator = ( const AST& source );
};

class NameAST: public AST
{
public:
    typedef std::auto_ptr<NameAST> Ptr;
    
public:
    NameAST();
    virtual ~NameAST();
        
    bool isGlobal() const;
    void setGlobal( bool b );
    
    void setNestedName( AST::Ptr& nestedName );
    AST* nestedName();
    
    void setUnqualifedName( AST::Ptr& unqualifiedName );
    AST* unqualifiedName();
    
private:
    bool m_global;
    AST::Ptr m_nestedName;
    AST::Ptr m_unqualifiedName;
    
private:
    NameAST( const NameAST& source );
    void operator = ( const NameAST& source );
};

class TranslationUnitAST: public AST
{
public:
    typedef std::auto_ptr<TranslationUnitAST> Ptr;
    
public:
    TranslationUnitAST();
    virtual ~TranslationUnitAST();
    
    void addDeclaration( AST::Ptr& ast );
    QPtrList<AST> declarations() { return m_declarations; }
    
private:
    QPtrList<AST> m_declarations;
   
private:
    TranslationUnitAST( const TranslationUnitAST& source );
    void operator = ( const TranslationUnitAST& source );
};

#endif 
