/***************************************************************************
                          classstore.cpp  -  description
                             -------------------
    begin                : Fri Mar 19 1999
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

#include "classstore.h"
#include <iostream.h>
#include <qregexp.h>
#include "programmingbycontract.h"
#include "main.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/
extern int ScopeLevel;
/*---------------------------------------- ClassStore::ClassStore()
 * ClassStore()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ClassStore::ClassStore() 
{
  setInstance(SourceInfoFactory::instance());


  // Initialize the persistant class store.
  //  globalStore.setPath( "/tmp"  );
  globalStore.setFilename( "classes.db" );

  // Open the store if it exists, else create it.
  globalStore.open();

  // Always use full path for the global container.
  globalContainer.setUseFullpath( true );
}

/*---------------------------------------- ClassStore::~ClassStore()
 * ~ClassStore()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ClassStore::~ClassStore()
{
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- ClassStore::wipeout()
 * wipeout()
 *   Remove all parsed classes and reset the state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::wipeout()
{
  ScopeLevel=0;
  globalContainer.clear();
}


/*-------------------------------- ClassStore::removeWithReferences()
 * removeWithReferences()
 *   Remove all items in the store with references to the file.
 *
 * Parameters:
 *   aFile          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::removeWithReferences( const QString &aFile )
{
  // Remove all classes with reference to this file.
  // Need to take care here as we are using an iterator on a
  // container that we can be deleting classes from.
  ParsedClass *aClass = globalContainer.classIterator.toFirst();
  while (aClass)
  {
    if( aClass->declaredInFile == aFile )
    {
      if ( aClass->definedInFile.isEmpty() ||
            aClass->definedInFile == aClass->declaredInFile )
        removeClass( aClass->path() );
      else
        aClass->removeWithReferences(aFile);
    }
    else
    {
      if ( aClass->definedInFile == aFile )
      {
        if ( aClass->declaredInFile.isEmpty() )
          removeClass( aClass->path() );
        else
          aClass->removeWithReferences(aFile);
      }
    }

    // Move to the next class if we arn't already there due
    // to the class being removed.
    if (aClass == globalContainer.classIterator.current())
      ++globalContainer.classIterator;

    aClass = globalContainer.classIterator.current();
  }
  
  // Remove all global functions, variables and structures.
  globalContainer.removeWithReferences( aFile );
}

/*-------------------------------- ClassStore::getDependentFiles()
 * getDependentFiles()
 *    Find all files that depends on the given file
 *
 * Parameters:
 *   fileList       - The files to check
 *   dependentList  - The dependent files are added to this list
 *
 * Returns:
 *    The added files in the dependentList parameter.
 *
 *-----------------------------------------------------------------*/
//void ClassStore::getDependentFiles(  QStrList& fileList,
//                                      QStrList& dependentList)
//{
//  for (QString thisFile = fileList.first();
//          thisFile;
//          thisFile = fileList.next())
//  {
//    // Find all classes with reference to this file.
//    for( globalContainer.classIterator.toFirst();
//         globalContainer.classIterator.current();
//         ++globalContainer.classIterator )
//    {
//      ParsedClass *aClass = globalContainer.classIterator.current();
//
//      if( aClass->declaredInFile  == thisFile &&
//          aClass->definedInFile   != thisFile)
//      {
//        if (dependentList.find(aClass->definedInFile) == -1)
//          dependentList.append(aClass->definedInFile);
//      }
//
//      // now scan methods for files
//      // ie a class in a.h is split into aa.cpp and ab.cpp
//      //
//      // TBD perhaps - as the above catches most situations
//    }
//  }
//}

/*------------------------------------------- ClassStore::storeAll()
 * storeAll()
 *   Store all parsed classes as a database.
 *
 * Parameters:
 *   aClass        The class to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::storeAll()
{
  QString str;
  ParsedClass *aClass;

  for( globalContainer.classIterator.toFirst();
       globalContainer.classIterator.current();
       ++globalContainer.classIterator )
  {
    aClass = globalContainer.classIterator.current();
    str = aClass->asPersistantString();

    debug( "Storing:" );
    debug( "----------" );
    debug( str );
    globalStore.storeClass( aClass );
    debug( "----------" );
  }
}

/*------------------------------------------- ClassStore::addScope()
 * addScope()
 *   Add a scope to the store.
 *
 * Parameters:
 *   aScope        The scope to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::addScope( ParsedScopeContainer *aScope )
{
  REQUIRE( "Valid scope", aScope != NULL );
  REQUIRE( "Valid scope name", !aScope->name.isEmpty() );
  REQUIRE( "Unique scope path", !hasScope( aScope->path() ) );

  globalContainer.addScope( aScope );
}

/*------------------------------------------- ClassStore::addClass()
 * addClass()
 *   Add a class to the store.
 *
 * Parameters:
 *   aClass        The class to add.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::addClass( ParsedClass *aClass )
{
  REQUIRE( "Valid class", aClass != NULL );
  REQUIRE( "Valid classname", !aClass->name.isEmpty() );
  REQUIRE( "Unique classpath", !hasClass( aClass->path() ) );

  globalContainer.addClass( aClass );

  if( globalStore.isOpen )
    globalStore.storeClass( aClass );
}

/*---------------------------------------- ClassStore::removeClass()
 * removeClass()
 *   Remove a class from the store.
 *
 * Parameters:
 *   aName        Name of the class to remove
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::removeClass( const QString &aName )
{
  REQUIRE( "Valid classname", aName != NULL );
  REQUIRE( "Valid classname length", strlen( aName ) > 0 );
  REQUIRE( "Class exists", hasClass( aName ) );

  globalContainer.removeClass( aName );

  if( globalStore.isOpen )
    globalStore.removeClass( aName );
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- ClassStore::asForest()
 * asForest()
 *   Return the store as a forest(collection of trees).
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ClassTreeNode> List of trees.
 *-----------------------------------------------------------------*/
