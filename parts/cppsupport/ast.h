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

class AST
{
public:
    typedef std::auto_ptr<AST> Ptr;
    
    AST()
	: start( 0 ), end( 0 ) 
    {}
    
    virtual ~AST() 
    {}
    
    int start;
    int end;

private:
    AST( const AST& source );
    void operator = ( const AST& source );
};

#endif 
