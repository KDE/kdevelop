/***************************************************************************
                          ParsedClass.cc  -  description
                             -------------------
    begin                : Mon Mar 15 1999
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
#include <stdio.h>
#include <stdlib.h>
#include "ProgrammingByContract.h"
#include "ParsedClass.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CParsedClass::CParsedClass()
 * CParsedClass()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClass::CParsedClass()
  : slotIterator( slotList ),
    signalIterator( signalList )
{
  setItemType( PIT_CLASS );

  parents.setAutoDelete( true );
  signalList.setAutoDelete( true );
  slotList.setAutoDelete( true );
  signalMaps.setAutoDelete( true );
  classes.setAutoDelete( false );

  isSubClass = false;
}

/*------------------------------------- CParsedClass::~CParsedClass()
 * ~CParsedClass()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClass::~CParsedClass()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*----------------------------------------- CParsedClass::addParent()
 * addParent()
 *   Add a parent.
 *
 * Parameters:
 *   aParent          The parent description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::addParent( CParsedParent *aParent )
{
  REQUIRE( "Valid parent", aParent != NULL );
  REQUIRE( "Valid parent name", !aParent->name.isEmpty() );

  parents.append( aParent );
}

/*------------------------------------------ CParsedClass::addSignal()
 * addSignal()
 *   Add a signal.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::addSignal( CParsedMethod *aMethod )
{
  REQUIRE( "Valid signal", aMethod != NULL );
  REQUIRE( "Valid signal name", !aMethod->name.isEmpty()  );

  aMethod->setDeclaredInScope( path() );
  signalList.append( aMethod );

  QString str;
  aMethod->asString( str );
  signalsByNameAndArg.insert( str, aMethod );
}

/*------------------------------------------- CParsedClass::addSlot()
 * addSlot()
 *   Add a signal.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::addSlot( CParsedMethod *aMethod )
{
  REQUIRE( "Valid slot", aMethod != NULL );
  REQUIRE( "Valid slot name", !aMethod->name.isEmpty() );

  QString str;

  aMethod->setDeclaredInScope( path() );
  slotList.append( aMethod );

  aMethod->asString( str );
  slotsByNameAndArg.insert( str, aMethod );
}

/*----------------------------------- CParsedClass::addSignalSlotMap()
 * addSignalSlotMap()
 *   Add a signal->slot mapping.
 *
 * Parameters:
 *   aMethod          The signal to slot mapping.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::addSignalSlotMap( CParsedSignalSlot *aSS )
{
  REQUIRE( "Valid signal slot map",  aSS != NULL );

  signalMaps.append( aSS );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ CParsedClass::getMethod()
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
CParsedMethod *CParsedClass::getMethod( CParsedMethod &aMethod )
{
  CParsedMethod *retVal = NULL;

  retVal = CParsedContainer::getMethod( aMethod );

  // If none was found try with the slots.
  if( retVal == NULL )
  {
    for( retVal = slotList.first(); 
         retVal != NULL && !retVal->isEqual( aMethod );
         retVal = slotList.next() )
      ;
  }

  return retVal;
}

/*----------------------------- CParsedClass::getSignalByNameAndArg()
 * getSignalByNameAndArg()
 *   Get a signal by using its' name and args using the same format
 *   as in CParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the signal to fetch.
 *
 * Returns:
 *   CParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
CParsedMethod *CParsedClass::getSignalByNameAndArg( const char *aName )
{
  REQUIRE1( "Valid signal name", aName != NULL, NULL );
  REQUIRE1( "Valid signal name length", strlen( aName ) > 0, NULL );

  return signalsByNameAndArg.find( aName );
}

/*----------------------------- CParsedClass::getSlotByNameAndArg()
 * getSlotByNameAndArg()
 *   Get a slot by using its' name and args using the same format
 *   as in CParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the slot to fetch.
 *
 * Returns:
 *   CParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
CParsedMethod *CParsedClass::getSlotByNameAndArg( const char *aName )
{
  REQUIRE1( "Valid slot name", aName != NULL, NULL );
  REQUIRE1( "Valid slot name length", strlen( aName ) > 0, NULL );

  return slotsByNameAndArg.find( aName );
}

/*-------------------------------- CParsedClass::hasParent()
 * hasParent()
 *   Check if this class has the named parent. 
 *
 * Parameters:
 *   aName              Name of the parent to check.
 *
 * Returns:
 *   bool               If the parent exists or not.
 *-----------------------------------------------------------------*/
