/***************************************************************************
                 ParsedClassContainer.cpp  -  implementation
                             -------------------
    begin                : Tue Aug 27 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
   
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
  //  assert( aClass != NULL );
  //  assert( !aClass->name.isEmpty() );
  //  assert( !hasClass( aClass->name ) );
  if(aClass == 0 ){
    cerr << "ERROR!!! in parser  CParsedClassContainer::addClass: \n";
    return;
  }

  classes.insert( aClass->name, aClass );
}

/*------------------------------- CParsedClassContainer::addSubClass()
 * addSubClass()
 *   Store a subclass pointer using its' hierarchy as the key.
 *
 * Parameters:
 *   key           The hierarchy.
 *   aClass        The subclass to store.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClassContainer::addSubClass( const char *key, 
                                         CParsedClass *aClass )
{
  //  assert( aClass != NULL );
  //  assert( !aClass->name.isEmpty() );
  //  assert( key != NULL );
  if(aClass == 0 ){
    cerr << "ERROR!!! in parser  CParsedClassContainer::addSubClass: \n";
    return;
  }
  if(key == 0 ){
    cerr << "ERROR!!! in parser  CParsedClassContainer::addSubClass: \n";
    return;
  }

  classes.insert( key, aClass );
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
  //  assert( aName != NULL );
  //  assert( strlen( aName ) > 0 );
  //  assert( hasClass( aName ) );
  if(aName == 0 ){
    cerr << "ERROR!!! in parser void CParsedClassContainer::removeClass( const char *aName ): \n";
    return;
  }

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
  //  assert( aName != NULL );
   if(aName == 0 ){
    cerr << "ERROR!!! in parser CParsedClass *CParsedClassContainer::getClassByName( const char *aName )  \n";
    return 0;
  }

  CParsedClass *aClass;

  aClass = classes.find( aName );

  return aClass;
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
  QList<CParsedClass> *retVal = new QList<CParsedClass>();
  QStrList srted;
  char *str;

  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
  {
    // Only add non-subclasses.
    if( !classIterator.current()->isSubClass() )
      srted.inSort( classIterator.current()->name );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getClassByName( str ) );
  }

  return retVal;
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
  QStrList * retVal = new QStrList();
  
  // Iterate over all classes in the store.
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
  {
    // Only add non-subclasses.
    if( !classIterator.current()->isSubClass() )
      retVal->inSort( classIterator.current()->name );
  }

  return retVal;
}
