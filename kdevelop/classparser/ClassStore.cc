/********************************************************************
* Name    : Implementation of a classStore.                         *
* ------------------------------------------------------------------*
* File    : ClassStore.cpp                                          *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Sun Mar 21 11:43:47 CET 1999                            *
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

#include "ClassStore.h"
#include <iostream.h>
#include <assert.h>
#include <qregexp.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------- CClassStore::CClassStore()
 * CClassStore()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassStore::CClassStore() 
  : classIterator( classes ),
    gvIterator( globalVariables ),
    gfIterator( gfNameAndArg ),
    gsIterator( globalStructures )
{
  classes.setAutoDelete( true );
  globalVariables.setAutoDelete( true );
  globalFunctions.setAutoDelete( true );
  globalStructures.setAutoDelete( true );
}

/*---------------------------------------- CClassStore::~CClassStore()
 * ~CClassStore()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassStore::~CClassStore()
{
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- CClassStore::wipeout()
 * wipeout()
 *   Remove all parsed classes and reset the state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassStore::wipeout()
{
  classes.clear();
  globalVariables.clear();
  globalFunctions.clear();
}

/*-------------------------------------- CClassStore::addGlobalVar()
 * addGlobalVar()
 *   Add a global variable to the store.
 *
 * Parameters:
 *   aVar          The variable to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassStore::addGlobalVar( CParsedAttribute *aVar )
{
  assert( aVar != NULL );

  globalVariables.insert( aVar->name, aVar );
}

/*---------------------------------- CClassStore::addGlobalFunction()
 * addGlobalFunction()
 *   Add a global function to the store.
 *
 * Parameters:
 *   aFunc         The variable to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassStore::addGlobalFunction( CParsedMethod *aFunc )
{
  assert( aFunc != NULL );

  QString str;

  globalFunctions.append( aFunc );
  
  aFunc->toString( str );
  gfNameAndArg.insert( str, aFunc );
}

/*------------------------------------ CClassStore::addGlobalStruct()
 * addGlobalStruct()
 *   Add a global structure.
 *
 * Parameters:
 *   aStruct       The structure to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassStore::addGlobalStruct( CParsedStruct *aStruct )
{
  assert( aStruct != NULL );
  
  globalStructures.insert( aStruct->name, aStruct );
}

/*------------------------------------------- CClassStore::storeAll()
 * storeAll()
 *   Store all parsed classes as a database.
 *
 * Parameters:
 *   aClass        The class to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassStore::storeAll( const char *aFilename )
{
  QString str;
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
  {
    classIterator.current()->asPersistantString( str );
    debug( "Storing:" );
    debug( "----------" );
    debug( str );
    debug( "----------" );
  }
}

/*------------------------------------------- CClassStore::addClass()
 * addClass()
 *   Add a class to the store.
 *
 * Parameters:
 *   aClass        The class to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassStore::addClass( CParsedClass *aClass )
{
  assert( aClass != NULL && !aClass->name.isEmpty() && !hasClass( aClass->name ) );

  classes.insert( aClass->name, aClass );
}

/*------------------------------------------------- CClassStore::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassStore::out()
{
  CParsedMethod *aMethod;

  // Output all classes.
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
    classIterator.current()->out();

  cout << "Global declarations\n";
  cout << "  Functions:\n";
  for( aMethod = globalFunctions.first(); 
       aMethod != NULL; 
       aMethod = globalFunctions.next() )
    aMethod->out();

  cout << "  Variables:\n";
  for( gvIterator.toFirst();
       gvIterator.current();
       ++gvIterator )
    gvIterator.current()->out();

  cout << "  Structures:\n";
  for( gsIterator.toFirst();
       gsIterator.current();
       ++gsIterator )
    gsIterator.current()->out();
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- CClassStore::hasClass()
 * hasClass()
 *   Tells if a class exist in the store.
 *
 * Parameters:
 *   aName          Name of the class to check.
 *
 * Returns:
 *   bool           Result of the lookup.
 *-----------------------------------------------------------------*/
