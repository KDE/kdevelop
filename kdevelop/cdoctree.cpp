/***************************************************************************
           cdoctree.cpp -
                             -------------------                                         

    version              :                                   
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

CDocTree::CDocTree(QWidget*parent,const char* name) : KTreeList(parent,name){
  icon_loader = KApplication::getKApplication()->getIconLoader();
}

CDocTree::~CDocTree(){
}

void CDocTree::refresh(CProject* prj){ 
  clear();
 
  KPath path;
  QString str;
  QString file;
  QString str_path;
  QString str_sub_path;

  QPixmap folder_pix = icon_loader->loadMiniIcon("folder.xpm");
  QPixmap book_pix = icon_loader->loadMiniIcon("mini-book1.xpm");
  str = "Documentation";
  path.push(&str);   
  insertItem("Documentation",&folder_pix);
  addChildItem("Tutorial",&book_pix,&path);
  addChildItem("Manual",&book_pix,&path);

  
  addChildItem("Libraries",&folder_pix,&path);
  addChildItem("Project",&folder_pix,&path);
  
  //  add the Libraries
  str_path = "Libraries";
  path.push(&str_path);
  addChildItem("Qt-Library",&book_pix,&path);
  addChildItem("KDE-Core-Library",&book_pix,&path);
  addChildItem("KDE-UI-Library",&book_pix,&path);
  addChildItem("KDE-KFile-Library",&book_pix,&path);
  addChildItem("KDE-HTMLW-Library",&book_pix,&path);
  addChildItem("KDE-KFM-Library",&book_pix,&path);
  addChildItem("KDE-KAB-Library",&book_pix,&path);
  addChildItem("KDE-KSpell-Library",&book_pix,&path);

  // add the Project-Doc
  str_path = "Project";
  path.pop();
  path.push(&str_path);
  if(prj->valid){
    addChildItem("User-Manual",&book_pix,&path);
    addChildItem("API-Documentation",&book_pix,&path);  
  }
  
  setExpandLevel(1);
}

