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


KDlgItem_PushButton::MyWidget::MyWidget(QWidget* parent, const char* name )
  : QPushButton("Button", parent,name)
{
}


void KDlgItem_PushButton::MyWidget::paintEvent ( QPaintEvent *e )
{
  QPushButton::paintEvent(e);

  if (isItemActive)
    KDlgItemsPaintRects(this,e);
}

KDlgItem_PushButton::KDlgItem_PushButton( QWidget *parent , const char* name )
  : KDlgItem_Base(0,false,name)
{
  item = new MyWidget(parent);
//  item->setGeometry(10,10,100,50);
  item->show();
  repaintItem();
}

void KDlgItem_PushButton::repaintItem(QPushButton *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);
}
