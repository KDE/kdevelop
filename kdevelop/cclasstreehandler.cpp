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
void CClassTreeHandler::setTree( KTreeList *aTree )
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

/*---------------------------------- CClassTreeHandler::updateClass()
 * updateClass()
 *   Update the class in the view using the supplied path.
 *
 * Parameters:
 *   aClass       Class to update.
 *   aPath        The path of the class.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::updateClass( CParsedClass *aClass, KPath *aPath )
{
  KTreeListItem *top;
  KTreeListItem *current;
  KTreeListItem *next;
  
  top = tree->itemAt( aPath );
  current = top->getChild();
  
  while( current != NULL )
  {
    next = current->getSibling();
    top->removeChild( current );
    current = next;
  }

  // Add parts of the class
  addMethodsFromClass( aClass, *aPath, CTHALL );
  addAttributesFromClass( aClass, *aPath, CTHALL );
  addSlots( aClass, *aPath );
  addSignals( aClass, *aPath );
}

/*---------------------------------- CClassTreeHandler::addClass()
 * addClass()
 *   Add a class to the view. 
 *
 * Parameters:
 *   aClass       Class to add.
 *   aPath        The path of the class.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addClass( CParsedClass *aClass, KPath &path )
{
  assert( aClass != NULL );

  tree->addChildItem( aClass->name, icons[ CVCLASS ], &path );
}

/*-------------------------- CClassTreeHandler::addMethodsFromClass()
 * addMethodsFromClass()
 *   Add a the selected methods from the class to the view. 
 *
 * Parameters:
 *   aClass       Class with methods to add.
 *   aPath        The path of the class.
 *   filter       The selection.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addMethodsFromClass( CParsedClass *aClass, KPath &path,
                                             CTHFilter filter )
{
  addMethods( aClass->getSortedMethodList(), path, filter );
}

/*------------------------------------ CClassTreeHandler::addMethods()
 * addMethods()
 *   Add all methods from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *   filter          The selection.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addMethods( QList<CParsedMethod> *list, 
                                    KPath &path, CTHFilter filter )
{
  assert( list != NULL );

  CParsedMethod *aMethod;

  // Add the methods
  for( aMethod = list->first();
       aMethod != NULL;
       aMethod = list->next() )
  {
    if( filter == CTHALL || filter == aMethod->export )
      addMethod( aMethod, path );
  }
}

/*------------------------------------ CClassTreeHandler::addMethod()
 * addMethod()
 *   Add a method to the view.
 *
 * Parameters:
 *   aMethod         Method to add.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addMethod( CParsedMethod *aMethod, KPath &path )
{
  assert( aMethod );

  QPixmap *icon;
  QString str;

  if( aMethod->isPublic() )
    icon = icons[ PUBLIC_METHOD ];
  else if( aMethod->isProtected() )
    icon = icons[ PROTECTED_METHOD ];
  else if( aMethod->isPrivate() )
    icon = icons[ PRIVATE_METHOD ];
  else // Global
    icon = icons[ PUBLIC_METHOD ];
  
  aMethod->toString( str );
  tree->addChildItem( str, icon, &path );
}

/*-------------------------- CClassTreeHandler::addAttributesFromClass()
 * addAttributesFromClass()
 *   Add a the selected methods from the class to the view. 
 *
 * Parameters:
 *   aClass       Class with methods to add.
 *   aPath        The path of the class.
 *   filter       The selection.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addAttributesFromClass( CParsedClass *aClass, 
                                                KPath &path,
                                                CTHFilter filter )
{
  addAttributes( aClass->getSortedAttributeList(), path, filter );
}

/*--------------------------------- CClassTreeHandler::addAttributes()
 * addAttributes()
 *   Add all attributes from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addAttributes( QList<CParsedAttribute> *list,
                                       KPath &path, CTHFilter filter )
{
  CParsedAttribute *aAttr;

  // Add the methods
  for( aAttr = list->first();
       aAttr != NULL;
       aAttr = list->next() )
  {
    if( filter == CTHALL || aAttr->export == filter )
      addAttribute( aAttr, path );
  }
}

/*------------------------------------ CClassTreeHandler::addAttribute()
 * addAttribute()
 *   Add an attribute to the view.
 *
 * Parameters:
 *   aMethod         Method to add
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addAttribute( CParsedAttribute *aAttr, KPath &path )
{
  assert( aAttr != NULL );

  QPixmap *icon;
  
  if( aAttr->isPublic() )
    icon = icons[ PUBLIC_ATTR ];
  else if( aAttr->isProtected() )
    icon = icons[ PROTECTED_ATTR ];
  else if( aAttr->isPrivate() )
    icon = icons[ PRIVATE_ATTR ];
  else // Global
    icon = icons[ PUBLIC_ATTR ];
    
  tree->addChildItem( aAttr->name, icon, &path );
}

/*------------------------------------- CClassTreeHandler::addSlots()
 * addSlots()
 *   Add all slots from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addSlots( CParsedClass *aPC, KPath &path )
{
  CParsedMethod *aMethod;

  // Add the methods
  for( aMethod = aPC->slotList.first();
       aMethod != NULL;
       aMethod = aPC->slotList.next() )
    tree->addChildItem( aMethod->name, icons[ STRUCT ], &path );
}

/*----------------------------------- CClassTreeHandler::addSignals()
 * addSignals()
 *   Add all signals from a class to the view.
 *
 * Parameters:
 *   aPC             Class that holds the data.
 *   path            Current path in the view.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassTreeHandler::addSignals( CParsedClass *aPC, KPath &path )
{
  CParsedMethod *aMethod;

  // Add the methods
  for( aMethod = aPC->signalList.first();
       aMethod != NULL;
       aMethod = aPC->signalList.next() )
    tree->addChildItem( aMethod->name, icons[ STRUCT ], &path );
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
  debug( "Fetching pixmaps from: %s", pixDir.data() );

  il = KApplication::getKApplication()->getIconLoader();

  // Load the icons
  icons[ PROJECT ] = new QPixmap( il->loadMiniIcon( "folder.xpm" ) );
  icons[ CVCLASS ] = new QPixmap(pixDir + "CVclass.xpm");
  icons[ STRUCT ] = new QPixmap(pixDir + "CVstruct.xpm");
  icons[ PUBLIC_ATTR ] = new QPixmap(pixDir + "CVpublic_var.xpm");
  icons[ PROTECTED_ATTR ] = new QPixmap(pixDir + "CVprotected_var.xpm");
  icons[ PRIVATE_ATTR ] = new QPixmap(pixDir + "CVprivate_var.xpm");
  icons[ PUBLIC_METHOD ] = new QPixmap(pixDir + "CVpublic_meth.xpm");
  icons[ PROTECTED_METHOD ] = new QPixmap(pixDir + "CVprotected_meth.xpm");
  icons[ PRIVATE_METHOD ] = new QPixmap(pixDir + "CVprivate_meth.xpm");
}
