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
  QListViewItem *current_item;

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
    current_item = treeH->addItem( info.classname,THC_FILE, top_item );
  }
  treeH->setLastItem( current_item );
  
  setOpen(top_item, true);
}
/** Initialize popupmenus. */
void KDlgDialogs::initPopups(){
}
  /** Get the current popupmenu. */
KPopupMenu* KDlgDialogs::getCurrentPopup(){
}
/*********************************************************************
 *                                                                   *
 *                              SLOTS                                *
 *                                                                   *
 ********************************************************************/ 
void KDlgDialogs::slotSelectionChanged( QListViewItem* item)
{
  if( mouseBtn == LeftButton && treeH->itemType() == THC_FILE )
    emit kdlgdialogsSelected(project->getProjectDir() + project->getSubDir() + 
			    QString(item->text(0)).lower() + ".kdevdlg");
}
