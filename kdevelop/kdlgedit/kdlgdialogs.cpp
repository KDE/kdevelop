/***************************************************************************
                          kdlgdialogs.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Sandy Meier 
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


#include "kdlgdialogs.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/ 

KDlgDialogs::KDlgDialogs(QWidget *parent, const char *name ) : CTreeView(parent,name) {
  connect(this,
          SIGNAL(selectionChanged(QListViewItem*)),
          SLOT(slotSelectionChanged(QListViewItem *)));
  initPopups();
}
KDlgDialogs::~KDlgDialogs(){
}
/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/ 
void KDlgDialogs::refresh(CProject* prj){
  QListViewItem *top_item;
  QListViewItem *current_item =0;

  project = prj;

  // Remove all entries.
  treeH->clear();
  // Add the root item.
  top_item = treeH->addRoot( prj->getProjectName(), THPROJECT );


  QStrList top_dialogs;
  prj->getAllTopLevelDialogs(top_dialogs);
  QString str;
  TDialogFileInfo info;

  for(str = top_dialogs.first();str != 0;str = top_dialogs.next()){
    info = prj->getDialogFileInfo(str);
    current_item = treeH->addItem( info.rel_name,THC_FILE, top_item );
  }
  if (current_item) treeH->setLastItem( current_item );
  
  setOpen(top_item, true);
}
/** Initialize popupmenus. */
void KDlgDialogs::initPopups(){
  dialog_pop.setTitle(i18n("Dialogs"));
  dialog_pop.insertItem(i18n("New Dialog..."),this,SLOT(slotNewDialog()));
  
  dialog_pop2.setTitle(i18n("Dialogs"));
  dialog_pop2.insertItem(i18n("New Dialog..."),this,SLOT(slotNewDialog()));
  dialog_pop2.insertItem(i18n("Delete Dialog..."),this,SLOT(slotDeleteDialog()));
  
}
  /** Get the current popupmenu. */
KPopupMenu* KDlgDialogs::getCurrentPopup(){
  KPopupMenu *popup = NULL;
  
  switch( treeH->itemType() )
    {
    case THPROJECT:
       popup = &dialog_pop;
      break;
    case THFOLDER:
      //      popup = &group_pop;
      break;
    case THC_FILE:
      popup = &dialog_pop2;
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
void KDlgDialogs::slotSelectionChanged( QListViewItem* item)
{
  
  if( mouseBtn == LeftButton && treeH->itemType() == THC_FILE ||
  	mouseBtn == MidButton && treeH->itemType() == THC_FILE){
      // current_dialog = (project->getProjectDir() + project->getSubDir() 
// 			+ QString(item->text(0)).lower() + ".kdevdlg" );
    current_dialog = (project->getProjectDir() + QString(item->text(0)));
    
      emit kdlgdialogsSelected(current_dialog);
  }
}

void KDlgDialogs::slotNewDialog(){
  emit newDialog();
}
void KDlgDialogs::slotDeleteDialog(){
  QListViewItem* item = currentItem();
  if( item != 0)
    emit deleteDialog(QString(item->text(0)));
}
