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

#include <kiconloader.h>
#include <kstddirs.h>
#include <kdebug.h>

#include <qlist.h>
#include <qlistview.h>
#include <qpixmap.h>

#include <assert.h>

#include "ctreehandler.h"

QPixmap* CTreeHandler::icons[THEND_POS]; // = new QPixmap *[ THEND_POS ];
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

  if( !iconsRead )
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

  // hope that's correct, it prevents a code dump though (rokrau 6/29/01)
  if (!item)
    return THFOLDER;
  p = item->pixmap( 0 );
  if (!p)
    return THCLASS;

  for( idx=0;
       idx < THEND_POS && p->serialNumber() != icons[ idx ]->serialNumber();
       idx++ )
  {}
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
QListViewItem *CTreeHandler::addRoot( const QString &aName, 
                                      THType iconType )
{
  assert( !aName.isEmpty() );

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
QListViewItem *CTreeHandler::addItem( const QString & aName, 
                                      THType iconType,
                                      QListViewItem *parent,
                                      const QString & label2 /*=QString::null*/, const QString & label3 /*=QString::null*/,
                                      const QString & label4/*=QString::null*/, const QString & label5/*=QString::null*/ )
{
  assert( !aName.isEmpty() );
  assert( parent != NULL );

  QListViewItem* item = 0L;
  QListViewItem* originalParent = parent;

  if (iconType == THFOLDER) {
    QString str = aName;
    int pos = str.find('/');
    QListViewItem* oldC = 0L;
    while (!str.isEmpty()) {
      // each subfolder gets an own tree entry, create them step by step
      QString subfolderName;
      if (pos == -1) {
        subfolderName = str;
      }
      else {
        subfolderName = str.left(pos);
      }
      if (!subfolderName.isEmpty()) {
        // check if such subfolder already exists
        bool bExists = false;
        oldC = 0L;
        QListViewItem* c = parent->firstChild();
        while (c && !bExists) {
          oldC = c;
          QString curText = c->text(0);
          if (curText == subfolderName) {
            bExists = true; // ok, found; now also go to the last sibling of children of this
            parent = c;
            item = c;
          }
          else {
            c = c->nextSibling();
          }
        }
        // if no such subfolder exists, create it
        if (!bExists) {
          item = new QListViewItem( parent, subfolderName);
          item->setPixmap( 0, *(icons[THFOLDER]) );
          parent = item;
        }
        else if (subfolderName == str) {
          // we're in the last loop step, now go to the right sibling for setLastItem
          c = item->firstChild();
          while (c) {
            oldC = c;
            c = c->nextSibling();
          }
        }
      }
      if (pos == -1) {
        break;
      }
      str = str.right(str.length() - pos-1);
      pos = str.find('/');
    }
    // set the last item to the belowest sibling
    if (oldC) {
      setLastItem(oldC);
    }
    else if (item) {
      setLastItem(item);
    }
  }
  else {
    item = new QListViewItem( parent, lastItem, aName, label2, label3, label4, label5 );
    //item->setText( 0, aName );
    item->setPixmap( 0, *(getIcon( iconType )) );
    // Save this as the last entry.
    setLastItem( item );
  }

  return item;
}

//-----------------------------------------------------------------
QStringList CTreeHandler::pathListOfAllOpenedItems()
{
  QStringList list;
  if (!tree)
    return list;

  QListViewItem *pCurItem = tree->firstChild();
  appendOpenedItemsOfSubtreeToPathList( &list, pCurItem);
  list.sort();
  return list;
}

//-----------------------------------------------------------------
QString CTreeHandler::pathToSelectedItem()
{
  if (!tree) return "";
  QListViewItem *item = tree->currentItem(); // the currently selected one
  return pathToItem(item);
}

//-----------------------------------------------------------------
void CTreeHandler::openItems(QStringList pathList)
{
  for (QStringList::Iterator it = pathList.begin(); it != pathList.end(); ++it ) {
    QString curPath = *it;
    kdDebug() << "reopen: " << curPath << endl;
    goToItem( curPath, true, false);
  }
}

//-----------------------------------------------------------------
void CTreeHandler::activateItem(const char* path)
{
  goToItem( path, false, true);
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CClassTreeHandler::readIcons()
 * readIcons()
 *   Read the icons from dactivateItemisk and store them in the class.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CTreeHandler::readIcons()
{
  // Allocate the array.
  //  icons = new QPixmap *[ THEND_POS ];
  for( int i=0; i<THEND_POS; i++ )
    icons[ i ] = NULL;

  // Load the icons
  icons[ THFOLDER ]           = new QPixmap( SmallIcon( "folder"));
  icons[ THFOLDER_OPEN ]      = new QPixmap( SmallIcon( "folder_open"));
  icons[ THCLASS ]            = new QPixmap( SmallIcon( "CVclass"));
  icons[ THPROJECT ]          = new QPixmap( SmallIcon( "kdevelop"));
  icons[ THBOOK ]             = new QPixmap( SmallIcon( "contents2"));
  icons[ THBOOK_OPEN ]        = new QPixmap( SmallIcon( "contents"));
  icons[ THDOC ]              = new QPixmap( SmallIcon( "mini_doc"));
  icons[ THSTRUCT ]           = new QPixmap( SmallIcon( "CVstruct"));
  icons[ THSCOPE ]            = new QPixmap( SmallIcon( "CVnamespace"));
  icons[ THPUBLIC_ATTR ]      = new QPixmap( SmallIcon( "CVpublic_var"));
  icons[ THPROTECTED_ATTR ]   = new QPixmap( SmallIcon( "CVprotected_var"));
  icons[ THPRIVATE_ATTR ]     = new QPixmap( SmallIcon( "CVprivate_var"));
  icons[ THPUBLIC_SLOT ]      = new QPixmap( SmallIcon( "CVpublic_slot"));
  icons[ THPROTECTED_SLOT ]   = new QPixmap( SmallIcon( "CVprotected_slot"));
  icons[ THPRIVATE_SLOT ]     = new QPixmap( SmallIcon( "CVprivate_slot"));
  icons[ THSIGNAL ]           = new QPixmap( SmallIcon( "CVpublic_signal"));
  icons[ THGLOBAL_VARIABLE ]  = new QPixmap( SmallIcon( "CVglobal_var"));
  icons[ THPUBLIC_METHOD ]    = new QPixmap( SmallIcon( "CVpublic_meth"));
  icons[ THPROTECTED_METHOD ] = new QPixmap( SmallIcon( "CVprotected_meth"));
  icons[ THPRIVATE_METHOD ]   = new QPixmap( SmallIcon( "CVprivate_meth"));
  icons[ THGLOBAL_FUNCTION ]  = new QPixmap( SmallIcon( "CVglobal_meth"));
  icons[ THC_FILE ]           = new QPixmap( SmallIcon( "source"));
  icons[ THINSTALLED_FILE ]   = new QPixmap( SmallIcon( "inst_file"));
  icons[ THDELETE ]           = new QPixmap( SmallIcon( "editdelete"));

  iconsRead = true;
}

//-----------------------------------------------------------------
void CTreeHandler::appendOpenedItemsOfSubtreeToPathList(QStringList *pList, QListViewItem *pCurItem)
{
  while (pCurItem) {
    if (pCurItem->isOpen()) {
      pList->append(pathToItem(pCurItem));
      // recursive check of the children
      QListViewItem* pFirstChild = pCurItem->firstChild();
      appendOpenedItemsOfSubtreeToPathList( pList, pFirstChild);
    }
    pCurItem = pCurItem->nextSibling();
  }
}

//-----------------------------------------------------------------
QString CTreeHandler::pathToItem(QListViewItem* pItem)
{
  QString path;
  if (pItem) {
    path = pItem->text(0);
    pItem = pItem->parent();
  }
  while (pItem) {
    path = pItem->text(0) + "*" + path; // "*" is the separator
    pItem = pItem->parent();
  }
  return path;
}

//-----------------------------------------------------------------
void CTreeHandler::goToItem(const char* path, bool bAction_OpenTheItem, bool bAction_SelectTheItem)
{
  if (!tree) return;

  QString p = path;
  QString curItemText;
  QListViewItem *pCurItem = tree->firstChild();
  int pos = p.find("*");

  // go along the path and expand the appropriate tree branches
  while (pCurItem) {
    if (pos != -1) {
      curItemText = p.left(pos);
    }
    else {
      curItemText = p;
    }

    // try to find the right item in the current level
    bool bReachedEnd = false;
    QListViewItem* pCandidate = pCurItem;
    while (!bReachedEnd && (pCandidate->text(0) != curItemText)) {
      pCandidate = pCandidate->nextSibling();
      if (!pCandidate) {
        bReachedEnd = true;
      }
    }
    if (!bReachedEnd) {
      // has found this part of the path,
      // if possible and desired, expand the tree here.
      if (pCandidate->firstChild()) {
        if (bAction_OpenTheItem) {
          pCandidate->setOpen(true);
        }
      }
      // if desired and the path is completely processed, highlight the item
      if (bAction_SelectTheItem && (p == curItemText)) {
        tree->setSelected( pCandidate, true);
      }
      // prepare to enter the next subtree
      pCurItem = pCandidate->firstChild();

    }
    else {
      pCurItem = 0L;
    }
    p = p.right(p.length() - pos - 1);
    pos = p.find("*");
  }
}
