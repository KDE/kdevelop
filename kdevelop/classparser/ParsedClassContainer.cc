/***************************************************************************
                 ParsedClassContainer.cc  -  implementation
                             -------------------
    begin                : Tue Aug 27 1999
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

#include "ParsedClassContainer.h"
#include <iostream.h>
#include "ProgrammingByContract.h"


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*--------------------- CParsedClassContainer::CParsedClassContainer()
 * CParsedClassContainer()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClassContainer::CParsedClassContainer()
  : classIterator( classes )
{
  classes.setAutoDelete( true );
}

/*------------------- CParsedClassContainer::~CParsedClassContainer()
 * ~CParsedClassContainer()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClassContainer::~CParsedClassContainer()
{
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CParsedClassContainer::clear()
 * clear()
 *   Clear the internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClassContainer::clear()
{
  CParsedContainer::clear();

  classes.clear();
}

/*--------------------------------- CParsedClassContainer::addClass()
 * addClass()
 *   Add a class to the store.
 *
 * Parameters:
 *   aClass        The class to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClassContainer::addClass( CParsedClass *aClass )
{
  REQUIRE( "Valid class", aClass != NULL );
  REQUIRE( "Valid classname", !aClass->name.isEmpty() );
  REQUIRE( "Unique class", !hasClass( useFullPath ? aClass->path() : aClass->name ) );

  if( !path().isEmpty() )
    aClass->setDeclaredInScope( path() );

  // If this is a class, and we're adding another class that class
  // is a subclass.
  aClass->setIsSubClass( itemType == PIT_CLASS );

  classes.insert( useFullPath ? aClass->path() : aClass->name, aClass );
}

/*------------------------------ CParsedClassContainer::removeClass()
 * removeClass()
 *   Remove a class from the store.
 *
 * Parameters:
 *   aName        Name of the class to remove
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClassContainer::removeClass( const char *aName )
{
  REQUIRE( "Valid classname", aName != NULL );
  REQUIRE( "Valid classname", strlen( aName ) > 0 );
  REQUIRE( "Class exists", hasClass( aName ) );

  classes.remove( aName );
}

/*---------------------- CParsedClassContainer::removeWithReferences()
 * removeWithReferences()
 *   Remove all items in the store with references to the file.
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClassContainer::removeWithReferences( const char *aFile )
{
  REQUIRE( "Valid filename", aFile != NULL );
  REQUIRE( "Valid filename length", strlen( aFile ) > 0 );

  CParsedContainer::removeWithReferences( aFile );
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*--------------------------------- CParsedClassContainer::hasClass()
 * hasClass()
 *   Tells if a class exist in the store.
 *
 * Parameters:
 *   aName          Name of the class to check.
 *
 * Returns:
 *   bool           Result of the lookup.
 *-----------------------------------------------------------------*/
bool CParsedClassContainer::hasClass( const char *aName )
{
  REQUIRE1( "Valid classname", aName != NULL, false );
  REQUIRE1( "Valid classname length", strlen( aName ) > 0, false );

  return classes.find( aName ) != NULL;
}

/*---------------------------- CParsedClassContainer::getClassByName()
 * getClassByName()
 *   Get a class or subclass from the container by using its' name.
 *
 * Parameters:
 *   aName          Name of the class to fetch.
 *
 * Returns:
 *   CParsedClass * The class we looked for.
 *   NULL           Otherwise.
 *-----------------------------------------------------------------*/
CParsedClass *CParsedClassContainer::getClassByName( const char *aName )
{
  REQUIRE1( "Valid classname", aName != NULL, NULL );
  REQUIRE1( "Valid classname length", strlen( aName ) > 0, NULL );

  return classes.find( aName );
}

/*------------------------ CParsedClassContainer::getSortedClassList()
 * getSortedClassList()
 *   Get all classes in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedClass> * The classes.
 *-----------------------------------------------------------------*/
QList<CParsedClass> *CParsedClassContainer::getSortedClassList()
{
  return getSortedDictList<CParsedClass>( classes );
}

/*-------------------- CParsedClassContainer::getSortedClassNameList()
 * getSortedClassNameList()
 *   Get all classnames in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList * The classnames.
 *-----------------------------------------------------------------*/
QStrList *CParsedClassContainer::getSortedClassNameList()
{
  return getSortedIteratorNameList( classIterator );
}
