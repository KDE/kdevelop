/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
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

#include <qdialog.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include "classtoolwidget.h"
#include "parseditem.h"

class ClassViewPart;
class ClassStore;
class ParsedClass;


class ClassToolDialog : public QWidget
{
    Q_OBJECT

public:
    enum Operations { ViewParents, ViewChildren, ViewClients, ViewSuppliers,
                      ViewMethods, ViewAttributes, ViewNone };
    
    ClassToolDialog( ClassViewPart *part );
    ~ClassToolDialog();

    void setClassName(const QString &name);

public slots:

    /** View nothing. */
    void viewNone();
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

private slots:
    void setLanguageSupport(KDevLanguageSupport *ls);
    void refresh();

    void delayedClose();
    void slotClose();
    void slotAccessComboChoice(const QString &str);
    void slotClassComboChoice(const QString &str);


private:
    void updateCaptionAndButtons();
    void buildTree();

    ClassToolWidget *class_tree;
    QComboBox *class_combo;
    
    QToolButton *parents_button;
    QToolButton *children_button;
    QToolButton *clients_button;
    QToolButton *suppliers_button;
    QToolButton *methods_button;
    QToolButton *attributes_button;
    QComboBox *access_combo;
    
    /** Store that holds all classes in the system. */
    ClassStore *m_store;
    KDevLanguageSupport *m_ls;
    ClassViewPart *m_part;
    
    /** The class we are currently viewing. */
    ParsedClass *currentClass;
    /** The current exportstatus selected in the combo. */
    PIAccess comboAccess;
    /** Stores what operation the user selected last. */
    Operations currentOperation;
};

#endif
