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

#include "./classparser/ClassParser.h"
#include "ctreehandler.h"

typedef enum
{
  CTHALL = -1, 
  CTHPUBLIC = PUBLIC, 
  CTHPROTECTED = PROTECTED, 
  CTHPRIVATE = PRIVATE
} CTHFilter;

class CClassTreeHandler : public CTreeHandler
{
public: // Constructor & destructor

  CClassTreeHandler();
  ~CClassTreeHandler();

public: // Public methods to set attribute values.

  /** Set the classtore. */
  void setStore( CClassStore *aStore );

public: // Public methods

  /** Update methods/attr... of a certain class. */
  void updateClass( CParsedClass *aClass, QListViewItem *parent );

  /** Add a list of classes to the view. */
  void addClasses( QList<CParsedClass> *list, QListViewItem *parent );

  /** Add a class to the view. */
  QListViewItem *addClass( CParsedClass *aClass, QListViewItem *parent );
  QListViewItem *addClass( const char *aClass, QListViewItem *parent );

  /** Add all subclasses from the class. */
  void addSubclassesFromClass( CParsedClass *aClass,
                               QListViewItem *parent );

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

  /** Add a list of global variables to the view. */
  void addGlobalVariables( QList<CParsedAttribute> *list, QListViewItem *parent );

  /** Add a global function to the view. */
  void addGlobalFunc( CParsedMethod *aMethod, QListViewItem *parent );

  /** Add a global variable to the view. */
  void addGlobalVar( CParsedAttribute *aAttr, QListViewItem *parent );

  /** Add a list of global structures to the view. */
  void addGlobalStructs( QList<CParsedStruct> *list,
                         QListViewItem *parent );

  /** Add a structure to the view. */
  void addStruct( CParsedStruct *aStruct,
                  QListViewItem *parent );

  /** Add all signals to the view. */
  void addSlotsFromClass( CParsedClass *aPC, QListViewItem *parent );

  /** Add all signals to the view. */
  void addSignalsFromClass( CParsedClass *aPC, QListViewItem *parent );

private: // Private attributes

  /** The classtore. */
  CClassStore *store;

};

#endif 
