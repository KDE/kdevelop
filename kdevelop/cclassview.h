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
#include <ktreelist.h>
#include <kpopmenu.h>
#include "ClassParser.h"
#include "cproject.h"

class CClassView; 

enum
{ 
  PROJECT, CVCLASS, METHOD, ATTRIBUTE, 
  CVGLOBAL_FUNCTION, CVGLOBAL_VARIABLE,
  PUBLIC_METHOD, PROTECTED_METHOD, PRIVATE_METHOD, 
  PUBLIC_ATTR, PROTECTED_ATTR, PRIVATE_ATTR,
  STRUCT, END_POS
};

/** Class that holds the data for a class in the view. */
class CCVClass
{
public: // Constructor & Destructor

  CCVClass()      {}
  ~CCVClass()     {}

public: // Public attributes.
  
  /** Menu ID. */
  int id;

  /** Name of the class. */
  QString name;

  /** Name of the .h file. */
  QString hFilename;

  /** Name of the .cpp file. */
  QString implFilename;

public: // Public methods to set attribute values

  /** Set the menu ID of this class. */
  void setId( int aId ) { id = aId; }

  /** Set the name of the class. */
  void setName( QString &aName ) { name = aName; }

  /** Set the .h filename. */
  void setHFilename( QString &aName ) { hFilename = aName; }

  /** Set the .cpp filename. */
  void setImplFilename( QString &aName ) { implFilename = aName; }
};


/** Tree-like classview for kdevelop utilizing the classparser lib.
  * @author Jonas Nordin
  */

class CClassView : public KTreeList
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

  /** Does the class exist in the view? */
  bool hasClass( QString &aName );

  /** Return the type of an object in the view. */
  int indexType( int aIdx );

public: // Public methods

  /** Add a class to the view. */
  void addClass( CCVClass *aClass );

public: // Public refreshmethods
  
  /** Refresh the whole view using the project. */
  void refresh( CProject *proj );

  /** Refresh the whole view. */
  void refresh();

  /** Refresh a class by using its' menu ID. */
  void refreshClassById( int aID );

  /** Refresh a class by using its' name. */
  void refreshClassByName( QString &aName );

protected: // Protected signals and slots
  protected slots:
    void slotSingleSelected(int index);
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
    void slotViewDefinition();
    void slotViewDeclaration();

  signals:
    void selectedFileNew();
    void selectedClassNew();
    void selectedProjectOptions();
    void selectedViewDeclaration(int index);
    void selectedViewDefinition(int index);

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

  /** Array containing all pixmaps used */
  QPixmap **icons;

  /** All classes in the view. */
  QList<CCVClass> classes;

  /** The classparser. */	
  CClassParser cp;

private: // Private methods

  /** Event to be executed on a mousepress. */
  void mousePressEvent(QMouseEvent* event);

  /** Initialize popupmenus. */
  void initPopups();
  
  /** Initalize the icon array and read the icons from disk. */
  void readIcons();

  /** Add all methods to the view. */
  void addMethods( QList<CParsedMethod> *list, KPath &path );

  /** Add all signals to the view. */
  void addAttributes( QDictIterator<CParsedAttribute> &iter, KPath &path );

  /** Add all signals to the view. */
  void addSlots( CParsedClass *aPC, KPath &path );

  /** Add all signals to the view. */
  void addSignals( CParsedClass *aPC, KPath &path );

};

#endif
