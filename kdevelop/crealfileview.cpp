/***************************************************************************
                    kprojectdirtreelist.cpp - 
                             -------------------                                         

    version              :                                   
    begin                : 9 Sept 1998                                        
    copyright            : (C) 1998 by Stefan Bartel                         
    email                : bartel@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "qdir.h"
#include "qstrlist.h"
#include "crealfileview.h"
#include "qheader.h"
#include <kmsgbox.h>
#include <kprocess.h>
#include <qfileinfo.h>
#include <iostream.h>

CRealFileView::CRealFileView(QWidget*parent,const char* name):QListView(parent,name){

  loader = kapp->getIconLoader();
  folder_pix = loader->loadMiniIcon("folder.xpm");
  file_pix = loader->loadMiniIcon("c_src.xpm");
  inst_file_pix.load(KApplication::kde_datadir()+"/kdevelop/pics/mini/inst_file.xpm");

  file_col=addColumn("file");
  header()->hide();
  setRootIsDecorated(true);
  setSorting(-1);

  installed_file_menu = new KPopupMenu();
  installed_file_menu->setTitle(i18n("installed:"));
  installed_file_menu->insertItem(i18n("Remove from Project..."),this,SLOT(slotRemoveFileFromProject()));
  installed_file_menu->insertItem(i18n("Delete physically..."),this,SLOT(slotDeleteFilePhys()));
  installed_file_menu->insertSeparator();
  installed_file_menu->insertItem(i18n("Properties..."),this,SLOT(slotShowFileProperties()));

  other_file_menu = new KPopupMenu();
  other_file_menu->setTitle(i18n("other:"));
  other_file_menu->insertItem(i18n("Add to Project..."),this,SLOT(slotAddFileToProject()));
  other_file_menu->insertItem(i18n("Delete physically..."),this,SLOT(slotDeleteFilePhys()));
  //other_file_menu->insertSeparator();
  //other_file_menu->insertItem(i18n("Properties..."),this,SLOT(slotShowFileProperties()));

  connect(this, SIGNAL(selectionChanged(QListViewItem*)), SLOT(slotSelectionChanged(QListViewItem*)));
  connect(this, SIGNAL(rightButtonPressed(QListViewItem*,const QPoint &,int)), SLOT(slotRightButtonPressed( QListViewItem *,const QPoint &,int)));
}

CRealFileView::~CRealFileView(){
}

void CRealFileView::refresh(CProject* prj) {

  project=prj;
  QString projectdir=project->getProjectDir();
  if(projectdir.right(1)=="/") {
    projectdir.truncate(projectdir.length()-1);
  }
  QDir dir(projectdir);
  if (!dir.exists()) {
    return;
  }
  project->getAllFiles(filelist);
  clear();
  pRootItem = new QListViewItem(this,projectdir);
  pRootItem->setPixmap(file_col,folder_pix);
  pRootItem->setOpen(true);
  scanDir(projectdir,pRootItem);
  repaint();
}


void CRealFileView::scanDir(const QString& directory,QListViewItem* parent) {
  
  QDir dir(directory);
  if (!dir.exists()) {
    return;
  }
  dir.setSorting(QDir::Name);
  dir.setFilter(QDir::Dirs);
  QStrList dirList = *(dir.entryList());
  dirList.first();
  dirList.remove();
  dirList.remove();
  QString current_str;
  QListViewItem* pItem;
  QListViewItem* pItem2=parent;
  while( (dirList.current()) ) {
    pItem=new QListViewItem(parent,pItem2,dirList.current());
    pItem->setPixmap(file_col,folder_pix);
    pItem->setOpen(true);
    current_str = directory+"//"+dirList.current();
    scanDir(current_str,pItem);
    dirList.next();
    pItem2=pItem;
  }

  dir.setFilter(QDir::Files);
  QStrList fileList=*(dir.entryList());
  fileList.first();
  while( (fileList.current()) ) {
    pItem=new QListViewItem(parent,pItem2,fileList.current());
    if (IsInstalledFile(getRelFilename(pItem))) {
      pItem->setPixmap(file_col,inst_file_pix);
    } else {
      pItem->setPixmap(file_col,file_pix);
    }
    fileList.next();
    pItem2=pItem;
  }
}

QString CRealFileView::getRelFilename(QListViewItem* pItem) {

  QString filename = pItem->text(file_col);
  if (pItem != pRootItem) {
    pItem=pItem->parent();
  }
  while (pItem != pRootItem) {
    filename="/"+filename;
    filename = pItem->text(file_col)+filename;
    pItem = pItem->parent();
  }
  return filename;
}

QString CRealFileView::getFullFilename(QListViewItem* pItem) {

  QString filename = pItem->text(file_col);
  while (pItem != pRootItem) {
    pItem = pItem->parent();
    filename="/"+filename;
    filename = pItem->text(file_col)+filename;
  }
  return filename;
}

bool CRealFileView::IsInstalledFile(QString filename) {

  if (filelist.contains(filename)) {
    return true;
  } else {
    return false;
  }
}

bool CRealFileView::IsDirectory(QString filename) {

  QFileInfo fi(filename);
  return fi.isDir();
}

void CRealFileView::slotSelectionChanged(QListViewItem* selection) {

  QString filename=getFullFilename(selection);
  if (IsDirectory(filename)) {
    return;
  }
  if(leftButton()){ // right button: return
      emit fileSelected(getFullFilename(selection));
  }
}

bool CRealFileView::leftButton(){
  return left_button;
}
bool CRealFileView::rightButton(){
  return right_button;
}

void CRealFileView::slotRightButtonPressed( QListViewItem *pItem,const QPoint & mouse_pos,int col){
    
    if(pItem != 0) {
	QString filename=getRelFilename(pItem);
	setCurrentItem(pItem);
	setSelected(pItem,true);
	if (IsDirectory(getFullFilename(pItem))) {	    
	} 
	else {
	    if (IsInstalledFile(filename)) {
		filename=" "+filename;
		filename=i18n("registered:");
		installed_file_menu->setTitle(filename);
		installed_file_menu->popup(mouse_pos);
	    } else {
		filename=" "+filename;
		filename=i18n("other:");
		other_file_menu->setTitle(filename);
		other_file_menu->popup(mouse_pos);
	    }
	}
    }
}

void CRealFileView::slotAddFileToProject() {

  QString filename=getRelFilename(currentItem());
  if(KMsgBox::yesNo(0,i18n("Warning"),i18n("Do you want to add the file:\n"+filename+"\n to the project ?"),KMsgBox::EXCLAMATION) == 2){
    return;
  }
  emit addFileToProject(filename);
  refresh(project);
}

void CRealFileView::slotRemoveFileFromProject() {

  QString filename=getRelFilename(currentItem());
  if(KMsgBox::yesNo(0,i18n("Warning"),i18n("Do you want to remove the file:\n"+filename+"\n from the project ?\nIt will remain on disk."),KMsgBox::EXCLAMATION) == 2){
    return;
  }
  emit removeFileFromProject(filename);
  refresh(project);
}

void CRealFileView::slotDeleteFilePhys() {

  QString filename=getRelFilename(currentItem());
  if(KMsgBox::yesNo(0,i18n("Warning"),i18n("Do you want to remove the file:\n"+filename+"\n from your filesystem ?"),KMsgBox::EXCLAMATION) == 2){
    return;
  }
  if (IsInstalledFile(filename)) {
    emit removeFileFromProject(filename);
  }
  filename = getFullFilename(currentItem());
  KShellProcess* proc = new KShellProcess;
  QFileInfo info(filename);
  QString command = "rm -f " + filename;
  //  cerr << "\n\n" << command << "\n\n";
  *proc << command;
  proc->start();
  refresh(project);
}

void CRealFileView::slotShowFileProperties() {
  emit showFileProperties(getRelFilename(currentItem()));
}

void CRealFileView::mousePressEvent(QMouseEvent* event){
    if(event->button() == RightButton){    
	left_button = false;
	right_button = true;
    }
    if(event->button() == LeftButton){
	left_button = true;
	right_button = false;
    }
    mouse_pos.setX(event->pos().x());
    mouse_pos.setY(event->pos().y());
    QListView::mousePressEvent(event); 
}
