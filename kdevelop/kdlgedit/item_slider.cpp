/***************************************************************************
                     item_slider.cpp  -  description
                             -------------------                                         
    begin                :  Tue May 20 1999                                           
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
#define INC_SLIDER
#include "items.h"
#include "itemsglobal.h"

#define CALLER_ITEMCLASS_NAME KDlgItem_Slider
#define ITEMCLASS_NAME KDlgItem_Slider
#define ITEMCLASS_TYPE QSlider

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

  props->addProp("Value",      "",        "General",        ALLOWED_INT);
  props->addProp("MinValue",      "0",        "General",        ALLOWED_INT);
  props->addProp("MaxValue",      "99",        "General",        ALLOWED_INT);
  props->addProp("isTracking",           "",       "General",        ALLOWED_BOOL);
  props->addProp("Orientation",           "Horizontal",       "General",        ALLOWED_ORIENTATION);
  
}

void ITEMCLASS_NAME::repaintItem(ITEMCLASS_TYPE *it)
{
  ITEMCLASS_TYPE *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  #define strIsDef(s) (!Prop2Str(s).isNull())
  #define intIsDef(s) (!Prop2Str(s).isEmpty())

 
  itm->setTracking(Prop2Bool("isTracking") == 1 ? TRUE : FALSE);
  
  if(intIsDef("Value"))
    itm->setValue(Prop2Int("Value"));
  
  itm->setRange(Prop2Int("MinValue"),Prop2Int("MaxValue"));
  if(Prop2Str("Orientation") == "Horizontal"){
    itm->setOrientation(QSlider::Horizontal);
  }
  else{
    itm->setOrientation(QSlider::Vertical);
  }
}
