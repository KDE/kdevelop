/***************************************************************************
           clogfileview.cpp - the logical file view,draw the contents of 
		           a projectfile into a tree
                             -------------------                                         

    version              :                                   
    begin                : 20 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#include "clogfileview.h"
#include <iostream.h>
#include <kmsgbox.h>
#include <qregexp.h>
#include <kprocess.h>
#include "cgrouppropertiesdlg.h"
#include "debug.h"

CLogFileView::CLogFileView(QWidget*parent,const char* name) : KTreeList(parent,name){
 
  icon_loader = KApplication::getKApplication()->getIconLoader();
  left_button = true;
  right_button = false;
  file_pop = new KPopupMenu();
  file_pop->setTitle("File:");
  file_pop->insertItem(i18n("New File..."),this,SLOT(slotNewFile()));
  file_pop->insertItem(i18n("Remove File"),this,SLOT(slotFileRemove()));
  file_pop->insertItem(i18n("Delete File..."),this,SLOT(slotFileDelete()));
  file_pop->insertSeparator();
  file_pop->insertItem(i18n("Properties..."),this,SLOT(slotFileProp()));
  group_pop = new KPopupMenu();
  
  group_pop->setTitle("LFV-Group:");
  group_pop->insertItem(i18n("New LFV-Group..."),this,SLOT(slotNewGroup()));
  group_pop->insertSeparator();
  group_pop->insertItem(i18n("Remove Group"),this,SLOT(slotGroupRemove()));
  group_pop->insertSeparator();
  group_pop->insertItem(i18n("Properties..."),this,SLOT(slotGroupProp()));

  project_pop = new KPopupMenu();
  
  project_pop->setTitle("Project:");
  project_pop->insertItem(i18n("New File..."),this,SLOT(slotNewFile()));
  project_pop->insertItem(i18n("New Class..."),this,SLOT(slotNewClass()));
  project_pop->insertItem(i18n("New LFV-Group..."),this,SLOT(slotNewGroup()));

  connect(this,SIGNAL(singleSelected(int)),SLOT(slotSingleSelected(int)));
}

CLogFileView::~CLogFileView(){
}

void CLogFileView::refresh(CProject* prj){ 
  clear();
  if (!prj->valid){
    return; // no correct project
  }
  project = prj;
  KPath path;
  QString filter_str;
  QString filename;
  QStrList files;
  QStrList groups;
  QStrList filters;
  QStrList temp_files;
  QRegExp filter_exp("",true,true);// set Wildcard
  QPixmap folder_pix = icon_loader->loadMiniIcon("folder.xpm");
  QPixmap source_pix = icon_loader->loadMiniIcon("c_src.xpm");
  QPixmap project_pix = icon_loader->loadMiniIcon("kwm.xpm");
  QString temp_str;
  QString* project_str = new QString;
  QString* group_str = new QString;
  
  *project_str = prj->getProjectName();
  path.push(project_str);  
  insertItem(*project_str,&project_pix);
  prj->getAllFiles(files);
  prj->getLFVGroups(groups);
 //  for(filename=files.first();filename!=0;filename = files.next())  { 
//     cerr << filename << ":" << endl;
//   }
  
  
  for(*group_str=groups.first();(*group_str)!=0;*group_str=groups.next()){ 
    // every group
    //    cerr << "GRUPPE:" << *group_str << endl;
    addChildItem(*group_str,&folder_pix,&path);
    path.push(group_str);
    prj->getFilters(*group_str,filters);
    for(filter_str=filters.first();filter_str!=0;filter_str=filters.next()){
      // every filter
      filter_exp=filter_str;
      //      cerr << "FILTER:"  << filter_str;
      for(filename=files.first();filename!=0;filename = files.next())  { 
	// check every file
	if(filename.find(filter_exp) != -1){ // if found
	  addChildItem(filename,&source_pix,&path);
	  temp_files.append(filename);
	  //	  cerr << "FILENAME:"  << filename << endl;
	}
      }
      // remove the saved files from the filelist
      for(temp_str = temp_files.first();temp_str!=0;temp_str=temp_files.next()){
	files.remove(temp_str);
      }
      temp_files.clear();
    }  
    path.pop();// and now the next group
  }
  
  setExpandLevel(2);
}


void CLogFileView::mousePressEvent(QMouseEvent* event){
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
  KTreeList::mousePressEvent(event); 
}

bool CLogFileView::isGroup(int index){
  KTreeListItem* current = itemAt(index);
  if(current == 0) return false;
  KTreeListItem* parent = current->getParent();
  if(parent == 0) return false;
  KTreeListItem* pparent = parent->getParent();
  if(pparent == 0) return false;  
  KTreeListItem* ppparent = pparent->getParent();
  if(ppparent == 0) return true;
  return false;
}
bool CLogFileView::isFile(int index){
  KTreeListItem* current = itemAt(index);
  if(current == 0) return false;
  KTreeListItem* parent = current->getParent();
  if(parent == 0) return false;
  KTreeListItem* pparent = parent->getParent();
  if(pparent == 0) return false;
  KTreeListItem* ppparent = pparent->getParent();
  if(ppparent == 0) return false;
  return true;
}
bool CLogFileView::leftButton(){
  return left_button;
}
bool CLogFileView::rightButton(){
  return right_button;
}
void CLogFileView::slotSingleSelected(int index){
  if(rightButton()){
    if(isFile(index)){
      file_pop->popup(this->mapToGlobal(mouse_pos));
    }
    else if(isGroup(index)){
      group_pop->popup(this->mapToGlobal(mouse_pos));
    } 
    else{
      project_pop->popup(this->mapToGlobal(mouse_pos));
    }
  }
}
void CLogFileView::slotNewClass(){
  emit selectedNewClass();
}
void CLogFileView::slotNewFile(){
  emit selectedNewFile();
}
void CLogFileView::slotFileProp(){
  emit selectedFileProp();
}
void CLogFileView::slotGroupProp(){
  QStrList filters;
  QString filter_str,str;
  QString name = getCurrentItem()->getText();
  CGroupPropertiesDlg dlg;
  dlg.setCaption(i18n("Group Properties..."));
  dlg.name_edit->setText(name);
  dlg.name_edit->setEnabled(false);
  
  project->getFilters(name,filters);
  for(str = filters.first();str !=0;str = filters.next()){
    filter_str = filter_str + str +",";
  }
  dlg.filters_edit->setText(filter_str);
  if(dlg.exec()){ // ok clicked
    filter_str = dlg.filters_edit->text(); // get text
    split(filter_str,filters); // fill the filterlist
    project->setFilters(name,filters);
    refresh(project);
  }
  
  //emit selectedGroupProp();
}
void CLogFileView::slotNewGroup(){
  CGroupPropertiesDlg dlg;
  QString current_group = getCurrentItem()->getText();
  QStrList filters;
  dlg.setCaption(i18n("New Group ..."));
  if(dlg.exec()){// if clicked ok
    QString ace_group;
    QString name = dlg.name_edit->text();
    if(!name.isEmpty()){
      project->addLFVGroup(name,current_group);
      split(dlg.filters_edit->text(),filters);
      project->setFilters(name,filters);
      refresh(project);
    }
  } 
}
void CLogFileView::slotFileRemove(){
  emit selectedFileRemove();
}
void CLogFileView::slotFileDelete(){

  if(KMsgBox::yesNo(0,i18n("Warning"),i18n("Do you really want to delete the selected file?\n        There is no way to restore it!"),KMsgBox::EXCLAMATION) == 2){
    return;
  }
  QString name = getCurrentItem()->getText();
  name = project->getProjectDir() + name;
  KShellProcess* proc = new KShellProcess;
  QFileInfo info(name);
  QString command = "rm -f " + name;
  //  cerr << "\n\n" << command << "\n\n";
  *proc << command;
  proc->start();
  
  emit selectedFileRemove();
  
}
void CLogFileView::slotGroupRemove(){
  QString name = getCurrentItem()->getText();
  project->removeLFVGroup(name);
  refresh(project);
  
}
void CLogFileView::split(QString str,QStrList& filters){
  int pos=0;
  int next=0;
  QString str2;
  filters.clear();
  if(str[str.length()-1] != ','){
    str = str + ',';
  }
  
  while(next != -1){
    next = str.find(',',pos);
    if(next != -1){
      str2 = str.mid(pos,next-pos);
      filters.append(str2);
      KDEBUG1(KDEBUG_INFO,CLOGFILEVIEW,"%s",str2.data());
    }
    pos = next+1;
  }
  //  str.findRev
  //  number = str.contains(','); // numbers of ,
  //   if(number == 0){
  //     filters.append(str);
  //     return ;
  //   }
  //   for(;number>0;--number){ // split the string into strings
  //     pos = str.find(',');
  //     str2 = str.left(pos+1);
  //     str.remove(0,pos+1);
  //     filters.append(str2);
  //     cerr << str2 << "\n";
  //     if(str.length() != 0){
  //       filters.append(str);
  //     }
  //   }
  return ;
}











