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
  
  while( current != NULL )
  {
    next = current->nextSibling();
    parent->removeItem( current );
    current = next;
  }

  // Add parts of the class
  addMethodsFromClass( aClass, parent, CTHALL );
  addAttributesFromClass( aClass, parent, CTHALL );
  addSlots( aClass, parent );
  addSignals( aClass, parent );
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

  addMethods( aClass->getSortedMethodList(), parent, filter );
}

/*------------------------------------ CClassTreeHandler::addMethods()
 * addMethods()
 *   Add all methods from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   parent          The parent item.
 *   filter          The selection.
 *
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
    if( filter == CTHALL || filter == aMethod->export )
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
  
  aMethod->toString( str );
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

  addAttributes( aClass->getSortedAttributeList(), parent, filter );
}

/*--------------------------------- CClassTreeHandler::addAttributes()
 * addAttributes()
 *   Add all attributes from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   parent       The parent item.
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
    if( filter == CTHALL || aAttr->export == filter )
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

/*------------------------------------ CClassTreeHandler::addGlobalFunctions()
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

/*------------------------------------ CClassTreeHandler::addGlobalFunc()
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

  aMethod->toString( str );
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
void CClassTreeHandler::addSlots( CParsedClass *aPC, QListViewItem *parent )
{
  CParsedMethod *aMethod;
  QString str;

  // Add the methods
  for( aMethod = aPC->slotList.first();
       aMethod != NULL;
       aMethod = aPC->slotList.next() )
  {
    aMethod->toString( str );
    addItem( str, THSTRUCT, parent );
  }
}

/*----------------------------------- CClassTreeHandler::addSignals()
 * addSignals()
 *   Add all signals from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   parent          The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addSignals( CParsedClass *aPC, QListViewItem *parent )
{
  CParsedMethod *aMethod;
  QString str;

  // Add the methods
  for( aMethod = aPC->signalList.first();
       aMethod != NULL;
       aMethod = aPC->signalList.next() )
  {
    aMethod->toString( str );
    addItem( str, THSTRUCT, parent );
  }
}
