/***************************************************************************
                     item_listbox.cpp  -  description
                             -------------------                                         
    begin                :  Tue May 18 1999                                           
    copyright            : (C) 1999 by Sandy Meier
    email                : smeier@rz.uni-potsdam.de
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
#define INC_LISTBOX
#include "items.h"
#include "itemsglobal.h"

#define CALLER_ITEMCLASS_NAME KDlgItem_ListBox
#define ITEMCLASS_NAME KDlgItem_ListBox
#define ITEMCLASS_TYPE QListBox

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
  props->addProp("isAutoUpdate",   "TRUE",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoScroll",   "TRUE",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoScrollBar",   "TRUE",          "General",        ALLOWED_BOOL);
  props->addProp("isAutoBottomScrollBar",   "TRUE",          "General",        ALLOWED_BOOL);
  props->addProp("isBottomScrollBar",   "FALSE",          "General",        ALLOWED_BOOL);
  props->addProp("isDragSelect",   "TRUE",             "General",        ALLOWED_BOOL);
  props->addProp("isSmoothScrolling",   "TRUE",             "General",        ALLOWED_BOOL);
  
  props->addProp("setFixedVisibleLines",   "",             "Geometry",        ALLOWED_INT);
}

void ITEMCLASS_NAME::repaintItem(ITEMCLASS_TYPE *it)
{
  ITEMCLASS_TYPE *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
  itm->setAutoUpdate(Prop2Bool("isAutoUpdate") == 1 ? TRUE : FALSE);
  itm->setAutoScroll(Prop2Bool("isAutoScroll") == 1 ? TRUE : FALSE);
  itm->setAutoScrollBar(Prop2Bool("isAutoScrollBar") == 1 ? TRUE : FALSE);
  itm->setAutoBottomScrollBar(Prop2Bool("isAutoBottomScrollBar") == 1 ? TRUE : FALSE);
  itm->setBottomScrollBar(Prop2Bool("isBottomScrollBar") == 1 ? TRUE : FALSE);
  itm->setDragSelect(Prop2Bool("isDragSelect") == 1 ? TRUE : FALSE);
  itm->setSmoothScrolling(Prop2Bool("isSmoothScrolling") == 1 ? TRUE : FALSE);
  if(Prop2Str("setFixedVisibleLines") != ""){
    itm->setFixedVisibleLines(Prop2Int("setFixedVisibleLines"));
  }
  
}
