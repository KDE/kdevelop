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


#include "qdir.h"
#include "qstrlist.h"
#include "crealfileview.h"
#include <kmsgbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <iostream.h>
#include <assert.h>

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
  // Create the popupmenus.
  initPopups();

  file_col = 0;
  
  connect(this, 
          SIGNAL(selectionChanged(QListViewItem*)), 
          SLOT(slotSelectionChanged(QListViewItem*)));
}

CRealFileView::~CRealFileView(){
}

/*---------------------------------------- CRealFileView::initPopups()
 * initPopups()
 *   Initialze all popupmenus.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CRealFileView::initPopups()
{
  installed_file_menu.setTitle(i18n("File (Registered)"));
  installed_file_menu.insertItem(i18n("Remove File from Project..."),this,SLOT(slotRemoveFileFromProject()));
  installed_file_menu.insertItem( *(treeH->getIcon( THDELETE )), i18n("Remove File from Disc..."),this,SLOT(slotDeleteFilePhys()));
  installed_file_menu.insertSeparator();
  installed_file_menu.insertItem(i18n("Properties..."),this,SLOT(slotShowFileProperties()));

  other_file_menu.setTitle(i18n("File"));
  other_file_menu.insertItem(i18n("Add File to Project..."),this,SLOT(slotAddFileToProject()));
  other_file_menu.insertItem( *(treeH->getIcon( THDELETE )), i18n("Remove File from Disc..."),this,SLOT(slotDeleteFilePhys()));
  //other_file_menu->insertSeparator();
  //other_file_menu->insertItem(i18n("Properties..."),this,SLOT(slotShowFileProperties()));
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
  assert( prj );

  project=prj;

  QString projectdir=project->getProjectDir();
  if(projectdir.right(1)=="/") {
    projectdir.truncate(projectdir.length()-1);
  }

  QDir dir(projectdir);
  if (!dir.exists()) {
    return;
  }

  // Remove all entries.
  treeH->clear();

  // Save all files registered in the project in the filelist variable.
  project->getAllFiles(filelist);

  // Add the root item.
  pRootItem = treeH->addRoot( projectdir, THFOLDER );
  pRootItem->setOpen(true);

  scanDir(projectdir, pRootItem);
}

void CRealFileView::addFilesFromDir( const QString& directory, 
                                     QListViewItem* parent )
{
  QDir theDir( directory );
  QStrList fileList;
  QListViewItem* item;

  // Add all files for this directory
  theDir.setFilter(QDir::Files);
  fileList=*(theDir.entryList());
  for( fileList.first();
       fileList.current();
       fileList.next() )
  {
    item = treeH->addItem( fileList.current(), THC_FILE, parent );
    
    // If this is an installed file, we change the icon.
    if( isInstalledFile( getRelFilename( item ) ) )
      item->setPixmap( file_col, *treeH->getIcon( THINSTALLED_FILE ) );
  }
}

void CRealFileView::scanDir(const QString& directory, QListViewItem* parent) 
{
  QString currentPath;
  QListViewItem* lastFolder;
  QStrList dirList;
  QDir dir(directory);

  // Stop recursion if the directory doesn't exist.
  if (!dir.exists()) {
    return;
  }
  
  dir.setSorting(QDir::Name);
  dir.setFilter(QDir::Dirs);
  dirList = *(dir.entryList());
  
  // Remove '.' and  '..'
  dirList.first();
  dirList.remove();
  dirList.remove();
  
  // Recurse through all directories
  while( dirList.current() ) 
  {
    lastFolder = treeH->addItem( dirList.current(), THFOLDER, parent );
    lastFolder->setOpen( true );
    
    // Recursive call to fetch subdirectories
    currentPath = directory+"//"+dirList.current();
    scanDir( currentPath, lastFolder );
    
    // Add the files in the recursed directory.
    //    addFilesFromDir( currentPath, lastFolder );
    
    treeH->setLastItem( lastFolder );
    
    dirList.next();
  } 
  
  // Add files in THIS directory as well.
  addFilesFromDir( directory, parent );
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
  KPopupMenu *popup;

  switch( treeH->itemType() )
  {
    case THINSTALLED_FILE:
      popup = &installed_file_menu;
      break;
    case THC_FILE:
      popup = &other_file_menu;
      break;
    default:
      popup = NULL;
      break;
  }

  return popup;
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

bool CRealFileView::isInstalledFile(QString filename) 
{
  return ( filelist.contains(filename) > 0 );
}

/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/

void CRealFileView::slotSelectionChanged(QListViewItem* selection) 
{
  if( mouseBtn == LeftButton && treeH->itemType() != THFOLDER ||
      mouseBtn == MidButton && treeH->itemType() != THFOLDER)
    emit fileSelected(getFullFilename(selection));
}

void CRealFileView::slotAddFileToProject() {

  QString filename=getFullFilename(currentItem());
  if(KMsgBox::yesNo(0,i18n("Question"),
                    i18n("Do you want to add the file:\n"+filename+"\n to the project ?"),
                    KMsgBox::QUESTION) == 2){
    return;
  }

  currentItem()->setPixmap( file_col, *treeH->getIcon( THINSTALLED_FILE ) );
  emit addFileToProject(filename);
}

void CRealFileView::slotRemoveFileFromProject() {

  QString filename=getRelFilename(currentItem());
  if(KMsgBox::yesNo(0,i18n("Warning"),i18n("Do you really want to remove the file from project?\n\t\tIt will remain on disk."),KMsgBox::EXCLAMATION) == 2){
    return;
  }
  emit removeFileFromProject(filename);
}

void CRealFileView::slotDeleteFilePhys() {

  QString filename=getRelFilename(currentItem());
  if(KMsgBox::yesNo(0,i18n("Warning"),i18n("Do you really want to delete the selected file?\n        There is no way to restore it!"),KMsgBox::EXCLAMATION) == 2){
    return;
  }
  filename = getFullFilename(currentItem());
  QFile::remove(filename);

  filename=getRelFilename(currentItem());
  if (isInstalledFile(filename)) {
    emit removeFileFromProject(filename);
}
  refresh(project);
}

void CRealFileView::slotShowFileProperties() {
  emit showFileProperties(getRelFilename(currentItem()));
}
