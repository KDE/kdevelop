/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLASSTOOLDLG_H_
#define _CLASSTOOLDLG_H_

#include <qlist.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include "classtoolwidget.h"
#include "parseditem.h"

class ClassView;
class ClassStore;
class ParsedClass;

/** This dialog gives the user the possibility to view classhierarchies,
 * inherited methods and attributes. The user can also choose to filter
 * based on export.
 * @author Jonas Nordin
 */
class ClassToolDialog : public QWidget
{
  Q_OBJECT

public: // Constructor & Destructor

    enum Operations { ViewParents, ViewChildren, ViewClients, ViewSuppliers,
                      ViewMethods, ViewAttributes, ViewNone };
    
    ClassToolDialog( ClassView *part );
    ~ClassToolDialog();

public: // Public methods to set attribute values

  /** Set the class to view. */
  void setClassName(const QString &name);

public slots:
  /** Set the store to use to search for classes. */
  void setClassStore( ClassStore *aStore );

  void setLanguageSupport( KDevLanguageSupport *ls );

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

protected slots:
  void refresh();
  void slotExportComboChoice(const QString &str);
  void slotClassComboChoice(const QString &str);
  
private: // Private attribues

  ClassToolWidget classTree;
  QLabel classLbl;
  QComboBox classCombo;
  
  QVBoxLayout topLayout;
  QGridLayout comboLayout;
  QHBoxLayout btnLayout;

  QToolButton parentsBtn;
  QToolButton childrenBtn;
  QToolButton clientsBtn;
  QToolButton suppliersBtn;

  QLabel filler;
  QToolButton methodsBtn;
  QToolButton attributesBtn;
  QComboBox exportCombo;

  /** Store that holds all classes in the system. */
  ClassStore *m_store;
  KDevLanguageSupport *m_ls;
  ClassView *m_part;

  /** The class we are currently viewing. */
  ParsedClass *currentClass;

  /** The current exportstatus selected in the combo. */
  PIExport comboExport;

  /** Stores what operation the user selected last. */
  Operations currentOperation;

private: // Private methods

  /** Set all initial values of all widgets in the dialog. */
  void setWidgetValues();
  /** Set all signal<->slot mappings. */
  void setCallbacks();
  /** Read and set the icons of all the buttons in the dialog. */
  void readIcons();
  /** Set tooltip strings. */
  void setTooltips();
  /** Change the caption depending on the current operation. */
  void updateCaptionAndButtons();
  void buildTree();
};

#endif
