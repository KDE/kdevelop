/***************************************************************************
                          creehandler.h  -  description
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

#ifndef _CTREEHANDLER_H_INCLUDED
#define _CTREEHANDLER_H_INCLUDED

#include <kapp.h>
#include <qlistview.h>
#include <qpixmap.h> 
#include <qlist.h>

typedef enum
{ 
  THFOLDER, THFOLDER_OPEN, THCLASS, THPROJECT, THBOOK, THBOOK_OPEN, THDOC,
  THGLOBAL_FUNCTION, THGLOBAL_VARIABLE,
  THPUBLIC_METHOD, THPROTECTED_METHOD, THPRIVATE_METHOD, 
  THPUBLIC_ATTR, THPROTECTED_ATTR, THPRIVATE_ATTR,
  THPUBLIC_SLOT, THPROTECTED_SLOT, THPRIVATE_SLOT,
  THSIGNAL,
  THSTRUCT, THC_FILE, THINSTALLED_FILE, THDELETE, THEND_POS
} THType;

/** Generic class to handle the troblesome process of adding items to
 * a tree. 
 * This class also has methods to check the type of the currently
 * selected item. The type-checking is based upon the unique id 
 * every pixmap gets when it is loaded.
 * @author Jonas Nordin
 */
class CTreeHandler
{
public: // Constructor & destructor

  CTreeHandler();
  ~CTreeHandler();

public: // Public methods to set attribute values.

  /** Set the tree to draw in. 
   * @param aTree The list view to make operations on.
   */
  void setTree( QListView *aTree );

  /** Set the last added item. 
   * @param aItem The last item added to the view.
   */
  void setLastItem( QListViewItem *aItem );

public: // Public methods

  /** Clear the view and reset internal state. */
  void clear();

  /** Add an item at root level. 
   * @param aName The name to show in the view.
   * @param iconType What kind of icon should be shown.
   */
  QListViewItem *addRoot( const char *aName, THType iconType );

  /** Add an item with the selected icon. 
   * @param aName The name to show in the view.
   * @param iconType What kind of icon should be shown.
   * @param parent Which parent should this item be added to.
   */
  QListViewItem *addItem( const char *aName, THType iconType,
                          QListViewItem *parent,
													const char* =0, const char* =0,
													const char* =0, const char* =0 );

public: // Public queries

  /** Return the selected pixmap. 
   * @param anIcon Type for which we want to fetch the icon.
   * @return Pointer to the wanted icon.
   */
  QPixmap *getIcon( THType anIcon );

  /** Return the type of the current icon. 
   * @return The type of the currently selected item.
   */
  THType itemType();

  /** Return the type of the item. 
   * @param item Item to check the type on.
   * @return The type of a supplied item.
   */
  THType itemType( QListViewItem *item );

protected: // Protected attributes

  /** The tree to draw in. */
  QListView *tree;

private: // Private attributes

  /** The last item added to the tree. */
  QListViewItem *lastItem;

  /** The last root-item added to the tree. */
  QListViewItem *lastRootItem;

  /** Array containing all pixmaps used */
  static QPixmap **icons;

  /** Tells if the icons has been read or not. */
  static bool iconsRead;

private: // Private methods

  /** Initalize the icon array and read the icons from disk. */
  void readIcons();
};

#endif
