
#include <assert.h>
#include "ParsedContainer.h"

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
CParsedContainer::CParsedContainer()
  : methodIterator( methods ),
    attributeIterator( attributes ),
    structIterator( structs )
{
  attributes.setAutoDelete( true );
  methods.setAutoDelete( true );
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
  assert( aStruct != NULL );
  assert( !aStruct->name.isEmpty() );

  structs.insert( aStruct->name, aStruct );  
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
  assert( anAttribute != NULL );
  assert( !anAttribute->name.isEmpty() );

  attributes.insert( anAttribute->name, anAttribute );
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
  assert( aMethod != NULL );
  assert( !aMethod->name.isEmpty() );

  QString str;

  methods.append( aMethod );

  aMethod->asString( str );
  methodsByNameAndArg.insert( str, aMethod );
}

/*********************************************************************
 *                                                                   *
 *                           PUBLIC METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------ CParsedContainer::getMethodByName()
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
CParsedMethod *CParsedContainer::getMethodByName( const char *aName )
{
  CParsedMethod *retVal = NULL;

  for( retVal = methods.first(); 
       retVal != NULL && retVal->name != aName;
       retVal = methods.next() )
    ;
    
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
  char *str;
  QStrList srted;
  QString m;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( methodIterator.toFirst();
       methodIterator.current();
       ++methodIterator )
  {
    methodIterator.current()->asString( m );
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
  assert( aName != NULL );

  return attributes.find( aName );
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
  assert( aName != NULL );

  return structs.find( aName );
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
  QList<CParsedStruct> *retVal = new QList<CParsedStruct>();
  char *str;
  QStrList srted;
  
  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( structIterator.toFirst();
       structIterator.current();
       ++structIterator )
  {
    srted.inSort( structIterator.current()->name );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getStructByName( str ) );
  }

  return retVal;
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
