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
#include "items.h"
#include "kdlgeditwidget.h"
#include "kpopmenu.h"

KDlgItems::KDlgItems(CKDevelop *CKPar, QWidget *parent, const char *name ) : QWidget(parent,name)
{
  pCKDevel = CKPar;
  treelist = new QListView( this );

  treelist->setRootIsDecorated(true);
  treelist->clear();
  treelist->addColumn("");
  treelist->header()->hide();
  connect( treelist, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
                       SLOT(rightButtonPressed(QListViewItem *, const QPoint &, int)));
  connect ( treelist, SIGNAL(selectionChanged ()), SLOT(itemSelected()));

  KIconLoader *icon_loader = KApplication::getKApplication()->getIconLoader();

  folder_pix = icon_loader->loadMiniIcon("folder.xpm");
  entry_pix = icon_loader->loadMiniIcon("mini-default.xpm");

}

KDlgItems::~KDlgItems()
{
  delete treelist;
}

void KDlgItems::rightButtonPressed ( QListViewItem *it, const QPoint &p, int d)
{
  #define mkQPixTb(fn) QPixmap(KApplication::kde_toolbardir() + QString("/") +fn)
  #define mkQPixDd(fn) QPixmap(KApplication::kde_datadir() + QString("/kdevelop/toolbar/") + fn)

  KDlgEditWidget *edwid = pCKDevel->kdlg_get_edit_widget();
  if (!edwid)
    return;

  if (((MyTreeListItem*)it)->getItem())
    {
      if ( ((MyTreeListItem*)it)->getItem() != edwid->selectedWidget() )
        edwid->selectWidget( ((MyTreeListItem*)it)->getItem() );
//      treelist->setSelected(it,true);
    }
  else
    {
      return;
    }

  KPopupMenu phelp;
  phelp.setTitle( edwid->selectedWidget()->itemClass() );
  if (edwid->mainWidget() != edwid->selectedWidget())
    {
      phelp.insertItem( mkQPixTb("prev.xpm"), i18n("&Raise"), edwid, SLOT(slot_raiseSelected()) );
      phelp.insertItem( mkQPixTb("next.xpm"), i18n("&Lower"), edwid, SLOT(slot_lowerSelected()) );
      phelp.insertItem( mkQPixTb("top.xpm"), i18n("Raise to &top"),    edwid, SLOT(slot_raiseTopSelected()) );
      phelp.insertItem( mkQPixTb("bottom.xpm"), i18n("Lower to &bottom"), edwid, SLOT(slot_lowerBottomSelected()) );
      phelp.insertSeparator();
      phelp.insertItem( mkQPixDd("cut.xpm"), i18n("C&ut"),   edwid, SLOT(slot_cutSelected()) );
      phelp.insertItem( mkQPixTb("delete.xpm"), i18n("&Delete"),edwid, SLOT(slot_deleteSelected()) );
      phelp.insertItem( mkQPixDd("copy.xpm"), i18n("&Copy"),  edwid, SLOT(slot_copySelected()) );
    }
  phelp.insertItem( mkQPixDd("paste.xpm"), i18n("&Paste"), edwid, SLOT(slot_pasteSelected()) );
  phelp.insertSeparator();
  phelp.insertItem( mkQPixTb("help.xpm"), i18n("&Help"),  edwid, SLOT(slot_helpSelected()) );
  phelp.exec(QCursor::pos());
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

void KDlgItems::refreshList()
{
  addWidgetChilds( pCKDevel->kdlg_get_edit_widget()->mainWidget());

}


void KDlgItems::itemSelected ()
{
  setCursor(KCursor::waitCursor());
  if (!treelist->currentItem())
    return;

  KDlgItem_Widget *itm = ((MyTreeListItem*)treelist->currentItem())->getItem();

  if (!itm)
    return;

  pCKDevel->kdlg_get_edit_widget()->selectWidget((KDlgItem_Base*)itm);
  setCursor(KCursor::arrowCursor());
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
      if (treelist->firstChild())
        delete treelist->firstChild();
      treelist->clear();
      treelist->setUpdatesEnabled( FALSE );
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

