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
}

void KDlgItem_LineEdit::repaintItem(QLineEdit *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
}
