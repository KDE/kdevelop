/***************************************************************************
           clogfileview.cpp - the logical file view,draw the contents of 
		           a projectfile into a tree
                             -------------------                                         

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
#include <assert.h>

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

CLogFileView::CLogFileView(QWidget*parent,const char* name)
  : CTreeView(parent,name)
{
  // Create the popupmenus.
  initPopups();
 
  connect(this,
          SIGNAL(selectionChanged(QListViewItem*)),
          SLOT(slotSelectionChanged(QListViewItem *)));

  preselectitem = ""; // no preselect
  firstitemselect = false;
  allgroups_opened= false;
}

CLogFileView::~CLogFileView(){
}

/*---------------------------------------- CLogFileView::initPopups()
 * initPopups()
 *   Initialze all popupmenus.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CLogFileView::initPopups()
{
  file_pop.setTitle("File");
  file_pop.insertItem(i18n("New File..."),this,SLOT(slotNewFile()));
  file_pop.insertItem(i18n("Remove File"),this,SLOT(slotFileRemove()));
  file_pop.insertItem(*(treeH->getIcon( THDELETE )),i18n("Delete File..."),this,SLOT(slotFileDelete()));
  file_pop.insertSeparator();
  file_pop.insertItem(i18n("Properties..."),this,SLOT(slotFileProp()));
  
  group_pop.setTitle("Group");
  group_pop.insertItem(i18n("New Group..."),this,SLOT(slotNewGroup()));
  group_pop.insertSeparator();
  group_pop.insertItem(i18n("Remove Group"),this,SLOT(slotGroupRemove()));
  group_pop.insertSeparator();
  group_pop.insertItem(i18n("Properties..."),this,SLOT(slotGroupProp()));

  project_pop.setTitle("Project");
  project_pop.insertItem(i18n("New File..."),this,SLOT(slotNewFile()));
  project_pop.insertItem(i18n("New Class..."),this,SLOT(slotNewClass()));
  project_pop.insertItem(i18n("New Group..."),this,SLOT(slotNewGroup()));
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------------- CLogFileView::refresh()
 * refresh()
 *   Add all files in the project.
 *
 * Parameters:
 *   proj          The project specification.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CLogFileView::refresh(CProject* prj)
{
  assert( prj != NULL );

  QListViewItem *lastGrp;
  QListViewItem *top_item;
  QListViewItem *current_item;
  QStrList files;
  QStrList groups;
  QStrList filters;
  QStrList temp_files;
  
  // get all opengroups
  QStrList opengroups;
  prj->getLFVOpenGroups(opengroups);
  
  QRegExp filter_exp("",true,true);// set Wildcard
  char *group_str;
  char *filter_str;
  char *temp_str;
  QString filename;
  bool item_already_selected = false;

  // Remove all entries.
  treeH->clear();

  // Not a valid project.
  if (!prj->valid)
  {
    return;
  }

  project = prj;
  
  // Add the root item.
  top_item = treeH->addRoot( prj->getProjectName(), THPROJECT );

  prj->getLFVGroups(groups);
  prj->getAllFiles(files);
  for(group_str = groups.first();
      group_str != NULL;
      group_str = groups.next())
  { 
    // Add the group item.
    lastGrp = treeH->addItem( group_str, THFOLDER, top_item );

    // Add files belonging to the group.
    prj->getFilters( group_str, filters );
    for( filter_str = filters.first();
         filter_str != 0;
         filter_str=filters.next())
    {
      filter_exp = filter_str;

      // Check every file
      for( filename = files.first();
           !filename.isEmpty();
           filename = files.next())
      { 
        // If found
        if( filename.find( filter_exp ) != -1)
        {
          current_item = treeH->addItem( filename, THC_FILE, lastGrp );
	  if(firstitemselect == true && item_already_selected == false){
	    setSelected(current_item,true);
	    item_already_selected = true;
	  }
	  if(filename == preselectitem) setSelected(current_item,true);
          temp_files.append(filename);
        }
      }

      // Remove the saved files from the filelist
      for(temp_str = temp_files.first();
          temp_str != NULL;
          temp_str = temp_files.next())
      {
        files.remove(temp_str);
      }

      temp_files.clear();
    }

    treeH->setLastItem( lastGrp );
    if(opengroups.contains(lastGrp->text(0)) > 0){
      setOpen( lastGrp, true );
    }
    if(allgroups_opened){
      setOpen( lastGrp, true );
    }
  }

  setOpen(top_item, true);

  preselectitem =""; // no preselect on the next refresh
  popupmenu_disable = false;
}
/** set the filename that will be selected after a refresh*/

void CLogFileView::setPreSelectedItem(QString rel_filename){
  preselectitem = rel_filename;
}
/** select the first item, after a refresh*/
void CLogFileView::setFirstItemSelected(){
  firstitemselect = true;
}

/** disabled the popupmenus, used in cfilepropdialog */
void CLogFileView::setPopupMenusDisabled(){
  popupmenu_disable = true;
}

/** Get the current popupmenu. */
KPopupMenu *CLogFileView::getCurrentPopup()
{
  if(popupmenu_disable == true) return 0; // popupmenu
  
  KPopupMenu *popup = NULL;
  
  switch( treeH->itemType() )
  {
    case THPROJECT:
      popup = &project_pop;
      break;
    case THFOLDER:
      popup = &group_pop;
      break;
    case THC_FILE:
      popup = &file_pop;
      break;
    default:
      break;
  }

  return popup;
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CLogFileView::slotSelectionChanged( QListViewItem* item)
{
  if( mouseBtn == LeftButton && treeH->itemType() == THC_FILE )
    emit logFileTreeSelected(project->getProjectDir() + item->text(0));
}

void CLogFileView::slotNewClass(){
  emit selectedNewClass();
}
void CLogFileView::slotNewFile(){
  emit selectedNewFile();
}
void CLogFileView::slotFileProp(){
  emit showFileProperties(currentItem()->text(0));
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
void CLogFileView::storeState(CProject* prj){
  assert( prj != NULL );

  if(childCount() == 0) return; // save no empty tree
  QStrList opengroups;
  
  QListViewItem* ch_grp_item = firstChild();
  if(ch_grp_item != 0){
    ch_grp_item = ch_grp_item->firstChild();
    if(ch_grp_item != 0){
      while(ch_grp_item){
	if(isOpen(ch_grp_item)){
	  opengroups.append(ch_grp_item->text(0));
	}
	ch_grp_item = ch_grp_item->nextSibling();
      }
    }
  }
  prj->setLFVOpenGroups(opengroups);
}
