/***************************************************************************
                          cclonefunctiondlg.h  -  description
                             -------------------
    begin                : Sat Nov 18 2000
    copyright            : (C) 2000 by The KDevelop Team
                           (c) 2000-2001 August Hörandl
    email                : august.hoerandl@gmx.at
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CCLONEFUNCTIONDLG_H
#define CCLONEFUNCTIONDLG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>

class ClassStore;
class ParsedMethod;
class ParsedClass;

/**clone an existing function/method
  *@author The KDevelop Team
  */

class CCloneFunctionDlg : public QDialog  {
   Q_OBJECT
public: 
  CCloneFunctionDlg( ClassStore * store, ClassStore * libstore,
		     const QString& currentClass, QWidget *parent=0, const char *name=0);
  /** get the selected method */
  bool getMethod(QString& type, QString& name, QString& decl, QString& comment,
                  bool& ispriv, bool& isprot, bool& ispub, bool& isvirt, bool& isSlot, bool& isSignal, bool& isconst);
private:
  ParsedMethod* searchMethod(ParsedClass* theClass, QString selected);
  /** fill the comboboxes with all classes */
  void initClasses();

protected slots:
  void OK();

protected: // Private widgets


  /** Layouts */
  QGridLayout* Form1Layout;
  QVBoxLayout* LayoutAll;
  QHBoxLayout* LayoutButton;

  /** listboxes */
  QComboBox* allclasses;
  QComboBox* methods;

  /** Button groups */
  QPushButton* okBtn;
  QPushButton* cancelBtn;

  /** pointer to all the classes */
  const QString& classname;
  ClassStore* classtree;
  ClassStore* libclasstree;

private slots: // Private slots
  /** update methods */
  void slotNewClass(const QString& name);
};

#endif
