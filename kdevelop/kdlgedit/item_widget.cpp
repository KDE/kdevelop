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


#include "item_widget.h"
#include <qpainter.h>

class QWidget;

KDlgItem_Widget::MyWidget::MyWidget(QWidget* parent , bool isMainWidget, const char* name )
  : QFrame(parent,name)
{
  isItemActive = false;

  if (isMainWidget)
    setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  else
    {
      setFrameStyle( QFrame::Panel | QFrame::Plain );
      setLineWidth(1);
    }
  show();
}

void KDlgItem_Widget::MyWidget::paintEvent ( QPaintEvent *e )
{
  QFrame::paintEvent(e);

  QPainter p(this);
  p.setClipRect(e->rect());

  for (int x = 0; x < width(); x+=10)
   for (int y = 0; y < height(); y+=10)
     p.drawPoint(x,y);

  if (isItemActive)	
    KDlgItemsPaintRects(&p,width(),height());
}


KDlgItem_Widget::KDlgItem_Widget( QWidget *parent , bool ismainwidget = false, const char* name )
   : KDlgItem_Base(parent,ismainwidget,name)
{
  childs = new KDlgItemDatabase();
  item = new MyWidget(parent, ismainwidget);
  item->show();
  props = new KDlgPropertyBase();
  repaintItem();
}

void KDlgItem_Widget::repaintItem(QWidget *it)
{
  QWidget *itm = it ? it : item;

  if ((!itm) || (!props))
    return;

  KDlgItem_Base::repaintItem(itm);

}

