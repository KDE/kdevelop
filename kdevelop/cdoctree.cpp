/***************************************************************************
           cdoctree.cpp -
                             -------------------                                         

    begin                : 3 Oct 1998                                        
    copyright            : (C) 1998,1999 by Sandy Meier                         
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
#include <qfileinfo.h>
#include <kmsgbox.h>
#include <assert.h>
#include "cdoctreepropdlg.h"
#include "cdoctree.h"

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

CDocTree::CDocTree(QWidget*parent,const char* name,KConfig* config)
 : CTreeView(parent,name)
{
  config_kdevelop = config;

  // Create the popupmenus.
  initPopups();

  connect(this,
          SIGNAL(selectionChanged(QListViewItem*)),
          SLOT(slotSelectionChanged( QListViewItem *)));
}

CDocTree::~CDocTree()
{
}

/*--------------------------------------------- CDocTree::initPopups()
 * initPopups()
 *   Initialze all popupmenus.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CDocTree::initPopups()
{
  others_pop.setTitle(i18n("Folder"));
  others_pop.insertItem(i18n("Add Entry..."),this,SLOT(slotAddDocumentation()));
  
  doc_pop.setTitle(i18n("Others"));
  doc_pop.insertItem(i18n("Add Entry..."),this,SLOT(slotAddDocumentation()));
  doc_pop.insertItem(i18n("Remove Entry"),this,SLOT(slotRemoveDocumentation()));
  doc_pop.insertSeparator();
  doc_pop.insertItem(i18n("Properties..."),this,SLOT(slotDocumentationProp()));
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*------------------------------------------------ CDocTree::refresh()
 * refresh()
 *   Add all documents.
 *
 * Parameters:
 *   proj          The project specification.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CDocTree::refresh(CProject* prj)
{ 

  QListViewItem* top_item;
  QListViewItem* project_item;
  QListViewItem* others_item;
  QListViewItem* lib_item;
  QListViewItem* kdevelop_item;
  QStrList others_list;
  char *others_str;
  QString kde_path;
  QString chk_khtmlw_file;
  QString chk_kdeutils_file;

  project = prj;

  // Clear the tree.
  treeH->clear();
 
  // Add the top item.
  top_item = treeH->addRoot( i18n("Documentation"), THFOLDER );
  
  // kdevelop
  kdevelop_item = treeH->addItem( i18n("KDevelop"), THFOLDER, top_item );
  treeH->addItem( i18n("Manual"), THBOOK, kdevelop_item );
  treeH->addItem( i18n("Tutorial"), THBOOK, kdevelop_item );
  treeH->addItem( i18n("C/C++ Reference"), THBOOK, kdevelop_item );
  treeH->setLastItem( kdevelop_item );

  //Qt/KDE Libraries
  lib_item = treeH->addItem( i18n("Qt/KDE Libraries"), THFOLDER, top_item );
  
  config_kdevelop->setGroup("Doc_Location");
  kde_path=config_kdevelop->readEntry("doc_kde");

  treeH->addItem( i18n("Qt-Library"), THBOOK, lib_item );
  treeH->addItem( i18n("KDE-Core-Library"), THBOOK, lib_item );
  treeH->addItem( i18n("KDE-UI-Library"), THBOOK, lib_item );
  treeH->addItem( i18n("KDE-KFile-Library"), THBOOK, lib_item );

  // Check for khtml or khtmlw installed in documentation. 
  // If no documentation present, use khtml
  chk_khtmlw_file=kde_path+"khtmlw/index.html";
  if(QFileInfo(chk_khtmlw_file).exists())
    treeH->addItem( i18n("KDE-KHTMLW-Library"), THBOOK, lib_item );
  else
    treeH->addItem( i18n("KDE-KHTML-Library"), THBOOK, lib_item );

  treeH->addItem( i18n("KDE-KFM-Library"), THBOOK, lib_item );

  // Insert the kdeutils library if exists.
  chk_kdeutils_file=kde_path+"kdeutils/index.html";
  if(QFileInfo(chk_kdeutils_file).exists())
    treeH->addItem( i18n("KDE-KDEutils-Library"), THBOOK, lib_item );

  treeH->addItem( i18n("KDE-KAB-Library"), THBOOK, lib_item );
  treeH->addItem( i18n("KDE-KSpell-Library"), THBOOK, lib_item );
  treeH->setLastItem( lib_item );

  // Others
  others_item = treeH->addItem( i18n("Others"), THFOLDER, top_item );
  config_kdevelop->setGroup("Other_Doc_Location");
  config_kdevelop->readListEntry("others_list",others_list);
  for(others_str=others_list.first();
      others_str!=0;
      others_str=others_list.next())
  {
    treeH->addItem( others_str, THBOOK, others_item );
  }
  treeH->setLastItem( others_item );

  // current Project
  project_item = treeH->addItem( i18n("Current Project"), THFOLDER, top_item );
  
  // add the Project-Doc
  if(prj != 0){
      if(prj->valid)
	  {
	      treeH->addItem( i18n("API-Documentation"), THBOOK, project_item );
	      treeH->addItem( i18n("User-Manual"), THBOOK, project_item );
	  }
  }

  treeH->setLastItem( project_item );

  // Open all documentation folders.
  setOpen ( top_item,true );
  setOpen ( lib_item,true );
  setOpen ( kdevelop_item,true );
  setOpen ( others_item,true );
  setOpen ( project_item,true );
}

/** Get the current popupmenu. */
KPopupMenu *CDocTree::getCurrentPopup()
{
  KPopupMenu *popup = NULL;
  QListViewItem *item;

  item = currentItem();
  
  if( strcmp( item->text(0), i18n("Others") ) == 0 )
    popup = &others_pop;
  else if( item->parent() &&
           strcmp( item->parent()->text(0), i18n("Others") ) == 0 )
    popup = &doc_pop;

  return popup;
}

