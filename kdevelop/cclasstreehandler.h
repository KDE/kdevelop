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
#include <ktreelist.h>
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
  void setTree( KTreeList *aTree );

  /** Set the classtore. */
  void setStore( CClassStore *aStore );

public: // Public methods

  /** Update methods/attr... of a certain class. */
  void updateClass( CParsedClass *aClass, KPath *aPath );

  /** Add a class to the view. */
  void addClass( CParsedClass *aClass, KPath &path );

  /** Add the selected methods from the class. */
  void addMethodsFromClass( CParsedClass *aClass, KPath &path,
                            CTHFilter filter = CTHALL );

  /** Add all methods to the view. */
  void addMethods( QList<CParsedMethod> *list,  KPath &path,
                   CTHFilter filter = CTHALL );

  /** Add one method to the tree. */
  void addMethod( CParsedMethod *aMethod, KPath &path );

  /** Add the selected attributes from the class. */
  void addAttributesFromClass( CParsedClass *aClass, 
                               KPath &path,
                               CTHFilter filter = CTHALL );

  /** Add all signals to the view. */
  void addAttributes( QList<CParsedAttribute> *list, KPath &path,
                      CTHFilter filter = CTHALL );

  /** Add one attribute to the view. */
  void addAttribute( CParsedAttribute *aAttr, KPath &path );

  /** Add all signals to the view. */
  void addSlots( CParsedClass *aPC, KPath &path );

  /** Add all signals to the view. */
  void addSignals( CParsedClass *aPC, KPath &path );

public: // Public queries

  QPixmap *getIcon( CTHType anIcon );

private: // Private attributes

  /** The tree to draw in. */
  KTreeList *tree;

  /** The classtore. */
  CClassStore *store;

  /** Array containing all pixmaps used */
  QPixmap **icons;

private: // Private methods

  /** Initalize the icon array and read the icons from disk. */
  void readIcons();

};

#endif 
