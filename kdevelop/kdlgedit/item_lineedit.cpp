/***************************************************************************
                     item_lineedit.cpp  -  description
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

#define CALLER_ITEMCLASS_NAME KDlgItem_LineEdit
#include "item_all.cpp.inc"

KDlgItem_LineEdit::MyWidget::MyWidget(KDlgItem_LineEdit* wid, QWidget* parent, const char* name )
  : QLineEdit(parent,name)
{
  parentObject = wid;
  isItemActive = false;
  isMBPressed = false;
  isMainwidget = false;
  setCursor(arrowCursor);
}

void KDlgItem_LineEdit::MyWidget::paintEvent ( QPaintEvent *e )
{
  QLineEdit::paintEvent(e);

  if (isItemActive)
    KDlgItemsPaintRects(this,e);
}

KDlgItem_LineEdit::KDlgItem_LineEdit( KDlgEditWidget* editwid, QWidget *parent , const char* name )
  : KDlgItem_Base(editwid, parent,false,name)
{
  parentWidgetItem = 0;
  item = new MyWidget(this, parent);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
  addMyPropEntrys();
}

void KDlgItem_LineEdit::addMyPropEntrys()
{
  if (!props)
    return;

  props->addProp("Text",           "",       "General",        ALLOWED_STRING);
  props->addProp("hasFrame",       "TRUE",   "General",        ALLOWED_BOOL);
  props->addProp("MaxLength",      "",       "General",        ALLOWED_INT);
  props->addProp("CursorPositon",  "",       "General",        ALLOWED_INT);
  props->addProp("isTextSelected", "FALSE",  "General",        ALLOWED_BOOL);
}

void KDlgItem_LineEdit::repaintItem(QLineEdit *it)
{
  QLineEdit *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

  #define strIsDef(s) (!Prop2Str(s).isNull())
  #define intIsDef(s) (!Prop2Str(s).isEmpty())

  if (strIsDef("Text"))
    itm->setText(Prop2Str("Text"));

  if (intIsDef("MaxLength"))
    itm->setMaxLength(Prop2Int("MaxLength",32767));
  else
    itm->setMaxLength(32767);

  if (intIsDef("CursorPosition"))
    itm->setCursorPosition(Prop2Int("CursorPosition",32767));
  else
    itm->setCursorPosition(0);

  itm->setFrame(Prop2Bool("hasFrame") == 1 ? TRUE : FALSE);

  if (Prop2Bool("isTextSelected") == 1)
    itm->selectAll();
  else
    itm->deselect();

}
