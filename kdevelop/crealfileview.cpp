/***************************************************************************
                    crealfileview.cpp - 
                             -------------------                                         
                            
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

#include "crealfileview.h"

#include "ccvaddfolderdlg.h"
#include "clibpropdlgimpl.h"
#include "cproject.h"
#include "vc/versioncontrol.h"
#include "resource.h"
#include "ctreehandler.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kprocess.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>

//#include <assert.h>
//#include <iostream>
//using namespace std;

//#include <kapp.h>
//#include <qwidget.h>


/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/


/*------------------------------------- CRealFileView::CRealFileView()
 * CRealFileView()
 *   Constructor.
 *
 * Parameters:
 *   parent         Parent widget.
 *   name           The name of this widget.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CRealFileView::CRealFileView(QWidget*parent,const char* name)
  : CTreeView(parent,name)
{
	addColumn("");
	addColumn("");
	header()->hide();
	header()->setClickEnabled(false);

  // Create the popupmenus.
  popup = 0;
  showNonPrjFiles=true;
  file_col = 0;
  connect(this,
          SIGNAL(executed(QListViewItem*)),
          SLOT(slotSelectionChanged(QListViewItem*)));
  connect( this,
          SIGNAL(returnPressed(QListViewItem *)),
          SLOT(slotSelectionChanged(QListViewItem *)) );
}

CRealFileView::~CRealFileView(){
  if (popup)
      delete popup;
}


/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------ CRealFileView::refresh()
 * refresh()
 *   Add all files in the project directory.
 *
 * Parameters:
 *   proj          The project specification.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CRealFileView::refresh(CProject* prj) 
{
  // assert( prj );
  if (!prj)
	return;

  project=prj;
  showNonPrjFiles = project->getShowNonProjectFiles();

	if (showNonPrjFiles) {
		setColumnWidthMode(0,Maximum);
		setColumnWidthMode(1,Maximum);
		setColumnWidthMode(2,Maximum);
		setColumnWidth(0,0);
		setColumnWidth(1,0);
		setColumnWidth(2,0);
	} else {
		setColumnWidthMode(0,Maximum);
		setColumnWidthMode(1,Manual);
		setColumnWidthMode(2,Manual);
		setColumnWidth(0,0);
		setColumnWidth(1,0);
		setColumnWidth(2,0);
	}

  QString projectdir=project->getProjectDir();
  if(projectdir.right(1)=="/") {
    projectdir.truncate(projectdir.length()-1);
  }
  this->projectDir=projectdir;
  QDir dir(projectdir);
  if (!dir.exists()) {
    return;
  }

  // memorize which parts of the tree are open and also the selected item
  QStringList pathList = treeH->pathListOfAllOpenedItems();
  QString curSelectedPath = treeH->pathToSelectedItem();

  // Remove all entries.
  treeH->clear();

  // Save all files registered in the project in the filelist variable.
  project->getAllFiles(filelist);

  // Add the root item.
  pRootItem = treeH->addRoot( projectdir, THPROJECT );
  pRootItem->setOpen(true);

  scanDir(projectdir, pRootItem);

  // reopen the tree and select the item again
  treeH->openItems(pathList);
  treeH->activateItem(curSelectedPath);
}

void CRealFileView::addFilesFromDir( const QString& directory, QListViewItem* parent ) {

    QDir theDir( directory );
    QStringList fl;
    QListViewItem* item;
    QString f;
    QString d=directory;
    d.remove(0,projectDir.length());
    VersionControl* vc;
    VersionControl::State reg;

    // Add all files for this directory
    theDir.setFilter(QDir::Files);
    fl=theDir.entryList();
    
    for ( QStringList::Iterator it = fl.begin(); it != fl.end(); ++it )
    {
        f=d;
        f.append("/");
        f.append((*it));
        f.remove(0,1);
        if( isInstalledFile(f) )
        {
            item = treeH->addItem( QFile::encodeName(*it), THINSTALLED_FILE, parent);
            if (showNonPrjFiles)
            {
                item->setText(1,i18n("registered"));
                vc=project->getVersionControl();
                if (vc!=0)
                {
                    reg=vc->registeredState(QFile::encodeName(directory+'/'+(*it)));
                    if (reg & VersionControl::canBeCommited)
                        item->setText(2,i18n("VCS"));
                    else
                        item->setText(2,i18n("local"));
                }
            }
        }
        else
        {
            if (showNonPrjFiles)
            {
                item = treeH->addItem( QFile::encodeName(*it), THC_FILE, parent,"");
                vc=project->getVersionControl();
                if (vc!=0)
                {
                    reg=vc->registeredState(QFile::encodeName(directory+'/'+(*it)));
                    if (reg & VersionControl::canBeCommited)
                        item->setText(2,i18n("VCS"));
                    else
                        item->setText(2,i18n("local"));
                }
            }
        }
    }
}

void CRealFileView::scanDir(const QString& directory, QListViewItem* parent) 
{
  QString currentPath;
  QListViewItem* lastFolder;
  QStringList dirList;
  QDir dir(directory);

  // Stop recursion if the directory doesn't exist.
  if (!dir.exists()) {
    return;
  }

  dir.setSorting(QDir::Name);
  dir.setFilter(QDir::Dirs);
  dirList = dir.entryList();

  // Remove '.' and  '..'
  QStringList::Iterator it = dirList.begin();
  it = dirList.remove(it);
  it = dirList.remove(it);
  it = dirList.end();

  // Add files in THIS directory as well.
  addFilesFromDir( directory, parent );

  if (it == dirList.begin()) {
    return; // there aint subdirs
  }

  bool bLastLoopStep = false;
  // Recurse through all directories
  do {
    it--;
    if (it == dirList.begin()) {
      bLastLoopStep = true;
    }

    lastFolder = treeH->addItem( QFile::encodeName(*it), THFOLDER, parent );
    lastFolder->setOpen( false );

    // Recursive call to fetch subdirectories
    currentPath = directory+"/"+(*it);
    scanDir( currentPath, lastFolder );
    
    // Add the files in the recursed directory.
    //    addFilesFromDir( currentPath, lastFolder );
    
    treeH->setLastItem( lastFolder );
  }
  while (!bLastLoopStep);
}

/*----------------------------------- CRealFileView::getCurrentPopup()
 * getCurrentPopup()
 *   Get the current popupmenu.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
KPopupMenu *CRealFileView::getCurrentPopup()
{
  if (popup)
    delete popup;
  bool cvs=project->getVersionControl();
  QString dir_name = getFullFilename(currentItem());
  QString reldir_name = getRelFilename(currentItem());
  QString prjdir=project->getProjectDir()+"po";
  TMakefileAmInfo info=project->getMakefileAmInfo(reldir_name+"/Makefile.am");

  switch( treeH->itemType() )
  {
  case THPROJECT :
    popup = new KPopupMenu(i18n("RFV Options"));
    popup->insertItem( i18n("Show Non-project Files"),this,
        SLOT(slotShowNonPrjFiles()), 0, ID_RFV_SHOW_NONPRJFILES );
    popup->setCheckable(true);
    if(showNonPrjFiles) popup->setItemChecked(ID_RFV_SHOW_NONPRJFILES, true);

    popup->insertSeparator();
    popup->insertItem(i18n("Update all Makefile.am"), this, SLOT(slotUpdateMakefileAm()), 0, ID_PROJECT_UPDATE_AM);
    popup->setItemEnabled(ID_PROJECT_UPDATE_AM,
      project->getProjectType() != "normal_empty" && QFileInfo(dir_name+"/Makefile.am").exists());

    if (cvs)
    {
    	popup->insertSeparator();
      popup->insertItem( i18n("Update"),this, SLOT(slotUpdate()),0,ID_PROJECT_CVS_UPDATE );
    	popup->insertItem( i18n("Commit"), this, SLOT(slotCommit()),0,ID_PROJECT_CVS_COMMIT );
    	popup->insertItem( i18n("Add to Repository"),this,
    	    SLOT(slotAddToRepository()),0,ID_PROJECT_CVS_ADD );
    	popup->insertItem( i18n("Remove From Repository (and Disk)"), this,
    	    SLOT(slotRemoveFromRepository()),0,ID_PROJECT_CVS_REMOVE );
     }
    break;

  case THINSTALLED_FILE:
    popup = new KPopupMenu(i18n("File (Registered)"));
    popup->insertItem( i18n("Remove File From Project..."), this,
          SLOT(slotRemoveFileFromProject()),0,ID_PROJECT_REMOVE_FILE);
    popup->insertItem( *(treeH->getIcon( THDELETE )), i18n("Remove File From Disk..."),this,
          SLOT(slotDeleteFilePhys()),0,ID_FILE_DELETE);
    popup->insertSeparator();
    popup->insertItem( i18n("Properties..."),this,
          SLOT(slotShowFileProperties()),0,ID_PROJECT_FILE_PROPERTIES);
    break;

  case THC_FILE:
    popup = new KPopupMenu(i18n("File"));
    popup->insertItem( i18n("Add File to Project..."), this,
        SLOT(slotAddFileToProject()),0,ID_PROJECT_ADD_FILE_EXIST );
    popup->insertItem( *(treeH->getIcon( THDELETE )), i18n("Remove File from Disk..."),this,
        SLOT(slotDeleteFilePhys()),0,ID_FILE_DELETE);
    break;

  case THFOLDER:
    popup = new KPopupMenu(i18n("Folder"));
    popup->insertItem( SmallIconSet("filenew"),i18n("New File..."), this, SLOT(slotFileNew()),0, ID_FILE_NEW);
    popup->insertItem( SmallIconSet("classnew"),i18n("New Class..."), this, SLOT(slotClassNew()), 0, ID_PROJECT_NEW_CLASS);
    popup->insertSeparator();
    popup->insertItem( SmallIconSet("folder_new"),i18n("Add Folder..."), this, SLOT( slotFolderNew()),0, ID_CV_FOLDER_NEW);
//    popup->insertItem(i18n("Delete Folder..."), this, SLOT( slotFolderDelete()),0, ID_CV_FOLDER_DELETE);
    popup->insertSeparator();
    popup->insertItem(i18n("Update Makefile.am"), this, SLOT(slotUpdateMakefileAm()), 0, ID_PROJECT_UPDATE_AM);

    popup->insertItem(i18n("Make"), this, SLOT(slotMakeDir()), 0, ID_PROJECT_MAKE_DIR);
    popup->setItemEnabled(ID_PROJECT_MAKE_DIR,
      project->getProjectType() != "normal_empty" && QFileInfo(dir_name+"/Makefile").exists());
//    popup->insertItem(i18n("Change to static lib"), this, SLOT(slotChangeToStatic()), 0, ID_PROJECT_CHANGE_TO_STATIC);
//    popup->insertItem(i18n("Change to shared lib"), this, SLOT(slotChangeToShared()), 0, ID_PROJECT_CHANGE_TO_SHARED);
    popup->insertItem(i18n("Properties..."), this, SLOT(slotLibProperties()), 0, ID_PROJECT_LIB_PROPERTIES);
    if (cvs)
    {
    	popup->insertSeparator();
      popup->insertItem( i18n("Update"),this, SLOT(slotUpdate()),0,ID_PROJECT_CVS_UPDATE);
      popup->insertItem( i18n("Commit"),this, SLOT(slotCommit()),0,ID_PROJECT_CVS_COMMIT );
      popup->insertItem( i18n("Add to Repository"),this,
          SLOT(slotAddToRepository()),0,ID_PROJECT_CVS_ADD);
      popup->insertItem( i18n("Remove From Repository (and Disk)"),this,
          SLOT(slotRemoveFromRepository()),0,ID_PROJECT_CVS_REMOVE );

     }
    if(dir_name.contains(prjdir)){
      popup->setItemEnabled(ID_FILE_NEW, false);
      popup->setItemEnabled(ID_PROJECT_NEW_CLASS, false);
      popup->setItemEnabled(ID_CV_FOLDER_NEW, false);
    }

    popup->setItemEnabled(ID_PROJECT_UPDATE_AM, false);
//    popup->setItemEnabled(ID_PROJECT_CHANGE_TO_STATIC, false);
    popup->setItemEnabled(ID_PROJECT_LIB_PROPERTIES, false);

    if (project->getProjectType() != "normal_empty" && QFileInfo(dir_name+"/Makefile.am").exists())
    {
      popup->setItemEnabled(ID_PROJECT_UPDATE_AM, true);
//      if (info.type=="shared_library")
//       popup->setItemEnabled(ID_PROJECT_CHANGE_TO_STATIC, true);
      if (info.type=="static_library" || info.type=="shared_library")
       popup->setItemEnabled(ID_PROJECT_LIB_PROPERTIES, true);
    }
    break;
  default:
    popup = 0;
  }

  VersionControl *vc = project->getVersionControl();
  if ( (treeH->itemType() == THINSTALLED_FILE || treeH->itemType() == THC_FILE)
       && vc)
  {
    VersionControl::State reg = vc->registeredState(getFullFilename(currentItem()));
    int id;
    popup->insertSeparator();
    id = popup->insertItem( i18n("Update"), this, SLOT(slotUpdate()),0,ID_PROJECT_CVS_UPDATE );
    popup->setItemEnabled(id, reg & VersionControl::canBeCommited);
    id = popup->insertItem( i18n("Commit"),this, SLOT(slotCommit()),0,ID_PROJECT_CVS_COMMIT );
    popup->setItemEnabled(id, reg & VersionControl::canBeCommited);
    id = popup->insertItem( i18n("Add to Repository"),this, SLOT(slotAddToRepository()),0,ID_PROJECT_CVS_ADD );
    popup->setItemEnabled(id, reg & VersionControl::canBeAdded);
    id = popup->insertItem( i18n("Remove From Repository (and disk)"),this, SLOT(slotRemoveFromRepository()),0,ID_PROJECT_CVS_REMOVE);
    popup->setItemEnabled(id, !(reg & VersionControl::canBeAdded));
  }

  if (popup)
      connect(popup, SIGNAL(highlighted(int)), SIGNAL(menuItemHighlighted(int)));

  return popup;
}

QString CRealFileView::getRelFilename(QListViewItem* pItem) {

  QString filename = pItem->text(file_col);
  if (pItem != pRootItem) {
    pItem=pItem->parent();
  }
  while (pItem != pRootItem) {
    filename = QString(pItem->text(file_col)) + "/" + filename;
    pItem = pItem->parent();
  }
  return filename;
}

QString CRealFileView::getFullFilename(QListViewItem* pItem) {

  QString filename = pItem->text(file_col);
  while (pItem != pRootItem) {
    pItem = pItem->parent();
    filename = QString(pItem->text(file_col)) + "/" + filename;
  }
  return filename;
}

bool CRealFileView::isInstalledFile(QString filename) 
{
	return (filelist.contains(filename) > 0);
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CRealFileView::slotSelectionChanged(QListViewItem* selection) 
{
  THType itemType=treeH->itemType();

  if( itemType!=THFOLDER && itemType!=THPROJECT /*&&
	(mouseBtn == LeftButton || mouseBtn == MidButton)*/)
    emit fileSelected(getFullFilename(selection));
}

