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
#include <kiconloader.h>
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
{
  lastItem = NULL;
  lastRootItem = NULL;

  readIcons();
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
  for( int i=0; i<END_POS; i++ )
    delete icons[ i ];

  delete []icons;
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CClassTreeHandler::setTree()
 * setTree()
 *   Set the tree to draw in.
 *
 * Parameters:
 *   aTree          The tree.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::setTree( QListView *aTree )
{
  assert( aTree != NULL );

  tree = aTree;
}

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

/*--------------------------------------- CClassTreeHandler::getIcon()
 * getIcon()
 *   Get an icon from the handler.
 *
 * Parameters:
 *   anIcon         The icon to fetch
 *
 * Returns:
 *   QPixmap        The icon.
 *-----------------------------------------------------------------*/
QPixmap *CClassTreeHandler::getIcon( CTHType anIcon )
{
  return icons[ anIcon ];
}

/** Clear the view and reset internal state. */
/*---------------------------------------- CClassTreeHandler::clear()
 * clear()
 *   Clear the view and reset internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::clear()
{
  tree->clear();
  lastRootItem=NULL;
  lastItem=NULL;
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

/*---------------------------------- CClassTreeHandler::addRoot()
 * addRoot()
 *   Add an item to the view at root level.
 *
 * Parameters:
 *   aName        Name of the item.
 *   iconType     The icontype.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QListViewItem *CClassTreeHandler::addRoot( const char *aName, 
                                           CTHType iconType )
{
  assert( aName != NULL );

  QListViewItem *item;

  // Make sure the entry gets added AFTER the last entry.
  if( lastRootItem != NULL )
  {
    item =  new QListViewItem( tree, lastRootItem );
    item->setText( 0, aName );
  }
  else
    item = new QListViewItem( tree, aName );

  item->setPixmap(0, *(getIcon( iconType )) );

  // Save this as the last entry.
  lastRootItem = item;
  lastItem = item;

  return item;
}

/*---------------------------------- CClassTreeHandler::addItem()
 * addItem()
 *   Add an item to the view with the selected parent and icon.
 *
 * Parameters:
 *   aName        Name of the item.
 *   iconType     The icontype.
 *   parent       The parent item.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
QListViewItem *CClassTreeHandler::addItem( const char *aName, 
                                           CTHType iconType,
                                           QListViewItem *parent )
{
  assert( aName != NULL );
  assert( parent != NULL );

  QListViewItem *item = new QListViewItem( parent, lastItem );
  item->setText( 0, aName );
  item->setPixmap( 0, *(getIcon( iconType )) );

  // Save this as the last entry.
  lastItem = item;

  return item;
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

  return addItem( aClass->name, CVCLASS, parent );
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

  return addItem( aName, CVCLASS, parent );
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

  CTHType type = PUBLIC_METHOD;
  QString str;

  if( aMethod->isProtected() )
    type = PROTECTED_METHOD;
  else if( aMethod->isPrivate() )
    type = PRIVATE_METHOD;
  
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

  CTHType type = PUBLIC_ATTR;
  
  if( aAttr->isProtected() )
    type = PROTECTED_ATTR;
  else if( aAttr->isPrivate() )
    type = PRIVATE_ATTR;
    
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
  addItem( str, CVGLOBAL_FUNCTION, parent );
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

  addItem( aAttr->name
, CVGLOBAL_FUNCTION, parent );
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
    addItem( str, STRUCT, parent );
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
    addItem( str, STRUCT, parent );
  }
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CClassTreeHandler::readIcons()
 * readIcons()
 *   Read the icons from disk and store them in the class.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::readIcons()
{
  QString PIXPREFIX = "/kdevelop/pics/mini/";
  QString projIcon = "folder.xpm";
  QString pixDir;
  KIconLoader *il;

  // Allocate the array.
  icons = new QPixmap *[ END_POS ];
  for( int i=0; i<END_POS; i++ )
    icons[ i ] = NULL;

  pixDir = KApplication::kde_datadir() + PIXPREFIX;

  il = KApplication::getKApplication()->getIconLoader();

  // Load the icons
  icons[ PROJECT ] = new QPixmap( il->loadMiniIcon( "folder.xpm" ) );
  icons[ CVCLASS ] = new QPixmap(pixDir + "CVclass.xpm");
  icons[ STRUCT ] = new QPixmap(pixDir + "CVstruct.xpm");
  icons[ PUBLIC_ATTR ] = new QPixmap(pixDir + "CVpublic_var.xpm");
  icons[ PROTECTED_ATTR ] = new QPixmap(pixDir + "CVprotected_var.xpm");
  icons[ PRIVATE_ATTR ] = new QPixmap(pixDir + "CVprivate_var.xpm");
  icons[ CVGLOBAL_VARIABLE ] = new QPixmap( pixDir + "CVglobal_var.xpm");
  icons[ PUBLIC_METHOD ] = new QPixmap(pixDir + "CVpublic_meth.xpm");
  icons[ PROTECTED_METHOD ] = new QPixmap(pixDir + "CVprotected_meth.xpm");
  icons[ PRIVATE_METHOD ] = new QPixmap(pixDir + "CVprivate_meth.xpm");
  icons[ CVGLOBAL_FUNCTION ] = new QPixmap( pixDir + "CVglobal_meth.xpm");
}
