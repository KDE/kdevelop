/***************************************************************************
                         item_widget.cpp  -  description
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
#include <qpainter.h>

class QWidget;

#define CALLER_ITEMCLASS_NAME KDlgItem_Widget
#include "item_all.cpp.inc"


KDlgItem_Widget::MyWidget::MyWidget( KDlgItem_Widget* wid, QWidget* parent , bool isMainWidget, const char* name )
  : QFrame(parent,name)
{
  parentObject = wid;
  isItemActive = false;
  isMBPressed = false;

  isMainwidget = isMainWidget;

  if (isMainWidget)
    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  else
    {
      setFrameStyle( QFrame::Panel | QFrame::Plain );
      setLineWidth(1);
    }
  show();
}

void KDlgItem_Widget::addMyPropEntrys()
{
}


void KDlgItem_Widget::MyWidget::paintEvent ( QPaintEvent *e )
{
  QFrame::paintEvent(e);

  QPainter p(this);
  p.setClipRect(e->rect());

  int x,y;
  for (x = 0; x < width(); x+=10)
   for (y = 0; y < height(); y+=10)
     p.drawPoint(x,y);

  if (isItemActive)	
    KDlgItemsPaintRects(&p,width(),height());
}


KDlgItem_Widget::KDlgItem_Widget( KDlgEditWidget *editwid, QWidget *parent , bool ismainwidget = false, const char* name )
   : KDlgItem_Base(editwid, parent,ismainwidget,name)
{
  parentWidgetItem = 0;
  childs = new KDlgItemDatabase();
  item = new MyWidget(this, parent, ismainwidget);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}


KDlgItem_Widget::KDlgItem_Widget( KDlgEditWidget *editwid, QWidget *parent , const char* name )
   : KDlgItem_Base(editwid, parent,false,name)
{
  parentWidgetItem = 0;
  childs = new KDlgItemDatabase();
  item = new MyWidget(this, parent, false);
  item->show();
  item->setMouseTracking(true);
  repaintItem();
}

void KDlgItem_Widget::repaintItem(QFrame *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

}