void CDocTree::slotAddDocumentation(){
  CDocTreePropDlg dlg;
  dlg.setCaption(i18n("Add Entry..."));
  QStrList others_list;
  int pos;
  if(dlg.exec()){
    config_kdevelop->setGroup("Other_Doc_Location");
    // add the entry to the list
    config_kdevelop->readListEntry("others_list",others_list);

    // find the correct place
    if(QString(currentItem()->text(0)) == i18n("Others") ){ 
      others_list.insert(0,dlg.name_edit->text());
    }
    else{
      pos = others_list.find(QString(currentItem()->text(0)));
      others_list.insert(pos+1,dlg.name_edit->text());
    }
    //write the list
    config_kdevelop->writeEntry("others_list",others_list);
    // write the props
    config_kdevelop->writeEntry(dlg.name_edit->text(),dlg.file_edit->text());
    config_kdevelop->sync();
    refresh(project);
  }
}

void CDocTree::slotRemoveDocumentation(){
  QString name = currentItem()->text(0);
  QStrList others_list;
  
  config_kdevelop->setGroup("Other_Doc_Location");
  config_kdevelop->readListEntry("others_list",others_list);
  others_list.remove(name);
  config_kdevelop->writeEntry("others_list",others_list);
  
  refresh(project);
}
void CDocTree::slotDocumentationProp(){
  QString name = currentItem()->text(0);
  config_kdevelop->setGroup("Other_Doc_Location");
  QString filename = config_kdevelop->readEntry(name);
  
  CDocTreePropDlg dlg;
  dlg.setCaption(i18n("Properties..."));
  dlg.name_edit->setText(name);
  dlg.name_edit->setEnabled(false);
  dlg.file_edit->setText(filename);
  
  if(dlg.exec()){
    config_kdevelop->setGroup("Other_Doc_Location");
    config_kdevelop->writeEntry(name,dlg.file_edit->text());
    config_kdevelop->sync();
  }
}

void CDocTree::slotSelectionChanged(QListViewItem* item){
    QString text = item->text(0);
    m_text = text;
    if (item->childCount() > 0) return; // no action
    
    KLocale *kloc = KApplication::getKApplication()->getLocale();
    
    QString strpath = KApplication::kde_htmldir().copy() + "/";
    QString file;
    
    config_kdevelop->setGroup("Doc_Location");
    
    if(text == i18n("Tutorial") ){
	// first try the locale setting
	file = strpath + kloc->language() + '/' + "kdevelop/tutorial.html";
	if( !QFileInfo( file ).exists() ){
	    // not found: use the default
	    file = strpath + "default/" + "kdevelop/tutorial.html";
	}
	
	emit fileSelected(file);
	return;
    }
    if(text == i18n("Manual") ){
	// first try the locale setting
	file = strpath + kloc->language() + '/' + "kdevelop/index.html";
	
	if( !QFileInfo( file ).exists() ){
	    // not found: use the default
	    file = strpath + "default/" + "kdevelop/index.html";
	}
	emit fileSelected(file);
	return;
    }
    if(text == i18n("C/C++ Reference") ){
	// first try the locale setting
	file = strpath + kloc->language() + '/' + "kdevelop/reference/C/cref.html";
	
	if( !QFileInfo( file ).exists() ){
	    // not found: use the default
	    file = strpath + "default/" + "kdevelop/reference/C/cref.html";
	}
	if( !QFileInfo( file ).exists() ){
	    // show the translated error page
	    file = strpath + kloc->language() + '/' + "kdevelop/cref.html";
	}
	if( !QFileInfo( file ).exists() ){
	    // not found: use the default error page
	    file = strpath + "default/" + "kdevelop/cref.html";
	}
	emit fileSelected(file);
	return;
    }
    if(text == i18n("Qt-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_qt") + "index.html");
	return;
    }
    if(text == i18n("KDE-Core-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "kdecore/index.html");
	return;
    }
    if(text == i18n("KDE-UI-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "kdeui/index.html");
	return;
    }
    if(text == i18n("KDE-KFile-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "kfile/index.html");
	return;
    }
    if(text == i18n("KDE-KHTMLW-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "khtmlw/index.html");
	return;
    }
    if(text == i18n("KDE-KHTML-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "khtml/index.html");
	return;
    }
    if(text == i18n("KDE-KFM-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "kfmlib/index.html");
	return;
    }
    if(text == i18n("KDE-KDEutils-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "kdeutils/index.html");
	return;
    }
    if(text == i18n("KDE-KAB-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "kab/index.html");
	return;
    }
    if(text == i18n("KDE-KSpell-Library") ){
	emit fileSelected(config_kdevelop->readEntry("doc_kde") + "kspell/index.html");
	return;
    }
    if(text == i18n("API-Documentation") ){
	emit fileSelected("API-Documentation");
	return;
    }
    if(text == i18n("User-Manual") ){
	emit fileSelected("User-Manual");
	return;
    }
    
    config_kdevelop->setGroup("Other_Doc_Location");
    QFileInfo file_info(config_kdevelop->readEntry(text));
    if(file_info.isFile()){
	emit fileSelected(config_kdevelop->readEntry(text));
    }
}
