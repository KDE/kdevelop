/***************************************************************************
                         kdlgitems.h  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
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


#ifndef KDLGITEMS_H
#define KDLGITEMS_H

#include <qwidget.h>
#include <qpixmap.h>
#include <ktreelist.h>

//class KTreeList;
//class KTreeListItem;
class KDlgItem_Widget;
class CKDevelop;

/**
  *@author Pascal Krahmer <pascal@beast.de>
  */

class KDlgItems : public QWidget  {
  Q_OBJECT
  protected:
    virtual void resizeEvent ( QResizeEvent * );

    class MyTreeListItem : public KTreeListItem
    {
      private:
      Q_OBJECT
      public:
        MyTreeListItem (KDlgItem_Widget *itemp, const QString& theText, const QPixmap *thePixmap = 0);
        KDlgItem_Widget *getItem() { return itemptr; }

      protected:
        KDlgItem_Widget *itemptr;
    };

    KTreeList *treelist;
    QPixmap folder_pix;
    QPixmap entry_pix;
    CKDevelop *pCKDevel;
  public:
    KDlgItems(CKDevelop *CKPar, QWidget *parent=0, const char *name=0);
    ~KDlgItems();

    KTreeList *getTreeList() { return treelist; }

    void addWidgetChilds(KDlgItem_Widget *wd, MyTreeListItem *itm=0);

  public slots:
    void itemSelected (int index);
};

#endif
