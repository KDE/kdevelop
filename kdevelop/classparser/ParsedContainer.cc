
#include <assert.h>
#include "ParsedContainer.h"
#include <iostream.h>

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
  //  assert( aStruct != NULL );
  //  assert( !aStruct->name.isEmpty() );
  if(aStruct == 0 ){
    cerr << "ERROR!!! in parser  CParsedContainer::addStruct(: \n";
    return;
  }

  aStruct->setDeclaredInClass( name );
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
  //  assert( anAttribute != NULL );
  //  assert( !anAttribute->name.isEmpty() );
  if(anAttribute == 0 ){
    cerr << "ERROR!!! in parser void CParsedContainer::addAttribute( CParsedAttribute *anAttribute ) \n";
    return;
  }

  anAttribute->setDeclaredInClass( name );
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
  //  assert( aMethod != NULL );
  //  assert( !aMethod->name.isEmpty() );
  if(aMethod == 0 ){
    cerr << "ERROR!!! in parser  CParsedContainer::addMethod( : \n";
    return;
  }
  
  QString str;
  
  aMethod->setDeclaredInClass( name );
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
  //  assert( aName != NULL );
  if(aName == 0 ){
    cerr << "ERROR!!! in parser  CParsedAttribute *CParsedContainer::getAttributeByName(: \n";
    return 0;
  }

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
  QStrList *retVal = new QStrList();
  QString str;

  // Iterate over all attributes.
  for( attributeIterator.toFirst();
       attributeIterator.current();
       ++attributeIterator )
  {
    retVal->inSort( attributeIterator.current()->asString( str ) );
  }

  return retVal;
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

  // Iterate over all attributes.
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
  //  assert( aName != NULL );
  if(aName == 0 ){
    cerr << "ERROR!!! in parser void CParsedStruct *CParsedContainer::getStructByName( const char *aName ) \n";
    return 0;
  }

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
  QStrList *retVal = new QStrList();

  // Iterate over all structures.
  for( structIterator.toFirst();
       structIterator.current();
       ++structIterator )
  {
    retVal->inSort( structIterator.current()->name );
  }

  return retVal;
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
  //  assert( aName != NULL );
  //  assert( strlen( aName ) > 0 );
   if(aName == 0 ){
    cerr << "ERROR!!! in parse void CParsedContainer::removeAttribute( const char *aName )\n";
    return;
  }

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
  //  assert( aName != NULL );
  //  assert( strlen( aName ) > 0 );
  if(aName == 0 ){
    cerr << "ERROR!!! in parser  void CParsedContainer::removeStruct( \n";
    return;
  }
  
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