QList<ClassTreeNode> *ClassStore::asForest()
{
  ParsedClass *aClass;
  ParsedParent *aParent;  
  ClassTreeNode *childNode;
  ClassTreeNode *parentNode;
  QDict<ClassTreeNode> ctDict;
  QList<ClassTreeNode> *retVal = new QList<ClassTreeNode>;

  // Iterate over all parsed classes.
  for( globalContainer.classIterator.toFirst();
       globalContainer.classIterator.current();
       ++globalContainer.classIterator )
  {
    aClass = globalContainer.classIterator.current();

    // Check if we have added the child.
    childNode = ctDict.find( aClass->name );
    
    // If not in the table already, we add a new node.
    if( childNode == NULL )
    {
      childNode = new ClassTreeNode();

      ctDict.insert( aClass->name, childNode );
    }
    else if( !childNode->isInSystem )
      retVal->removeRef( childNode );
    
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
        // Check if we have added the parent already.
        parentNode = ctDict.find( aParent->name );
        
        // Add a new node for the parent if not added already.
        if( parentNode == NULL )
        {
          // Create the parentnode.
          parentNode = new ClassTreeNode();
          parentNode->setName( aParent->name );
          parentNode->setIsInSystem( false );

          retVal->append( parentNode );
          ctDict.insert( parentNode->name, parentNode );
        }
        
        // Add the child to the parent node.
        parentNode->addChild( childNode );
      }
    }
  }

  return retVal;
}

/*-------------------------------------------- ClassStore::hasScope()
 * hasScope()
 *   Tells if a scope exist in the store.
 *
 * Parameters:
 *   aName          Name of the scope to check for.
 * Returns:
 *   bool           Result of the lookup.
 *-----------------------------------------------------------------*/
bool ClassStore::hasScope( const QString &aName )
{
  REQUIRE1( "Valid scope name", aName != NULL, false );
  REQUIRE1( "Valid scope name length", strlen( aName ) > 0, false );

  return globalContainer.hasScope( aName );
}

/*-------------------------------------- ClassStore::getScopeByName()
 * getScopeByName()
 *   Get a scope from the store by using its' name.
 *
 * Parameters:
 *   aName          Name of the scope to fetch.
 * Returns:
 *   Pointer to the scope or NULL if not found.
 *-----------------------------------------------------------------*/
ParsedScopeContainer *ClassStore::getScopeByName( const QString &aName )
{
  REQUIRE1( "Valid scope name", aName != NULL, NULL );
  REQUIRE1( "Valid scope name length", aName.length() > 0, NULL );

  return globalContainer.getScopeByName( aName );
}


/*-------------------------------------------- ClassStore::hasClass()
 * hasClass()
 *   Tells if a class exist in the store.
 *
 * Parameters:
 *   aName          Name of the class to check.
 * Returns:
 *   bool           Result of the lookup.
 *-----------------------------------------------------------------*/
bool ClassStore::hasClass( const QString &aName )
{
  REQUIRE1( "Valid classname", aName != NULL, false );
  REQUIRE1( "Valid classname length", aName.length() > 0, false );

  return globalContainer.hasClass( aName ) || 
    ( globalStore.isOpen && globalStore.hasClass( aName ) );
  //return classes.find( aName ) != NULL;
}

/*-------------------------------------- ClassStore::getClassByName()
 * getClassByName()
 *   Get a class from the list by using its' name.
 *
 * Parameters:
 *   aName          Name of the class to fetch.
 *
 * Returns:
 *   ParsedClass * The class we looked for.
 *   NULL           Otherwise.
 *-----------------------------------------------------------------*/
