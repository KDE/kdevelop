/***************************************************************************
                          cclasstreehandler.h  -  description
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

#ifndef _CCLASSTREEHANDLER_H_INCLUDED
#define _CCLASSTREEHANDLER_H_INCLUDED

#include <kapp.h>
#include <qlistview.h>
#include "./classparser/ClassParser.h"
#include <qpixmap.h> 
#include <qlist.h>

typedef enum
{ 
  PROJECT, CVCLASS, METHOD, ATTRIBUTE, 
  CVGLOBAL_FUNCTION, CVGLOBAL_VARIABLE,
  PUBLIC_METHOD, PROTECTED_METHOD, PRIVATE_METHOD, 
  PUBLIC_ATTR, PROTECTED_ATTR, PRIVATE_ATTR,
  STRUCT, END_POS
} CTHType;

typedef enum
{
  CTHALL = -1, 
  CTHPUBLIC = PUBLIC, 
  CTHPROTECTED = PROTECTED, 
  CTHPRIVATE = PRIVATE
} CTHFilter;

class CClassTreeHandler
{
public: // Constructor & destructor

  CClassTreeHandler();
  ~CClassTreeHandler();

public: // Public methods to set attribute values.

  /** Set the tree to draw in. */
  void setTree( QListView *aTree );

  /** Set the classtore. */
  void setStore( CClassStore *aStore );

  /** Set the last added item. */
  void setLastItem( QListViewItem *aItem ) { lastItem = aItem; }

public: // Public methods

  /** Clear the view and reset internal state. */
  void clear();

  /** Update methods/attr... of a certain class. */
  void updateClass( CParsedClass *aClass, QListViewItem *parent );

  /** Add an item at root level. */
  QListViewItem *addRoot( const char *aName, CTHType iconType );

  /** Add an item with the selected icon. */
  QListViewItem *addItem( const char *aName, CTHType iconType,
                          QListViewItem *parent );

  /** Add a class to the view. */
  QListViewItem *addClass( CParsedClass *aClass, QListViewItem *parent );
  QListViewItem *addClass( const char *aClass, QListViewItem *parent );

  /** Add the selected methods from the class. */
  void addMethodsFromClass( CParsedClass *aClass, QListViewItem *parent,
                            CTHFilter filter = CTHALL );

  /** Add all methods to the view. */
  void addMethods( QList<CParsedMethod> *list,  QListViewItem *parent,
                   CTHFilter filter = CTHALL );

  /** Add one method to the tree. */
  void addMethod( CParsedMethod *aMethod, QListViewItem *parent );

  /** Add the selected attributes from the class. */
  void addAttributesFromClass( CParsedClass *aClass, 
                               QListViewItem *parent,
                               CTHFilter filter = CTHALL );

  /** Add all signals to the view. */
  void addAttributes( QList<CParsedAttribute> *list, QListViewItem *parent,
                      CTHFilter filter = CTHALL );

  /** Add one attribute to the view. */
  void addAttribute( CParsedAttribute *aAttr, QListViewItem *parent );

  /** Add a list of global functions to the view. */
  void addGlobalFunctions( QList<CParsedMethod> *list, QListViewItem *parent );

  /** Add a global function to the view. */
  void addGlobalFunc( CParsedMethod *aMethod, QListViewItem *parent );

  /** Add a global variable to the view. */
  void addGlobalVar( CParsedAttribute *aAttr, QListViewItem *parent );

  /** Add all signals to the view. */
  void addSlots( CParsedClass *aPC, QListViewItem *parent );

  /** Add all signals to the view. */
  void addSignals( CParsedClass *aPC, QListViewItem *parent );

public: // Public queries

  QPixmap *getIcon( CTHType anIcon );

private: // Private attributes

  /** The tree to draw in. */
  QListView *tree;

  /** The last item added to the tree. */
  QListViewItem *lastItem;

  /** The last rootitem added to the tree. */
  QListViewItem *lastRootItem;

  /** The classtore. */
  CClassStore *store;

  /** Array containing all pixmaps used */
  QPixmap **icons;

private: // Private methods

  /** Initalize the icon array and read the icons from disk. */
  void readIcons();

};

#endif 
