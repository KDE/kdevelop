/***************************************************************************
                          kdlgitems.cpp  -  description
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


#include "kdlgitems.h"

#include <ktreelist.h>
#include <kiconloader.h>
#include <kapp.h>
#include <qstring.h>
#include "kdlgproplvis.h"
#include "../ckdevelop.h"
#include "item_widget.h"

KDlgItems::KDlgItems(CKDevelop *CKPar, QWidget *parent, const char *name ) : QWidget(parent,name)
{
  pCKDevel = CKPar;
  treelist = new QListView( this );

  treelist->setRootIsDecorated(true);
  treelist->clear();
  treelist->addColumn(i18n("Widgets & Items"));

  KIconLoader *icon_loader = KApplication::getKApplication()->getIconLoader();

  folder_pix = icon_loader->loadMiniIcon("folder.xpm");
  entry_pix = icon_loader->loadMiniIcon("mini-default.xpm");

  connect ( treelist, SIGNAL(selectionChanged ()), SLOT(itemSelected()));
}

KDlgItems::MyTreeListItem::MyTreeListItem(MyTreeListItem* parent, KDlgItem_Widget *itemp, const QString& theText , const QPixmap *thePixmap )
  : QListViewItem((QListViewItem*)parent)
{
  setText( 0, (const char*)theText );
  setPixmap( 0, *thePixmap );
  itemptr = itemp;
}

KDlgItems::MyTreeListItem::MyTreeListItem(QListView* parent, KDlgItem_Widget *itemp, const QString& theText , const QPixmap *thePixmap )
  : QListViewItem(parent, (const char*)theText)
{
  setText( 0, (const char*)theText );
  setPixmap( 0, *thePixmap );
  itemptr = itemp;
}

KDlgItems::~KDlgItems()
{
}

void KDlgItems::itemSelected ()
{
  if (!treelist->currentItem())
    return;

  KDlgItem_Widget *itm = ((MyTreeListItem*)treelist->currentItem())->getItem();

  if (!itm)
    return;

  pCKDevel->kdlg_get_edit_widget()->selectWidget((KDlgItem_Base*)itm);
}

void KDlgItems::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent(e);

  treelist->setGeometry( 0,0, width(), height() );
}

void KDlgItems::addWidgetChilds(KDlgItem_Widget *wd, MyTreeListItem *itm)
{
  if ((!wd) || (!wd->getChildDb()))
    return;

  QString s;
  QString str;
  MyTreeListItem *item = itm;

  if (!itm)
    {
      s.sprintf("%s [%s]", (const char*)i18n("Main Widget"), (const char*)wd->getProps()->getProp("Name")->value);
      treelist->setUpdatesEnabled( FALSE );
      treelist->clear();
      item=new MyTreeListItem(treelist, wd, QString(s),&folder_pix);
      item->setOpen(true);
    }


  KDlgItem_Base *w = wd->getChildDb()->getFirst();
  do {
    if (w)
      {
        if (w->getProps())
          {
            s.sprintf("%s [%s]", (const char*)w->itemClass(), (const char*)w->getProps()->getProp("Name")->value);
          }

        if (w->getChildDb())
          {
            MyTreeListItem *it=new MyTreeListItem(item, (KDlgItem_Widget*)w,s,&folder_pix);
            it->setOpen(true);
            addWidgetChilds((KDlgItem_Widget*)w, it);
          }
        else
          {
            (new MyTreeListItem(item, (KDlgItem_Widget*)w,s,&entry_pix))->setOpen(true);
          }
      }

    w = wd->getChildDb()->getNext();
  } while (w);

  if (!itm)
    {
//      treelist->setExpandLevel(2);
      treelist->setUpdatesEnabled( TRUE );
//      treelist->expandItem(0);
      treelist->repaint();
    }
}

