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

#include <qcombobox.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <kdialog.h>
#include <klocale.h>

#include "kdevlanguagesupport.h"
#include "classviewpart.h"
#include "classstore.h"
#include "classtoolwidget.h"
#include "digraphview.h"
#include "hierarchydlg.h"


HierarchyDialog::HierarchyDialog( ClassViewPart *part )
    : QDialog(0, "hierarchy dialog", WDestructiveClose)
{
    class_combo = new QComboBox(false, this);

    QPushButton *close_button = new QPushButton(i18n("&Close"), this);

    QSplitter *splitter = new QSplitter(Vertical, this);
    digraph = new DigraphView(splitter, "digraph view");
    member_tree = new ClassToolWidget(part, splitter);

    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    QBoxLayout *combo_layout = new QHBoxLayout();
    layout->addLayout(combo_layout);
    combo_layout->addWidget(class_combo);
    combo_layout->addStretch();
    combo_layout->addWidget(close_button);
    layout->addWidget(splitter);

    connect( class_combo, SIGNAL(activated(const QString&)),
             this, SLOT(slotClassComboChoice(const QString&)) );
    connect( close_button, SIGNAL(clicked()),
             this, SLOT(reject()) );
    connect( digraph, SIGNAL(selected(const QString&)),
             this, SLOT(classSelected(const QString&)) );

    m_part = part;
    m_part->registerHierarchyDialog(this);
    refresh();
}


HierarchyDialog::~HierarchyDialog()
{
    m_part->unregisterHierarchyDialog(this);
}


void HierarchyDialog::refresh()
{
    class_combo->clear();
    digraph->clear();

    QList<ParsedClass> *list = m_part->classStore()->getSortedClassList();
    QListIterator<ParsedClass> it(*list);
    for (; it.current(); ++it) {
        class_combo->insertItem(it.current()->name);
        QListIterator<ParsedParent> it2(it.current()->parents);
        for (; it2.current(); ++it2)
            digraph->addEdge(it2.current()->name, it.current()->name);
    }
    delete list;

    digraph->process();
}


void HierarchyDialog::setLanguageSupport(KDevLanguageSupport *ls)
{
    if (ls)
        connect(ls, SIGNAL(updatedSourceInfo()), this, SLOT(refresh()));
    else
        refresh();
}


void HierarchyDialog::slotClassComboChoice(const QString &name)
{
    QString className = class_combo->currentText();
    digraph->setSelected(className);
    digraph->ensureVisible(className);
    classSelected(name);
}


void HierarchyDialog::classSelected(const QString &name)
{
    ParsedClass *currentClass = m_part->classStore()->getClassByName(name);
    member_tree->clear();
    if (currentClass) {
        member_tree->insertAllClassMethods(currentClass, (PIAccess)-1);
        member_tree->insertAllClassAttributes(currentClass, (PIAccess)-1);
    }
}
#include "hierarchydlg.moc"
