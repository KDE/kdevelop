/***************************************************************************
                          parsedscopecontainer.cpp  -  description
                             -------------------
    begin                : Thu Nov 18 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <iostream.h>
#include "parsedclass.h"
#include "parsedscopecontainer.h"
#include "programmingbycontract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/
 // arrghh... global Variable
 int ScopeLevel;

/*-------------------- ParsedScopeContainer::ParsedScopeContainer()
 * ParsedScopeContainer()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedScopeContainer::ParsedScopeContainer()
    : scopeIterator( scopes )
{
    setItemType( PIT_SCOPE );
    scopes.setAutoDelete( true );
}

/*------------------- ParsedScopeContainer::~ParsedScopeContainer()
 * ~ParsedScopeContainer()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedScopeContainer::~ParsedScopeContainer()
{
}

/*********************************************************************
 *                                                                   *
 *                            PUBLIC METHODS                         *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ ParsedScopeContainer::clear()
 * clear()
 *   Clear the internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedScopeContainer::clear()
{
    int templevel=ScopeLevel;
    
    ScopeLevel=1;
    ParsedClassContainer::clear(templevel==0);
    scopes.clear();
}

/*--------------------------------- ParsedScopeContainer::hasScope()
 * hasScope()
 *   Check for a scope is defined in this scope.
 *
 * Parameters:
 *   aName      Name/Path of the scope to check for.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
bool ParsedScopeContainer::hasScope( const QString &aName )
{
    REQUIRE1( "Valid scope name", aName != NULL, false ); 
    REQUIRE1( "Valid scope name length", aName.length() > 0, false ); 
    
    return ( scopes.find( aName ) != NULL );
}

/*--------------------------------- ParsedScopeContainer::getScopeByName()
 * getScopeByName()
 *   Fetch a scope by using its name.
 *
 * Parameters:
 *   aName      Name/Path of the scope to check for.
 * Returns:
 *   Pointer to the scope or NULL if not found.
 *-----------------------------------------------------------------*/
ParsedScopeContainer *ParsedScopeContainer::getScopeByName( const QString &aName )
{
    REQUIRE1( "Valid scope name", aName != NULL, NULL );
    REQUIRE1( "Valid scope name length", strlen( aName ) > 0, NULL );
    
    return scopes.find( aName );
}

/*--------------------------------- ParsedScopeContainer::addScope()
 * addScope()
 *   Add a new sub-scope.
 *
 * Parameters:
 *   aScope     The subscope to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedScopeContainer::addScope( ParsedScopeContainer *aScope )
{
    REQUIRE( "Valid scope", aScope != NULL );
    REQUIRE( "Valid scope name", !aScope->name().isEmpty() );
    REQUIRE( "Unique scope", !hasScope( _useFullPath ? aScope->path() : aScope->name() ) );
    
    if ( !path().isEmpty() )
        aScope->setDeclaredInScope( path() );
    
    scopes.insert( ( _useFullPath ? aScope->path() : aScope->name() ), aScope );
}

/*------------------------------ ParsedScopeContainer::removeScope()
 * removeScope()
 *   Add a new sub-scope.
 *
 * Parameters:
 *   aScope     The subscope to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedScopeContainer::removeScope( const QString &aName )
{
    REQUIRE( "Valid scope name", aName != NULL );
    REQUIRE( "Valid scope name length", strlen( aName ) > 0 );
    
    scopes.remove( aName );
}

/*------------------------ ParsedScopeContainer::getSortedScopeList()
 * getSortedScopeList()
 *   Returns a list with all scopes in this scope in alphabetical 
 *   order.
 *
 * Parameters:
 *   -
 * Returns:
 *   A sorted list of scopes.
 *-----------------------------------------------------------------*/
QList<ParsedScopeContainer> *ParsedScopeContainer::getSortedScopeList()
{
    return getSortedDictList<ParsedScopeContainer>( scopes, _useFullPath );
}


/*-------------------- ParsedScopeContainer::getSortedScopeNameList()
 * getSortedScopeNameList()
 *   Returns a list with the names of all scopes in this scope in
 *   alphabetical order.
 *
 * Parameters:
 *   -
 * Returns:
 *   A list of scopenames.
 *-----------------------------------------------------------------*/
QStrList *ParsedScopeContainer::getSortedScopeNameList()
{
    return getSortedIteratorNameList<ParsedScopeContainer>( scopeIterator );
}


/*------------------------------------ ParsedScopeContainer::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedScopeContainer::out()
{
    if ( !_comment.isEmpty() )
        cout << _comment << endl;
    
    if ( !path().isEmpty() ) {
        cout << "Namespace " << _name << " @ line " << _declaredOnLine;
        cout << " - " << _declarationEndsOnLine << endl;
        cout << "  Defined in files:" << endl;
        cout << "    " << _declaredInFile << endl;
        cout << "    " << _definedInFile << endl;
    }
    
    if ( path().isEmpty() )
        cout << "Global ";
    cout << "Namespaces:" << endl;
    for ( scopeIterator.toFirst(); 
          scopeIterator.current();
          ++scopeIterator )
        scopeIterator.current()->out();
    
    if ( path().isEmpty() )
        cout << "Global ";
    cout << "Classes:" << endl;
    for ( classIterator.toFirst(); 
          classIterator.current();
          ++classIterator )
        classIterator.current()->out();
    if ( path().isEmpty() )
        cout << "Global ";
    cout << "Structures:" << endl;
    for ( structIterator.toFirst(); 
          structIterator.current();
          ++structIterator )
        structIterator.current()->out();
    if ( path().isEmpty() )
        cout << "Global ";
    cout << "Functions:" << endl;
    for ( methodIterator.toFirst(); 
          methodIterator.current();
          ++methodIterator )
        methodIterator.current()->out();
    if ( path().isEmpty() )
        cout << "Global ";
    cout << "Variables:" << endl;
    for ( attributeIterator.toFirst(); 
          attributeIterator.current();
          ++attributeIterator )
        attributeIterator.current()->out();
}
