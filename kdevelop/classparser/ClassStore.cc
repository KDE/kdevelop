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
    gfIterator( gfNameAndArg )
{
  classes.setAutoDelete( true );
  globalVariables.setAutoDelete( true );
  globalFunctions.setAutoDelete( true );
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
