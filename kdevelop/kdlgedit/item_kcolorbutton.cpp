/***************************************************************************
                     item_kcolorbutton.cpp  -  description
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
#define INC_KCOLORBUTTON
#include "items.h"
#include "itemsglobal.h"

#define CALLER_ITEMCLASS_NAME KDlgItem_KColorButton
#define ITEMCLASS_NAME KDlgItem_KColorButton
#define ITEMCLASS_TYPE KColorButton

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
  props->addProp("Text",           "Button",       "General",        ALLOWED_STRING);
  props->addProp("isDefault",      "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoDefault" , "",             "General",        ALLOWED_BOOL);
  props->addProp("isToggleButton", "",             "General",        ALLOWED_BOOL);
  props->addProp("isToggledOn",    "",             "General",        ALLOWED_BOOL);
  props->addProp("isMenuButton",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoResize",   "",             "General",        ALLOWED_BOOL);
  props->addProp("isAutoRepeat",   "",             "General",        ALLOWED_BOOL);
  props->addProp("DisplayedColor", "",             "General",        ALLOWED_COLOR);
}

void ITEMCLASS_NAME::repaintItem(ITEMCLASS_TYPE *it)
{
  ITEMCLASS_TYPE *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
  #define strIsDef(s) (!Prop2Str(s).isNull())

  if (strIsDef("Text"))
    itm->setText(Prop2Str("Text"));

  if (Prop2Str("Pixmap").isEmpty())
    {
      if (itm->pixmap())
        itm->setPixmap(QPixmap());
    }
  else
    itm->setPixmap(QPixmap(Prop2Str("Pixmap")));

  itm->setDefault(Prop2Bool("isDefault") == 1 ? TRUE : FALSE);
  itm->setIsMenuButton(Prop2Bool("isMenuButton") == 1 ? TRUE : FALSE);
  itm->setAutoResize(Prop2Bool("isAutoResize") == 1 ? TRUE : FALSE);
  itm->setToggleButton(Prop2Bool("isToggleButton") == 1 ? TRUE : FALSE);
  itm->setOn((Prop2Bool("isToggledOn") == 1 ? TRUE : FALSE) && (Prop2Bool("isToggleButton") == 1 ? TRUE : FALSE));

  if (strIsDef("DisplayedColor"))
    itm->setColor(Str2Color(Prop2Str("DisplayedColor")));
  else
    itm->setColor(QColor());
}



