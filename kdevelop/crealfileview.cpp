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


CRealFileView::CRealFileView(QWidget*parent,const char* name):KTreeList(parent,name){
   loader = kapp->getIconLoader();
}
CRealFileView::~CRealFileView(){
}

void CRealFileView::refresh(QString projectdir) {

  clear();
  QDir dir(projectdir);
  if (!dir.exists()) {
    return;
  }
  QPixmap icon = loader->loadMiniIcon("folder.xpm");
  KPath path;
  path.push(&projectdir);
  insertItem(projectdir,&icon);
  scanDir(projectdir,path);
}


void CRealFileView::scanDir(QString& directory,KPath& path) {
  
  QDir dir(directory);
  if (!dir.exists()) {
    return;
  }
  QPixmap folder_pix = loader->loadMiniIcon("folder.xpm");
  QPixmap file_pix = loader->loadMiniIcon("c_src.xpm");
  QString pstr;
  dir.setSorting(QDir::Name);  

  dir.setFilter(QDir::Dirs);
  QStrList dirList = *(dir.entryList());
  dirList.first();
  dirList.remove();
  dirList.remove();
  QString current_str;
  while( (dirList.current()) ) {
    addChildItem(dirList.current(),&folder_pix,&path);
    pstr=dirList.current();
    path.push(&pstr);
    current_str = directory+"//"+dirList.current();
    scanDir(current_str,path);
    path.pop();
    dirList.next();
  }

  dir.setFilter(QDir::Files);
  QStrList fileList=*(dir.entryList());
  fileList.first();
  while( (fileList.current()) ) {
    addChildItem(fileList.current(),&file_pix,&path);
    fileList.next();
  }
  
}
