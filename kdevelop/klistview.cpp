/***************************************************************************
                          klistview.cpp  -  Smart list view 
                             -------------------
    begin                : Sat Jun 26 1999
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 *   Optionally, you can choose to distribute this file under the          *
 *   conditions of the GNU Library General Public License.                 *
 *                                                                         *
 ***************************************************************************/


#include "klistview.h"


KListView::KListView(QWidget *parent, const char *name)
    : QListView(parent, name), last(0)
{}


KListView::~KListView()
{}


QListViewItem *KListView::lastChild() const
{
    QListViewItem *item = firstChild();
    if (!item)
      return 0;    
    while (item->nextSibling())
      item = item->nextSibling();
    return item;
}


KListViewItem::KListViewItem(KListView *parent, const char *text, const char *id)
    : QListViewItem(parent, parent->lastChild(), text), last(0), idnt(id)
{}


KListViewItem::KListViewItem(KListViewItem *parent, const char *text, const char *id)
    : QListViewItem(parent, parent->lastChild(), text), last(0), idnt(id)
{}


KListViewItem::~KListViewItem()
{}


void KListViewItem::insertItem(QListViewItem *item)
{
    last = item;
    QListViewItem::insertItem(item);
}


KCheckListItem::KCheckListItem(QListView *parent, const char *text, const char *id)
    : QCheckListItem(parent, text, QCheckListItem::CheckBox), idnt(id)
{}
