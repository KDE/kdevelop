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
  THFOLDER, THCLASS, THPROJECT, THBOOK,
  THGLOBAL_FUNCTION, THGLOBAL_VARIABLE,
  THPUBLIC_METHOD, THPROTECTED_METHOD, THPRIVATE_METHOD, 
  THPUBLIC_ATTR, THPROTECTED_ATTR, THPRIVATE_ATTR,
  THSTRUCT, THC_FILE, THINSTALLED_FILE, THEND_POS
} THType;

class CTreeHandler
{
public: // Constructor & destructor

  CTreeHandler();
  ~CTreeHandler();

public: // Public methods to set attribute values.

  /** Set the tree to draw in. */
  void setTree( QListView *aTree );

  /** Set the last added item. */
  void setLastItem( QListViewItem *aItem ) { lastItem = aItem; }

public: // Public methods

  /** Clear the view and reset internal state. */
  void clear();

  /** Add an item at root level. */
  QListViewItem *addRoot( const char *aName, THType iconType );

  /** Add an item with the selected icon. */
  QListViewItem *addItem( const char *aName, THType iconType,
                          QListViewItem *parent );

public: // Public queries

  /** Return the selected pixmap. */
  QPixmap *getIcon( THType anIcon );

  /** Return the type of the current icon. */
  THType itemType();


private: // Private attributes

  /** The tree to draw in. */
  QListView *tree;

  /** The last item added to the tree. */
  QListViewItem *lastItem;

  /** The last rootitem added to the tree. */
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
