/*
 *  File : snippetitem.cpp
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <qstring.h>
//Added by qt3to4:
#include <Q3PtrList>


#include "snippetitem.h"

SnippetItem::SnippetItem(Q3ListView * parent, QString name, QString text )
			: Q3ListViewItem( parent, name )
{
  strName = name;
  strText = text;
  iParent = -1;
}

SnippetItem::SnippetItem(Q3ListViewItem * parent, QString name, QString text)
			: Q3ListViewItem( parent, name )
{
  strName = name;
  strText = text;
  iParent = ((SnippetGroup *)parent)->getId();
}

SnippetItem::~SnippetItem()
{
}


/*!
    \fn SnippetItem::getName()
 */
QString SnippetItem::getName()
{
  return strName;
}


/*!
    \fn SnippetItem::getText
 */
QString SnippetItem::getText()
{
  return strText;
}


/*!
    \fn SnippetItem::setText(QString text)
 */
void SnippetItem::setText(QString text)
{
  strText = text;
}


/*!
    \fn SnippetItem::setName(QString name)
 */
void SnippetItem::setName(QString name)
{
  strName = name;
}

void SnippetItem::resetParent()
{
  SnippetGroup * group = dynamic_cast<SnippetGroup*>(parent());
  if (group)
    iParent = group->getId();
}

SnippetItem * SnippetItem::findItemByName(QString name, Q3PtrList<SnippetItem> &list)
{
  for ( SnippetItem * item = list.first(); item; item = list.next() ) {  //write the snippet-list
    if (item->getName() == name)
        return item;
  }
  return NULL;
}

SnippetGroup * SnippetItem::findGroupById(int id, Q3PtrList<SnippetItem> &list)
{
  for ( SnippetItem * item = list.first(); item; item = list.next() ) {  //write the snippet-list
    SnippetGroup * group = dynamic_cast<SnippetGroup*>(item);
    if (group && group->getId() == id)
        return group;
  }
  return NULL;
}


/* * * * * * * * * * * * * * * * * * * *
Deklaration for class SnippetGroup
* * * * * * * * * * * * * * * * * * * */

int SnippetGroup::iMaxId = 1;

SnippetGroup::SnippetGroup(Q3ListView * parent, QString name, int id, QString lang)
 : SnippetItem(parent, name, "GROUP")
{
    if (id > 0) {
      iId = id;
      if (id >= iMaxId)
        iMaxId = id+1;
    } else {
      iId = iMaxId;
      iMaxId++;
    }
    
    strLanguage = lang;
}

SnippetGroup::~SnippetGroup()
{
}

void SnippetGroup::setId(int id)
{
    iId = id; 
    if (iId >= iMaxId)
        iMaxId = iId+1;
}