bool CClassStore::hasClass( const char *aName )
{
  return ( getClassByName( aName ) != NULL );
}

/*-------------------------------------- CClassStore::getClassByName()
 * getClassByName()
 *   Get a class from the list by using its' name.
 *
 * Parameters:
 *   aName          Name of the class to fetch.
 *
 * Returns:
 *   CParsedClass * The class we looked for.
 *   NULL           Otherwise.
 *-----------------------------------------------------------------*/
CParsedClass *CClassStore::getClassByName( const char *aName )
{
  assert( aName != NULL );
    
  return classes.find( aName );
}

/*--------------------------- CClassStore::getGlobalFunctionByName()
 * getGlobalFunctionByName()
 *   Get a global function from the store by using its' name.
 *
 * Parameters:
 *   aName          Name of the function to fetch.
 *
 * Returns:
 *   CParsedMethod* The function we looked for.
 *   NULL           Otherwise.
 *-----------------------------------------------------------------*/
CParsedMethod *CClassStore::getGlobalFunctionByName( const char *aName )
{
  CParsedMethod *retVal = NULL;

  for( retVal = globalFunctions.first(); 
       retVal != NULL && retVal->name != aName;
       retVal = globalFunctions.next() )
    ;
    
  return retVal;
}

/*----------------------- CClassStore::getGlobalFunctionByNameAndArg()
 * getGlobalFunctionByNameAndArg()
 *   Get a global function from the store by using its' name and 
 *   arguments.
 *
 * Parameters:
 *   aName          Name of the function to fetch.
 *
 * Returns:
 *   CParsedMethod* The function we looked for.
 *   NULL           Otherwise.
 *-----------------------------------------------------------------*/
CParsedMethod *CClassStore::getGlobalFunctionByNameAndArg( const char *aName )
{
  return gfNameAndArg.find( aName );
}

/*--------------------------------- CClassStore::getGlobalVarByName()
 * getGlobalVarByName()
 *   Get a global variable from the store by using its' name.
 *
 * Parameters:
 *   aName             Name of the variable to fetch.
 *
 * Returns:
 *   CParsedAttribute* The variable we looked for.
 *   NULL              Otherwise.
 *-----------------------------------------------------------------*/
CParsedAttribute *CClassStore::getGlobalVarByName( const char *aName )
{
  return globalVariables.find( aName );
}

/*--------------------------------- CClassStore::getGlobalStructByName()
 * getGlobalStructByName()
 *   Get a global structure from the store by using its' name.
 *
 * Parameters:
 *   aName             Name of the variable to fetch.
 *
 * Returns:
 *   CParsedStruct*    The structure we looked for.
 *   NULL              Otherwise.
 *-----------------------------------------------------------------*/
CParsedStruct *CClassStore::getGlobalStructByName( const char *aName )
{
  return globalStructures.find( aName );
}

/*--------------------------------- CClassStore::getClassesByParent()
 * getClassesByParent()
 *   Get all classes with a certain parent.
 *
 * Parameters:
 *   aName             Name of the parent.
 *
 * Returns:
 *   QList<CParsedClass> * The classes with the desired parent.
 *-----------------------------------------------------------------*/
QList<CParsedClass> *CClassStore::getClassesByParent( const char *aName )
{
  QList<CParsedClass> *retVal = new QList<CParsedClass>();

  retVal->setAutoDelete( false );
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
  {
    if( classIterator.current()->hasParent( aName ) )
      retVal->append( classIterator.current() );
  }

  return retVal;
}

/*------------------------------------ CClassStore::getClassClients()
 * getClassClients()
 *   Fetches all clients of a named class.
 *
 * Parameters:
 *   aName             Name of the class.
 *
 * Returns:
 *   QList<CParsedClass> * The clients of the class.
 *-----------------------------------------------------------------*/
