/***************************************************************************
                          cclassview.h  -  description
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


#ifndef CCLASSVIEW_H
#define CCLASSVIEW_H

#include <kapp.h>
#include <qstring.h>
#include <qlist.h>
#include "ctreeview.h"
#include "./classparser/ClassParser.h"
#include "cclasstreehandler.h"
#include "cctviewdefinitioncmd.h"
#include "cctviewdeclarationcmd.h"

/** Tree-like classview for kdevelop utilizing the classparser lib.
  * @author Jonas Nordin
  */
class CClassView : public CTreeView
{
  Q_OBJECT

public: // Constructor & Destructor

  CClassView( QWidget* parent = 0,const char* name = 0 );
  ~CClassView();

public: // Public constants

  /** Name of the class-root. */
  static QString CLASSROOTNAME;

  /** Name of the root for globals. */
  static QString GLOBALROOTNAME;

public: // Public attributes

  /** The classtore */
  CClassStore *store;

public: // Public refreshmethods
  
  /** Refresh the whole view using the project. */
  void refresh( CProject *proj );

  /** Refresh the whole view. */
  void refresh();

  /** Add a source file, parse it and update the tree. */
  void addFile( const char *aName );

  /** Refresh a class by using its' name. */
  void refreshClassByName( const char *aName );

public: // Public methods

  /** Views the definition. */
  void viewDefinition( const char *className, const char *declName, THType type );

  /** Views the declaration. */
  void viewDeclaration( const char *className, const char *declName, THType type );


signals:
  void setStatusbarProgressSteps(int);
  void setStatusbarProgress(int);
  void resetStatusbarProgress();
protected: // Protected signals and slots
  protected slots:
    void slotProjectOptions();
    void slotGraphicalView();
    void slotFileNew();
    void slotClassNew();
    void slotClassDelete();
    void slotClassViewSelected();
    void slotMethodNew();
    void slotMethodDelete();
    void slotAttributeNew();
    void slotAttributeDelete();
    void slotImplementVirtual();
    void slotAddSlotSignal();
    void slotFolderNew();
    void slotFolderDelete();
    void slotClassBaseClasses();
    void slotClassDerivedClasses();
    void slotClassTool();
    void slotViewDefinition();
    void slotViewDeclaration();
    void slotMoveToFolder();
  signals:
    void selectedFileNew();
    void selectedClassNew();
    void selectedProjectOptions();
    void selectedViewDeclaration(const char *, const char *,THType);
    void selectedViewDefinition(const char *, const char *,THType);
    void signalAddMethod( CParsedMethod * );
    void signalAddAttribute( CParsedAttribute * );

protected: // Implementations of virtual methods.

  /** Initialize popupmenus. */
  void initPopups();

  /** Get the current popupmenu. */
  KPopupMenu *getCurrentPopup();

private: // Popupmenus

  /** Popupmenu for the toplevel icon. */
  KPopupMenu projectPopup;

  /** Popupmenu for classes. */
  KPopupMenu classPopup;

  /** Popupmenu for methods. */
  KPopupMenu methodPopup;

  /** Popupmenu for attributes. */
  KPopupMenu attributePopup;

  /** Popupmenu for slots. */
  KPopupMenu slotPopup;

  /** Popupmenu for folders. */
  KPopupMenu folderPopup;

private: // Private attributes

  /** The classparser. */	
  CClassParser cp;

  /** The class item. */
  QListViewItem *classesItem;

  /** The globals item. */
  QListViewItem *globalsItem;

  /** The project. */
  CProject *project;

  /** Command used by the classtool to view definitions. */
  CCTViewDefinitionCmd definitionCmd;

  /** Command used by the classtool to view declarations. */
  CCTViewDeclarationCmd declarationCmd;

private: // Private methods

  int getTreeStrItem( const char *str, int pos, char *buf );
  void buildTree( const char *treeStr );
  void buildInitalClassTree();

  /** Make a string of the tree. */
  void buildTreeStr( QListViewItem *item, QString &str );

  /** Fetches the currently selected class from the store. */
  CParsedClass *getCurrentClass();

  /** Return this view as a treestring. */
  const char *asTreeStr();
};

#endif
