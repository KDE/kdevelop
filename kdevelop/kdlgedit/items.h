/***************************************************************************
                           item.h  -  description
                             -------------------
    begin                : Thu Apr 08 1999
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


#ifndef KDLG_ITEMS_H
#define KDLG_ITEMS_H

#include <qframe.h>
#include "itemsglobal.h"
#include "kdlgpropwidget.h"
#include <kcursor.h>
#include "kdlgeditwidget.h"
#include <kruler.h>
#include <qlineedit.h>
#include <qpushbutton.h>
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

    void deleteMyself();

    bool isMainWidget;
  protected:
    KDlgItemDatabase *childs;
    QWidget *item;
    KDlgPropertyBase *props;
    KDlgEditWidget* editWidget;
};


class KDlgItem_Widget : public KDlgItem_Base
{
  Q_OBJECT

  public:
    KDlgItem_Widget( KDlgEditWidget* editwid , QWidget *parent, bool ismainwidget, const char* name = 0 );

  #define classname KDlgItem_Widget
  #define widgettype QFrame
  #define classdesc "QWidget"
  #define MyWidgetAdd  public: MyWidget(KDlgItem_Widget* wid, QWidget* parent = 0, bool isMainWidget = false, const char* name = 0);
  #include "item_class.cpp.inc"
};



class KDlgItem_LineEdit : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_LineEdit
  #define widgettype QLineEdit
  #define classdesc "QLineEdit"
  #define MyWidgetAdd virtual void keyPressEvent ( QKeyEvent * ) {}
  #include "item_class.cpp.inc"
};

class KDlgItem_PushButton : public KDlgItem_Base
{
  Q_OBJECT

  #define classname KDlgItem_PushButton
  #define widgettype QPushButton
  #define classdesc "QPushButton"
  #include "item_class.cpp.inc"
};


#endif