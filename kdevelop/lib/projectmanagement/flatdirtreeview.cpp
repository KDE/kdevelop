/***************************************************************************
                          flatdirtreeview.cpp  -  description
                             -------------------
    begin                : Thu Oct 5 2000
    copyright            : (C) 2000 by Sandy Meier
    email                : smeier@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "flatdirtreeview.h"
#include <qheader.h>
#include <qstringlist.h>
#include <qdir.h>
#include <iostream.h>
#include <kglobal.h>
#include <kiconloader.h>


FlatDirTreeView::FlatDirTreeView(QWidget *parent, const char *name ) : QListView(parent,name) {
  header()->hide();
  addColumn("first");
  connect(this,SIGNAL(clicked(QListViewItem*)),this,SLOT(slotClicked(QListViewItem*)));
}

FlatDirTreeView::~FlatDirTreeView(){
}

void FlatDirTreeView::setDirLocation(QString absPath){
  m_currentDir = absPath;
  KIconLoader* pLoader = KGlobal::iconLoader();
  QPixmap openFolder = pLoader->loadIcon("folder_open",KIcon::Small);
  QPixmap closedFolder = pLoader->loadIcon("folder_yellow",KIcon::Small);

  clear();
  QStringList dirs = QStringList::split("/",absPath);
  QListViewItem* pDir = new QListViewItem(this,"/");
  QListViewItem* pDir2 = 0;
  pDir->setOpen(true);
  pDir->setPixmap(0,openFolder);
  
  for ( QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it ) {
    pDir = new QListViewItem(pDir,*it);
    pDir->setOpen(true);
    pDir->setPixmap(0,openFolder);
  }
  pDir->setSelected(true);
  QDir dirInfo(absPath);
  dirs = dirInfo.entryList(QDir::Dirs);
  for ( QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it ) {
    if((*it) != "." &&  (*it) != ".."){
      pDir2 = new QListViewItem(pDir,*it);
      pDir2->setPixmap(0,closedFolder);
    }
  }
}
void FlatDirTreeView::slotClicked(QListViewItem* pItem){
  // please don't look at this code, it's braindamaged :-)
  if(pItem == 0) return;
  QString path;
  path.prepend(pItem->text(0) + "/");
  while (pItem !=0){
    pItem = pItem->parent();
    if(pItem !=0){
      path = path.prepend(pItem->text(0) + "/");
    }
  }
  path.remove(0,1); // remove the first /
  setDirLocation(path);
  m_currentDir = path;
  emit dirSelected(path);
  
}
QString FlatDirTreeView::currentDir(){
  return m_currentDir;
}

#include "flatdirtreeview.moc"
