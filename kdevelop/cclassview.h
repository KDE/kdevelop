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
#include <qpoint.h> 
#include <qpixmap.h> 
#include <qlistview.h>
#include <kpopmenu.h>
#include "./classparser/ClassParser.h"
#include "cproject.h"
#include "cclasstreehandler.h"

/** Tree-like classview for kdevelop utilizing the classparser lib.
  * @author Jonas Nordin
  */

class CClassView : public QListView
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

  /** Tells which(if any) of the buttons where pressed. */
  int mouseBtn;

  /** The classtore */
  CClassStore *store;

public: // Public queries

  /** Return the type of the selected item. */
  int indexType();

public: // Public refreshmethods
  
  /** Refresh the whole view using the project. */
  void refresh( CProject *proj );

  /** Refresh the whole view. */
  void refresh();

  /** Refresh a class by using its' name. */
  void refreshClassByName( const char *aName );

protected: // Protected signals and slots
  protected slots:
    void slotRightButtonPressed(QListViewItem *,const QPoint &,int);
    void slotProjectOptions();
    void slotFileNew();
    void slotClassNew();
    void slotClassDelete();
    void slotMethodNew();
    void slotMethodDelete();
    void slotAttributeNew();
    void slotAttributeDelete();
    void slotFolderNew();
    void slotClassBaseClasses();
    void slotClassDerivedClasses();
    void slotClassTool();
    void slotViewDefinition();
    void slotViewDeclaration();

  signals:
    void selectedFileNew();
    void selectedClassNew();
    void selectedProjectOptions();
    void selectedViewDeclaration();
    void selectedViewDefinition();
    void signalAddMethod( CParsedMethod *aMethod );
    void signalAddAttribute( CParsedAttribute *aAttribute );

private: // Private constants

private: // Popupmenus

  /** Popupmenu for the toplevel icon. */
  KPopupMenu projectPopup;

  /** Popupmenu for classes. */
  KPopupMenu classPopup;

  /** Popupmenu for methods. */
  KPopupMenu methodPopup;

  /** Popupmenu for attributes. */
  KPopupMenu attributePopup;

private: // Private attributes

  /** The position at the last mousepress-event. */
  QPoint mousePos;

  /** The classparser. */	
  CClassParser cp;

  /** The utilityclass to draw the tree. */
  CClassTreeHandler treeH;

private: // Private methods

  /** Event to be executed on a mousepress. */
  void mousePressEvent(QMouseEvent* event);

  /** Fetches the currently selected class from the store. */
  CParsedClass *getCurrentClass();

  /** Initialize popupmenus. */
  void initPopups();
  
};

#endif
