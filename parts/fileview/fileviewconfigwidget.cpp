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

#include <qdom.h>
#include <qlistview.h>

#include "fileviewpart.h"
#include "fileviewconfigwidget.h"


FileViewConfigWidget::FileViewConfigWidget(FileViewPart *part,
                                           QWidget *parent, const char *name)
    : FileViewConfigWidgetBase(parent, name)
{
    m_part = part;

    readConfig();
}


FileViewConfigWidget::~FileViewConfigWidget()
{}


void FileViewConfigWidget::readConfig()
{
    QDomDocument projectDom = *m_part->document();
    QDomElement docEl = projectDom.documentElement();
    QDomElement fileviewEl = docEl.namedItem("kdevfileview").toElement();
    QDomElement groupsEl = fileviewEl.namedItem("groups").toElement();

    QDomElement groupEl = groupsEl.firstChild().toElement();
    while (!groupEl.isNull()) {
        if (groupEl.tagName() == "group")
            new QListViewItem(listview, groupEl.attribute("name"), groupEl.attribute("pattern"));
        groupEl = groupEl.nextSibling().toElement();
    }
}


void FileViewConfigWidget::storeConfig()
{
    QDomDocument projectDom = *m_part->document();
    QDomElement docEl = projectDom.documentElement();
    QDomElement fileviewEl = docEl.namedItem("kdevfileview").toElement();
    QDomElement groupsEl = fileviewEl.namedItem("groups").toElement();

    // Clear old entries
    while (!groupsEl.firstChild().isNull())
        groupsEl.removeChild(groupsEl.firstChild());

    QListViewItem *item = listview->firstChild();
    while (item) {
            QDomElement groupEl = projectDom.createElement("group");
            groupEl.setAttribute("name", item->text(0));
            groupEl.setAttribute("pattern", item->text(1));
            groupsEl.appendChild(groupEl);
    }
}


void FileViewConfigWidget::addGroup()
{
}


void FileViewConfigWidget::removeGroup()
{
    if (listview->currentItem())
        delete listview->currentItem();
}


void FileViewConfigWidget::accept()
{
    storeConfig();
    m_part->refresh();
}

#include "fileviewconfigwidget.moc"