void CRealFileView::slotAddFileToProject() {

  QString filename=getFullFilename(currentItem());
  QString msg;
  msg.sprintf(i18n("Do you want to add the file\n%s\nto the project?"), filename.data());
  if (KMessageBox::questionYesNo(0, msg) == KMessageBox::No)
    return;

  emit addFileToProject(filename);
}

void CRealFileView::slotRemoveFileFromProject() {

  QString filename=getRelFilename(currentItem());
  QString msg = i18n("Do you really want to remove the file\n%1\nfrom project?\n\t\tIt will remain on disk.").arg(filename);
  if (KMessageBox::questionYesNo(0, msg) == KMessageBox::No)
    return;

  emit removeFileFromProject(filename);
}

void CRealFileView::slotDeleteFilePhys() {

  QString filename=getRelFilename(currentItem());
  QString fullfilename=getFullFilename(currentItem());
  QString msg = i18n("Do you really want to delete the file\n%1\nfrom the disk?\nThere is no way to restore it!").arg(filename);
  if(KMessageBox::questionYesNo(0, msg) == KMessageBox::No)
    return;

  QFile::remove(fullfilename);

  if (isInstalledFile(filename)) 
    emit removeFileFromProject(filename);
  else
    refresh(project);
  emit removeFileFromEditlist(fullfilename);
}

