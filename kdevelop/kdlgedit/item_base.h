/***************************************************************************
                          item_base.h  -  description
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


#ifndef ITEM_BASE_H
#define ITEM_BASE_H

#include <qobject.h>
#include "itemsglobal.h"
/**
  *@author Pascal Krahmer <pascal@beast.de>
  */

class KDlgEditWidget;


class KDlgItem_Base : public QObject
{
  Q_OBJECT
  public:
    KDlgItem_Base( KDlgEditWidget* editwid = 0, QWidget *parent = 0, bool ismainwidget = false, const char* name = 0 );
    virtual ~KDlgItem_Base() {}

    virtual QString itemClass() { return QString("[Base]"); }

    virtual QWidget *getItem() { return item; }
    virtual void repaintItem(QWidget *it = 0);

    KDlgPropertyBase *getProps() { return props; }

    KDlgItemDatabase *getChildDb() { return childs; }
    int getNrOfChilds() { return childs->numItems(); }
    bool addChild(KDlgItem_Base *itm) { return childs->addItem(itm); }

    KDlgEditWidget* getEditWidget() { return editWidget; }

    virtual void select() { }
    virtual void deselect() { }

    bool isMainWidget;
  protected:
    KDlgItemDatabase *childs;
    QWidget *item;
    KDlgPropertyBase *props;
    KDlgEditWidget* editWidget;
};


#endif