QList<CParsedClass> *CClassStore::getClassClients( const char *aName )
{
  assert( aName != NULL );

  bool exit;
  CParsedClass *aClass;
  CParsedAttribute *aAttr;
  QList<CParsedClass> *retVal = new QList<CParsedClass>();

  retVal->setAutoDelete( false );
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
  {
    aClass = classIterator.current();
    if( aClass->name != aName )
    {
      exit = false;
      for( aClass->attributeIterator.toFirst();
           aClass->attributeIterator.current() && !exit;
           ++(aClass->attributeIterator) )
      {
        aAttr = aClass->attributeIterator.current();
        exit = ( aAttr->type.find( aName ) != -1 );
      }

      if( exit )
        retVal->append( aClass );
    }
  }

  return retVal;
}

/*------------------------------------ CClassStore::getClassSuppliers()
 * getClassSuppliers()
 *   Fetches all suppliers of a named class.
 *
 * Parameters:
 *   aName             Name of the class.
 *
 * Returns:
 *   QList<CParsedClass> * The suppliers to the class.
 *-----------------------------------------------------------------*/
QList<CParsedClass> *CClassStore::getClassSuppliers( const char *aName )
{
  assert( aName != NULL );
  assert( hasClass( aName ) );

  CParsedClass *aClass;
  CParsedClass *toAdd;
  QString str;
  QList<CParsedClass> *retVal = new QList<CParsedClass>();

  retVal->setAutoDelete( false );

  aClass = getClassByName( aName );
  for( aClass->attributeIterator.toFirst();
       aClass->attributeIterator.current();
       ++aClass->attributeIterator )
  {
    str = aClass->attributeIterator.current()->type;

    // Remove all unwanted stuff.
    str = str.replace( "[\*&]", "" );
    str = str.replace( "const", "" );
    str = str.replace( "void", "" );
    str = str.replace( "bool", "" );
    str = str.replace( "uint", "" );
    str = str.replace( "int", "" );
    str = str.replace( "char", "" );
    str = str.stripWhiteSpace();

    // If this isn't the class and the string contains data, we check for it.
    if( str != aName && !str.isEmpty() )
    {
      debug( "Checking if '%s' is a class", str.data() );
      toAdd = getClassByName( str );
      if( toAdd )
        retVal->append( toAdd );
    }
  }

  return retVal;
}

/*------------------------------------ CClassStore::getSortedClasslist()
 * getSortedClasslist()
 *   Get all classes in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedClass> * The classes.
 *-----------------------------------------------------------------*/
QList<CParsedClass> *CClassStore::getSortedClasslist()
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

/*------------------------------ CClassStore::getSortedGlobalVarList()
 * getSortedGlobalVarList()
 *   Get all global variables in a sorted list.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedAttribute> * The variables.
 *-----------------------------------------------------------------*/
QList<CParsedAttribute> *CClassStore::getSortedGlobalVarList()
{
  QList<CParsedAttribute> *retVal = new QList<CParsedAttribute>();
  QStrList srted;
  char *str;

  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( gvIterator.toFirst();
       gvIterator.current();
       ++gvIterator )
  {
    srted.inSort( gvIterator.current()->name );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getGlobalVarByName( str ) );
  }

  return retVal;
}

/*------------------------------ CClassStore::getSortedGlobalStructList()
 * getSortedGlobalStructList()
 *   Get all global structures in a sorted list.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedAttribute> * The variables.
 *-----------------------------------------------------------------*/
QList<CParsedStruct> *CClassStore::getSortedGlobalStructList()
{
  QList<CParsedStruct> *retVal = new QList<CParsedStruct>();
  QStrList srted;
  char *str;

  retVal->setAutoDelete( false );

  // Ok... This sucks. But I'm lazy.
  for( gsIterator.toFirst();
       gsIterator.current();
       ++gsIterator )
  {
    srted.inSort( gsIterator.current()->name );
  }

  for( str = srted.first();
       str != NULL;
       str = srted.next() )
  {
    retVal->append( getGlobalStructByName( str ) );
  }

  return retVal;
}
