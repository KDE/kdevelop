/***************************************************************************
                          ParsedContainer.cc  -  description
                             -------------------
    begin                : Mon Nov 21 1999
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

#include "ParsedContainer.h"
#include <iostream.h>
#include "ProgrammingByContract.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*------------------------------- CParsedContainer::CParsedContainer()
 * CParsedContainer()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedContainer::CParsedContainer(bool autodel)
  : useFullPath( false ),
    methodIterator( methods ),
    attributeIterator( attributes ),
    structIterator( structs )
{
  attributes.setAutoDelete( autodel );
  methods.setAutoDelete( autodel );
}

/*----------------------------- CParsedContainer::~CParsedContainer()
 * ~CParsedContainer()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedContainer::~CParsedContainer()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------- CParsedContainer::addStruct()
 * addStruct()
 *   Add a structure.
 *
 * Parameters:
 *   aStruct          The structure description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::addStruct( CParsedStruct *aStruct ) 
{
  REQUIRE( "Valid struct", aStruct != NULL );
  REQUIRE( "Valid structname", !aStruct->name.isEmpty() );
  REQUIRE( "Unique struct", !hasStruct( useFullPath ? aStruct->path() : aStruct->name ) );
  
  if( !path().isEmpty() )
    aStruct->setDeclaredInScope( path() );

  structs.insert( ( useFullPath ? aStruct->path() : aStruct->name ), aStruct );  
}

/*-------------------------------------- CParsedContainer::addAttribute()
 * addAttribute()
 *   Add an attribute.
 *
 * Parameters:
 *   anAttribute      The attribute description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::addAttribute( CParsedAttribute *anAttribute )
{
  REQUIRE( "Valid attribute", anAttribute != NULL );
  REQUIRE( "Valid attributename", !anAttribute->name.isEmpty() );
  REQUIRE( "Unique attribute", !hasAttribute( useFullPath ? anAttribute->path() : anAttribute->name ) );

  if( !path().isEmpty() )
    anAttribute->setDeclaredInScope( path() );

  attributes.insert( anAttribute->name,  anAttribute );
}

/*------------------------------------------ CParsedContainer::addMethod()
 * addMethod()
 *   Add a method.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::addMethod( CParsedMethod *aMethod )
{
  REQUIRE( "Valid method", aMethod != NULL );
  REQUIRE( "Valid methodname", !aMethod->name.isEmpty() );
  
  QString str;
  
  if( !path().isEmpty() )
    aMethod->setDeclaredInScope( path() );

  methods.append( aMethod );
  
  aMethod->asString( str );

  methodsByNameAndArg.insert( str, aMethod );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CParsedContainer::getMethod()
 * getMethod()
 *   Get a method by comparing with another method.
 *
 * Parameters:
 *   aMethod            Method to compare with.
 *
 * Returns:
 *   CParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
CParsedMethod *CParsedContainer::getMethod( CParsedMethod &aMethod )
{
  REQUIRE1( "Valid methodname", !aMethod.name.isEmpty(), NULL );

  CParsedMethod *retVal = NULL;

  for( retVal = methods.first(); 
       retVal != NULL && !retVal->isEqual( aMethod );
       retVal = methods.next() )
    ;

  return retVal;
}

/*------------------------------ CParsedContainer::getMethodByName()
 * getMethodByName()
 *   Get all methods matching the supplied name. 
 *
 * Parameters:
 *   aName                   Name of the method to fetch.
 *
 * Returns:
 *   QList<CParsedMethod> *  The methods.
 *   NULL                    If not found.
 *-----------------------------------------------------------------*/
QList<CParsedMethod> *CParsedContainer::getMethodByName( const char *aName )
{
  REQUIRE1( "Valid methodname", aName != NULL, new QList<CParsedMethod>() );
  REQUIRE1( "Valid methodname length", strlen( aName ) > 0, new QList<CParsedMethod>() );

  QList<CParsedMethod> *retVal = new QList<CParsedMethod>();
  CParsedMethod *aMethod;

  retVal->setAutoDelete( false );

  for( aMethod = methods.first(); 
       aMethod != NULL;
       aMethod = methods.next() )
  {
    // If the name matches the supplied one we append the method to the 
    // returnvalue.
    if( aMethod->name == aName )
      retVal->append( aMethod );
  }
    
  return retVal;
}

