/***************************************************************************
                     item_pushbutton.cpp  -  description
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
#define INC_PUSHBUTTON
#include "items.h"
#include <qpushbutton.h>
#include "itemsglobal.h"


#define CALLER_ITEMCLASS_NAME KDlgItem_PushButton
#include "item_all.cpp.inc"

KDlgItem_PushButton::MyWidget::MyWidget(KDlgItem_PushButton* wid, QWidget* parent, const char* name )
  : QPushButton("Button", parent,name)
{
  parentObject = wid;
  isItemActive = false;
  isMBPressed = false;
  isMainwidget = false;
}

void KDlgItem_PushButton::MyWidget::paintEvent ( QPaintEvent *e )
{
  QPushButton::paintEvent(e);

  if (isItemActive)
    KDlgItemsPaintRects(this,e);
}

KDlgItem_PushButton::KDlgItem_PushButton( KDlgEditWidget* editwid, QWidget *parent , const char* name )
  : KDlgItem_Base(editwid, parent,false,name)
{
  addMyPropEntrys();
  parentWidgetItem = 0;
  item = new MyWidget(this, parent);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}

void KDlgItem_PushButton::recreateItem()
{
  item->recreate((QWidget*)parent(), 0, item->pos(), true);
  item->setMouseTracking(true);
}

void KDlgItem_PushButton::addMyPropEntrys()
{

  props->addProp("Text",           "Button",       "General",        ALLOWED_STRING);
  props->addProp("isDefault",      "FALSE",        "General",        ALLOWED_BOOL);
  props->addProp("isAutoDefault" , "FALSE",        "General",        ALLOWED_BOOL);
  props->addProp("isToggleButton", "FALSE",        "General",        ALLOWED_BOOL);
  props->addProp("isToggledOn",    "FALSE",        "General",        ALLOWED_BOOL);
  props->addProp("isMenuButton",   "FALSE",        "General",        ALLOWED_BOOL);
  props->addProp("isAutoResize",   "FALSE",        "General",        ALLOWED_BOOL);
  props->addProp("isAutoRepeat",   "FALSE",        "General",        ALLOWED_BOOL);

  props->addProp("Pixmap",         "",             "Appearance",     ALLOWED_FILE);
}

void KDlgItem_PushButton::repaintItem(QPushButton *it)
{
  QPushButton *itm = it ? it : item;

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
}
