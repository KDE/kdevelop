/***************************************************************************
 *   Copyright (C) 2005 by Mathieu Chouinard                               *
 *   mchoui@e-c.qc.ca                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "annotationitem.h"
#include <qstring.h>

annotationItem::annotationItem(QListView *parent,QString name, QString text)
 : QListViewItem(parent, name)
{
  m_itemName=name;
  m_itemText=text;
  m_itemParent=-1;
  
}

annotationItem::annotationItem(QListViewItem *parent,QString name,QString text):QListViewItem( parent, name)
{
  m_itemName=name;
  m_itemText=text;
 // m_itemParent=(annotationItem*)parent->getParent();
}
    annotationItem::~annotationItem()
{
}

QString annotationItem::getName()
{
  return m_itemName;
}
QString annotationItem::getText()
{
  return m_itemText;
}
void annotationItem::setName(QString name)
{
  m_itemName=name;
}
void annotationItem::setText(QString text)
{
  m_itemText=text;
}
