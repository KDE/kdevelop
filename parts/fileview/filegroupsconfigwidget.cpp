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

#include "filegroupsconfigwidget.h"

#include <qlistview.h>
#include <knotifyclient.h>
#include <klocale.h>

#include "domutil.h"
#include "addfilegroupdlg.h"
#include "filegroupspart.h"


FileGroupsConfigWidget::FileGroupsConfigWidget(FileGroupsPart *part,
                                               QWidget *parent, const char *name)
    : FileGroupsConfigWidgetBase(parent, name)
{
    m_part = part;

    listview->setSorting(-1);
    
    readConfig();
}


FileGroupsConfigWidget::~FileGroupsConfigWidget()
{}


void FileGroupsConfigWidget::readConfig()
{
    QDomDocument &dom = *m_part->projectDom();
    DomUtil::PairList list = DomUtil::readPairListEntry(dom, "/kdevfileview/groups",
                                                        "group", "name", "pattern");

    QListViewItem *lastItem = 0;

    DomUtil::PairList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        QListViewItem *newItem = new QListViewItem(listview, (*it).first, (*it).second);
        if (lastItem)
            newItem->moveItem(lastItem);
        lastItem = newItem;
    }
}


void FileGroupsConfigWidget::storeConfig()
{
    DomUtil::PairList list;
    
    QListViewItem *item = listview->firstChild();
    while (item) {
        list << DomUtil::Pair(item->text(0), item->text(1));
        item = item->nextSibling();
    }

    DomUtil::writePairListEntry(*m_part->projectDom(), "/kdevfileview/groups",
                                "group", "name", "pattern", list);
}


void FileGroupsConfigWidget::addGroup()
{
    AddFileGroupDialog dlg;
    dlg.setCaption(i18n("Add File Group"));
    if (!dlg.exec())
        return;

    (void) new QListViewItem(listview, dlg.title(), dlg.pattern());
}


void FileGroupsConfigWidget::editGroup()
{
    if (listview->childCount()==0 || listview->currentItem()==0)
        return;
    AddFileGroupDialog dlg(listview->currentItem()->text(0),listview->currentItem()->text(1));
    dlg.setCaption(i18n("Edit File Group"));
    if (!dlg.exec() || dlg.title().isEmpty() || dlg.pattern().isEmpty())
        return;
    listview->currentItem()->setText(0,dlg.title());
    listview->currentItem()->setText(1,dlg.pattern());
}


void FileGroupsConfigWidget::removeGroup()
{
    delete listview->currentItem();
}


void FileGroupsConfigWidget::moveUp()
{
    if (listview->currentItem() == listview->firstChild()) {
        KNotifyClient::beep();
        return;
    }

    QListViewItem *item = listview->firstChild();
    while (item->nextSibling() != listview->currentItem())
        item = item->nextSibling();
    item->moveItem(listview->currentItem());
}


void FileGroupsConfigWidget::moveDown()
{
   if (listview->currentItem()->nextSibling() == 0) {
        KNotifyClient::beep();
        return;
   }

   listview->currentItem()->moveItem(listview->currentItem()->nextSibling());
}


void FileGroupsConfigWidget::accept()
{
    storeConfig();
    m_part->refresh();
}

#include "filegroupsconfigwidget.moc"
