/***************************************************************************
                    noslot.cpp - some no slot functions in CKDevelop
                             -------------------                                         

    version              :                                   
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
#include "ckdevelop.h"
#include <iostream.h>
#include <kmsgbox.h>
#include "debug.h"

void CKDevelop::refreshTrees(){
  doc_tree->refresh(prj);
  if (!project){
    return; // no project
  }
  class_tree->refresh(prj);
  refreshClassCombos();
  log_file_tree->refresh(prj);
  real_file_tree->refresh(prj->getProjectDir());
  real_file_tree->setExpandLevel(1);
 
}
 

int CKDevelop::getTabLocation(QString filename){
  if(filename.right(4) == ".cpp" || filename.right(3) == ".cc" || filename.right(2) == ".C"
     || filename.right(2) == ".c"){
    return CPP;
  }
  else{
    return HEADER;
  }
  
}
void CKDevelop::switchToFile(QString filename){
  QString test_name = edit_widget->getName();
  TEditInfo* info;
  TEditInfo* actual_info;

  // check if the file exists
  if(!QFile::exists(filename) && filename != "Untitled.h" && filename != "Untitled.cpp"){
    KMsgBox::message(this,i18n("Attention"),filename +"\n\nFile does not exist!");
    return;
  }
  
  // set the correct edit_widget
  if (getTabLocation(filename) == HEADER){
    edit_widget = header_widget;
    s_tab_view->setCurrentTab(HEADER);
    disableCommand(ID_BUILD_COMPILE_FILE);

    //output_widget->append("current edit_widget = header_widget");
  }
  else{
    edit_widget = cpp_widget;
    s_tab_view->setCurrentTab(CPP);
    enableCommand(ID_BUILD_COMPILE_FILE);
    //output_widget->append("current edit_widget = cpp_widget");
  }
  edit_widget->setFocus();
  if (filename == edit_widget->getName()){
    //    cerr << endl <<endl << "Filename:" << filename 
    // << "EDITNAME:" << edit_widget->getName() <<"no action---:" << endl;
    return;
  }
  
  // search the current file which would be changed
  
  for(actual_info=edit_infos.first();actual_info != 0;actual_info=edit_infos.next()){
    if (actual_info->filename == edit_widget->getName() ){
      break;
    }
  }
  
  if(actual_info == 0){
    KDEBUG(KDEBUG_FATAL,CKDEVELOP,"actual_info in switchtoFile() is NULL!!!!!");
  }
    // rescue the old file
  actual_info->text = edit_widget->text();
  actual_info->modified = edit_widget->isModified();
  actual_info->cursor_line = edit_widget->currentLine();
  actual_info->cursor_col = edit_widget->currentColumn();
  // output_widget->append("auszuwechseldes file:" + actual_info->filename);

  // already in the list ?
  for(info=edit_infos.first();info != 0;info=edit_infos.next()){
    if (info->filename == filename ) { // if found in list
      
      //      cerr << "******already****\n" << info->text << "**************\n";
      edit_widget->setText(info->text);
      edit_widget->setName(filename);
      edit_widget->toggleModified(info->modified);
      edit_widget->setCursorPosition(info->cursor_line,info->cursor_col);

      //      output_widget->append ("File: was was already there");
      setCaption("KDevelop " + version + ":  "+prj->getProjectName()+":  " + filename);
      return;
    }
  }
  // not found -> generate a new edit_info,loading
  
  // build a new info
  QFileInfo fileinfo(filename);
  info = new TEditInfo;
  
  info->id = menu_buffers->insertItem(fileinfo.fileName(),-2,0); // insert at first index
  info->filename = filename.copy(); // a bugfix,that takes me 30 mins :-( (Sandy Meier)
  info->modified = false;
  info->cursor_line = 0;
  info->cursor_col = 0;

  // update the widget
  KDEBUG1(KDEBUG_INFO,CKDEVELOP,"switchToFile: %s",filename.data());
  edit_widget->clear();
  edit_widget->loadFile(filename,1);
  edit_widget->setName(filename);
  edit_widget->setFocus();
  info->text = edit_widget->text();
  edit_infos.append(info); // add to the list
  if(project){
    setCaption("KDevelop " + version + ":  "+prj->getProjectName()+":  "+ filename);
  }
  else{
    setCaption("KDevelop " + version + ": "+ filename);
  }
  
 
}

void CKDevelop::setToolMenuProcess(bool enable){

  if (enable){
    if(s_tab_view->getCurrentTab() == CPP){
      enableCommand(ID_BUILD_COMPILE_FILE);
    }
    enableCommand(ID_BUILD_RUN);
    enableCommand(ID_BUILD_DEBUG);
    enableCommand(ID_BUILD_MAKE);
    enableCommand(ID_BUILD_REBUILD_ALL);
    enableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    enableCommand(ID_BUILD_DISTCLEAN);
    enableCommand(ID_BUILD_AUTOCONF);
    enableCommand(ID_BUILD_CONFIGURE);
    disableCommand(ID_BUILD_STOP);
    enableCommand(ID_BUILD_MAKE_PROJECT_API);
    enableCommand(ID_BUILD_MAKE_USER_MANUAL);
  }
  else {
    
    // set the popupmenus enable or disable
    disableCommand(ID_BUILD_COMPILE_FILE);
    disableCommand(ID_BUILD_RUN);
    disableCommand(ID_BUILD_DEBUG);
    disableCommand(ID_BUILD_MAKE);
    disableCommand(ID_BUILD_REBUILD_ALL);
    disableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    disableCommand(ID_BUILD_DISTCLEAN);
    disableCommand(ID_BUILD_AUTOCONF);
    disableCommand(ID_BUILD_CONFIGURE);
    enableCommand(ID_BUILD_STOP);
    disableCommand(ID_BUILD_MAKE_PROJECT_API);
    disableCommand(ID_BUILD_MAKE_USER_MANUAL);
  }
}























