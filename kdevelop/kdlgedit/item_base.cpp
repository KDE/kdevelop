/***************************************************************************
                         item_base.cpp  -  description
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "item_base.h"
#include <qwidget.h>
#include "itemsglobal.h"

KDlgItem_Base::KDlgItem_Base( KDlgEditWidget* editwid , QWidget *parent , bool ismainwidget, const char* name )
  : QObject(parent,name)
{
  editWidget = editwid;
  childs = 0;
  isMainWidget = ismainwidget;
  item = new QWidget(parent);
  item->setMouseTracking(true);

  props = new KDlgPropertyBase();
  repaintItem();
}

void KDlgItem_Base::repaintItem(QWidget *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  itm->setGeometry(isMainWidget ? RULER_WIDTH : (props->getIntFromProp("X",itm->x())),
                   isMainWidget ? RULER_HEIGHT : (props->getIntFromProp("Y",itm->y())),
                   props->getIntFromProp("Width",itm->width()),
                   props->getIntFromProp("Height",itm->height()));
}
