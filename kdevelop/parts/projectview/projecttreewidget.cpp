/***************************************************************************                                *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include "projecttreewidget.h"
#include "projectspace.h"
#include <iostream.h>
#include <qheader.h>
#include <qregexp.h>
#include <kmimetype.h>
#include <qfileinfo.h>
#include <kiconloader.h>


ProjectTreeWidget::ProjectTreeWidget(ProjectView *part)
  : QListView(0, "project tree widget"){
  m_pProjectSpace = 0;
  addColumn("1");
  header()->hide();
 
}

ProjectTreeWidget::~ProjectTreeWidget()
{}

void ProjectTreeWidget::setProjectSpace(ProjectSpace* pProjectSpace){
  cerr << "kdevelop (projectview): ProjectTreeWidget::setProjectSpace" << endl;
  
  ProjectItem* pProjectItem =0;
  GroupItem* pGroupItem = 0;
  FileItem* pFileItem =0;
  QList<Project>* pProjects=0;
  Project* pProject=0;
  QStringList fileNames;
  FileGroup* pFileGroup;
  QList<FileGroup> defaultFileGroups;
  QStringList filters;
  QStringList files;
  QFileInfo fileInfo;
  KIconLoader* pLoader = KGlobal::iconLoader();
  QPixmap folderPixmap = pLoader->loadIcon("folder",KIcon::Small);
  
  QRegExp rexExp("",true,true);
  m_pProjectSpace = pProjectSpace;
  ProjectSpaceItem* pProjectSpaceItem = new ProjectSpaceItem(this);
  pProjectSpaceItem->setOpen(true);
  
  pProjectSpaceItem->setText(0,m_pProjectSpace->name());
  pProjectSpaceItem->setPixmap(0,folderPixmap);
  defaultFileGroups = m_pProjectSpace->defaultFileGroups();
  pProjects = m_pProjectSpace->allProjects();
  cerr << "kdevelop (projectview): init (end)" << endl;
  for(pProject=pProjects->first();pProject !=0;pProject=pProjects->next()){ // projects
    cerr << "kdevelop (projectview): project found" << endl;
    pProjectItem = new ProjectItem(pProjectSpaceItem);
    pProjectItem->setText(0,pProject->name());
    pProjectItem->setPixmap(0,folderPixmap);
    if (pProject == m_pProjectSpace->currentProject()) {
      pProjectItem->setOpen(true);
    }
    
    fileNames = pProject->allAbsoluteFileNames();
    for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it ) {
      cerr << "FILEPROJECTVIEW:" << *it << endl;
    }
    
    for(pFileGroup=defaultFileGroups.first();pFileGroup !=0
	  ;pFileGroup=defaultFileGroups.next()){ // groups
      
      pGroupItem = new GroupItem(pProjectItem);
      pGroupItem->setText(0,pFileGroup->name());
      pGroupItem->setPixmap(0,folderPixmap);
      filters = QStringList::split(';',pFileGroup->filter());
      
      for ( QStringList::Iterator filterIt = filters.begin(); filterIt != filters.end(); ++filterIt ) {
	cerr << "FILTER" << *filterIt << endl;
	rexExp.setPattern(*filterIt);
	files = fileNames.grep(rexExp);
	// files
	for ( QStringList::Iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
	  cerr << "FILE" << *fileIt << endl;
	  fileInfo.setFile(*fileIt);
	  pFileItem = new FileItem(pGroupItem);
	  pFileItem->setText(0,fileInfo.fileName());
	  pFileItem->setPixmap(0,KMimeType::pixmapForURL(*fileIt,0,KIcon::Small));
	  pFileItem->setAbsFileName(*fileIt);
	  fileNames.remove(*fileIt);// remove the showed files from the filelist
	}
      } // end for filter
    } // end for groups

    // rest files
    for ( QStringList::Iterator fileIt = fileNames.begin(); fileIt != fileNames.end(); ++fileIt ) {
      cerr << "FILE" << *fileIt << endl;
      fileInfo.setFile(*fileIt);
      pFileItem = new FileItem(pProjectItem);
      pFileItem->setText(0,fileInfo.fileName());
      pFileItem->setPixmap(0,KMimeType::pixmapForURL(*fileIt,0,KIcon::Small));
      pFileItem->setAbsFileName(*fileIt);
    }
  } // end for projects
}

void FileItem::setAbsFileName(QString fileName){
  m_absFileName = fileName;
}
QString FileItem::absFileName(){
  return m_absFileName;
}
