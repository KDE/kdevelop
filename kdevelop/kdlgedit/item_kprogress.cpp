/***************************************************************************
                     item_kcombo.cpp  -  description
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


#define DONTINC_ALL
#define INC_KPROGRESS
#include "items.h"
#include "itemsglobal.h"

#define CALLER_ITEMCLASS_NAME KDlgItem_KProgress
#define ITEMCLASS_NAME KDlgItem_KProgress
#define ITEMCLASS_TYPE KProgress

#include "item_all.cpp.inc"


ITEMCLASS_NAME::MyWidget::MyWidget(ITEMCLASS_NAME* wid, QWidget* parent, const char* /*name*/ )
  : ITEMCLASS_TYPE(parent)
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

  props->addProp("Value",        "0",          "Appearance",        ALLOWED_INT );
  props->addProp("minValue",     "0",          "Appearance",        ALLOWED_INT );
  props->addProp("maxValue",     "100",        "Appearance",        ALLOWED_INT );
  props->addProp("BarStyle",     "Blocked",    "Appearance",        ALLOWED_COMBOLIST,  "Solid\nBlocked"  );
  props->addProp("Orientation",  "Horizontal", "Appearance",        ALLOWED_ORIENTATION );
  props->addProp("BarColor",     "",           "Appearance",        ALLOWED_COLOR );
  props->addProp("BarPixmap",    "",           "Appearance",        ALLOWED_FILE );
  props->addProp("isTextEnable", "",           "Appearance",        ALLOWED_BOOL );
}

void ITEMCLASS_NAME::repaintItem(ITEMCLASS_TYPE *it)
{
  ITEMCLASS_TYPE *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  #define strIsDef(s) (!Prop2Str(s).isNull())
  #define intIsDef(s) (!Prop2Str(s).isEmpty())

}
