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

#include <qlistview.h>
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
void CClassTreeHandler::setStore( ClassStore *aStore )
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
void CClassTreeHandler::addScopes( QValueList<ParsedScopeContainer*> list,
                                    QListViewItem *parent )
{
  ParsedScopeContainer *scope;

  QValueList<ParsedScopeContainer*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
      {
          scope = *listit;
          addScope( scope, parent );
      }
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
void CClassTreeHandler::addScope( ParsedScopeContainer *aScope,
                                  QListViewItem *parent )
{
  assert( aScope != NULL );
  assert( parent != NULL );

  QValueList<ParsedScopeContainer*> scopeList;
  QValueList<ParsedClass*> classList;
  QValueList<ParsedStruct*> structList;
  QValueList<ParsedMethod*> methodList;
  QValueList<ParsedAttribute*> attrList;

  QListViewItem *item = addItem( aScope->name(), THSCOPE, parent );

  // Add namespaces
  scopeList = aScope->getSortedScopeList();
  addScopes( scopeList, item );

  // Add classes
  classList = aScope->getSortedClassList();
  addClasses( classList, item );

  // Add structs
  structList = aScope->getSortedStructList();
  addStructs( structList, item, CTHALL );

  // Add functions
  methodList = aScope->getSortedMethodList();
  addMethods( methodList, item, CTHALL );

  // Add variables
  attrList = aScope->getSortedAttributeList();
  addAttributes( attrList, item, CTHALL );
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
void CClassTreeHandler::updateClass( ParsedClass *aClass,
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
void CClassTreeHandler::addClasses( QValueList<ParsedClass*> list, QListViewItem *parent )
{
  QListViewItem *item;
  ParsedClass *aPC;

  QValueList<ParsedClass*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    aPC = *listit;
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
QListViewItem *CClassTreeHandler::addClass( ParsedClass *aClass,
                                            QListViewItem *parent )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  return addItem( aClass->name(), THCLASS, parent );
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
void CClassTreeHandler::addSubclassesFromClass( ParsedClass *aClass,
                                                QListViewItem *parent )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QListViewItem *ci;

  for( aClass->classIterator.toFirst();
       aClass->classIterator.current();
       ++aClass->classIterator )
  {
    ci = addClass( aClass->classIterator.current()->name(), parent );
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
void CClassTreeHandler::addStructsFromClass( ParsedClass *aClass,
                                             QListViewItem *parent,
                                             CTHFilter filter )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QValueList<ParsedStruct*> list;

  list = aClass->getSortedStructList();
  addStructs( list, parent, filter );
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
void CClassTreeHandler::addStructs( QValueList<ParsedStruct*> list,
                                    QListViewItem *parent,
                                    CTHFilter filter )
{
  assert( parent != NULL );

  ParsedStruct *aStruct;

  // Add the structures
  QValueList<ParsedStruct*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    aStruct = *listit;
    if( filter == CTHALL || filter == (CTHFilter)aStruct->access() )
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
void CClassTreeHandler::addStruct( ParsedStruct *aStruct,
                                   QListViewItem *parent )
{
  QListViewItem *root;
  ParsedAttribute *anAttr;
  ParsedMethod* anMethod; // --- Daniel
  QValueList<ParsedAttribute*> list;
  QValueList<ParsedMethod*> method; // --- Daniel
  QValueList<ParsedStruct*> structList;
  ParsedStruct *childStruct;

  root = addItem( aStruct->name(), THSTRUCT, parent );

  structList = aStruct->getSortedStructList();
  QValueList<ParsedStruct*>::ConstIterator structListIt;
  for (structListIt = structList.begin(); structListIt != structList.end(); ++structListIt)
  {
    childStruct = *structListIt;
    addStruct( childStruct, root );
  }

  list = aStruct->getSortedAttributeList();

  QValueList<ParsedAttribute*>::ConstIterator attributeListIt;
  for (attributeListIt = list.begin(); attributeListIt != list.end(); ++attributeListIt)
  {
    anAttr = *attributeListIt;
    addAttribute( anAttr, root );
  }

  // --- added by Daniel
  method = aStruct->getSortedMethodList( );
  QValueList<ParsedMethod*>::ConstIterator methodIt;
  for (methodIt = method.begin(); methodIt != method.end(); ++methodIt)
  {
      anMethod = *methodIt;
      addMethod( anMethod, root );
    }
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
void CClassTreeHandler::addMethodsFromClass( ParsedClass *aClass,
                                             QListViewItem *parent,
                                             CTHFilter filter )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QValueList<ParsedMethod*> list;

  list = aClass->getSortedMethodList();

  addMethods( list, parent, filter );
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
void CClassTreeHandler::addMethods( QValueList<ParsedMethod*> list,
                                    QListViewItem *parent,
                                    CTHFilter filter )
{
  assert( parent != NULL );

  ParsedMethod *aMethod;

  // Add the methods
  QValueList<ParsedMethod*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    aMethod = *listit;
    if( filter == CTHALL || filter == (CTHFilter)aMethod->access() )
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
void CClassTreeHandler::addMethod( ParsedMethod *aMethod,
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

  str = aMethod->asString();
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
void CClassTreeHandler::addAttributesFromClass( ParsedClass *aClass,
                                                QListViewItem *parent,
                                                CTHFilter filter )
{
  assert( aClass != NULL );
  assert( parent != NULL );

  QValueList<ParsedAttribute*> list;

  list = aClass->getSortedAttributeList();

  addAttributes( list, parent, filter );
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
void CClassTreeHandler::addAttributes( QValueList<ParsedAttribute*> list,
                                       QListViewItem *parent,
                                       CTHFilter filter )
{
  assert( parent != NULL );

  ParsedAttribute *aAttr;

  // Add the methods
  QValueList<ParsedAttribute*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    aAttr = *listit;
    if( filter == CTHALL || filter == (CTHFilter)aAttr->access() )
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
void CClassTreeHandler::addAttribute( ParsedAttribute *aAttr,
                                      QListViewItem *parent )
{
  assert( aAttr != NULL );
  assert( parent != NULL );

  THType type = THPUBLIC_ATTR;

  if( aAttr->isProtected() )
    type = THPROTECTED_ATTR;
  else if( aAttr->isPrivate() )
    type = THPRIVATE_ATTR;

  addItem( aAttr->name(), type, parent );
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
void CClassTreeHandler::addSlotsFromClass( ParsedClass *aPC, QListViewItem *parent )
{
  ParsedMethod *aMethod;
  QString str;
  QValueList<ParsedMethod*> list;

  THType type = THPUBLIC_SLOT;

  list = aPC->getSortedSlotList();

  // Add the methods
  QValueList<ParsedMethod*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    aMethod = *listit;
    if( aMethod->isProtected() )
      type = THPROTECTED_SLOT;
    else if( aMethod->isPrivate() )
      type = THPRIVATE_SLOT;

    str = aMethod->asString();
    addItem( str, type, parent );
  }
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
void CClassTreeHandler::addSignalsFromClass( ParsedClass *aPC, QListViewItem *parent )
{
  ParsedMethod *aMethod;
  QString str;
  QValueList<ParsedMethod*> list;

  // Add all signals.
  list = aPC->getSortedSignalList();
  QValueList<ParsedMethod*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    aMethod = *listit;
    str = aMethod->asString();
    addItem( str, THSIGNAL, parent );
  }
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
void CClassTreeHandler::addGlobalFunctions( QValueList<ParsedMethod*> list,
                                            QListViewItem *parent )
{
  assert( parent != NULL );

  ParsedMethod *aMeth;

  QValueList<ParsedMethod*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
  {
    aMeth = *listit;
    addGlobalFunc( aMeth, parent );
  }
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
void CClassTreeHandler::addGlobalFunc( ParsedMethod *aMethod,
                                       QListViewItem *parent )
{
  assert( aMethod != NULL );
  assert( parent != NULL );

  QString str;

  str = aMethod->asString();
  addItem( str, THGLOBAL_FUNCTION, parent );
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
void CClassTreeHandler::addGlobalVariables( QValueList<ParsedAttribute*> list, QListViewItem *parent )
{
  assert( parent != NULL );

  ParsedAttribute *aAttr;

  QValueList<ParsedAttribute*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
      {
          aAttr = *listit;
          addGlobalVar( aAttr, parent );
      }
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
void CClassTreeHandler::addGlobalVar( ParsedAttribute *aAttr,
                                      QListViewItem *parent )
{
  assert( aAttr != NULL );
  assert( parent != NULL );

  addItem( aAttr->name(), THGLOBAL_VARIABLE, parent );
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
void CClassTreeHandler::addGlobalStructs( QValueList<ParsedStruct*> list,
                                          QListViewItem *parent )
{
  ParsedStruct *aStruct;

  QValueList<ParsedStruct*>::ConstIterator listit;
  for (listit = list.begin(); listit != list.end(); ++listit)
      {
          aStruct = *listit;
          addStruct( aStruct, parent );
      }
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
void CClassTreeHandler::getCurrentNames( QString &parentPath,
                                         QString &itemName,
                                         THType &parentType,
                                         THType &aItemType )
{
  QListViewItem *iter;
  QListViewItem *item;
  QListViewItem *parent;
  bool isContainer;

  item = tree->currentItem();
  parent = item->parent();

  if (!parent) return ;

  aItemType = itemType();
  parentType = itemType(parent);

  // Set the container flag
  isContainer  = ( aItemType ==THCLASS ||
                   aItemType == THSTRUCT ||
                   aItemType == THSCOPE );

  // If we're viewing a container we start the classname iteration at the
  // current item
  if( isContainer )
  {
    parentPath = item->text( 0 );
    itemName = "";
    parentType = itemType();
    aItemType = parentType;

    iter = parent;
  }
  else if( parentType != THFOLDER ) // Start at the parent.
  {
    parentPath = parent->text( 0 );
    itemName = item->text(0);
    parentType = itemType( parent );
    aItemType = itemType();

    iter = parent->parent();
  }
  else // Global methods and attributes
  {
    parentPath = "";
    itemName = item->text(0);
    parentType = THFOLDER;
    aItemType = itemType();

    iter = NULL;
  }

  // Build the rest of the path
  while( iter != NULL && itemType( iter ) != THFOLDER )
  {
    parentPath = "." + parentPath;
    parentPath = iter->text(0) + parentPath;

    iter = iter->parent();
  }
}