void CRealFileView::slotShowFileProperties() {
  emit showFileProperties(getRelFilename(currentItem()));
}


void CRealFileView::slotAddToRepository()
{
    project->getVersionControl()->add(getFullFilename(currentItem()));
    refresh(project);
}


void CRealFileView::slotRemoveFromRepository()
{
   QString fullfilename=getFullFilename(currentItem());
   QString filename = getRelFilename(currentItem());

   project->getVersionControl()->remove(fullfilename);
   if (isInstalledFile(filename))
     emit removeFileFromProject(filename);
   emit removeFileFromEditlist(fullfilename);
   refresh(project);
}
 

void CRealFileView::slotUpdate()
{
    QString file_dir_name = getFullFilename(currentItem());
    QFileInfo file_info(file_dir_name);
    if (file_info.isFile()){ 
      emit updateFileFromVCS(file_dir_name);
    }
    if(file_info.isDir()){
      emit updateDirFromVCS(file_dir_name);
      refresh(project);
    }
}


void CRealFileView::slotCommit()
{
    QString file_dir_name = getFullFilename(currentItem());
    QFileInfo file_info(file_dir_name);
    if (file_info.isFile()){ 
      emit commitFileToVCS(file_dir_name);
    }
    if(file_info.isDir()){
      emit commitDirToVCS(file_dir_name);
    }
}
 