/*--------------------------- CParsedContainer::getMethodByNameAndArg()
 * getMethodByNameAndArg()
 *   Get a method by using its' name and args using the same format
 *   as in CParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the method to fetch.
 *
 * Returns:
 *   CParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
CParsedMethod *CParsedContainer::getMethodByNameAndArg( const char *aName )
{
  REQUIRE1( "Valid methodname", aName != NULL, NULL );
  REQUIRE1( "Valid methodname length", strlen( aName ) > 0,  NULL );

  return methodsByNameAndArg.find( aName );
}

/*--------------------------- CParsedContainer::getSortedMethodList()
 * getSortedMethodList()
 *   Get all methods in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<CParsedMethod> *CParsedContainer::getSortedMethodList()
{
  QList<CParsedMethod> *retVal = new QList<CParsedMethod>();
  CParsedMethod *aMethod;
  char *str;
  QStrList srted;
  QString m;

  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( aMethod = methods.first();
       aMethod != NULL;
       aMethod = methods.next() )
  {
    aMethod->asString( m );
    srted.inSort( m );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getMethodByNameAndArg( str ) );
  }

  return retVal;
}

/*--------------------------- CParsedContainer::getAttributeByName()
 * getAttributeByName()
 *   Get a attribute by using its' name.
 *
 * Parameters:
 *   aName              Name of the attribute to fetch.
 *
 * Returns:
 *   CParsedAttribute * The attribute.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
CParsedAttribute *CParsedContainer::getAttributeByName( const char *aName )
{    
  REQUIRE1( "Valid attributename", aName != NULL, NULL );
  REQUIRE1( "Valid attributename length", strlen( aName ) > 0, NULL );

  return attributes.find( aName );
}

/*------------------------ CParsedContainer::getSortedAttributeList()
 * getSortedAttributeList()
 *   Get all attributes in their string reprentation in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList *         List of attributes in sorted order.
 *-----------------------------------------------------------------*/
QStrList *CParsedContainer::getSortedAttributeAsStringList()
{
  return getSortedIteratorNameList<CParsedAttribute>( attributeIterator );
}

/*------------------------ CParsedContainer::getSortedAttributeList()
 * getSortedAttributeList()
 *   Get all attributes in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<CParsedAttribute> *CParsedContainer::getSortedAttributeList()
{
  return getSortedDictList<CParsedAttribute>( attributes, false );
}

/*--------------------------- CParsedContainer::getStructByName()
 * getStructByName()
 *   Get a structure by using its' name.
 *
 * Parameters:
 *   aName              Name of the struct to fetch.
 *
 * Returns:
 *   CParsedStruct *  The structure.
 *   NULL             If not found.
 *-----------------------------------------------------------------*/
CParsedStruct *CParsedContainer::getStructByName( const char *aName )
{    
  REQUIRE1( "Valid structname", aName != NULL, NULL );
  REQUIRE1( "Valid structname length", strlen( aName ), NULL );

  return structs.find( aName );
}

/*----------------------- CParsedContainer::getSortedStructNameList()
 * getSortedStructNameList()
 *   Get the names of all structures in a sorted list.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList *       List of all structs in alpabetical order.
 *-----------------------------------------------------------------*/
QStrList *CParsedContainer::getSortedStructNameList()
{
  return getSortedIteratorNameList<CParsedStruct>( structIterator );
}

/*---------------------------- CParsedContainer::getSortedStructList()
 * getSortedStructList()
 *   Get all structs in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<CParsedStruct> *CParsedContainer::getSortedStructList()
{
  return getSortedDictList<CParsedStruct>( structs, useFullPath );
}

/*--------------------------- CParsedContainer::removeWithReferences()
 * removeWithReferences()
 *   Remove all items in the store with references to the file.
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::removeWithReferences( const char *aFile )
{
  REQUIRE( "Valid filename", aFile != NULL );
  REQUIRE( "Valid filename length", strlen( aFile ) > 0 );

}

/*----------------------------------- CParsedContainer::removeMethod()
 * removeMethod()
 *   Remove a method matching the specification. 
 *
 * Parameters:
 *   aMethod        Specification of the method.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::removeMethod( CParsedMethod *aMethod )
{
  REQUIRE( "Valid method", aMethod != NULL );
  REQUIRE( "Valid methodname", !aMethod->name.isEmpty() );

  QString str;
  CParsedMethod *m;

  aMethod->asString( str );

  m = getMethodByNameAndArg( str );

  methodsByNameAndArg.remove( str );
  methods.removeRef( m );
}

/*-------------------------------- CParsedContainer::removeAttribute()
 * removeAttribute()
 *   Remove an attribute with a specified name. 
 *
 * Parameters:
 *   aName          Name of the attribute to remove.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::removeAttribute( const char *aName ) 
{
  REQUIRE( "Valid attribute name", aName != NULL );
  REQUIRE( "Valid attribute name length", strlen( aName ) > 0 );

  attributes.remove( aName );
}

/*----------------------------------- CParsedContainer::removeStruct()
 * removeStruct()
 *   Remove a struct with a specified name. 
 *
 * Parameters:
 *   aName          Name of the struct to remove.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::removeStruct( const char *aName )
{
  REQUIRE( "Valid struct name", aName != NULL );
  REQUIRE( "Valid struct name length", strlen( aName ) > 0 );
  
  structs.remove( aName );
}

/*------------------------------------------ CParsedContainer::clear()
 * clear()
 *   Clear the internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedContainer::clear()
{
  attributes.clear();
  methods.clear();
  methodsByNameAndArg.clear();
  structs.clear();
}
