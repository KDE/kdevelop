/***************************************************************************
                          item_pushbutton.h  -  description
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


#ifndef ITEM_PUSHBUTTON_H
#define ITEM_PUSHBUTTON_H

#include "item_base.h"
#include <qpushbutton.h>
#include <qpainter.h>

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */


class KDlgItem_PushButton : public KDlgItem_Base
{
  Q_OBJECT
  protected:
    class MyWidget : public QPushButton
    {
      public:
        MyWidget(QWidget* parent = 0, const char* name = 0);

        bool isItemActive;
      protected:
        virtual void paintEvent ( QPaintEvent * );
        virtual void mousePressEvent ( QMouseEvent * ) { isItemActive = !isItemActive; repaint(); }
    };

    MyWidget *item;

  public:
    KDlgItem_PushButton( QWidget *parent = 0, const char* name = 0 );
    virtual ~KDlgItem_PushButton() {}

    virtual QString itemClass() { return QString("PushButton"); }

    virtual MyWidget* getItem() { return item; }
    virtual void repaintItem(QPushButton *it = 0);
};


#endif





