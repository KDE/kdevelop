/***************************************************************************
                          creehandler.h  -  implementation
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

#include "ctreehandler.h"
#include <assert.h>
#include <kiconloader.h>

QPixmap **CTreeHandler::icons = new QPixmap *[ THEND_POS ];
bool CTreeHandler::iconsRead = false;


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CTreeHandler::CTreeHandler()
 * CTreeHandler()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CTreeHandler::CTreeHandler()
{
  lastItem = NULL;
  lastRootItem = NULL;

  if( !CTreeHandler::iconsRead )
    readIcons();
}

/*-------------------------------------- CTreeHandler::~CTreeHandler()
 * ~CTreeHandler()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CTreeHandler::~CTreeHandler()
{
}

/*********************************************************************
 *                                                                   *
 *                    METHODS TO SET ATTRIBUTE VALUES                *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------- CTreeHandler::setTree()
 * setTree()
 *   Set the tree to draw in.
 *
 * Parameters:
 *   aTree          The tree.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CTreeHandler::setTree( QListView *aTree )
{
  assert( aTree != NULL );

  tree = aTree;
}

/*--------------------------------------- CTreeHandler::setLastItem()
 * setLastItem()
 *   Set the last added item(==item to add next item below).
 *
 * Parameters:
 *   aItem          The item to add.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CTreeHandler::setLastItem( QListViewItem *aItem )
{ 
  assert( aItem != NULL );

  lastItem = aItem;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC QUERIES                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- CTreeHandler::getIcon()
 * getIcon()
 *   Get an icon from the handler.
 *
 * Parameters:
 *   anIcon         The icon to fetch
 *
 * Returns:
 *   QPixmap        The icon.
 *-----------------------------------------------------------------*/
QPixmap *CTreeHandler::getIcon( THType anIcon )
{
  return icons[ anIcon ];
}

/*-------------------------------------------- CTreeHandler::itemType()
 * itemType()
 *   Returns the type of the currently selected item
 *
 * Parameters:
 *   -
 *
 * Returns:
 *   THType    The selected items' type.
 *-----------------------------------------------------------------*/
THType CTreeHandler::itemType()
{
  return itemType( tree->currentItem() );
}

/*-------------------------------------------- CTreeHandler::itemType()
 * itemType()
 *   Return the type of the item.
 *
 * Parameters:
 *   -
 *
 * Returns:
 *   THType    The selected items' type.
 *-----------------------------------------------------------------*/
THType CTreeHandler::itemType( QListViewItem *item )
{
  int idx;
  const QPixmap *p;
  
  p = item->pixmap( 0 );
  for( idx=0; 
       idx < THEND_POS && p->serialNumber() != icons[ idx ]->serialNumber();
       idx++ )
    ;

  return (THType)idx;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------------- CTreeHandler::clear()
 * clear()
 *   Clear the view and reset internal state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CTreeHandler::clear()
{
  tree->clear();
  lastRootItem=NULL;
  lastItem=NULL;
}

/*-------------------------------------------- CTreeHandler::addRoot()
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
QListViewItem *CTreeHandler::addRoot( const char *aName, 
                                      THType iconType )
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
  setLastItem( item );

  return item;
}

/*-------------------------------------------- CTreeHandler::addItem()
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
QListViewItem *CTreeHandler::addItem( const char *aName, 
                                      THType iconType,
                                      QListViewItem *parent,
																			const char* label2=0, const char* label3=0,
																			const char* label4=0, const char* label5=0 )
{
  assert( aName != NULL );
  assert( parent != NULL );

  QListViewItem *item = new QListViewItem( parent, lastItem, aName, label2, label3, label4, label5 );
  //item->setText( 0, aName );
  item->setPixmap( 0, *(getIcon( iconType )) );

  // Save this as the last entry.
  setLastItem( item );

  return item;
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
void CTreeHandler::readIcons()
{
  QString PIXPREFIX = "/kdevelop/pics/mini/";
  QString pixDir;
  KIconLoader *il;

  // Allocate the array.
  icons = new QPixmap *[ THEND_POS ];
  for( int i=0; i<THEND_POS; i++ )
    icons[ i ] = NULL;

  pixDir = KApplication::kde_datadir() + PIXPREFIX;

  il = KApplication::getKApplication()->getIconLoader();

  // Load the icons
  icons[ THFOLDER ] = new QPixmap( il->loadMiniIcon( "folder.xpm" ) );
  icons[ THFOLDER_OPEN ] = new QPixmap( il->loadMiniIcon( "folder_open.xpm" ) );
  icons[ THCLASS ] = new QPixmap(pixDir + "CVclass.xpm");
  icons[ THPROJECT ] = new QPixmap( il->loadMiniIcon( "kwm.xpm" ) );
  icons[ THBOOK ] = new QPixmap( il->loadMiniIcon( "mini-book1.xpm" ) );
  icons[ THBOOK_OPEN ] = new QPixmap( il->loadMiniIcon( "mini-book2.xpm" ) );
  icons[ THDOC ] = new QPixmap( il->loadMiniIcon( "mini-doc.xpm") );
  icons[ THSTRUCT ] = new QPixmap(pixDir + "CVstruct.xpm");
  icons[ THSCOPE ] = new QPixmap( pixDir + "CVnamespace.xpm" );
  icons[ THPUBLIC_ATTR ] = new QPixmap(pixDir + "CVpublic_var.xpm");
  icons[ THPROTECTED_ATTR ] = new QPixmap(pixDir + "CVprotected_var.xpm");
  icons[ THPRIVATE_ATTR ] = new QPixmap(pixDir + "CVprivate_var.xpm");
  icons[ THPUBLIC_SLOT ] = new QPixmap(pixDir + "CVpublic_slot.xpm");
  icons[ THPROTECTED_SLOT ] = new QPixmap(pixDir + "CVprotected_slot.xpm");
  icons[ THPRIVATE_SLOT ] = new QPixmap(pixDir + "CVprivate_slot.xpm");
  icons[ THSIGNAL ] = new QPixmap(pixDir + "CVpublic_signal.xpm");
  icons[ THGLOBAL_VARIABLE ] = new QPixmap( pixDir + "CVglobal_var.xpm");
  icons[ THPUBLIC_METHOD ] = new QPixmap(pixDir + "CVpublic_meth.xpm");
  icons[ THPROTECTED_METHOD ] = new QPixmap(pixDir + "CVprotected_meth.xpm");
  icons[ THPRIVATE_METHOD ] = new QPixmap(pixDir + "CVprivate_meth.xpm");
  icons[ THGLOBAL_FUNCTION ] = new QPixmap( pixDir + "CVglobal_meth.xpm");
  icons[ THC_FILE ] = new QPixmap( il->loadMiniIcon( "c_src.xpm" ) );
  icons[ THINSTALLED_FILE ] = new QPixmap( il->loadMiniIcon( "inst_file.xpm" ) );
  icons[ THDELETE ] = new QPixmap( il->loadMiniIcon( "delete.xpm" ) );

  CTreeHandler::iconsRead = true;
}


