/*
 *  File : snippetitem.cpp
 *
 *  Author: Robert Gruber <rgruber@users.sourceforge.net>
 *
 *  Copyright: See COPYING file that comes with this distribution
 */

#include <qstring.h>


#include "snippetitem.h"

SnippetItem::SnippetItem( QListView * parent, QString name, QString text )
			: QListViewItem( parent, name )
{
  strName = name;
  strText = text;
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
