/***************************************************************************
                          item_widget.h  -  description
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


#ifndef ITEM_WIDGET_H
#define ITEM_WIDGET_H

#include <qframe.h>

#include "item_base.h"
#include "itemsglobal.h"

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */
	
#include "kdlgeditwidget.h"
#include <kruler.h>


class KDlgItem_Widget : public KDlgItem_Base
{
  Q_OBJECT
  protected:
    class MyWidget : public QFrame
    {
      public:
        MyWidget(KDlgItem_Widget* wid, QWidget* parent = 0, bool isMainWidget = false, const char* name = 0);

        int recPosX(int px) { int xnew = px+x(); if ((!parent()) || (parent()!=parentObject->getEditWidget())) return ((MyWidget*)parent())->recPosX(xnew); else return px; }
        int recPosY(int py) { int ynew = py+y(); if ((!parent()) || (parent()!=parentObject->getEditWidget())) return ((MyWidget*)parent())->recPosY(ynew); else return py; }
        bool isItemActive;
      protected:
        void moveRulers( QMouseEvent *e ) { if (!parentObject) return; parentObject->getEditWidget()->horizontalRuler()->setValue(e->pos().x()+recPosX(0));parentObject->getEditWidget()->verticalRuler()->setValue(e->pos().y()+recPosY(0)); }

        KDlgItem_Widget* parentObject;
        virtual void paintEvent ( QPaintEvent * );
        virtual void mousePressEvent ( QMouseEvent * ) { isItemActive = !isItemActive; repaint(); }
        virtual void mouseMoveEvent ( QMouseEvent *e ) { moveRulers(e); }
    };           	

    MyWidget *item;

  public:
    KDlgItem_Widget( KDlgEditWidget* editwid = 0, QWidget *parent = 0, bool ismainwidget = false, const char* name = 0 );
    virtual ~KDlgItem_Widget() {}

    virtual QString itemClass() { return QString("QWidget"); }

    virtual MyWidget* getItem() { return item; }
    virtual void repaintItem(QWidget *it = 0);
};



#endif





