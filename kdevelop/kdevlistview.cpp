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


#include "kdevlistview.h"


KDevListView::KDevListView(QWidget *parent, const char *name)
    : QListView(parent, name)
{}


KDevListView::~KDevListView()
{}


void KDevListView::insertItem(QListViewItem *item)
{
    QListView::insertItem(item);
}


void KDevListView::mousePressEvent(QMouseEvent * event)
{
    QListViewItem* item;
    item = itemAt(event->pos());
    if (isSelected(item) && event->button() != RightButton){
      emit selectionChanged (item);
    }
    
    QListView::mousePressEvent( event );
}

QListViewItem* KDevListView::lastChild() const
{
  QListViewItem* child = firstChild();
  if (child)
    while (QListViewItem* nextChild = child->nextSibling())
      child = nextChild;

  return child;
}


KDevListViewItem::KDevListViewItem(KDevListView *parent, const char *text, const char *id)
    : QListViewItem(parent, parent->lastChild(), text), idnt(id)
{}


KDevListViewItem::KDevListViewItem(KDevListViewItem *parent, const char *text, const char *id)
    : QListViewItem(parent, parent->lastChild(), text), idnt(id)
{}


KDevListViewItem::~KDevListViewItem()
{}


void KDevListViewItem::insertItem(QListViewItem *item)
{
    QListViewItem::insertItem(item);
}


QListViewItem* KDevListViewItem::lastChild() const
{
  QListViewItem* child = firstChild();
  if (child)
    while (QListViewItem* nextChild = child->nextSibling())
      child = nextChild;

  return child;
}
