/***************************************************************************
               cclasstooldlg.h  -  description

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

#ifndef CCLASSTOOLDLG_H
#define CCLASSTOOLDLG_H

#include <qdialog.h>
#include <ktreelist.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include "ClassStore.h"

enum{ CTPARENT, CTCHILD, CTCLIENT, CTSUPP, CTATTR, CTMETH, CTVIRT, CTNONE };

class CClassToolDlg : public QDialog
{
  Q_OBJECT
public: // Constructor & Destructor

  CClassToolDlg( QWidget *parent=0, const char *name=0 );

public: // Public methods to set attribute values

  /** Set the store to use to search for classes. */
  void setStore( CClassStore *aStore );

  /** Set the class to view. */
  void setClass( const char *aName );
  void setClass( CParsedClass *aClass );

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

protected: // Private widgets

 QLabel classLbl;
 QComboBox classCombo;

 QPushButton parentsBtn;
 QPushButton childrenBtn;
 QPushButton clientsBtn;
 QPushButton suppliersBtn;

 QPushButton attributesBtn;
 QPushButton methodsBtn;
 QPushButton virtualsBtn;
 QComboBox exportCombo;
 KTreeList classTree;

 QPixmap *classPm;

 protected slots:

  void slotParents();
  void slotChildren();
  void slotClients();
  void slotSuppliers();
  void slotAttributes();
  void slotMethods();
  void slotVirtuals();
  void slotExportComboChoice(int idx);
  void slotClassComboChoice(int idx);
  void OK();

private: // Private attribues

 /** Store that holds all classes in the system. */
 CClassStore *store;

 /** The class we are currently viewing. */
 CParsedClass *currentClass;

 /** The current exportstatus selected in the combo. */
 int export;

 /** Tells if we only should view virtual methods. */
 bool onlyVirtual;

 /** Stores what operation the user selected last. */
 int currentOperation;

private: // Private methods

  void setWidgetValues();
  void setCallbacks();
  void readIcons();
  void setTooltips();
  void setActiveClass( const char *aName );

  void addClassAndAttributes( CParsedClass *aClass );
  void addClassAndMethods( CParsedClass *aClass );
  void addAllClassMethods();
  void addRoot( KPath &classPath );

  /** Change the caption depending on the current operation. */
  void changeCaption();
};

#endif