void CRealFileView::slotShowNonPrjFiles() {
    if ( !project )
      return;
    showNonPrjFiles=!showNonPrjFiles;
    project->setShowNonProjectFiles( showNonPrjFiles );
    refresh(project);
}

/**  */
void CRealFileView::slotFileNew(){

  QString dir_name = getFullFilename(currentItem());
  emit selectedFileNew(dir_name);
}
/**  */
void CRealFileView::slotClassNew(){
  QString prjdir=project->getProjectDir();
  QString dir_name = getFullFilename(currentItem());

  dir_name.remove(0,prjdir.length());
  emit selectedClassNew(dir_name+"/");
}
/**  */
void CRealFileView::slotFolderNew(){

  CCVAddFolderDlg dlg;

  if( dlg.exec() )
  {
    QString dir_name = getFullFilename(currentItem())+"/"+dlg.folderEdit->text();
    QDir dir;
    dir.setPath(dir_name);
    if(!dir.exists())
      dir.mkdir(dir_name);
  }
  refresh(project);
}
/**  */
void CRealFileView::slotFolderDelete(){
    // That´s not as simple as it look like...
    //  you also have to delete this subdir from the project
    //  so please don´t activate this until you have fixed this - W. Tasin 2000/04/02
    KShellProcess childproc("/bin/sh");
    QString dir_name = getFullFilename(currentItem());
    childproc << "rm -rf " << dir_name;

    childproc.start(KProcess::Block, KProcess::Communication(KProcess::Stdout|KProcess::Stderr));
    refresh(project);
}

