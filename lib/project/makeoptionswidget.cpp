/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makeoptionswidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlistview.h>
#include "domutil.h"
#include "addenvvardlg.h"


void MakeOptionsWidget::addVarClicked()
{
    AddEnvvarDialog dlg;
    if (!dlg.exec())
        return;

    (void) new QListViewItem(listview, dlg.varname(), dlg.value());
}


void MakeOptionsWidget::removeVarClicked()
{
    delete listview->currentItem();
}


MakeOptionsWidget::MakeOptionsWidget(QDomDocument &dom, const QString &configGroup,
                                   QWidget *parent, const char *name)
    : MakeOptionsWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    abort_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/make/abortonerror"));
    jobs_box->setValue(DomUtil::readIntEntry(dom, configGroup + "/make/numberofjobs"));
    dontact_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/make/dontact"));
    makebin_edit->setText(DomUtil::readEntry(dom, configGroup + "/make/makebin"));
    
	DomUtil::PairList list =
        DomUtil::readPairListEntry(dom, configGroup + "/makeenvvars", "envvar", "name", "value");
    
    QListViewItem *lastItem = 0;

    DomUtil::PairList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QListViewItem *newItem = new QListViewItem(listview, (*it).first, (*it).second);
        if (lastItem)
            newItem->moveItem(lastItem);
        lastItem = newItem;
    }
}


MakeOptionsWidget::~MakeOptionsWidget()
{}


void MakeOptionsWidget::accept()
{
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/make/abortonerror", abort_box->isChecked());
    DomUtil::writeIntEntry(m_dom, m_configGroup + "/make/numberofjobs", jobs_box->value());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/make/dontact", dontact_box->isChecked());
    DomUtil::writeEntry(m_dom, m_configGroup + "/make/makebin", makebin_edit->text());

    DomUtil::PairList list;
    QListViewItem *item = listview->firstChild();
    while (item) {
        list << DomUtil::Pair(item->text(0), item->text(1));
        item = item->nextSibling();
    }

    DomUtil::writePairListEntry(m_dom, m_configGroup + "/makeenvvars", "envvar", "name", "value", list);
}

#include "makeoptionswidget.moc"
