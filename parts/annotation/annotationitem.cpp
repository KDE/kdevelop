//
// C++ Implementation: annotationitem
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
