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


#include "item_pushbutton.h"
#include <qpushbutton.h>
#include "itemsglobal.h"


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
  item = new MyWidget(this, parent);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}

void KDlgItem_PushButton::repaintItem(QPushButton *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
}
