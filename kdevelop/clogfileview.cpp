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

#include "cgrouppropertiesdlg.h"
#include "cproject.h"
#include "ctreehandler.h"
#include "debug.h"
#include "resource.h"
#include "vc/versioncontrol.h"

#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kiconloader.h>

#include <qfileinfo.h>
#include <qlineedit.h>
#include <qregexp.h>

#include <assert.h>


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

CLogFileView::CLogFileView(bool s_path, QWidget*parent,const char* name)
  : CTreeView(parent,name)
{
  show_path = s_path;
  // Create the popupmenus.
  popup = 0;
  connect(this,SIGNAL(executed(QListViewItem*)),
          SLOT(slotSelectionChanged(QListViewItem *)));
  connect( this,SIGNAL(returnPressed(QListViewItem *)),
          SLOT(slotSelectionChanged(QListViewItem *)) );
  preselectitem = ""; // no preselect
  firstitemselect = false;
  allgroups_opened= false;
  dict = new QPtrDict <char>;
  dict->setAutoDelete(true);
}

CLogFileView::~CLogFileView(){
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
  QStringList groups;
  QStringList filters;
  QStrList temp_files;


  // get all opengroups
  QStringList opengroups;
  prj->getLFVOpenGroups(opengroups);

  QRegExp filter_exp("",true,true);// set Wildcard
  QString group_str;
  QString filter_str;
  QString temp_str;
  QString filename;
  QString* p_filename;
  bool item_already_selected = false;

  // Remove all entries.
  treeH->clear();
  dict->clear();
  // Not a valid project.
  if (!prj->isValid())
  {
    return;
  }

  project = prj;

  // Add the root item.
  top_item = treeH->addRoot( prj->getProjectName(), THPROJECT );

  prj->getLFVGroups(groups);
  prj->getAllFiles(files);

  for (QStringList::Iterator itGroups = groups.begin(); itGroups != groups.end(); ++itGroups ) {
    group_str = *itGroups;

    // Add the group item.
    lastGrp = treeH->addItem( group_str, THFOLDER, top_item );

    // Add files belonging to the group.
    prj->getFilters( group_str, filters );
    for (QStringList::Iterator itFilters = filters.begin(); itFilters != filters.end(); ++itFilters ) {
      filter_str = *itFilters;

      filter_exp.setPattern(filter_str);

      // Check every file
      for( filename = files.first();
           !filename.isEmpty();
           filename = files.next())
      {
        // If found
        if( filename.find( filter_exp ) != -1)
        {
          if(show_path) {
             current_item = treeH->addItem(filename, THC_FILE, lastGrp );
	  }
	  else{  // fill the dict, because the path is not stored into the tree
	    QString fname = QFileInfo("/"+filename).fileName();
      if (fname.isEmpty()) {
        continue; // avoid trying to call treeH->addItem
      }
	    current_item = treeH->addItem( fname , THC_FILE, lastGrp );
	    p_filename = new QString;
	    dict->insert(current_item,*p_filename = filename);
	  }
	  //	  cerr << ":" << current_item << ":" << filename << endl;
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

    if (lastGrp) {
      treeH->setLastItem(lastGrp);
      if(opengroups.contains(lastGrp->text(0)) > 0){
        setOpen(lastGrp, true);
      }
      if(allgroups_opened){
        setOpen(lastGrp, true);
      }
    }
  }

  setOpen(top_item, true);

  preselectitem =""; // no preselect on the next refresh
  popupmenu_disable = false;

  // Sort the damn list.
  setSorting (0, TRUE);
  sort ();
}
/** set the filename that will be selected after a refresh*/

void CLogFileView::setPreSelectedItem(const QString& rel_filename){
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
  
  if (popup)
    delete popup;
  
  switch( treeH->itemType() )
  {
    case THPROJECT:
      popup = new KPopupMenu(i18n("Project/LFV"));
      popup->insertItem( SmallIconSet("filenew"),i18n("New File..."),
                         this, SLOT(slotNewFile()), 0, ID_FILE_NEW);
      popup->insertItem( SmallIconSet("classnew"), i18n("New Class..."),
                         this, SLOT(slotNewClass()), 0, ID_PROJECT_NEW_CLASS );
      popup->insertItem( i18n("New Group..."), 
                         this, SLOT(slotNewGroup()), 0, ID_LFV_NEW_GROUP );
      popup->insertSeparator();
      popup->insertItem( i18n("Show relative path"),
                         this, SLOT(slotShowPath()), 0, ID_LFV_SHOW_PATH_ITEM );
      popup->setCheckable(true);
      if(show_path) popup->setItemChecked(ID_LFV_SHOW_PATH_ITEM, true);
      break;
    case THFOLDER:
      popup = new KPopupMenu(i18n("Group"));
      popup->insertItem( i18n("New Group..."),
                         this, SLOT(slotNewGroup()), 0, ID_LFV_NEW_GROUP );
      popup->insertSeparator();
      popup->insertItem( i18n("Remove Group"),
                         this, SLOT(slotGroupRemove()), 0, ID_LFV_REMOVE_GROUP );
      popup->insertSeparator();
      popup->insertItem( i18n("Properties..."),
                         this, SLOT(slotGroupProp()), 0, ID_LFV_GROUP_PROP );
      break;
    case THC_FILE:
      popup = new KPopupMenu(i18n("File"));
      popup->insertItem( SmallIconSet("filenew"), i18n("New File..."),
                         this,SLOT(slotNewFile()), 0, ID_FILE_NEW );
      popup->insertItem( i18n("Remove File From Project..."),
                         this,SLOT(slotFileRemove()), 0, ID_PROJECT_REMOVE_FILE );
      popup->insertItem( *(treeH->getIcon( THDELETE )), i18n("Remove File From Disk..."),
                         this, SLOT(slotFileDelete()), 0, ID_FILE_DELETE );
      popup->insertSeparator();
      popup->insertItem( i18n("Properties..."),
                         this, SLOT(slotFileProp()), 0, ID_PROJECT_FILE_PROPERTIES );

      if (project->getVersionControl())
          {
              VersionControl::State reg =
                  project->getVersionControl()->registeredState(getFullFilename(currentItem()));
              int id;
              popup->insertSeparator();
              id = popup->insertItem( i18n("Update"),
                                      this, SLOT(slotUpdate()) );
              popup->setItemEnabled(id, reg & VersionControl::canBeCommited);
              id = popup->insertItem( i18n("Commit"),
                                      this, SLOT(slotCommit()) );
              popup->setItemEnabled(id, reg & VersionControl::canBeCommited);
              id = popup->insertItem( i18n("Add to Repository"),
                                      this, SLOT(slotAddToRepository()) );
              popup->setItemEnabled(id, reg & VersionControl::canBeAdded);
              id = popup->insertItem( i18n("Remove From Repository (and Disk)"),
                                      this, SLOT(slotRemoveFromRepository()) );
              popup->setItemEnabled(id, !(reg & VersionControl::canBeAdded));
          }
      break;
    default:
      popup = 0;
      break;
  }

  if (popup)
      connect(popup, SIGNAL(highlighted(int)), SIGNAL(menuItemHighlighted(int)));
  return popup;
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CLogFileView::slotSelectionChanged( QListViewItem* item)
{
  if ( /*(mouseBtn == LeftButton || mouseBtn == MidButton)
       &&*/ treeH->itemType() == THC_FILE )
    emit logFileTreeSelected(getFullFilename(item));
}


void CLogFileView::slotNewClass(){
  emit selectedNewClass();
}


void CLogFileView::slotNewFile(){
  emit selectedNewFile();
}


void CLogFileView::slotFileProp()
{
  emit showFileProperties(getFileName(currentItem()));
}


void CLogFileView::slotGroupProp(){
  QStringList filters;
  QString filter_str,str;
  QString name = currentItem()->text(0);
  CGroupPropertiesDlg dlg;
  dlg.setCaption(i18n("Group Properties"));
  dlg.name_edit->setText(name);
  dlg.name_edit->setEnabled(false);
  
  project->getFilters(name,filters);
  for ( QStringList::Iterator itFilters = filters.begin(); itFilters != filters.end(); ++itFilters ) {
    str = *itFilters;
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
  QStringList filters;
  dlg.setCaption(i18n("New Group"));
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


void CLogFileView::slotFileRemove()
{
  QString filename=getFileName(currentItem());
  QString msg;
  msg.sprintf(i18n("Do you really want to remove the file\n%s from project?\nIt will remain on disk."), filename.data());
  if (KMessageBox::questionYesNo(0, msg, i18n("Warning")) == KMessageBox::No)
    return;

  emit selectedFileRemove(filename);
}


void CLogFileView::slotFileDelete()
{

  if(KMessageBox::questionYesNo(0,i18n("Do you really want to delete the selected file?\n        There is no way to restore it!"),
				i18n("Warning")) == KMessageBox::No)
    {
      return;
    }
  QString fullname = getFullFilename(currentItem());
  QString name = getFileName(currentItem());

  QFile::remove(fullname);

  emit selectedFileRemove(name);
  emit removeFileFromEditlist(fullname);
}


QString CLogFileView::getFileName(QListViewItem* item)
{
  QString name;
  if(show_path)
    name=item->text(0);
  else
    name=dict->find(item);
  return name;
}


QString CLogFileView::getFullFilename(QListViewItem* item)
{
  return project->getProjectDir() + getFileName(item);
}


void CLogFileView::slotGroupRemove(){
  QString name = currentItem()->text(0);
  project->removeLFVGroup(name);
  refresh(project);
  
}


void CLogFileView::split(QString str,QStringList& filters){
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
      filters.append(str2.local8Bit().data());
//      KDEBUG1(KDEBUG_INFO,CLOGFILEVIEW,"%s",str2.data());
    }
    pos = next+1;
  }
  //  str.findRev
  //  number = str.contains(','); // numbers of ,
  //   if(number == 0){
  //     filters.append(str.local8Bit().data());
  //     return ;
  //   }
  //   for(;number>0;--number){ // split the string into strings
  //     pos = str.find(',');
  //     str2 = str.left(pos+1);
  //     str.remove(0,pos+1);
  //     filters.append(str2.local8Bit().data());
  //     cerr << str2 << "\n";
  //     if(str.length() != 0){
  //       filters.append(str.local8Bit().data());
  //     }
  //   }
  return ;
}
void CLogFileView::storeState(CProject* prj){
  assert( prj != NULL );

  if(childCount() == 0) return; // save no empty tree
  QStringList opengroups;
  
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



/**  */
void CLogFileView::slotShowPath()
{
  show_path = !show_path;
  refresh(project);	
}


void CLogFileView::slotAddToRepository()
{
    project->getVersionControl()->add(getFullFilename(currentItem()));
}


void CLogFileView::slotRemoveFromRepository()
{
    QString fullfilename=getFullFilename(currentItem());
    QString name = getFileName(currentItem());

    project->getVersionControl()->remove(fullfilename);
    emit selectedFileRemove(name);
    emit removeFileFromEditlist(fullfilename);
}
 

void CLogFileView::slotUpdate()
{
   emit updateFileFromVCS(getFullFilename(currentItem()));
}


void CLogFileView::slotCommit()
{
    emit commitFileToVCS(getFullFilename(currentItem()));
}

#include "clogfileview.moc"
