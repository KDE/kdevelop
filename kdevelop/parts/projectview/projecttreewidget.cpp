/***************************************************************************                                
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
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
#include <iostream.h>
#include <qheader.h>
#include <qregexp.h>
#include <kmimetype.h>
#include <qfileinfo.h>
#include <kiconloader.h>
#include <qfont.h>
#include <qpopupmenu.h>
#include <kaction.h>
#include "projectview.h"


ProjectTreeWidget::ProjectTreeWidget(ProjectView *pPart)
  : QListView(0, "project tree widget"){
  m_pProjectSpace = 0;
  m_pProjectView = pPart;
  addColumn("1");
  header()->hide();
  m_projectFileGroups.clear();
  connect(this,SIGNAL(rightButtonPressed( QListViewItem*, const QPoint&,int)),this,SLOT(slotRightButtonPressed( QListViewItem*, const QPoint&,int)));
}

ProjectTreeWidget::~ProjectTreeWidget()
{}

void ProjectTreeWidget::slotRightButtonPressed( QListViewItem* pItem, const QPoint& p,int){
   cerr << "kdevelop (projectview): ProjectTreeWidget::slotRightButtonPressed" << endl;
  ProjectTreeItem* pPItem = static_cast<ProjectTreeItem*> (pItem);
  QPopupMenu* pPopUp = createPopup(pPItem);
  if(pPopUp){
    pPopUp->exec(p);
  }
  delete pPopUp;
}

void ProjectTreeWidget::setProjectSpace(ProjectSpace* pProjectSpace){
  cerr << "kdevelop (projectview): ProjectTreeWidget::setProjectSpace" << endl;
  m_pProjectSpace = pProjectSpace;
  if(m_projectFileGroups.isEmpty()){ // there was no ProjectView tag in the projectfile
    createDefaultFileGroups();
  }
  refresh();
}
void ProjectTreeWidget::refresh(){
  clear();
  ProjectItem* pProjectItem =0;
  GroupItem* pGroupItem = 0;
  FileItem* pFileItem =0;
  QList<Project>* pProjects=0;
  Project* pProject=0;
  QStringList fileNames;
  FileGroup* pFileGroup;
  QList<FileGroup> defaultFileGroups;
  QList<FileGroup> fileGroups;
  QStringList filters;
  QStringList files;
  QFileInfo fileInfo;
  KIconLoader* pLoader = KGlobal::iconLoader();
  QPixmap folderPixmap = pLoader->loadIcon("folder_yellow",KIcon::Small);
  QPixmap projectSpacePixmap = pLoader->loadIcon("buttons",KIcon::Small);
  QPixmap projectPixmap = pLoader->loadIcon("queue",KIcon::Small);
  
  QRegExp regExp("",true,true);
  pProjects = m_pProjectSpace->allProjects();
  defaultFileGroups = m_pProjectSpace->defaultFileGroups();

  ProjectSpaceItem* pProjectSpaceItem = new ProjectSpaceItem(this);
  pProjectSpaceItem->setOpen(true);  
  pProjectSpaceItem->setText(0,"Projectspace '" + m_pProjectSpace->name() + "' : " 
			     + QString::number(pProjects->count()) + " project(s)");
  pProjectSpaceItem->setPixmap(0,projectSpacePixmap);
  
  
  for(pProject=pProjects->first();pProject !=0;pProject=pProjects->next()){ // projects
    cerr << "kdevelop (projectview): project found" << endl;
    pProjectItem = new ProjectItem(pProjectSpaceItem);
    pProjectItem->setText(0,pProject->name() + " files");
    pProjectItem->setPixmap(0,projectPixmap);
    if (pProject == m_pProjectSpace->currentProject()) {
      pProjectItem->setOpen(true);
      pProjectItem->setBold(true);
    }
    
    fileNames = pProject->allAbsoluteFileNames();
    for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it ) {
      //      cerr << "FILEPROJECTVIEW:" << *it << endl;
    }
    fileGroups = m_projectFileGroups[pProject->name()];
    if (fileGroups.isEmpty()){ // if it wasn't found
      fileGroups = defaultFileGroups;
    }    
    for(pFileGroup=fileGroups.first();pFileGroup !=0
	  ;pFileGroup=fileGroups.next()){ // groups
      
      pGroupItem = new GroupItem(pProjectItem);
      pGroupItem->setText(0,pFileGroup->name());
      pGroupItem->setPixmap(0,folderPixmap);
      filters = QStringList::split(';',pFileGroup->filter());
      
      for ( QStringList::Iterator filterIt = filters.begin(); filterIt != filters.end(); ++filterIt ) {
	//cerr << "FILTER" << *filterIt << endl;
	regExp.setPattern(*filterIt);
	files = fileNames.grep(regExp);
	// files
	for ( QStringList::Iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
	  //cerr << "FILE" << *fileIt << endl;
	  fileInfo.setFile(*fileIt);
	  pFileItem = new FileItem(pGroupItem);
	  pFileItem->setText(0,fileInfo.fileName());
	  pFileItem->setPixmap(0,KMimeType::pixmapForURL(*fileIt,0,KIcon::Small));
	  pFileItem->setAbsFileName(*fileIt);
	  pFileItem->setProjectName(pProject->name());
	  fileNames.remove(*fileIt);// remove the showed files from the filelist
	}
      } // end for filter
    } // end for groups

    // rest files
    for ( QStringList::Iterator fileIt = fileNames.begin(); fileIt != fileNames.end(); ++fileIt ) {
      //      cerr << "FILE" << *fileIt << endl;
      fileInfo.setFile(*fileIt);
      pFileItem = new FileItem(pProjectItem);
      pFileItem->setText(0,fileInfo.fileName());
      pFileItem->setPixmap(0,KMimeType::pixmapForURL(*fileIt,0,KIcon::Small));
      pFileItem->setAbsFileName(*fileIt);
    }
  } // end for projects
  
}
void ProjectTreeWidget::readProjectSpaceGlobalConfig(QDomDocument& doc){
  cerr << "kdevelop (projectview/projecttreewidget): readProjectSpaceGlobalConfig:" << endl;
  m_projectFileGroups.clear();
  QDomNodeList projectViewList = doc.elementsByTagName("ProjectView");

  if (projectViewList.count() != 0){ // ProjectView found
    QDomElement projectViewElement = projectViewList.item(0).toElement();
    QDomNodeList projectList = projectViewElement.elementsByTagName("Project");
    
    for (unsigned int i = 0; i < projectList.count(); ++i){
      QDomElement projectElement = projectList.item(i).toElement();
      QDomNodeList groupList = projectElement.elementsByTagName("FileGroup");
      QList<FileGroup> list;

      for (unsigned int n = 0; n < groupList.count(); ++n){
	QDomElement groupElement = groupList.item(n).toElement();
	FileGroup* pGroup = new FileGroup;
	pGroup->setName(groupElement.attribute("name"));
	pGroup->setFilter(groupElement.attribute("filter"));
	list.append(pGroup);
	//	cerr << "GROUP " << groupElement.attribute("name") << " add to " << projectElement.attribute("name") << endl;
      } // end for groups
      m_projectFileGroups.insert(projectElement.attribute("name"),list);
    } // end for project
  } // end if Projectview
}

void ProjectTreeWidget::writeProjectSpaceGlobalConfig(QDomDocument& doc){
  cerr << "kdevelop (projectview/projecttreewidget): writeProjectSpaceGlobalConfig" << endl;
  QDomElement rootElement = doc.documentElement();
  QDomElement projectViewElement = doc.createElement("ProjectView");
  QString projectName;
  QList<FileGroup> fileGroups;
  FileGroup* pFileGroup=0;
    

  QMap<QString,QList<FileGroup> >::Iterator it; 
  for( it = m_projectFileGroups.begin(); it != m_projectFileGroups.end(); ++it ){
    projectName = it.key();
    QDomElement projectElement = 
      projectViewElement.appendChild(doc.createElement("Project")).toElement();
    projectElement.setAttribute("name",projectName);
    fileGroups = m_projectFileGroups[projectName];
    for(pFileGroup=fileGroups.first();pFileGroup !=0;pFileGroup=fileGroups.next()){ // groups
      QDomElement groupElement = 
	projectElement.appendChild(doc.createElement("FileGroup")).toElement();
      groupElement.setAttribute("name",pFileGroup->name());
      groupElement.setAttribute("filter",pFileGroup->filter());
    }
  } // end for filegroup
  
  rootElement.appendChild( projectViewElement);
 
}

void ProjectTreeWidget::createDefaultFileGroups(){
  // create default one
  QList<Project>* pProjects=0;
  Project* pProject=0;
  cerr << "kdevelop (projectview/projecttreewidget): createDefaultFileGroups" << endl;
  if(m_pProjectSpace != 0) {
    QList<FileGroup>  defaultFileGroups = m_pProjectSpace->defaultFileGroups();
    pProjects = m_pProjectSpace->allProjects();
    for(pProject=pProjects->first();pProject !=0;pProject=pProjects->next()){ // projects
      QList<FileGroup> list;
      FileGroup* pFileGroup=0;
      for(pFileGroup=defaultFileGroups.first();pFileGroup !=0
	    ;pFileGroup=defaultFileGroups.next()){ // groups
	FileGroup* pGroup = new FileGroup;
	pGroup->setName(pFileGroup->name());
	pGroup->setFilter(pFileGroup->filter());
	list.append(pGroup);
      }
      m_projectFileGroups.insert(pProject->name(),list);
    }
  }
  else {
    cerr << "kdevelop (projectview/projecttreewidget): readProjectSpaceGlobalConfig: No ProjectSpace!!" << endl;
  } 
}
QPopupMenu* ProjectTreeWidget::createPopup(ProjectTreeItem* pItem){
  QPopupMenu *pPopup = new QPopupMenu();
  if(pItem->className() == QString("ProjectSpaceItem") ){
    pPopup->insertItem( i18n("Set active"),this, SLOT(slotGotoDeclaration()) );
  }
  if(pItem->className() == QString("ProjectItem") ){
    pPopup->insertItem( i18n("Set as Active Project"),this, SLOT(slotSetAsActiveProject()) );
    pPopup->insertSeparator();
    pPopup->insertItem( i18n("Properties..."),this, SLOT(slotProjectProperties()) );
  }
  if(pItem->className() == QString("GroupItem") ){
    pPopup->insertItem( i18n("New Folder..."),this, SLOT(slotNewGroup()) );
    pPopup->insertItem( i18n("Properties..."),this, SLOT(slotGroupProperties()) );
  }
  if(pItem->className() == QString("FileItem") ){
    pPopup->insertItem( i18n("Open"),this, SLOT(slotOpenFile()) );
    FileItem* pFileItem = static_cast<FileItem*>(pItem);
    // got the action from the part/kdevelopcomponent
    KDevFileNode* pNode = new KDevFileNode(pFileItem->absFileName(),
				       m_pProjectSpace->name(),pFileItem->projectName());
    QList<KAction>* pList = 
      m_pProjectView->assembleKDevNodeActions(pNode);
    
    KAction* pAction =0;
    for(pAction=pList->first();pAction!=0;pAction= pList->next()){
      pAction->plug(pPopup,-1);// add all available actions to the popupmenu
    }
  }
  return pPopup;
}

void ProjectTreeWidget::slotOpenFile(){
}

void ProjectTreeWidget::addedFileToProject(KDevFileNode* pNode){
  // this method isn't optimized, just a first try
  cerr << endl << "kdevelop (projectview): ProjectTreeWidget::addedFileToProject";

  QString projectName = pNode->projectName();
  QList<FileGroup> fileGroups;
  QString fileName= pNode->absoluteFileName();
  QFileInfo fileInfo(fileName);
  QStringList filters;
  fileGroups = m_projectFileGroups[pNode->projectName()];
  FileGroup* pFileGroup=0;
  QRegExp regExp("",true,true);
  QString foundFileGroup = "";
  
  for(pFileGroup=fileGroups.first();pFileGroup !=0
	;pFileGroup=fileGroups.next()){ // groups
    
    filters = QStringList::split(';',pFileGroup->filter());
    for ( QStringList::Iterator filterIt = filters.begin(); filterIt != filters.end(); ++filterIt ) {
      regExp.setPattern(*filterIt);
      if(fileName.contains(regExp) != 0){
	cerr << endl << "filter GROUP found" << pFileGroup->name() << endl;
	foundFileGroup = pFileGroup->name();
      }
    }
  }

  // ok, and now add it to the tree
  // first search the projectitem
  QListViewItem* pProjectSpaceItem = firstChild();
  QListViewItem* pProjectItem = pProjectSpaceItem->firstChild();
  QListViewItem* pTmpItem =0;
  QListViewItem* pGroupItem =0;
  
  // get the projectItem
  for(pTmpItem =pProjectItem;pTmpItem !=0;pTmpItem = pProjectSpaceItem->nextSibling()){
    if(pTmpItem->text(0) == projectName){
      pProjectItem = pTmpItem;
    }
  }
  if(foundFileGroup != ""){ // group found
    // ok found, now search the group in the tree 
    QListViewItemIterator it(pProjectItem); 
    for(;it.current();++it){
      if(it.current()->text(0) == foundFileGroup){
	GroupItem* pTmpGroupItem = static_cast<GroupItem*>(it.current());
	FileItem* pFileItem = new FileItem(pTmpGroupItem);
	pFileItem->setText(0,fileInfo.fileName());
	pFileItem->setPixmap(0,KMimeType::pixmapForURL(fileName,0,KIcon::Small));
	pFileItem->setAbsFileName(fileName);
	pFileItem->setProjectName(pNode->projectName());
	return; // only one file,done
      }
    }
  }
  else { // group found, add it to the end
    ProjectItem* pProject = static_cast<ProjectItem*>(pProjectItem);
    FileItem* pFileItem = new FileItem(pProject);
    pFileItem->setText(0,fileInfo.fileName());
    pFileItem->setPixmap(0,KMimeType::pixmapForURL(fileName,0,KIcon::Small));
    pFileItem->setAbsFileName(fileName);
  }
  
}

void ProjectTreeWidget::removedFileFromProject(KDevFileNode* pNode){
  cerr << endl << "kdevelop (projectview): ProjectTreeWidget::removeFileFromProject";
  QString projectName = pNode->projectName();
  QString fileName= pNode->absoluteFileName();

  QListViewItem* pProjectSpaceItem = firstChild();
  QListViewItem* pProjectItem = pProjectSpaceItem->firstChild();
  QListViewItem* pTmpItem=0;

  // get the projectItem
  for(pTmpItem =pProjectItem;pTmpItem !=0;pTmpItem = pProjectSpaceItem->nextSibling()){
    if(pTmpItem->text(0) == projectName){
      pProjectItem = pTmpItem;
    }
  }

  QListViewItemIterator it(pProjectItem);
  for(;it.current();++it){
    ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(it.current());
    if(pItem->inherits("FileItem")){
      FileItem* pFileItem = static_cast<FileItem*>(it.current());
      if(pFileItem->absFileName() == fileName){ // found
	delete pFileItem;
      }
    }
  }
}

void ProjectTreeItem::paintCell( QPainter * p, const QColorGroup & cg,
			     int column, int width, int align ){
  if(m_bold){
    QFont font = p->font();
    font.setBold(true);
    p->setFont(font);
  }
  QListViewItem::paintCell(p,cg,column,width,align);

}
void ProjectTreeItem::setBold(bool enable){
  m_bold = enable;
}
void FileItem::setAbsFileName(QString fileName){
  m_absFileName = fileName;
}
QString FileItem::absFileName(){
  return m_absFileName;
}
void FileItem::setProjectName(QString projectName){
  m_projectName = projectName;
}
QString FileItem::projectName(){
  return m_projectName;
}
