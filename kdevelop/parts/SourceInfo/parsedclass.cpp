/***************************************************************************
                          parsedclass.cpp  -  description
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
#include "programmingbycontract.h"
#include "parsedclass.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- ParsedClass::ParsedClass()
 * ParsedClass()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass::ParsedClass()
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

/*------------------------------------- ParsedClass::~ParsedClass()
 * ~ParsedClass()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass::~ParsedClass()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*---------------------- ParsedClass::removeWithReferences()
 * removeWithReferences()
 *   Remove references to all items in the parsed class that were
 *   obtained from the given file
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::removeWithReferences( const QString &aFile )
{
  REQUIRE( "Valid filename", aFile != NULL );
  REQUIRE( "Valid filename length", aFile.length() > 0 );

  ParsedMethod *aMethod = NULL;

  methodIterator.toFirst();
  while( ( aMethod = methodIterator.current() ) != (ParsedMethod *) NULL )
  {
    if( aMethod->declaredInFile == aFile ) {
    		if( aMethod->definedInFile.isNull() || aMethod->declaredInFile == aMethod->definedInFile ) {
    			ParsedContainer::removeMethod(aMethod);
    		} else {
    			aMethod->clearDeclaration();
    			++methodIterator;
    		}
    } else if( aMethod->definedInFile == aFile ) {
    		if( aMethod->declaredInFile.isNull() ) {
    			ParsedContainer::removeMethod(aMethod);
    		} else {
    			aMethod->clearDefinition();
    			++methodIterator;
    		}
    } else {
    		++methodIterator;
    }
  }

  slotIterator.toFirst();
  while( ( aMethod = slotIterator.current() ) != (ParsedMethod *) NULL )
  {
    if( aMethod->declaredInFile == aFile ) {
    		if( aMethod->definedInFile.isNull() || aMethod->declaredInFile == aMethod->definedInFile ) {
			slotList.removeRef( aMethod );
    		} else {
    			aMethod->clearDeclaration();
    			++slotIterator;
    		}
    } else if( aMethod->definedInFile == aFile ) {
    		if( aMethod->declaredInFile.isNull() ) {
			slotList.removeRef(aMethod);
    		} else {
    			aMethod->clearDefinition();
    			++slotIterator;
    		}
    } else {
    		++slotIterator;
    }
  }

  if( declaredInFile == aFile ) {
  	clearDeclaration();
  } else if( definedInFile == aFile ) {
  	clearDefinition();
  }
}

/*----------------------------------- ParsedClass::removeMethod()
 * removeMethod()
 *   Remove a method matching the specification.
 *
 * Parameters:
 *   aMethod        Specification of the method.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::removeMethod( ParsedMethod *aMethod )
{
  REQUIRE( "Valid method", aMethod != NULL );
  REQUIRE( "Valid methodname", !aMethod->name.isEmpty() );

  QString str = aMethod->asString();

  if ( slotList.removeRef( aMethod ) ) {
    slotsByNameAndArg.remove( str );
  } else {
  	ParsedContainer::removeMethod( aMethod );
  }
}

/*----------------------------------------- ParsedClass::clearDeclaration()
 * clearDeclaration()
 *   Clear all attributes which are only in the class declaration,
 *	 and not in the definition part. This excludes the 'methods'
 *   and 'slotList' lists, as these can contain parsed methods with
 *   definition data
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::clearDeclaration()
{
  attributes.clear();
  structs.clear();
  slotsByNameAndArg.clear();
  signalList.clear();
  signalsByNameAndArg.clear();
  parents.clear();
  friends.clear();
  signalMaps.clear();

  ParsedItem::clearDeclaration();
}

/*----------------------------------------- ParsedClass::addParent()
 * addParent()
 *   Add a parent.
 *
 * Parameters:
 *   aParent          The parent description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::addParent( ParsedParent *aParent )
{
  REQUIRE( "Valid parent", aParent != NULL );
  REQUIRE( "Valid parent name", !aParent->name.isEmpty() );

  parents.append( aParent );
}

/*------------------------------------------ ParsedClass::addSignal()
 * addSignal()
 *   Add a signal.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::addSignal( ParsedMethod *aMethod )
{
  REQUIRE( "Valid signal", aMethod != NULL );
  REQUIRE( "Valid signal name", !aMethod->name.isEmpty()  );

  aMethod->setDeclaredInScope( path() );
  signalList.append( aMethod );

  QString str = aMethod->asString();
  signalsByNameAndArg.insert( str, aMethod );
}

/*------------------------------------------- ParsedClass::addSlot()
 * addSlot()
 *   Add a signal.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::addSlot( ParsedMethod *aMethod )
{
  REQUIRE( "Valid slot", aMethod != NULL );
  REQUIRE( "Valid slot name", !aMethod->name.isEmpty() );

  aMethod->setDeclaredInScope( path() );

  slotList.append( aMethod );
  slotsByNameAndArg.insert( aMethod->asString(), aMethod );
}

/*----------------------------------- ParsedClass::addSignalSlotMap()
 * addSignalSlotMap()
 *   Add a signal->slot mapping.
 *
 * Parameters:
 *   aMethod          The signal to slot mapping.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::addSignalSlotMap( ParsedSignalSlot *aSS )
{
  REQUIRE( "Valid signal slot map",  aSS != NULL );

  signalMaps.append( aSS );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ ParsedClass::getMethod()
 * getMethod()
 *   Get a method by comparing with another method.
 *
 * Parameters:
 *   aMethod            Method to compare with.
 *
 * Returns:
 *   ParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedMethod *ParsedClass::getMethod( ParsedMethod &aMethod )
{
  ParsedMethod *retVal = NULL;

  retVal = ParsedContainer::getMethod( aMethod );

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

/*----------------------------- ParsedClass::getSignalByNameAndArg()
 * getSignalByNameAndArg()
 *   Get a signal by using its' name and args using the same format
 *   as in ParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the signal to fetch.
 *
 * Returns:
 *   ParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedMethod *ParsedClass::getSignalByNameAndArg( const QString &aName )
{
  REQUIRE1( "Valid signal name", aName != NULL, NULL );
  REQUIRE1( "Valid signal name length", aName.length() > 0, NULL );

  return signalsByNameAndArg.find( aName );
}

/*----------------------------- ParsedClass::getSlotByNameAndArg()
 * getSlotByNameAndArg()
 *   Get a slot by using its' name and args using the same format
 *   as in ParsedMethod::toString().
 *
 * Parameters:
 *   aName              Name and args of the slot to fetch.
 *
 * Returns:
 *   ParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
ParsedMethod *ParsedClass::getSlotByNameAndArg( const QString &aName )
{
  REQUIRE1( "Valid slot name", aName != NULL, NULL );
  REQUIRE1( "Valid slot name length", aName.length() > 0, NULL );

  return slotsByNameAndArg.find( aName );
}

/*-------------------------------- ParsedClass::hasParent()
 * hasParent()
 *   Check if this class has the named parent. 
 *
 * Parameters:
 *   aName              Name of the parent to check.
 *
 * Returns:
 *   bool               If the parent exists or not.
 *-----------------------------------------------------------------*/
