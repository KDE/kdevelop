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


class KDlgItem_Widget : public KDlgItem_Base
{
  Q_OBJECT
  protected:
    class MyWidget : public QFrame
    {
      public:
        MyWidget(QWidget* parent = 0, bool isMainWidget = false, const char* name = 0);

        bool isItemActive;
      protected:
        virtual void paintEvent ( QPaintEvent * );
        virtual void mousePressEvent ( QMouseEvent * ) { }
    };

    MyWidget *item;
    KDlgItemDatabase *childs;
    int nrOfChilds;

  public:
    KDlgItem_Widget( QWidget *parent = 0, bool ismainwidget = false, const char* name = 0 );
    virtual ~KDlgItem_Widget() {}

    virtual QString itemClass() { return QString("QWidget"); }

    virtual MyWidget* getItem() { return item; }
    virtual void repaintItem(QWidget *it = 0);

    KDlgItemDatabase *getChildDb() { return childs; }
    int getNrOfChilds() { return childs->numItems(); }
    bool addChild(KDlgItem_Base *item) { return childs->addItem(item); }

};



#endif