bool CParsedClass::hasParent( const char *aName )
{
  REQUIRE1( "Valid parent name", aName != NULL, false );
  REQUIRE1( "Valid parent name length", strlen( aName ) > 0, false );

  CParsedParent *aParent;

  for( aParent = parents.first();
       aParent != NULL && aParent->name != aName;
       aParent = parents.next() )
    ;

  return aParent != NULL;
}

/*-------------------------------- CParsedClass::hasVirtual()
 * hasVirtual()
 *   Check if the class has any virtual methods.
 *
 * Parameters:
 *   -
 * Returns:
 *   bool               If the class has virtual functions.
 *-----------------------------------------------------------------*/
bool CParsedClass::hasVirtual()
{
  bool retVal = false;

  for( methodIterator.toFirst();
       methodIterator.current() && !retVal;
       ++methodIterator )
  {
    retVal = methodIterator.current()->isVirtual;
  }

  return retVal;
}

/*------------------------------- CParsedClass::getSortedSignalList()
 * getSortedSignalList()
 *   Get all signals in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<CParsedMethod> *CParsedClass::getSortedSignalList()
{
  QList<CParsedMethod> *retVal = new QList<CParsedMethod>();
  char *str;
  QStrList srted;
  QString m;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( signalIterator.toFirst();
       signalIterator.current();
       ++signalIterator )
  {
    signalIterator.current()->asString( m );
    srted.inSort( m );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getSignalByNameAndArg( str ) );
  }

  return retVal;
}

/*------------------------------- CParsedClass::getSortedSlotList()
 * getSortedSlotList()
 *   Get all slots in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<CParsedMethod> *CParsedClass::getSortedSlotList()
{
  QList<CParsedMethod> *retVal = new QList<CParsedMethod>();
  char *str;
  QStrList srted;
  QString m;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( slotIterator.toFirst();
       slotIterator.current();
       ++slotIterator )
  {
    slotIterator.current()->asString( m );
    srted.inSort( m );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getSlotByNameAndArg( str ) );
  }

  return retVal;
}

/*------------------------------- CParsedClass::getVirtualMethodList()
 * getVirtualMethodList()
 *   Get all virtual methods.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The list.
 *-----------------------------------------------------------------*/
QList<CParsedMethod> *CParsedClass::getVirtualMethodList()
{
  QList<CParsedMethod> *retVal = new QList<CParsedMethod>();

  for( methodIterator.toFirst();
       methodIterator.current();
       ++methodIterator )
  {
    if( methodIterator.current()->isVirtual )
      retVal->append( methodIterator.current() );
  }

  return retVal;
}