bool ParsedClass::hasParent( const QString &aName )
{
  REQUIRE1( "Valid parent name", aName != NULL, false );
  REQUIRE1( "Valid parent name length", aName.length() > 0, false );

  ParsedParent *aParent;

  for( aParent = parents.first();
       aParent != NULL && aParent->name != aName;
       aParent = parents.next() )
    ;

  return aParent != NULL;
}

/*-------------------------------- ParsedClass::hasVirtual()
 * hasVirtual()
 *   Check if the class has any virtual methods.
 *
 * Parameters:
 *   -
 * Returns:
 *   bool               If the class has virtual functions.
 *-----------------------------------------------------------------*/
bool ParsedClass::hasVirtual()
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

/*------------------------------- ParsedClass::getSortedSignalList()
 * getSortedSignalList()
 *   Get all signals in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<ParsedMethod> *ParsedClass::getSortedSignalList()
{
  QList<ParsedMethod> *retVal = new QList<ParsedMethod>();
  char *str;
  QStrList srted;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( signalIterator.toFirst();
       signalIterator.current();
       ++signalIterator )
  {
    srted.inSort( signalIterator.current()->asString() );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getSignalByNameAndArg( str ) );
  }

  return retVal;
}

/*------------------------------- ParsedClass::getSortedSlotList()
 * getSortedSlotList()
 *   Get all slots in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<ParsedMethod> *ParsedClass::getSortedSlotList()
{
  QList<ParsedMethod> *retVal = new QList<ParsedMethod>();
  char *str;
  QStrList srted;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( slotIterator.toFirst();
       slotIterator.current();
       ++slotIterator )
  {
    srted.inSort( slotIterator.current()->asString() );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getSlotByNameAndArg( str ) );
  }

  return retVal;
}

/*------------------------------- ParsedClass::getVirtualMethodList()
 * getVirtualMethodList()
 *   Get all virtual methods.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedMethod> *  The list.
 *-----------------------------------------------------------------*/