ParsedClass *ClassStore::getClassByName( const QString &aName )
{
  REQUIRE1( "Valid classname", aName != NULL, NULL );
  REQUIRE1( "Valid classname length", strlen( aName ) > 0, NULL );
  
  ParsedClass *aClass;

  if( globalStore.isOpen && globalStore.hasClass( aName ) )
    aClass = globalStore.getClassByName( aName );
  else
    aClass = globalContainer.getClassByName( aName );

  return aClass;
}

/*--------------------------------- ClassStore::getClassesByParent()
 * getClassesByParent()
 *   Get all classes with a certain parent.
 *
 * Parameters:
 *   aName             Name of the parent.
 *
 * Returns:
 *   QList<ParsedClass> * The classes with the desired parent.
 *-----------------------------------------------------------------*/
QList<ParsedClass> *ClassStore::getClassesByParent( const QString &aName )
{
  REQUIRE1( "Valid classname", aName != NULL, new QList<ParsedClass>() );
  REQUIRE1( "Valid classname length", strlen( aName ) > 0, new QList<ParsedClass>() );

  QList<ParsedClass> *retVal = new QList<ParsedClass>();
  ParsedClass *aClass;

  retVal->setAutoDelete( false );
  for( globalContainer.classIterator.toFirst();
       globalContainer.classIterator.current();
       ++globalContainer.classIterator )
  {
    aClass = globalContainer.classIterator.current();
    if( aClass->hasParent( aName ) )
      retVal->append( aClass );
  }

  return retVal;
}

/*------------------------------------ ClassStore::getClassClients()
 * getClassClients()
 *   Fetches all clients of a named class.
 *
 * Parameters:
 *   aName             Name of the class.
 *
 * Returns:
 *   QList<ParsedClass> * The clients of the class.
 *-----------------------------------------------------------------*/
