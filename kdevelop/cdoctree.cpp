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
#include "cdoctree.h"
#include <iostream.h>
#include <kmsgbox.h>
#include "cdoctreepropdlg.h"

CDocTree::CDocTree(QWidget*parent,const char* name,KConfig* config) : KTreeList(parent,name){
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
  connect(this,SIGNAL(singleSelected(int)),SLOT(slotSingleSelected(int)));

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
  KTreeList::mousePressEvent(event); 
}
void CDocTree::refresh(CProject* prj){ 
  project = prj;
  clear();
 
  KPath path;
  QString str;
  QString file;
  QString str_path;
  QString str_sub_path;

  QPixmap folder_pix = icon_loader->loadMiniIcon("folder.xpm");
  QPixmap book_pix = icon_loader->loadMiniIcon("mini-book1.xpm");
  str = i18n("Documentation");
  path.push(&str);   
  insertItem(i18n("Documentation"),&folder_pix);
  
  
  
  addChildItem(i18n("KDevelop"),&folder_pix,&path);
  addChildItem(i18n("Qt/KDE Libraries"),&folder_pix,&path);
  addChildItem(i18n("Others"),&folder_pix,&path);
  addChildItem(i18n("Current Project"),&folder_pix,&path);
  

  //  add KDevelop
  str_path = i18n("KDevelop");
  path.push(&str_path);
  addChildItem(i18n("Manual"),&book_pix,&path);
  addChildItem(i18n("Tutorial"),&book_pix,&path);
  addChildItem(i18n("C/C++ Reference"),&book_pix,&path);
  
  //  add the Libraries
  str_path = i18n("Qt/KDE Libraries");
  path.pop();
  path.push(&str_path);
  addChildItem(i18n("Qt-Library"),&book_pix,&path);
  addChildItem(i18n("KDE-Core-Library"),&book_pix,&path);
  addChildItem(i18n("KDE-UI-Library"),&book_pix,&path);
  addChildItem(i18n("KDE-KFile-Library"),&book_pix,&path);
  addChildItem(i18n("KDE-HTMLW-Library"),&book_pix,&path);
  addChildItem(i18n("KDE-KFM-Library"),&book_pix,&path);
  addChildItem(i18n("KDE-KAB-Library"),&book_pix,&path);
  addChildItem(i18n("KDE-KSpell-Library"),&book_pix,&path);

  // add the others
  str_path = i18n("Others");
  path.pop();
  path.push(&str_path);

  config_kdevelop->setGroup("Other_Doc_Location");
  QStrList others_list;
  QString others_str;
  config_kdevelop->readListEntry("others_list",others_list);
  for(others_str=others_list.first();others_str !=0;others_str=others_list.next()){
    addChildItem(others_str,&book_pix,&path);
  }
  
  
  // add the Project-Doc
  str_path = i18n("Current Project");
  path.pop();
  path.push(&str_path);
  if(project){
    if(prj->valid){
      addChildItem(i18n("User-Manual"),&book_pix,&path);
      addChildItem(i18n("API-Documentation"),&book_pix,&path);  
    }
  }
  
  setExpandLevel(2);
}
void CDocTree::slotSingleSelected(int index){
  if(right_button){
    //cerr << itemAt(index)->getText();
    if(QString(itemAt(index)->getText()) == i18n("Others") ){
      others_pop->popup(this->mapToGlobal(mouse_pos));
     
    }
    else if(QString(itemAt(index)->getParent()->getText()) == i18n("Others")){
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
    if(QString(getCurrentItem()->getText()) == i18n("Others") ){ 
      others_list.insert(0,dlg.name_edit->text());
    }
    else{
      pos = others_list.find(QString(getCurrentItem()->getText()));
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
  QString name = QString(getCurrentItem()->getText());
  QStrList others_list;

  config_kdevelop->setGroup("Other_Doc_Location");
  config_kdevelop->readListEntry("others_list",others_list);
  others_list.remove(name);
  config_kdevelop->writeEntry("others_list",others_list);
  
  refresh(project);
}
void CDocTree::slotDocumentationProp(){
  QString name = QString(getCurrentItem()->getText());
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

