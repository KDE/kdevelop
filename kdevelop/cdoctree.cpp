/***************************************************************************
           cdoctree.cpp -
                             -------------------                                         

    begin                : 3 Oct 1998                                        
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
#include <iostream.h>

#include <qfileinfo.h>
#include <qheader.h>

#include <kmsgbox.h>

#include "cdoctreepropdlg.h"
#include "cdoctree.h"

CDocTree::CDocTree(QWidget*parent,const char* name,KConfig* config) : QListView(parent,name){
  config_kdevelop = config;
  icon_loader = KApplication::getKApplication()->getIconLoader();
  left_button = true;
  right_button = false;
  others_pop = new KPopupMenu();
  others_pop->setTitle(i18n("Others:"));
  others_pop->insertItem(i18n("Add Entry..."),this,SLOT(slotAddDocumentation()));
  
  doc_pop = new KPopupMenu();
  doc_pop->setTitle(i18n("Others:"));
  doc_pop->insertItem(i18n("Add Entry..."),this,SLOT(slotAddDocumentation()));
  doc_pop->insertItem(i18n("Remove Entry"),this,SLOT(slotRemoveDocumentation()));
  doc_pop->insertSeparator();
  doc_pop->insertItem(i18n("Properties..."),this,SLOT(slotDocumentationProp()));
  connect(this,SIGNAL(rightButtonPressed( QListViewItem *, const QPoint &, int )),SLOT(slotRightButtonPressed( QListViewItem *,const QPoint &,int)));
  
  setRootIsDecorated(true);
  addColumn("JK");
  header()->hide();

}

CDocTree::~CDocTree(){
}
void CDocTree::mousePressEvent(QMouseEvent* event){
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
void CDocTree::refresh(CProject* prj){ 
  project = prj;
  //  setUpdatesEnabled( false );
  clear();
  setSorting(-1,false); // no sorting
 

  QPixmap folder_pix = icon_loader->loadMiniIcon("folder.xpm");
  QPixmap book_pix = icon_loader->loadMiniIcon("mini-book1.xpm");

  QListViewItem* top_item= new QListViewItem(this,i18n("Documentation"));
  top_item->setPixmap(0,folder_pix);
  
  // current Project
  QListViewItem* project_item = new QListViewItem(top_item,i18n("Current Project"));
  project_item->setPixmap(0,folder_pix);
  
  QListViewItem* item =0;

  // add the Project-Doc
  if(project){
    if(prj->valid){
      item = new QListViewItem(project_item,i18n("User-Manual"));
      item->setPixmap(0,book_pix);
      item = new QListViewItem(project_item,i18n("API-Documentation"));
      item->setPixmap(0,book_pix);
    }
  }

  // Others
  QListViewItem* others_item = new QListViewItem(top_item,i18n("Others"));
  others_item->setPixmap(0,folder_pix);

  config_kdevelop->setGroup("Other_Doc_Location");
  QStrList others_list;
  QString others_str;
  config_kdevelop->readListEntry("others_list",others_list);
  for(others_str=others_list.first();others_str !=0;others_str=others_list.next()){
    item = new QListViewItem(others_item,others_str);
    item->setPixmap(0,book_pix);
  }
  //Qt/KDE Libraries
  QListViewItem* lib_item = new QListViewItem(top_item,i18n("Qt/KDE Libraries"));
  lib_item->setPixmap(0,folder_pix);
  
  //check for khtml or khtmlw installed in documentation. If no documentation present, use khtml
  config_kdevelop->setGroup("Doc_Location");
  QString kde_path=config_kdevelop->readEntry("doc_kde");
  // check for library documentations
  QString chk_khtmlw_file=kde_path+"khtmlw/index.html";
  QString chk_kdeutils_file=kde_path+"kdeutils/index.html";
 
  item = new QListViewItem(lib_item,i18n("KDE-KSpell-Library"));
  item->setPixmap(0,book_pix);
  item = new QListViewItem(lib_item,i18n("KDE-KAB-Library"));
  item->setPixmap(0,book_pix);
  if(QFileInfo(chk_kdeutils_file).exists()){
    item = new QListViewItem(lib_item,i18n("KDE-KDEutils-Library"));
    item->setPixmap(0,book_pix);// also insert the kdeutils library if exists
  }
  item = new QListViewItem(lib_item,i18n("KDE-KFM-Library"));
  item->setPixmap(0,book_pix);
  if(QFileInfo(chk_khtmlw_file).exists()){
    item = new QListViewItem(lib_item,i18n("KDE-KHTMLW-Library"));
    item->setPixmap(0,book_pix);
  }
  else{
    item = new QListViewItem(lib_item,i18n("KDE-KHTML-Library"));
    item->setPixmap(0,book_pix); // else insert khtml per defaull
  }
  item = new QListViewItem(lib_item,i18n("KDE-KFile-Library"));
  item->setPixmap(0,book_pix);
  item = new QListViewItem(lib_item,i18n("KDE-UI-Library"));
  item->setPixmap(0,book_pix);
  item = new QListViewItem(lib_item,i18n("KDE-Core-Library"));
  item->setPixmap(0,book_pix);
  item = new QListViewItem(lib_item,i18n("Qt-Library"));
  item->setPixmap(0,book_pix);
 

  // kdevelop
  QListViewItem* kdevelop_item = new QListViewItem(top_item,i18n("KDevelop"));
  kdevelop_item->setPixmap(0,folder_pix);
  item = new QListViewItem(kdevelop_item,i18n("C/C++ Reference"));
  item->setPixmap(0,book_pix);
  item = new QListViewItem(kdevelop_item,i18n("Tutorial"));
  item->setPixmap(0,book_pix);
  item = new QListViewItem(kdevelop_item,i18n("Manual"));
  item->setPixmap(0,book_pix);

  setOpen ( top_item,true );
  setOpen ( lib_item,true );
  setOpen ( kdevelop_item,true );
  setOpen ( others_item,true );
  setOpen ( project_item,true );

  repaint();

}
void CDocTree::slotRightButtonPressed( QListViewItem *item,const QPoint &,int){
  
  if(item != 0){
    QString text= item->text(0);
    setCurrentItem(item);
    if(text == i18n("Others") ){
      others_pop->popup(this->mapToGlobal(mouse_pos));
     
    }
    else if(QString(item->parent()->text(0)) == i18n("Others")){
      doc_pop->popup(this->mapToGlobal(mouse_pos));
    }
  }
  
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