QList<ParsedClass> *ClassStore::getClassClients( const QString &aName )
{
  REQUIRE1( "Valid classname", aName != NULL, new QList<ParsedClass>() );
  REQUIRE1( "Valid classname length", strlen( aName ) > 0, new QList<ParsedClass>() );

  bool exit;
  ParsedClass *aClass;
  ParsedAttribute *aAttr;
  QList<ParsedClass> *retVal = new QList<ParsedClass>();

  retVal->setAutoDelete( false );
  for( globalContainer.classIterator.toFirst();
       globalContainer.classIterator.current();
       ++globalContainer.classIterator )
  {
    aClass = globalContainer.classIterator.current();
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

/*------------------------------------ ClassStore::getClassSuppliers()
 * getClassSuppliers()
 *   Fetches all suppliers of a named class.
 *
 * Parameters:
 *   aName             Name of the class.
 *
 * Returns:
 *   QList<ParsedClass> * The suppliers to the class.
 *-----------------------------------------------------------------*/
QList<ParsedClass> *ClassStore::getClassSuppliers( const QString &aName )
{
  REQUIRE1( "Valid classname", aName != NULL, new QList<ParsedClass>() );
  REQUIRE1( "Valid classname length", aName.length() > 0, new QList<ParsedClass>() );
  REQUIRE1( "Class exists", hasClass( aName ), new QList<ParsedClass>() );

  ParsedClass *aClass;
  ParsedClass *toAdd;
  QString str;
  QList<ParsedClass> *retVal = new QList<ParsedClass>();

  retVal->setAutoDelete( false );

  aClass = getClassByName( aName );
  for( aClass->attributeIterator.toFirst();
       aClass->attributeIterator.current();
       ++aClass->attributeIterator )
  {
    str = aClass->attributeIterator.current()->type;

    // Remove all unwanted stuff.
    str = str.replace(  QRegExp("[\\*&]"), "" );
    str = str.replace(  QRegExp("const"), "" );
    str = str.replace(  QRegExp("void"), "" );
    str = str.replace(  QRegExp("bool"), "" );
    str = str.replace(  QRegExp("uint"), "" );
    str = str.replace(  QRegExp("int"), "" );
    str = str.replace(  QRegExp("char"), "" );
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

/*------------------------------------ ClassStore::getSortedClassList()
 * getSortedClassList()
 *   Get all classes in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<ParsedClass> * The classes.
 *-----------------------------------------------------------------*/
QList<ParsedClass> *ClassStore::getSortedClassList()
{
  QList<ParsedClass> *list = globalContainer.getSortedClassList();
  ParsedClass *aClass;

  // Remove all non-global classes.
  aClass = list->first();
  while (aClass != NULL)
  {
    if( !aClass->declaredInScope.isEmpty() )
    {
      list->remove();
      aClass = list->current();
    }
    else
      aClass = list->next();
  }

  return list;
}

/*---------------------------- ClassStore::getSortedClassNameList()
 * getSortedClassNameList()
 *   Get all classnames in sorted order.
 *
 * Parameters:
 *   -
 * Returns:
 *   QStrList * The classnames.
 *-----------------------------------------------------------------*/
QStrList *ClassStore::getSortedClassNameList()
{
  return globalContainer.getSortedClassNameList(true);
}

/*-------------------------- ClassStore::getVirtualMethodsForClass()
 * getVirtualMethodsForClass()
 *   Fetch all virtual methods, both implemented and not.
 *
 * Parameters:
 *   aName      Name of the class.
 *   implList   The list that will contain the implemented virtual 
 *              methods.
 *   availList  The list hat will contain the available virtual
 *              methods.
 * 
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::getVirtualMethodsForClass( const QString &aName, 
                                             QList<ParsedMethod> *implList,
                                             QList<ParsedMethod> *availList )
{
  REQUIRE( "Valid classname", aName != NULL );
  REQUIRE( "Valid classname length", aName.length() > 0 );
  REQUIRE( "Class exists", hasClass( aName ) );

  ParsedClass *aClass;
  ParsedParent *aParent;
  ParsedClass *parentClass;
  QList<ParsedMethod> *list;
  ParsedMethod *aMethod;
  QDict<char> added;
  QString str;

  // Start by reseting the lists.
  implList->setAutoDelete( false );
  availList->setAutoDelete( false );
  implList->clear();
  availList->clear();

  // Try to fetch the class
  aClass = getClassByName( aName );
  if( aClass != NULL )
  {
    // Iterate over all parents.
    for( aParent = aClass->parents.first();
         aParent != NULL;
         aParent = aClass->parents.next() )
    {
      // Try to fetch the parent.
      parentClass = getClassByName( aParent->name );
      if( parentClass != NULL )
      {
        list = parentClass->getVirtualMethodList();

        for( aMethod = list->first();
             aMethod != NULL;
             aMethod = list->next() )
        {
          // Check if we already have the method.
          if( aClass->getMethod( *aMethod ) != NULL )
          {
            implList->append( aMethod );
            added.insert( aMethod->asString(), "" );
          }
          else if( !aMethod->isConstructor && !aMethod->isDestructor )
            availList->append( aMethod );
        }
        
        delete list;
      }

    }
  }
}

/*---------------------------- ClassStore::getSortedClassNameList()
 * getSortedClassNameList()
 *   Get all global structures not declared in a scope.
 *
 * Parameters:
 *   -
 * Returns:
 *   A sorted list of global structures.
 *-----------------------------------------------------------------*/
QList<ParsedStruct> *ClassStore::getSortedStructList() 
{ 
  QList<ParsedStruct> *retVal = new QList<ParsedStruct>();

  // Iterate over all structs in the scope.
  for( globalContainer.structIterator.toFirst();
       globalContainer.structIterator.current();
       ++globalContainer.structIterator )
  {
    // Only append global structs.
    if( globalContainer.structIterator.current()->declaredInScope.isEmpty() )
      retVal->append( globalContainer.structIterator.current() );
  }

  return retVal; 
}

/*------------------------------------------------- ClassStore::out()
 * out()
 *   Output this object to stdout.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void ClassStore::out()
{
  QList<ParsedScopeContainer> *globalScopes;
  QList<ParsedMethod> *globalMethods;
  QList<ParsedAttribute> *globalAttributes;
  QList<ParsedStruct> *globalStructs;
  QList<ParsedClass> *classes;
  ParsedScopeContainer *aScope;
  ParsedClass *aClass;
  ParsedMethod *aMethod;
  ParsedAttribute *aAttr;
  ParsedStruct *aStruct;

  // Output all namespaces
  cout << "Global namespaces" << endl;
  globalScopes = globalContainer.getSortedScopeList();
  for( aScope = globalScopes->first();
       aScope != NULL;
       aScope = globalScopes->next() )
    aScope->out();


  // Output all classes.
  cout << "Global classes\n";
  classes = getSortedClassList();
  for( aClass = classes->first();
       aClass != NULL;
       aClass = classes->next() )
  {
    aClass->out();
  }
  delete classes;

  // Global methods
  cout << "Global functions\n";

  globalMethods = globalContainer.getSortedMethodList();
  for( aMethod = globalMethods->first();
       aMethod != NULL;
       aMethod = globalMethods->next() )
  {
    aMethod->out();
  }
  delete globalMethods;

  // Global structures
  cout << "Global variables\n";
  globalAttributes = globalContainer.getSortedAttributeList();
  for( aAttr = globalAttributes->first();
       aAttr != NULL;
       aAttr = globalAttributes->next() )
  {
    aAttr->out();
  }
  delete globalAttributes;  

  // Global structures
  cout << "Global structs\n";
  globalStructs = getSortedStructList();
  for( aStruct = globalStructs->first();
       aStruct != NULL;
       aStruct = globalStructs->next() )
  {
    aStruct->out();
  }
  delete globalStructs;
}

#include "classstore.moc"
