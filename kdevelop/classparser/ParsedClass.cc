/********************************************************************
* Name    : Implementation of a parsed class.                       *
* ------------------------------------------------------------------*
* File    : ParsedClass.h                                           *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Mon Mar 15 12:03:15 CET 1999                            *
*                                                                   *
* ------------------------------------------------------------------*
* Purpose :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Usage   :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Functions:                                                        *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Modifications:                                                    *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
*********************************************************************/

#include <iostream.h>
#include <stdio.h>
#include <assert.h>
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
  : methodIterator( methods ),
    attributeIterator( attributes ),
    slotIterator( slotList )
{
  parents.setAutoDelete( true );
  attributes.setAutoDelete( true );
  methods.setAutoDelete( true );
  signalList.setAutoDelete( true );
  slotList.setAutoDelete( true );
  signalMaps.setAutoDelete( true );
  textMaps.setAutoDelete( true );

  definedOnLine = -1;
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

/*------------------------------------------- CParsedClass::setName()
 * setName()
 *   Set the name of the class.
 *
 * Parameters:
 *   aName            The new name.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::setName( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );

  name = aName;
}


/*---------------------------------- CParsedClass::setDefinedOnLine()
 * setDefinedOnLine()
 *   Set the line where the class is defined.
 *
 * Parameters:
 *   aLine            The line where it is defined.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::setDefinedOnLine( int aLine )
{
  definedOnLine = aLine;
}

/*---------------------------------- CParsedClass::setHFilename()
 * setHFilename()
 *   Set the .h filename.
 *
 * Parameters:
 *   aName            The filename.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::setHFilename( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );

  hFilename = aName;
}
  
/*---------------------------------- CParsedClass::setImplFilename()
 * setImplFilename()
 *   Set the .cc/.cpp filename.
 *
 * Parameters:
 *   aName            The filename.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::setImplFilename( const char *aName )
{
  assert( aName != NULL && strlen( aName ) > 0 );

  implFilename = aName;
}

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
  assert( aParent != NULL );

  parents.append( aParent );
}

/*-------------------------------------- CParsedClass::addAttribute()
 * addAttribute()
 *   Add an attribute.
 *
 * Parameters:
 *   anAttribute      The attribute description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::addAttribute( CParsedAttribute *anAttribute )
{
  assert( anAttribute != NULL );

  anAttribute->setDeclaredInClass( name );
  attributes.insert( anAttribute->name, anAttribute );
}

/*------------------------------------------ CParsedClass::addMethod()
 * addMethod()
 *   Add a method.
 *
 * Parameters:
 *   aMethod          The method description.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::addMethod( CParsedMethod *aMethod )
{
  assert( aMethod != NULL );

  QString str;

  aMethod->setDeclaredInClass( name );
  methods.append( aMethod );

  aMethod->toString( str );
  methodsByNameAndArg.insert( str, aMethod );
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
  assert( aMethod != NULL );

  aMethod->setDeclaredInClass( name );
  signalList.append( aMethod );
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
  assert( aMethod != NULL );

  QString str;

  aMethod->setDeclaredInClass( name );
  slotList.append( aMethod );

  aMethod->toString( str );
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
  assert( aSS != NULL );

  signalMaps.append( aSS );
}

/*----------------------------------- CParsedClass::addSignalTextMap()
 * addSignalTextMap()
 *   Add a signal->text mapping.
 *
 * Parameters:
 *   aST              The signal to text mapping.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CParsedClass::addSignalTextMap( CParsedSignalText *aST )
{
  assert( aST != NULL );

  textMaps.append( aST );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*----------------------------------- CParsedClass::getMethodByName()
 * getMethodByName()
 *   Get a method by using its' name.
 *
 * Parameters:
 *   aName              Name of the method to fetch.
 *
 * Returns:
 *   CParsedMethod *    The method.
 *   NULL               If not found.
 *-----------------------------------------------------------------*/
CParsedMethod *CParsedClass::getMethodByName( const char *aName )
{
  CParsedMethod *retVal = NULL;

  for( retVal = methods.first(); 
       retVal != NULL && retVal->name != aName;
       retVal = methods.next() )
    ;
    
  return retVal;
}

/*----------------------------- CParsedClass::getMethodByNameAndArg()
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
CParsedMethod *CParsedClass::getMethodByNameAndArg( const char *aName )
{
  return methodsByNameAndArg.find( aName );
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
  return slotsByNameAndArg.find( aName );
}

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

  for( retVal = methods.first(); 
       retVal != NULL && !retVal->isEqual( aMethod );
       retVal = methods.next() )
    ;

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

/*-------------------------------- CParsedClass::getAttributeByName()
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
CParsedAttribute *CParsedClass::getAttributeByName( const char *aName )
{    
  return attributes.find( aName );
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
  CParsedParent *aParent;

  for( aParent = parents.first();
       aParent != NULL && aParent->name != aName;
       aParent = parents.next() )
    ;

  return aParent != NULL;
}

/*------------------------------- CParsedClass::getSortedMethodList()
 * getSortedMethodList()
 *   Get all methods in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<CParsedMethod> *CParsedClass::getSortedMethodList()
{
  QList<CParsedMethod> *retVal = new QList<CParsedMethod>();
  char *str;
  QStrList srted;
  QString m;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( methodIterator.toFirst();
       methodIterator.current();
       ++methodIterator )
  {
    methodIterator.current()->toString( m );
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

/*------------------------------- CParsedClass::getSortedAttributeList()
 * getSortedAttributeList()
 *   Get all attributes in sorted order. 
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedMethod> *  The sorted list.
 *-----------------------------------------------------------------*/
QList<CParsedAttribute> *CParsedClass::getSortedAttributeList()
{
  QList<CParsedAttribute> *retVal = new QList<CParsedAttribute>();
  char *str;
  QStrList srted;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( attributeIterator.toFirst();
       attributeIterator.current();
       ++attributeIterator )
  {
    srted.inSort( attributeIterator.current()->name );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getAttributeByName( str ) );
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
    slotIterator.current()->toString( m );
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
  char buf[10];
  QDictIterator<CParsedAttribute> ait( attributes );
  CParsedParent *aParent;
  CParsedMethod *aMethod;
  CParsedSignalSlot *aSS;
  CParsedSignalText *aST;
  char *str;

  sprintf( buf, "%d", definedOnLine );
  cout << "Class " << name << " @ line " << buf << "\n";
  cout << "  Defined in files:\n";
  cout << "    " << hFilename << "\n";
  cout << "    " << implFilename << "\n";
  cout << "  Parents:\n";
  for( aParent = parents.first(); aParent != NULL; aParent = parents.next() )
    aParent->out();
  cout << "  Friends:\n";
  for( str = friends.first(); str != NULL; str = friends.next() )
    cout << "   " << str << "\n";
  cout << "  Attributes:\n";
  for( ait.toFirst(); ait.current(); ++ait )
    ait.current()->out();
  cout << "  Methods:\n";
  for( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() )
    aMethod->out();
  cout << "  Signals:\n";
  for( aMethod = signalList.first(); aMethod != NULL; aMethod = signalList.next() )
    aMethod->out();
  cout << "  Slots:\n";
  for( aMethod = slotList.first(); aMethod != NULL; aMethod = slotList.next() )
    aMethod->out();
  cout << "  Signal to slot mappings:\n";
  for( aSS = signalMaps.first(); aSS != NULL; aSS = signalMaps.next() )
    aSS->out();
  cout << "  Signal to text mapplings:\n";
  for( aST = textMaps.first(); aST != NULL; aST= textMaps.next() )
    aST->out();
  
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
void CParsedClass::asPersistantString( QString &dataStr )
{
  CParsedParent *aParent;
  CParsedMethod *aMethod;
  QString str;

  dataStr = "";
  dataStr += name + "\n";
  dataStr += definedOnLine + "\n";
  
  // Add parents.
  dataStr += parents.count() + "\n";
  for( aParent = parents.first(); aParent != NULL; aParent = parents.next() )
  {
    aParent->asPersistantString( str );
    dataStr.append( str );
  }
  
  // Add friends.
  dataStr += friends.count() + "\n";
  for( str = friends.first(); str != NULL; str = friends.next() )
    dataStr += str + "\n";

  // Add methods.
  for( aMethod = methods.first(); aMethod != NULL; aMethod = methods.next() )
  {
    aMethod->asPersistantString( str );
    dataStr.append( str );
  }

  // Add attributes.
  dataStr += attributeIterator.count() + "\n";
  for( attributeIterator.toFirst(); 
       attributeIterator.current();
       ++attributeIterator)
  {
    attributeIterator.current()->asPersistantString( str );
    dataStr += str;
  }

  // Add signals.
  for( aMethod = signalList.first(); 
       aMethod != NULL; 
       aMethod = signalList.next() )
  {
    aMethod->asPersistantString( str );
    dataStr += str;
  }

  // Add slots.
  for( aMethod = slotList.first(); aMethod != NULL; aMethod = slotList.next() )
  {
    aMethod->asPersistantString( str );
    dataStr += str;
  }
}
