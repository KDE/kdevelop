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
#include <qheader.h>

CLogFileView::CLogFileView(QWidget*parent,const char* name) : QListView(parent,name){
 
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
  
  connect(this,SIGNAL(rightButtonPressed( QListViewItem *, const QPoint &, int )),SLOT(slotRightButtonPressed( QListViewItem *,const QPoint &,int)));
  connect(this,SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotSelectionChanged( QListViewItem *)));
  setRootIsDecorated(true);
  addColumn("JK");
  header()->hide();
  
}

CLogFileView::~CLogFileView(){
}

void CLogFileView::refresh(CProject* prj){ 
  clear();
  if (!prj->valid){
    return; // no correct project
  }
  setSorting(-1,false);

  project = prj;
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

  prj->getAllFiles(files);
  prj->getLFVGroups(groups);
  //  for(filename=files.first();filename!=0;filename = files.next())  { 
  //cerr << filename << ":" << endl;
  //}
  
  QListViewItem* top_item= new QListViewItem(this,prj->getProjectName());
  top_item->setPixmap(0,folder_pix);

  QListViewItem* group_item;
  QListViewItem* prev_group_item=0;
  QListViewItem* item;
  QListViewItem* prev_item=0;

  for(*group_str=groups.first();(*group_str)!=0;*group_str=groups.next()){ 
      // every group
      if(prev_group_item ==0){
	  group_item = new QListViewItem(top_item,*group_str);
	  group_item->setPixmap(0,folder_pix);
	  prev_group_item = group_item;
	  prev_item=0;
      }
      else{
	  group_item = new QListViewItem(top_item,prev_group_item,*group_str);
	  group_item->setPixmap(0,folder_pix);
	  prev_group_item = group_item;
	  prev_item=0;
      }
  
      prj->getFilters(*group_str,filters);
      for(filter_str=filters.first();filter_str!=0;filter_str=filters.next()){
	  // every filter
	  filter_exp=filter_str;
	  //      cerr << "FILTER:"  << filter_str;
	  for(filename=files.first();filename!=0;filename = files.next())  { 
	      // check every file
	      if(filename.find(filter_exp) != -1){ // if found
		  if(prev_item==0){
		      item = new QListViewItem(group_item,filename);
		      item->setPixmap(0,source_pix);
		      prev_item = item;
		  }
		  else{
		      item = new QListViewItem(group_item,prev_item,filename);
		      item->setPixmap(0,source_pix);
		      prev_item = item;
		  }
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
      setOpen ( group_item,true );
  }
  setOpen(top_item,true);
}
void CLogFileView::slotSelectionChanged( QListViewItem* item){
    emit logFileTreeSelected(item);
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
    QListView::mousePressEvent(event); 
}

bool CLogFileView::isGroup(QListViewItem* current){
    if(current == 0) return false;
    QListViewItem* parent = current->parent();
    if(parent == 0) return false;
    QListViewItem* pparent = parent->parent();
    if(pparent == 0) return true;  
    return false;
}
bool CLogFileView::isFile(QListViewItem* current){
    if(current == 0) return false;
    QListViewItem* parent = current->parent();
    if(parent == 0) return false;
    QListViewItem* pparent = parent->parent();
    if(pparent == 0) return false;
    return true;
}
bool CLogFileView::leftButton(){
  return left_button;
}
bool CLogFileView::rightButton(){
  return right_button;
}
void CLogFileView::slotRightButtonPressed( QListViewItem * item ,const QPoint &,int){
    if(isFile(item)){
	file_pop->popup(this->mapToGlobal(mouse_pos));
    }
    else if(isGroup(item)){
	group_pop->popup(this->mapToGlobal(mouse_pos));
    } 
    else{
	project_pop->popup(this->mapToGlobal(mouse_pos));
    }
    setCurrentItem(item);
    setSelected(item,true);
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
  QString name = currentItem()->text(0);
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
  QString current_group = currentItem()->text(0);
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
  QString name = currentItem()->text(0);
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
  QString name = currentItem()->text(0);
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












