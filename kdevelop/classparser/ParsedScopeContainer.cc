/***************************************************************************
                          ParsedScopeContainer.cc  -  description
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

#include "ParsedScopeContainer.h"
#include "ProgrammingByContract.h"
#include <iostream.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------- CParsedScopeContainer::CParsedScopeContainer()
 * CParsedScopeContainer()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedScopeContainer::CParsedScopeContainer()
  : scopeIterator( scopes )
{
  setItemType( PIT_SCOPE );
  scopes.setAutoDelete( true );
}

/*------------------- CParsedScopeContainer::~CParsedScopeContainer()
 * ~CParsedScopeContainer()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedScopeContainer::~CParsedScopeContainer()
{
}

/*********************************************************************
 *                                                                   *
 *                            PUBLIC METHODS                         *
 *                                                                   *
 ********************************************************************/

/*--------------------------------- CParsedScopeContainer::hasScope()
 * hasScope()
 *   Check for a scope is defined in this scope.
 *
 * Parameters:
 *   aName      Name/Path of the scope to check for.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
bool CParsedScopeContainer::hasScope( const char *aName )
{
  REQUIRE1( "Valid scope name", aName != NULL, false ); 
  REQUIRE1( "Valid scope name length", strlen( aName ) > 0, false ); 

  return ( scopes.find( aName ) != NULL );
}

/*--------------------------------- CParsedScopeContainer::getScopeByName()
 * getScopeByName()
 *   Fetch a scope by using it's name.
 *
 * Parameters:
 *   aName      Name/Path of the scope to check for.
 * Returns:
 *   Pointer to the scope or NULL if not found.
 *-----------------------------------------------------------------*/
CParsedScopeContainer *CParsedScopeContainer::getScopeByName( const char *aName )
{
  REQUIRE1( "Valid scope name", aName != NULL, NULL );
  REQUIRE1( "Valid scope name length", strlen( aName ) > 0, NULL );

  return scopes.find( aName );
}

/*--------------------------------- CParsedScopeContainer::addScope()
 * addScope()
 *   Add a new sub-scope.
 *
 * Parameters:
 *   aScope     The subscope to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedScopeContainer::addScope( CParsedScopeContainer *aScope )
{
  REQUIRE( "Valid scope", aScope != NULL );
  REQUIRE( "Valid scope name", !aScope->name.isEmpty() );
  REQUIRE( "Unique scope", !hasScope( useFullPath ? aScope->path() : aScope->name ) );

  aScope->setDeclaredInScope( path() );

  scopes.insert( ( useFullPath ? aScope->path() : aScope->name ), aScope );
}

/*------------------------------ CParsedScopeContainer::removeScope()
 * removeScope()
 *   Add a new sub-scope.
 *
 * Parameters:
 *   aScope     The subscope to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedScopeContainer::removeScope( const char *aName )
{
  REQUIRE( "Valid scope name", aName != NULL );
  REQUIRE( "Valid scope name length", strlen( aName ) > 0 );

  scopes.remove( aName );
}

/*------------------------ CParsedScopeContainer::getSortedScopeList()
 * getSortedScopeList()
 *   Returns a list with all scopes in this scope in alphabetical 
 *   order.
 *
 * Parameters:
 *   -
 * Returns:
 *   A sorted list of scopes.
 *-----------------------------------------------------------------*/
QList<CParsedScopeContainer> *CParsedScopeContainer::getSortedScopeList()
{
  return getSortedDictList<CParsedScopeContainer>( scopes );
}

/*-------------------- CParsedScopeContainer::getSortedScopeNameList()
 * getSortedScopeNameList()
 *   Returns a list with the names of all scopes in this scope in
 *   alphabetical order.
 *
 * Parameters:
 *   -
 * Returns:
 *   A list of scopenames.
 *-----------------------------------------------------------------*/
QStrList *CParsedScopeContainer::getSortedScopeNameList()
{
  return getSortedIteratorNameList<CParsedScopeContainer>( scopeIterator );
}
