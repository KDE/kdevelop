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

#include "classstore.h"
#include "ctreehandler.h"

typedef enum
{
  CTHALL = -1,
  CTHPUBLIC = PIE_PUBLIC,
  CTHPROTECTED = PIE_PROTECTED,
  CTHPRIVATE = PIE_PRIVATE
} CTHFilter;

/** This is an subclass of the treehandler specifically made for
  * presenting tree-hierarchies of classes. */
class CClassTreeHandler : public CTreeHandler
{
public: // Constructor & destructor

  CClassTreeHandler();
  ~CClassTreeHandler();

public: // Public methods to set attribute values.

  /** Set the classtore. */
  void setStore( ClassStore *aStore );

public: // Public methods

  /**
   * Add a list of global namespaces to the view.
   *
   * @param list List of all global namespaces.
   * @param parent Parent item of all namespaces.
   */
  void addScopes( QList<ParsedScopeContainer> *list, QListViewItem *parent );

  /**
   * Add a scope to the the view.
   *
   * @param aScope Scope to add.
   * @param parent Parent to add the item to.
   */
  void addScope( ParsedScopeContainer *aScope, QListViewItem *parent );

  /** Update methods/attr... of a certain class. */
  void updateClass( ParsedClass *aClass, QListViewItem *parent );

  /** Add a list of classes to the view. */
  void addClasses( QList<ParsedClass> *list, QListViewItem *parent );

  /** Add a class to the view. */
  QListViewItem *addClass( ParsedClass *aClass, QListViewItem *parent );
  QListViewItem *addClass( const char *aClass, QListViewItem *parent );

  /** Add all subclasses from the class. */
  void addSubclassesFromClass( ParsedClass *aClass,
                               QListViewItem *parent );

  /**
   * Add all structures from a class.
   *
   * @param aClass Class that holds the structs.
   * @param parent Parent to add the structs to.
   * @param filter Which items to show.
   */
  void addStructsFromClass( ParsedClass *aClass,
                            QListViewItem *parent,
                            CTHFilter filter );

  /** Add a structure to the view. */
  void addStruct( ParsedStruct *aStruct,
                  QListViewItem *parent );

  /**
   * Add a list of structures to the view.
   *
   * @param list   List with structures.
   * @param parent Parent to add the structs to.
   * @param filter Tells which items to show.
   */
  void addStructs( QList<ParsedStruct> *list,
                   QListViewItem *parent,
                   CTHFilter filter );

  /** Add the selected methods from the class. */
  void addMethodsFromClass( ParsedClass *aClass, QListViewItem *parent,
                            CTHFilter filter = CTHALL );

  /** Add all methods to the view. */
  void addMethods( QList<ParsedMethod> *list,  QListViewItem *parent,
                   CTHFilter filter = CTHALL );

  /** Add one method to the tree. */
  void addMethod( ParsedMethod *aMethod, QListViewItem *parent );

  /** Add the selected attributes from the class. */
  void addAttributesFromClass( ParsedClass *aClass,
                               QListViewItem *parent,
                               CTHFilter filter = CTHALL );

  /** Add all signals to the view. */
  void addAttributes( QList<ParsedAttribute> *list, QListViewItem *parent,
                      CTHFilter filter = CTHALL );

  /** Add one attribute to the view. */
  void addAttribute( ParsedAttribute *aAttr, QListViewItem *parent );

  /** Add a list of global functions to the view. */
  void addGlobalFunctions( QList<ParsedMethod> *list, QListViewItem *parent );

  /** Add a list of global variables to the view. */
  void addGlobalVariables( QList<ParsedAttribute> *list, QListViewItem *parent );

  /** Add a global function to the view. */
  void addGlobalFunc( ParsedMethod *aMethod, QListViewItem *parent );

  /** Add a global variable to the view. */
  void addGlobalVar( ParsedAttribute *aAttr, QListViewItem *parent );

  /** Add a list of global structures to the view. */
  void addGlobalStructs( QList<ParsedStruct> *list,
                         QListViewItem *parent );

  /** Add all signals to the view. */
  void addSlotsFromClass( ParsedClass *aPC, QListViewItem *parent );

  /** Add all signals to the view. */
  void addSignalsFromClass( ParsedClass *aPC, QListViewItem *parent );

  /** Get the path and type of the currently selected declaration.
   * If the item is a container itemName and itemType should be ignored.
   *
   * @param parentPath Path of the container. Empty for globals.
   * @param itemName   Name of the selected item. Empty for containers.
   * @param parentType Type of container. Ignored for globals.
   * @param itemType   Type of the selected item. Ignored for containers.
   */
  void getCurrentNames( QString &parentPath,
                        QString &itemName,
                        THType &parentType,
                        THType &aItemType );

private: // Private attributes

  /** The classtore. */
  ClassStore *store;

  /** Class string */
  QString ccstr;

};

#endif
