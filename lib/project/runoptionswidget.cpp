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

#include "runoptionswidget.h"

#include <klocale.h>

#include <qlineedit.h>
#include <qlistview.h>
#include <qfiledialog.h>

#include "domutil.h"
#include "addenvvardlg.h"



RunOptionsWidget::RunOptionsWidget(QDomDocument &dom, const QString &configGroup,
                                   QWidget *parent, const char *name)
    : RunOptionsWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    mainprogram_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/mainprogram"));
    progargs_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/programargs"));

    DomUtil::PairList list =
        DomUtil::readPairListEntry(dom, configGroup + "/envvars", "envvar", "name", "value");
    
    QListViewItem *lastItem = 0;

    DomUtil::PairList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QListViewItem *newItem = new QListViewItem(listview, (*it).first, (*it).second);
        if (lastItem)
            newItem->moveItem(lastItem);
        lastItem = newItem;
    }
}


RunOptionsWidget::~RunOptionsWidget()
{}


void RunOptionsWidget::accept()
{
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/mainprogram", mainprogram_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/programargs", progargs_edit->text());

    DomUtil::PairList list;
    QListViewItem *item = listview->firstChild();
    while (item) {
        list << DomUtil::Pair(item->text(0), item->text(1));
        item = item->nextSibling();
    }

    DomUtil::writePairListEntry(m_dom, m_configGroup + "/envvars", "envvar", "name", "value", list);
}


void RunOptionsWidget::addVarClicked()
{
    AddEnvvarDialog dlg;
    if (!dlg.exec())
        return;

    (void) new QListViewItem(listview, dlg.varname(), dlg.value());
}


void RunOptionsWidget::removeVarClicked()
{
    delete listview->currentItem();
}


#include "runoptionswidget.moc"
