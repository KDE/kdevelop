/***************************************************************************
                          cclasstreehandler.cpp  -  implementation
                             -------------------
    begin                : Fri Mar 19 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "cclasstreehandler.h"
#include <assert.h>


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*--------------------------- CClassTreeHandler::CClassTreeHandler()
 * CClassTreeHandler()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassTreeHandler::CClassTreeHandler()
  : CTreeHandler()
{
}

/*--------------------------- CClassTreeHandler::~CClassTreeHandler()
 * ~CClassTreeHandler()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassTreeHandler::~CClassTreeHandler()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CClassTreeHandler::setStore()
 * setStore()
 *   Set the classtore.
 *
 * Parameters:
 *   aStore         The store.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::setStore( CClassStore *aStore )
{
  assert( aStore != NULL );

  store = aStore;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CClassTreeHandler::addScopes()
 * addScopes()
 *   Add a list of scope to add to the view.
 *
 * Parameters:
 *   list            List of scopes.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addScopes( QList<CParsedScopeContainer> *list,  
                                    QListViewItem *parent )
{
  CParsedScopeContainer *scope;

  for( scope = list->first();
       scope != NULL;
       scope = list->next())
    addScope( scope, parent );
}

/*------------------------------------- CClassTreeHandler::addScope()
 * addScope()
 *   Add a scope to the view.
 *
 * Parameters:
 *   aMethod         Method to add
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/ 
void CClassTreeHandler::addScope( CParsedScopeContainer *aScope,
                                  QListViewItem *parent )
{
  assert( aScope != NULL );
  assert( parent != NULL );

  QList<CParsedScopeContainer> *scopeList;
  QList<CParsedClass> *classList;
  QList<CParsedStruct> *structList;
  QList<CParsedMethod> *methodList;
  QList<CParsedAttribute> *attrList;

  QListViewItem *item = addItem( aScope->name, THNAMESPACE, parent );
  
  // Add namespaces
  scopeList = aScope->getSortedScopeList();
  addScopes( scopeList, item );
  delete scopeList;

  // Add classes
  classList = aScope->getSortedClassList();
  addClasses( classList, item );
  delete classList;

  // Add structs
  structList = aScope->getSortedStructList();
  addStructs( structList, item, CTHALL );
  delete structList;

  // Add functions
  methodList = aScope->getSortedMethodList();
  addMethods( methodList, item, CTHALL );
  delete methodList;

  // Add variables
  attrList = aScope->getSortedAttributeList();
  addAttributes( attrList, item, CTHALL );
  delete attrList;
}

/*---------------------------------- CClassTreeHandler::updateClass()
 * updateClass()
 *   Update the class in the view using the supplied parent.
 *
 * Parameters:
 *   aClass       Class to update.
 *   parent       The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::updateClass( CParsedClass *aClass, 
                                     QListViewItem *parent )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QListViewItem *current;
  QListViewItem *next;
  
  current = parent->firstChild();
  
  // Remove all items belonging to this class.
  while( current != NULL )
  {
    next = current->nextSibling();
    parent->removeItem( current );
    current = next;
  }

  // Add parts of the class
  addSubclassesFromClass( aClass, parent );
  addStructsFromClass( aClass, parent, CTHALL );
  addMethodsFromClass( aClass, parent, CTHALL );
  addSlotsFromClass( aClass, parent );
  addSignalsFromClass( aClass, parent );
  addAttributesFromClass( aClass, parent, CTHALL );
}

/*---------------------------------- CClassTreeHandler::addClasses()
 * addClasses()
 *   Add a list of classes to the view. 
 *
 * Parameters:
 *   list         List of classes to add.
 *   parent       The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addClasses( QList<CParsedClass> *list, QListViewItem *parent )
{
  QListViewItem *item;
  CParsedClass *aPC;

  for( aPC = list->first();
       aPC != NULL;
       aPC = list->next() )
  {
    item = addClass( aPC, parent );
    updateClass( aPC, item );
    setLastItem( item );
  }
}

/*---------------------------------- CClassTreeHandler::addClass()
 * addClass()
 *   Add a class to the view. 
 *
 * Parameters:
 *   aClass       Class to add.
 *   parent       The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QListViewItem *CClassTreeHandler::addClass( CParsedClass *aClass, 
                                            QListViewItem *parent )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  return addItem( aClass->name, THCLASS, parent );
}

/*---------------------------------- CClassTreeHandler::addClass()
 * addClass()
 *   Add a class to the view. 
 *
 * Parameters:
 *   aName        Class to add.
 *   parent       The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QListViewItem *CClassTreeHandler::addClass( const char *aName, 
                                            QListViewItem *parent )
{
  assert( aName != NULL );
  assert( parent != NULL );

  return addItem( aName, THCLASS, parent );
}

/*------------------------ CClassTreeHandler::addSubclassesFromClass()
 * addSubclassesFromClass()
 *   Add all subclasses from the class to the view.
 *
 * Parameters:
 *   aClass       Class with subclasses to add.
 *   parent       The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addSubclassesFromClass( CParsedClass *aClass,
                                                QListViewItem *parent )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QListViewItem *ci;

  for( aClass->classIterator.toFirst();
       aClass->classIterator.current();
       ++aClass->classIterator )
  {
    ci = addClass( aClass->classIterator.current()->name, parent );
    updateClass( aClass->classIterator.current(), ci );
    setLastItem( ci );
  }
}

/*-------------------------- CClassTreeHandler::addStructsFromClass()
 * addStructsFromClass()
 *   Add a the selected structures from the class to the view. 
 *
 * Parameters:
 *   aClass       Class with structs to add.
 *   parent       The parent item.
 *   filter       The selection.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addStructsFromClass( CParsedClass *aClass,
                                             QListViewItem *parent,
                                             CTHFilter filter )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QList<CParsedStruct> *list;

  list = aClass->getSortedStructList();
  addStructs( list, parent, filter );
  delete list;
}

/*------------------------------------ CClassTreeHandler::addStructs()
 * addStructs()
 *   Add all structs from a list to the view.
 * Parameters:
 *   list            List with all structs to add
 *   parent          The parent item.
 *   filter          The selection.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addStructs( QList<CParsedStruct> *list,
                                    QListViewItem *parent,
                                    CTHFilter filter )
{
  assert( list != NULL );
  assert( parent != NULL );

  CParsedStruct *aStruct;

  // Add the structures
  for( aStruct = list->first();
       aStruct != NULL;
       aStruct = list->next() )
  {
    if( filter == CTHALL || filter == (CTHFilter)aStruct->exportScope )
      addStruct( aStruct, parent );
  }
}

/*------------------------------------ CClassTreeHandler::addStruct()
 * addStruct()
 *   Add a struct to the view.
 *
 * Parameters:
 *   aStruct         Structure to add
 *   parent          The parent item.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addStruct( CParsedStruct *aStruct,
                                   QListViewItem *parent )
{
  QListViewItem *root;
  CParsedAttribute *anAttr;
  QList<CParsedAttribute> *list;

  root = addItem( aStruct->name, THSTRUCT, parent );

  list = aStruct->getSortedAttributeList();

  for( anAttr = list->first();
       anAttr != NULL;
       anAttr = list->next() )
  {
    addAttribute( anAttr, root );
  }

  delete list;
}

/*-------------------------- CClassTreeHandler::addMethodsFromClass()
 * addMethodsFromClass()
 *   Add a the selected methods from the class to the view. 
 *
 * Parameters:
 *   aClass       Class with methods to add.
 *   parent       The parent item.
 *   filter       The selection.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addMethodsFromClass( CParsedClass *aClass,
                                             QListViewItem *parent,
                                             CTHFilter filter )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QList<CParsedMethod> *list;

  list = aClass->getSortedMethodList();

  addMethods( list, parent, filter );

  delete list;
}

/*------------------------------------ CClassTreeHandler::addMethods()
 * addMethods()
 *   Add a list of methods to the view.
 *
 * Parameters:
 *   list            List of methods to add.
 *   parent          The parent item.
 *   filter          The selection.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addMethods( QList<CParsedMethod> *list, 
                                    QListViewItem *parent, 
                                    CTHFilter filter )
{
  assert( list != NULL );
  assert( parent != NULL );

  CParsedMethod *aMethod;

  // Add the methods
  for( aMethod = list->first();
       aMethod != NULL;
       aMethod = list->next() )
  {
    if( filter == CTHALL || filter == (CTHFilter)aMethod->exportScope )
      addMethod( aMethod, parent );
  }
}

/*------------------------------------ CClassTreeHandler::addMethod()
 * addMethod()
 *   Add a method to the view.
 *
 * Parameters:
 *   aMethod         Method to add.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addMethod( CParsedMethod *aMethod, 
                                   QListViewItem *parent )
{
  assert( aMethod );
  assert( parent != NULL );

  THType type = THPUBLIC_METHOD;
  QString str;

  if( aMethod->isProtected() )
    type = THPROTECTED_METHOD;
  else if( aMethod->isPrivate() )
    type = THPRIVATE_METHOD;
  
  aMethod->asString( str );
  addItem( str, type, parent );
}

/*-------------------------- CClassTreeHandler::addAttributesFromClass()
 * addAttributesFromClass()
 *   Add a the selected methods from the class to the view. 
 *
 * Parameters:
 *   aClass       Class with methods to add.
 *   parent       The parent item.
 *   filter       The selection.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addAttributesFromClass( CParsedClass *aClass, 
                                                QListViewItem *parent,
                                                CTHFilter filter )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QList<CParsedAttribute> *list;

  list = aClass->getSortedAttributeList();

  addAttributes( list, parent, filter );

  delete list;
}

/*--------------------------------- CClassTreeHandler::addAttributes()
 * addAttributes()
 *   Add a list of attributes to the view.
 *
 * Parameters:
 *   list            List of methods to add.
 *   parent          The parent item.
 *   filter          The selection.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addAttributes( QList<CParsedAttribute> *list,
                                       QListViewItem *parent, 
                                       CTHFilter filter )
{
  assert( list != NULL );
  assert( parent != NULL );

  CParsedAttribute *aAttr;

  // Add the methods
  for( aAttr = list->first();
       aAttr != NULL;
       aAttr = list->next() )
  {
    if( filter == CTHALL || filter == (CTHFilter)aAttr->exportScope )
      addAttribute( aAttr, parent );
  }
}

/*------------------------------------ CClassTreeHandler::addAttribute()
 * addAttribute()
 *   Add an attribute to the view.
 *
 * Parameters:
 *   aMethod         Method to add
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addAttribute( CParsedAttribute *aAttr, 
                                      QListViewItem *parent )
{
  assert( aAttr != NULL );
  assert( parent != NULL );

  THType type = THPUBLIC_ATTR;
  
  if( aAttr->isProtected() )
    type = THPROTECTED_ATTR;
  else if( aAttr->isPrivate() )
    type = THPRIVATE_ATTR;

  addItem( aAttr->name, type, parent );
}

/*------------------------------------- CClassTreeHandler::addSlots()
 * addSlots()
 *   Add all slots from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addSlotsFromClass( CParsedClass *aPC, QListViewItem *parent )
{
  CParsedMethod *aMethod;
  QString str;
  QList<CParsedMethod> *list;

  THType type = THPUBLIC_SLOT;
  
  list = aPC->getSortedSlotList();

  // Add the methods
  for( aMethod = list->first();
       aMethod != NULL;
       aMethod = list->next() )
  {
    if( aMethod->isProtected() )
      type = THPROTECTED_SLOT;
    else if( aMethod->isPrivate() )
      type = THPRIVATE_SLOT;

    aMethod->asString( str );
    addItem( str, type, parent );
  }

  delete list;
}

/*------------------------- CClassTreeHandler::addSignalsFromClass()
 * addSignalsFromClass()
 *   Add all signals from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addSignalsFromClass( CParsedClass *aPC, QListViewItem *parent )
{
  CParsedMethod *aMethod;
  QString str;
  QList<CParsedMethod> *list;

  // Add all signals.
  list = aPC->getSortedSignalList();
  for( aMethod = list->first();
       aMethod != NULL;
       aMethod = list->next() )
  {
    aMethod->asString( str );
    addItem( str, THSIGNAL, parent );
  }

  delete list;
}

/*--------------------------- CClassTreeHandler::addGlobalFunctions()
 * addGlobalFunctions()
 *   Add a list of global functions to the view.
 *
 * Parameters:
 *   list            List of global functions.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addGlobalFunctions( QList<CParsedMethod> *list,
                                            QListViewItem *parent )
{
  assert( list != NULL );
  assert( parent != NULL );

  CParsedMethod *aMeth;

  for( aMeth = list->first();
       aMeth != NULL;
       aMeth = list->next() )
    addGlobalFunc( aMeth, parent );
}

/*--------------------------- CClassTreeHandler::addGlobalVariables()
 * addGlobalVariables()
 *   Add a list of global variables to the view.
 *
 * Parameters:
 *   list            List of global functions.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addGlobalVariables( QList<CParsedAttribute> *list, QListViewItem *parent )
{
  assert( list != NULL );
  assert( parent != NULL );

  CParsedAttribute *aAttr;

  for( aAttr = list->first();
       aAttr != NULL;
       aAttr = list->next() )
    addGlobalVar( aAttr, parent );
}

/*-------------------------------- CClassTreeHandler::addGlobalFunc()
 * addGlobalFunc()
 *   Add a global function to the view.
 *
 * Parameters:
 *   aMethod         Method to add
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addGlobalFunc( CParsedMethod *aMethod,
                                       QListViewItem *parent )
{
  assert( aMethod != NULL );
  assert( parent != NULL );


  QString str;

  aMethod->asString( str );
  addItem( str, THGLOBAL_FUNCTION, parent );
}

/*------------------------------------ CClassTreeHandler::addGlobalVar()
 * addGlobalVar()
 *   Add a global variable to the view.
 *
 * Parameters:
 *   aAttr           Attribute to add
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addGlobalVar( CParsedAttribute *aAttr,
                                      QListViewItem *parent )
{
  assert( aAttr != NULL );
  assert( parent != NULL );

  addItem( aAttr->name, THGLOBAL_VARIABLE, parent );
}


/*------------------------------ CClassTreeHandler::addGlobalStructs()
 * addGlobalStructs()
 *   Add a list of global structures to the view.
 *
 * Parameters:
 *   list            List of global structures.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addGlobalStructs( QList<CParsedStruct> *list,
                                          QListViewItem *parent )
{
  CParsedStruct *aStruct;

  for( aStruct = list->first();
       aStruct != NULL;
       aStruct = list->next() )
    addStruct( aStruct, parent );
}

/*------------------------------- CClassTreeHandler::getCurrentNames()
 * getCurrentNames()
 *   Get the names and types of the currently selected class/declaration.
 *   class == NULL for global declarations.
 *
 * Parameters:
 *   className       Name of the parent class(if any).
 *   declName        Name of the item(==NULL for classes).
 *   idxType         The type of the selected item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::getCurrentNames( const char **className, 
                                         const char **declName,
                                         THType *idxType )
{
  QListViewItem *iter;
  QListViewItem *item;
  QListViewItem *parent;
  THType parentType;

  item = tree->currentItem();
  parent = item->parent();
  parentType = itemType( parent );

  // Set the type of the current item.
  *idxType = itemType();

  // If we're viewing a class or struct declName should be empty.
  if( *idxType == THCLASS )
    *declName = NULL;
  else 
    *declName = item->text(0);

  // If we're viewing a class we start the classname iteration at the
  // current item 
  if( *idxType == THCLASS )
    iter = item;
  else // Start at the parent.
    iter = parent;

  // Set inital classname and first iteration step
  ccstr = iter->text(0);
  iter = iter->parent();

  while( iter != NULL && itemType( iter ) == THCLASS )
  {
    ccstr = "." + ccstr;
    ccstr = iter->text(0) + ccstr;

    iter = iter->parent();
  }

  // For global items, i.e parent isn't a class or struct, classname is NULL.
  if( parentType == THCLASS || parentType == THSTRUCT || 
      *idxType == THCLASS )
    *className = ccstr;
  else
    *className = NULL;
}
