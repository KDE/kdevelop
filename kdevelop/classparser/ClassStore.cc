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
  : classIterator( classes )
{
  classes.setAutoDelete( true );

  // Initialize the persistant class store.
  globalStore.setFilename( "classes.db" );

  // Open the store if it exists, else create it.
  if( globalStore.exists() )
    globalStore.open();
  else
    globalStore.create();
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
  globalContainer.clear();
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
void CClassStore::storeAll()
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
    globalStore.storeClass( classIterator.current() );
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
  assert( aClass != NULL );
  assert( !aClass->name.isEmpty() );
  assert( !hasClass( aClass->name ) );

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
  // Output all classes.
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
    classIterator.current()->out();

  cout << "Global declarations\n";
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- CClassStore::asForest()
 * asForest()
 *   Return the store as a forest(collection of trees).
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CClassTreeNode> List of trees.
 *-----------------------------------------------------------------*/
QList<CClassTreeNode> *CClassStore::asForest()
{
  CParsedClass *aClass;
  CParsedParent *aParent;  
  CClassTreeNode *childNode;
  CClassTreeNode *parentNode;
  QDict<CClassTreeNode> ctDict;
  QList<CClassTreeNode> *retVal = new QList<CClassTreeNode>;

  // Iterate over all parsed classes.
  for( classIterator.toFirst();
       classIterator.current();
       ++classIterator )
  {
    aClass = classIterator.current();

    debug( "Processing class %s", aClass->name.data() );
    
    // Check if we have added the child.
    childNode = ctDict.find( aClass->name );
    
    // If not in the table already, we add a new node.
    if( childNode == NULL )
    {
      childNode = new CClassTreeNode();

      debug( " Adding %s to dict", aClass->name.data() );

      ctDict.insert( aClass->name, childNode );
    }
    else if( !childNode->isInSystem )
    {
      debug( " Removing %s from retVal", childNode->name.data() );
      retVal->removeRef( childNode );
    }
    
    // Set childnode values.
    childNode->setName( aClass->name );
    childNode->setClass( aClass );
    childNode->setIsInSystem( true );

    // If this class has no parent, we add it as a rootnode in the forest.
    if( aClass->parents.count() == 0)
      retVal->append( childNode );
    else // Has parents
    {
      // Add this class to its' parents.
      for( aParent = childNode->theClass->parents.first();
           aParent != NULL;
           aParent = childNode->theClass->parents.next() )
      {
        debug( " Processing parent %s", aParent->name.data() );

        // Check if we have added the parent already.
        parentNode = ctDict.find( aParent->name );
        
        // Add a new node for the parent if not added already.
        if( parentNode == NULL )
        {
          // Create the parentnode.
          parentNode = new CClassTreeNode();
          parentNode->setName( aParent->name );
          parentNode->setIsInSystem( false );

          debug( "  Appending new parent %s to retVal", aParent->name.data() );

          retVal->append( parentNode );
          ctDict.insert( parentNode->name, parentNode );
        }
        
        // Add the child to the parent node.
        debug( "   Adding %s as child of %s", childNode->name.data(),
               parentNode->name.data() );
        parentNode->addChild( childNode );
      }
    }
  }

  return retVal;
}

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
  return classes.find( aName ) != NULL || globalStore.hasClass( aName );
  //return classes.find( aName ) != NULL;
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

  CParsedClass *aClass;

  if( globalStore.hasClass( aName ) )
    aClass = globalStore.getClassByName( aName );
  else
    aClass = classes.find( aName );

  return aClass;
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
    str = str.replace( "[\\*&]", "" );
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

