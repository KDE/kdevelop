/***************************************************************************
                          cclassview.h  -  description
                             -------------------
    begin                : Fri Mar 19 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef _CCLASSVIEW_H
#define _CCLASSVIEW_H

#include <kapp.h>
#include <qstring.h>
#include <qlist.h>
#include <qtooltip.h>

#include "ctreeview.h"
#include "./classparser/ClassParser.h"
#include "cclasstreehandler.h"

class CClassToolDlg;
class QStrList;

/** Tree-like classview for kdevelop utilizing the classparser lib.
  * @author Jonas Nordin
  */
class CClassView : public CTreeView
{
  Q_OBJECT

public: // Constructor & Destructor

  /** The constructor. Us this exactly as you would with QListView. */
  CClassView( QWidget* parent = 0,const char* name = 0 );

  /** Destructor. */
  ~CClassView();

private: // Private classes

  /** Class that handles dynamic tooltips in the CV. */
  class CCVToolTip : public QToolTip
  {
  public:
    CCVToolTip(QWidget *parent);
  protected:
    void maybeTip( const QPoint & );
  };

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

  /** Refresh the view regarding a certain file.
   * @param iHeaderList the header files to reparse.
   * @param iSourceList the source files to reparse.
   */
  void refresh( QStrList * iHeaderList, QStrList * iSourceList );


  /** Refresh the whole view. */
  void refresh();

  /** Add a source file, parse it and update the tree. */
  void addFile( const char *aName );

  /** Refresh a class by using its' name. */
  void refreshClassByName( const char *aName );

public: // Public methods

  /** View graphical classtree. */
  void viewGraphicalTree();

  /** Check and get a tooltip for a point. */
  void tip( const QPoint &p, QRect &r, QString &str );

signals:
  void setStatusbarProgressSteps(int);
  void setStatusbarProgress(int);
  void resetStatusbarProgress();

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
  /** Views a class definition. */
  void slotViewClassDefinition(CParsedClass *);
  /** Views the definition. */
  void slotViewDefinition( const char *parentPath, const char *itemName, 
                           THType parentType, THType itemType );
  
  /** Views the declaration. */
  void slotViewDeclaration( const char *parentPath, const char *itemName, 
                           THType parentType, THType itemType );
  void slotClassWizard();

signals:
  void selectedFileNew();
  void selectedClassNew();
  void selectedProjectOptions();
  void selectedViewDeclaration(const char *, const char *,THType,THType);
  void selectedViewDefinition(const char *, const char *,THType,THType);
  void selectFile(const QString &, int );

  /** Emitted when a user wants to add an attribute.
   * @param aClass Class to add an attribute to.
   */
  void signalAddMethod( const char * );

  /** Emitted when a user wants to add an attribute.
   * @param aClass Class to add an attribute to.
   */
  void signalAddAttribute( const char * );

  /** Emitted when the user wants to delete a method
   * @param aClass Name of the class that has the method.
   * @param aMethodName A string holding the name and parameters of the method.
   */
  void signalMethodDelete( const char *, const char * );
  void popupHighlighted(int);

protected: // Implementations of virtual methods.

  /** Initialize popupmenus. */
  void initPopups();

  /** Get the current popupmenu. 
   * @return Pointer to the current popupmenu or NULL if none applicable.
   */
  KPopupMenu *getCurrentPopup();

private: // Popupmenus

  /** Popupmenu for the toplevel icon. */
  KPopupMenu projectPopup;

  /** Popupmenu for classes. */
  KPopupMenu classPopup;

  /** Popupmenu for structures. */
  KPopupMenu structPopup;

  /** Popupmenu for methods. */
  KPopupMenu methodPopup;

  /** Popupmenu for attributes. */
  KPopupMenu attributePopup;

  /** Popupmenu for slots. */
  KPopupMenu slotPopup;

  /** Popupmenu for signals. */
  KPopupMenu signalPopup;

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

  /** The dynamic tooltip:er. */
  CCVToolTip * toolTip;

private: // Private methods

  /** Create a new ClassTool dialog and setup its' attributes.
   * @return A newly allocated classtool dialog.
   */
  CClassToolDlg *createCTDlg();

  /** Fetch one node from a tree string.
   * @param str        String containing the tree.
   * @param pos        Current position.
   * @param buf        Resulting string.
   * @return The new position.
   */
  int getTreeStrItem( const char *str, int pos, char *buf );

  /** Make the classtree from a treestring.
   * @param str The string representing the classtree.
   */
  void buildTree( const char *treeStr );

  /** Build the classtree without using a treestring. */
  void buildInitalClassTree();

  /** Make a string of the tree. The string is represented as
   * you would expect a lisp-person to write it. An example
   * would be <code>('foo' 1 ('bar' 0))</code> which would result
   * in the folder foo(opened) containing the class bar which is
   * closed.
   * @param item Root item
   * @param str Resulting treestring.
   */
  void buildTreeStr( QListViewItem *item, QString &str );

  /** Fetches the currently selected class from the store. */
  CParsedClass *getCurrentClass();

  /** 
   * Return this view as a treestring. 
   * 
   * @param str String to return the tree in.
   */
  void asTreeStr( QString &str );

  bool validClassDecl( const char *className, const char *declName, THType type );
};

#endif