void CRealFileView::slotUpdateMakefileAm()
{
  QString dir_name = getFullFilename(currentItem());
  QString reldir_name = getRelFilename(currentItem());
  if (QFileInfo(dir_name+"/Makefile.am").exists())
  {
    if (reldir_name.left(1)=="/")
     project->updateMakefilesAm();
    else
     project->updateMakefileAm(reldir_name+"/Makefile.am");
  }
}

void CRealFileView::slotMakeDir()
{
  QString dir_name = getFullFilename(currentItem());
  QString reldir_name = getRelFilename(currentItem());
  if (QFileInfo(dir_name+"/Makefile").exists())
  {
    emit (makeDir(reldir_name));
  }
}

//void CRealFileView::slotChangeToStatic()
//{
//  QString dir_name = getFullFilename(currentItem());
//  QString reldir_name = getRelFilename(currentItem());
//  if (QFileInfo(dir_name+"/Makefile.am").exists())
//  {
//    project->createLibraryMakefileAm(reldir_name+"/Makefile.am", "static");
//    project->changeLibraryType(reldir_name+"/Makefile.am", "static");
//    project->updateMakefilesAm();
//    refresh(project);
//  }
//}
//
//void CRealFileView::slotChangeToShared()
//{
//  QString dir_name = getFullFilename(currentItem());
//  QString reldir_name = getRelFilename(currentItem());
//  if (QFileInfo(dir_name+"/Makefile.am").exists())
//  {
//    project->createLibraryMakefileAm(reldir_name+"/Makefile.am", "shared");
//    project->changeLibraryType(reldir_name+"/Makefile.am", "shared");
//    project->updateMakefilesAm();
//    refresh(project);
//  }
//}

void CRealFileView::slotLibProperties()
{
  QString reldir_name = getRelFilename(currentItem());
  TMakefileAmInfo info=project->getMakefileAmInfo(reldir_name+"/Makefile.am");

  CLibPropDlgImpl dlg(&info);
  int result = dlg.exec();
  if (result)
    project->refreshMakefileAm(&info);
}

#include "crealfileview.moc"
