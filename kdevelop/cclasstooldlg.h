/***************************************************************************
               cclasstooldlg.h  -  description
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
#ifndef CCLASSTOOLDLG_H
#define CCLASSTOOLDLG_H

#include <qdialog.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>


#include "./sourceinfo/classstore.h"
#include "cclasstreehandler.h"
#include "cclasstooltreeview.h"

typedef enum _CTOperations
{
  CTPARENT, CTCHILD, CTCLIENT, CTSUPP, CTATTR, CTMETH, CTVIRT, CTSLOTS, CTSIGNALS, CTNONE
}CTDOperations;

/** This dialog gives the user the possibility to view classhierarchies,
 * inherited methods and attributes. The user can also choose to filter
 * based on export.
 * @author Jonas Nordin
 */
class CClassToolDlg : public QWidget
{
  Q_OBJECT
public: // Constructor & Destructor

  CClassToolDlg( QWidget *parent=0, const char *name=0 );

public: // Public attributes

public: // Public methods to set attribute values

  /** Set the store to use to search for classes. */
  void setStore( ClassStore *aStore );

//  /** Shares current class. */
//  ParsedClass *asClass() { return currentClass; }
//  /** Shares current store. */
//  ClassStore* asStore() { return store; }
  /** Set the class to view. */
  void setClass( const char *aName );
  void setClass( ParsedClass *aClass );

  /** View the parents of the current class. */
  void viewParents();

  /** View the children of the current class. */
  void viewChildren();

  /** View all classes that has this class as an attribute. */
  void viewClients();

  /** View all classes that this class has as attributes. */
  void viewSuppliers();

  /** View methods in this class and parents. */
  void viewMethods();

  /** View attributes in this class and parents. */
  void viewAttributes();

  /** View virtual methods in this class and parents. */
  void viewVirtuals();

  void viewSlots();
  void viewSignals();
  QString classToString();

public: // Public widgets

  /** The actual view. */
  CClassToolTreeView classTree;

protected: // Private widgets

  QLabel classLbl;
  QComboBox classCombo;

  QVBoxLayout topLayout;
  QGridLayout comboLayout;
  QHBoxLayout btnLayout;

  QPushButton parentsBtn;
  QPushButton childrenBtn;
  QPushButton clientsBtn;
  QPushButton suppliersBtn;

  QLabel filler;
  QPushButton attributesBtn;
  //QPushButton methodsBtn;
  QToolButton methodsBtn;
  //  QPushButton virtualsBtn;
  QComboBox exportCombo;
  virtual void resizeEvent ( QResizeEvent* );
protected slots:

  void slotParents();
  void slotChildren();
  void slotClients();
  void slotSuppliers();
  void slotAttributes();
  void slotMethods();
  void slotSlots();
  void slotSignals();
  void slotVirtuals();
  void slotExportComboChoice(int idx);
  void slotClassComboChoice(int idx);

  /** Slot from the classtree telling that the user wants to view a
   * declation. */
  void slotCTViewDecl( const char *className, const char *declName, THType type,THType );

  /** Slot from the classtree telling that the user wants to view a
   * defintion. */
  void slotCTViewDef( const char *className, const char *declName, THType type,THType );
  void OK();

signals: // Signals

  /** This signal is emitted when a user wants to view a declaration. */
  void signalViewDeclaration( const char *, const char *, THType,THType );

  /** This signal is emitted when a user wants to view a definition. */
  void signalViewDefinition( const char *, const char *, THType,THType );
  /** This signal is emitted when a user as choosed an other class */
  void signalClassChanged( ParsedClass*  );

private: // Private attribues

 /** Store that holds all classes in the system. */
 ClassStore *store;

 /** The class we are currently viewing. */
 ParsedClass *currentClass;

 /** The current exportstatus selected in the combo. */
 CTHFilter comboExport;

 /** Tells if we only should view virtual methods. */
 bool onlyVirtual;

 /** Stores what operation the user selected last. */
 CTDOperations currentOperation;

private: // Private methods

  /** Set all initial values of all widgets in the dialog. */
  void setWidgetValues();
  /** Set all signal<->slot mappings. */
  void setCallbacks();
  /** Read and set the icons of all the buttons in the dialog. */
  void readIcons();
  /** Set tooltip strings. */
  void setTooltips();
  /** Make the supplied class the selected one in the classcombo. */
  void setActiveClass( const char *aName );

  /**
   * Add a list of classes as subitems to the current class.
   *
   * @param list List of classes to add.
   */
  void addClasses( QList<ParsedClass> *list );
  void addClassAndAttributes( ParsedClass *aClass );
  void addClassAndMethods( ParsedClass *aClass );
  void addClassAndSlots( ParsedClass *aClass );
  void addClassAndSignals( ParsedClass *aClass );
  void addAllClassMethods();
  void addAllClassSlots();
  void addAllClassSignals();
  void addAllClassAttributes();

  /** Change the caption depending on the current operation. */
  void changeCaption();
};

#endif


