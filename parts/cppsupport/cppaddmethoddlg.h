/***************************************************************************
                          addclassmethoddialog.h  -  description
                             -------------------
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
    copyright            : (C) 2001 by August Hörandl
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

#ifndef _CPPADDMETHODDIALOG_H_
#define _CPPADDMETHODDIALOG_H_

#include "cppaddmethoddlgbase.h"

class ParsedMethod;
class ClassStore;

/**
 * Adds a new method to a class.
 */
class CppAddMethodDialog : public CppAddMethodDialogBase  {
    Q_OBJECT
public:
    CppAddMethodDialog(ClassStore *store, const QString &className,
                       QWidget *parent, const char *name);
    ~CppAddMethodDialog();

    ParsedMethod *asSystemObj();

public slots:
    /**
     * This slot is executed when the user clicks on the type rb.
     * It toggles the modifiers on/off.
     */
    void slotToggleModifier();
    
    /**
     * This slot is executed when the user click the virtual button.
     * It toggles the pure button on/off.
     */ 
    void slotVirtualClicked(); 
    
    void accept();
protected slots: // Protected slots
  /** add new Paramter */
  void slotNewPara();
  /** delete Paramter */
  void slotDelPara();
  /** move Paramter up*/
  void slotUpPara();
  /** move Paramter down */
  void slotDownPara();
private: // Private methods
  /** return documentation string */
  QString getDocu();
  /** return name + parameters */
  QString getDecl();
private slots: // Private slots
  /** Debug only - show Message boxes
with values */
  void slotDebug();
  /** clone a function */
  void slotClone();
  /** select a different Parameter */
  void slotParaHighLight( QListBoxItem * );
  /** update current parameter in listbox */
  void slotUpdateParameter(const QString&);
private: // Private attributes
  /** pointer to list: all classes */
  ClassStore * store;
  /** name of currently selected class */
  const QString& currentClass;
  /** - prevent loops via signal/slot calls
    * - if a widget is changed and emits a signal the signal handler shouldn't change
    * the widget (learned that the hard way ;-)
    */
  bool editactive;
};

#endif