QList<ParsedMethod> *ParsedClass::getVirtualMethodList()
{
  QList<ParsedMethod> *retVal = new QList<ParsedMethod>();

  for( methodIterator.toFirst();
       methodIterator.current();
       ++methodIterator )
  {
    if( methodIterator.current()->isVirtual )
      retVal->append( methodIterator.current() );
  }

  return retVal;
}

/*----------------------------------------------- ParsedClass::out()
 * out()
 *   Output this object as text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ParsedClass::out()
{
  QDictIterator<ParsedAttribute> ait( attributes );
  ParsedParent *aParent;
  ParsedMethod *aMethod;
  ParsedSignalSlot *aSS;
  char *str;

  if( !comment.isEmpty() )
    cout << comment << endl;

  cout << "Class " << path() << " @ line " << declaredOnLine;
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

/*--------------------------------- ParsedClass::asPersistantString()
 * asPersistantString()
 *   Return a string made for persistant storage.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QString ParsedClass::asPersistantString()
{
  ParsedParent *aParent;
  ParsedMethod *aMethod;
  QCString aFriend;

  QString str = name;
  str += "\n";
  str += QString::number(definedOnLine);
  str += "\n";
  
  // Add parents.
  str += QString::number(parents.count());
  str += "\n";
  for( aParent = parents.first(); aParent != NULL; aParent = parents.next() )
  {
    str += aParent->asPersistantString();
  }
  
  // Add friends.
  str += QString::number(friends.count());
  str += "\n";
  for( aFriend = friends.first(); aFriend != NULL; aFriend = friends.next() )
  {
    str += aFriend;
    str + "\n";
  }

  // Add methods. 
  str += QString::number(methods.count());
  str += "\n";
  for( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() )
  {
    str += aMethod->asPersistantString();
  }

  // Add attributes.
  str += QString::number(attributeIterator.count());
  str += "\n";
  for( attributeIterator.toFirst(); 
       attributeIterator.current();
       ++attributeIterator)
  {
    str += attributeIterator.current()->asPersistantString();
  }

  // Add signals.
  str += QString::number(signalList.count());
  str += "\n";
  for( aMethod = signalList.first(); 
       aMethod != NULL; 
       aMethod = signalList.next() )
  {
    str += aMethod->asPersistantString();
  }

  // Add slots.
  str += QString::number(slotList.count());
  str += "\n";
  for( aMethod = slotList.first(); aMethod != NULL; aMethod = slotList.next() )
  {
    str += aMethod->asPersistantString();
  }

  return str;
}

/*--------------------------------- ParsedClass::fromPersistantString()
 * fromPersistantString()
 *   Initialize the object from a persistant string.
 *
 * Parameters:
 *   dataStr      The string with the data.
 *   
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
int ParsedClass::fromPersistantString( const QString &str, int startPos )
{
  REQUIRE1( "Valid string", str != NULL, -1 );
  REQUIRE1( "Valid startpos", startPos > 0, -1 );

  ParsedParent *aParent;
  ParsedMethod *aMethod;
  ParsedAttribute *anAttribute;
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
    aParent = new ParsedParent();
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
    aMethod = new ParsedMethod();
    startPos = aMethod->fromPersistantString( str, startPos );
    addMethod( aMethod );
  }

  // Fetch attributes
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    anAttribute = new ParsedAttribute();
    startPos = anAttribute->fromPersistantString( str, startPos );
    addAttribute( anAttribute );
  }

  // Fetch signals
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    aMethod = new ParsedMethod();
    startPos = aMethod->fromPersistantString( str, startPos );
    addSignal( aMethod );
  }

  // Fetch slots
  startPos = getSubString( buf, str, startPos );
  count = atoi( buf );
  for( i=0; i<count; i++ )
  {
    aMethod = new ParsedMethod();
    startPos = aMethod->fromPersistantString( str, startPos );
    addSlot( aMethod );
  }

  return strlen( str );
}
