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

#include <qlineedit.h>
#include <qlistview.h>

#include "domutil.h"
#include "addenvvardlg.h"
#include "customprojectpart.h"
#include "runoptionswidget.h"


RunOptionsWidget::RunOptionsWidget(CustomProjectPart *part, QWidget *parent, const char *name)
    : RunOptionsWidgetBase(parent, name)
{
    m_part = part;

    QDomDocument dom = *m_part->projectDom();

    mainprogram_edit->setText(DomUtil::readEntry(dom, "/kdevcustomproject/run/mainprogram"));
    progargs_edit->setText(DomUtil::readEntry(dom, "/kdevcustomproject/run/programargs"));

    DomUtil::PairList list = DomUtil::readPairListEntry(dom, "/kdevcustomproject/envvars", "envvar",
                                                        "name", "value");
    
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
    QDomDocument dom = *m_part->projectDom();

    DomUtil::writeEntry(dom, "/kdevcustomproject/run/mainprogram", mainprogram_edit->text());
    DomUtil::writeEntry(dom, "/kdevcustomproject/run/programargs", progargs_edit->text());

    DomUtil::PairList list;
    QListViewItem *item = listview->firstChild();
    while (item) {
        list << DomUtil::Pair(item->text(0), item->text(1));
        item = item->nextSibling();
    }

    DomUtil::writePairListEntry(dom, "/kdevautoproject/envvars", "envvar",
                                "name", "value", list);
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
