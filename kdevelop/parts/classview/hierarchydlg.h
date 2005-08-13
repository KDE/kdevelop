/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _HIERARCHYDLG_H_
#define _HIERARCHYDLG_H_

#include <qdialog.h>

#include "codemodel.h"

class ClassViewPart;
class KDevLanguageSupport;
class DigraphView;
class ClassToolWidget;
class KComboView;
class QListViewItem;

class HierarchyDialog : public QDialog
{
    Q_OBJECT

public:
    HierarchyDialog( ClassViewPart *part );
    ~HierarchyDialog();

private slots:
    void setLanguageSupport(KDevLanguageSupport *ls);
    void slotClassComboChoice(QListViewItem *item);
    void slotNamespaceComboChoice(QListViewItem *item);
    void classSelected(const QString &className);

private:
    void refresh();
    void processNamespace(QString prefix, NamespaceDom dom);
    void processClass(QString prefix, ClassDom dom);

    QMap<QString, ClassDom> classes;
    //QMap<class name, fully qualified class name>
    //like MyClass - MyNamespace.MyClass
    QMap<QString, QString> uclasses;

    KComboView *namespace_combo;
    KComboView *class_combo;
    DigraphView *digraph;
//    ClassToolWidget *member_tree;
    ClassViewPart *m_part;
};

#endif
