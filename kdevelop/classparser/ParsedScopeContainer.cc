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
 // arrghh... global Variable
 int ScopeLevel;

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

/*------------------------------------ CParsedScopeContainer::clear()
 * clear()
 *   Clear the internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedScopeContainer::clear()
{
  int templevel=ScopeLevel;

  ScopeLevel=1;
  CParsedClassContainer::clear(templevel==0);
  scopes.clear();
}

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
  REQUIRE( "Unique scope <"+aScope->path()+">", !hasScope( useFullPath ? aScope->path() : aScope->name ) );

  if( !path().isEmpty() )
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
  return getSortedDictList<CParsedScopeContainer>( scopes, useFullPath );
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

/*------------------------------------ CParsedScopeContainer::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedScopeContainer::out()
{
  if( !comment.isEmpty() )
    kdDebug() << comment << endl;

  if( !path().isEmpty() )
  {
    kdDebug() << "Namespace " << name << " @ line " << declaredOnLine;
    kdDebug() << " - " << declarationEndsOnLine << endl;
    kdDebug() << "  Defined in files:" << endl;
    kdDebug() << "    " << declaredInFile << endl;
    kdDebug() << "    " << definedInFile << endl;
  }

  if( path().isEmpty() )
    kdDebug() << "Global ";
  kdDebug() << "Namespaces:" << endl;
    for( scopeIterator.toFirst(); 
       scopeIterator.current();
       ++scopeIterator )
    scopeIterator.current()->out();

  if( path().isEmpty() )
    kdDebug() << "Global ";
  kdDebug() << "Classes:" << endl;
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
    classIterator.current()->out();
  if( path().isEmpty() )
    kdDebug() << "Global ";
  kdDebug() << "Structures:" << endl;
  for( structIterator.toFirst();
       structIterator.current();
       ++structIterator )
    structIterator.current()->out();
  if( path().isEmpty() )
    kdDebug() << "Global ";
  kdDebug() << "Functions:" << endl;
  for( methodIterator.toFirst();
       methodIterator.current();
       ++methodIterator )
    methodIterator.current()->out();
  if( path().isEmpty() )
    kdDebug() << "Global ";
  kdDebug() << "Variables:" << endl;
  for( attributeIterator.toFirst(); 
       attributeIterator.current();
       ++attributeIterator )
    attributeIterator.current()->out();
}
