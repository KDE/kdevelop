/***************************************************************************
                     item_label.cpp  -  description
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


#include "items.h"
#include "itemsglobal.h"

#define CALLER_ITEMCLASS_NAME KDlgItem_Label
#define ITEMCLASS_NAME KDlgItem_Label
#define ITEMCLASS_TYPE QLabel

#include "item_all.cpp.inc"


ITEMCLASS_NAME::MyWidget::MyWidget(ITEMCLASS_NAME* wid, QWidget* parent, const char* name )
  : ITEMCLASS_TYPE(parent,name)
{
  parentObject = wid;
  isItemActive = false;
  isMBPressed = false;
  isMainwidget = false;
  setCursor(arrowCursor);
}

void ITEMCLASS_NAME::MyWidget::paintEvent ( QPaintEvent *e )
{
  ITEMCLASS_TYPE::paintEvent(e);

  if (isItemActive)
    KDlgItemsPaintRects(this,e);
}

ITEMCLASS_NAME::ITEMCLASS_NAME( KDlgEditWidget* editwid, QWidget *parent , const char* name )
  : KDlgItem_Base(editwid, parent,false,name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new MyWidget(this, parent);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}

void ITEMCLASS_NAME::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void ITEMCLASS_NAME::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Text",           "Label",       "General",        ALLOWED_STRING);
  props->addProp("Buddy",          ""     ,       "C++ Code",       ALLOWED_STRING);
  props->addProp("isAutoResize",   "FALSE",       "General",        ALLOWED_BOOL);
  props->addProp("Margin",         "",            "Appearance",     ALLOWED_INT);
}

void ITEMCLASS_NAME::repaintItem(ITEMCLASS_TYPE *it)
{
  ITEMCLASS_TYPE *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  #define strIsDef(s) (!Prop2Str(s).isNull())
  #define intIsDef(s) (!Prop2Str(s).isEmpty())

  if (strIsDef("Text"))
    itm->setText(Prop2Str("Text"));

  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);

  itm->setMargin(props->getIntFromProp("Margin",-1));

}