/*----------------------------------------------- CParsedClass::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::out()
{
  QDictIterator<CParsedAttribute> ait( attributes );
  CParsedParent *aParent;
  CParsedMethod *aMethod;
  CParsedSignalSlot *aSS;
  char *str;

  if( !comment.isEmpty() )
    cout << comment << endl;

  cout << "Class " << name << " @ line " << declaredOnLine;
  cout << " - " << declarationEndsOnLine << endl;
  cout << "  Defined in files:" << endl;
  cout << "    " << declaredInFile << endl;
  cout << "    " << definedInFile << endl;
  cout << "  Parents:" << endl;
  for( aParent = parents.first(); aParent != NULL; aParent = parents.next() )
    aParent->out();
  cout << "  Friends:" << endl;
  for( str = friends.first(); str != NULL; str = friends.next() )
    cout << "   " << str << endl;
  cout << "  Attributes:" << endl;
  for( ait.toFirst(); ait.current(); ++ait )
    ait.current()->out();
  cout << "  Methods:" << endl;
  for( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() )
    aMethod->out();
  cout << "  Signals:" << endl;
  for( aMethod = signalList.first(); aMethod != NULL; aMethod = signalList.next() )
    aMethod->out();
  cout << "  Slots:" << endl;
  for( aMethod = slotList.first(); aMethod != NULL; aMethod = slotList.next() )
    aMethod->out();
  cout << "  Signal to slot mappings:" << endl;
  for( aSS = signalMaps.first(); aSS != NULL; aSS = signalMaps.next() )
    aSS->out();
  cout << "  Classes:" << endl;
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
    classIterator.current()->out();
  
  cout << endl;
}

/*--------------------------------- CParsedClass::asPersistantString()
 * asPersistantString()
 *   Return a string made for persistant storage.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
const char *CParsedClass::asPersistantString( QString &dataStr )
{
  CParsedParent *aParent;
  CParsedMethod *aMethod;
  QString str;
  QString intStr;

  dataStr = "";
  dataStr += name + "\n";
  intStr.sprintf( "%d", definedOnLine );
  dataStr += intStr + "\n";
  
  // Add parents.
  intStr.sprintf( "%d", parents.count() );
  dataStr += intStr + "\n";
  for( aParent = parents.first(); aParent != NULL; aParent = parents.next() )
  {
    aParent->asPersistantString( str );
    dataStr.append( str );
  }
  
  // Add friends.
  intStr.sprintf( "%d", friends.count() );
  dataStr += intStr + "\n";
  for( str = friends.first(); str != NULL; str = friends.next() )
    dataStr += str + "\n";

  // Add methods. 
  intStr.sprintf( "%d", methods.count() );
  dataStr += intStr + "\n";
  for( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() )
  {
    aMethod->asPersistantString( str );
    dataStr.append( str );
  }

  // Add attributes.
  intStr.sprintf( "%d", attributeIterator.count() );
  dataStr += intStr + "\n";
  for( attributeIterator.toFirst(); 
       attributeIterator.current();
       ++attributeIterator)
  {
    attributeIterator.current()->asPersistantString( str );
    dataStr += str;
  }

  // Add signals.
  intStr.sprintf( "%d", signalList.count() );
  dataStr += intStr + "\n";
  for( aMethod = signalList.first(); 
       aMethod != NULL; 
       aMethod = signalList.next() )
  {
    aMethod->asPersistantString( str );
    dataStr += str;
  }

  // Add slots.
  intStr.sprintf( "%d", slotList.count() );
  dataStr += intStr + "\n";
  for( aMethod = slotList.first(); aMethod != NULL; aMethod = slotList.next() )
  {
    aMethod->asPersistantString( str );
    dataStr += str;
  }

  return dataStr;
}

/*--------------------------------- CParsedClass::fromPersistantString()
 * fromPersistantString()
 *   Initialize the object from a persistant string.
 *
 * Parameters:
 *   dataStr      The string with the data.
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
int CParsedClass::fromPersistantString( const char *str, int startPos )
{
  REQUIRE1( "Valid string", str != NULL, -1 );
  REQUIRE1( "Valid startpos", startPos > 0, -1 );

  CParsedParent *aParent;
  CParsedMethod *aMethod;
  CParsedAttribute *anAttribute;
  char buf[2048];
  int count;
  int i;

  // Fetch the classname
  startPos = getSubString( buf, str, startPos );
  setName( buf );

  // Fetch definedOnLine.
  startPos = getSubString( buf, str, startPos );
  setDefinedOnLine( atoi( buf ) );

  // Fetch parents
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    aParent = new CParsedParent();
    startPos = aParent->fromPersistantString( str, startPos );
    addParent( aParent );
  }

  // Fetch friends
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    startPos = getSubString( buf, str, startPos );
    addFriend( buf );
  }  

  // Fetch methods
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    aMethod = new CParsedMethod();
    startPos = aMethod->fromPersistantString( str, startPos );
    addMethod( aMethod );
  }

  // Fetch attributes
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    anAttribute = new CParsedAttribute();
    startPos = anAttribute->fromPersistantString( str, startPos );
    addAttribute( anAttribute );
  }

  // Fetch signals
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    aMethod = new CParsedMethod();
    startPos = aMethod->fromPersistantString( str, startPos );
    addSignal( aMethod );
  }

  // Fetch slots
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    aMethod = new CParsedMethod();
    startPos = aMethod->fromPersistantString( str, startPos );
    addSlot( aMethod );
  }

  return strlen( str );
}
