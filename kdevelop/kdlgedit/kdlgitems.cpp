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
#include "kdlgproplvis.h"
#include "../ckdevelop.h"

KDlgItems::KDlgItems(QWidget *parent, const char *name ) : QWidget(parent,name)
{
  treelist = new KTreeList( this, i18n("kdlgitems_treelist") );

  treelist->setUpdatesEnabled( false );
  treelist->clear();

  KPath path;
  QString str;
  QString file;
  QString str_path;
  QString str_sub_path;

  KIconLoader *icon_loader = KApplication::getKApplication()->getIconLoader();

  QPixmap folder_pix = icon_loader->loadMiniIcon("folder.xpm");
  QPixmap book_pix = icon_loader->loadMiniIcon("mini-default.xpm");
  str = i18n("Main Widget");
  path.push(&str);
  treelist->insertItem(i18n("Main Widget"),&folder_pix);


  treelist->addChildItem(new KTreeListItem(i18n("KDevelop"),&folder_pix),&path);
  treelist->addChildItem(i18n("Qt/KDE Libraries"),&folder_pix,&path);
  treelist->addChildItem(i18n("Others"),&folder_pix,&path);
  treelist->addChildItem(i18n("Current Project"),&folder_pix,&path);


  //  add KDevelop
  str_path = i18n("KDevelop");
  path.push(&str_path);
  treelist->addChildItem(i18n("Manual"),&book_pix,&path);
  treelist->addChildItem(i18n("Tutorial"),&book_pix,&path);
  treelist->addChildItem(i18n("C/C++ Reference"),&book_pix,&path);

  //  add the Libraries
  str_path = i18n("Qt/KDE Libraries");
  path.pop();
  path.push(&str_path);
  treelist->addChildItem(i18n("Qt-Library"),&book_pix,&path);
  treelist->addChildItem(i18n("KDE-Core-Library"),&book_pix,&path);
  treelist->addChildItem(i18n("KDE-UI-Library"),&book_pix,&path);
  treelist->addChildItem(i18n("KDE-KFile-Library"),&book_pix,&path);

  treelist->setExpandLevel(2);
  treelist->setUpdatesEnabled( TRUE );
  treelist->repaint();

//  ((CKDevelop*)parent)->kdlg_get_items_view()->addWidgetChilds(
}

KDlgItems::~KDlgItems()
{
}

void KDlgItems::resizeEvent ( QResizeEvent *e )
{
  QWidget::resizeEvent(e);

  treelist->setGeometry( 0,0, width(), height() );
}

void KDlgItems::addWidgetChilds(KDlgItem_Widget *wd)
{
  if ((!wd) || (!wd->getChildDb()))
    return;

  KDlgItem_Widget *w = (KDlgItem_Widget*)wd->getChildDb()->getFirst();
//  do {
    if ((w) && (w->getProps()))
      if (w->getProps()) w->getProps()->getProp(1);
//      printf("****** %s\n",(const char*)w->getProps()->getProp(1));
//    w = (KDlgItem_Widget*)wd->getChildDb()->getNext();
//  } while (w);
}

