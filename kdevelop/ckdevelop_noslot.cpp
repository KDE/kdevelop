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

#include <iostream.h>
#include <kmsgbox.h>
#include "ckdevelop.h"
#include "debug.h"
#include "cclassview.h"
#include "kswallow.h"
#include "ctoolclass.h"
#include "./kdlgedit/kdlgdialogs.h"
#include "./kdlgedit/kdlgedit.h"

void CKDevelop::refreshTrees(){
  doc_tree->refresh(prj);
  if (!project){
    return; // no project
  }
  class_tree->refresh(prj);
  refreshClassCombo();
  log_file_tree->storeState(prj);
  log_file_tree->refresh(prj);
  real_file_tree->refresh(prj);
  kdlg_dialogs_view->refresh(prj);
 
}
 

int CKDevelop::getTabLocation(QString filename){
  if(filename.right(4) == ".cpp" || filename.right(4) == ".CPP" || filename.right(2) == ".c"
     || filename.right(2) == ".C" || filename.right(3) == ".cc"|| filename.right(3) == ".CC"
     || filename.right(4) == ".cxx"|| filename.right(4) == ".CXX"  || filename.right(3) == ".ec"
     || filename.right(5) == ".ecpp"){
    return CPP;
  }
  else{
    return HEADER;
  }
  
}
void CKDevelop::switchToFile(QString filename){
  lastfile = edit_widget->getName();
  lasttab = s_tab_view->getCurrentTab();

  TEditInfo* info;
  TEditInfo* actual_info;

  // check if the file exists
  if(!QFile::exists(filename) && filename != "Untitled.h" && filename != "Untitled.cpp"){
    KMsgBox::message(this,i18n("Attention"),filename +"\n\nFile does not exist!");
    return;
  }
  // load kiconedit if clicked/loaded  an icon
  if((filename).right(4) == ".xpm"){
    if(!CToolClass::searchProgram("kiconedit")){
      return;
    }
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("kiconedit " + filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
  }
  //load ktranslator if clicked/loaded an po file
  if((filename).right(3) == ".po"){
    if(CToolClass::searchInstProgram("ktranslator")){
      showOutputView(false);
      s_tab_view->setCurrentTab(TOOLS);
      swallow_widget->sWClose(false);
      swallow_widget->setExeString("ktranslator "+ filename);
      swallow_widget->sWExecute();
      swallow_widget->init();
      return;
    }
  }

  if(filename.right(8) == ".kdevdlg"){
    switchToKDlgEdit();
    kdlgedit->slotOpenDialog(filename);
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

    if(build_menu->isItemEnabled(ID_BUILD_MAKE))			
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
  info->filename = filename.copy(); // a bugfix,that takes me 30 mins :-( -Sandy 
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

void CKDevelop::switchToFile(QString filename, int lineNo){
  switchToFile( filename );
  edit_widget->setCursorPosition( lineNo, 0 );
}

void CKDevelop::switchToKDevelop(){
  setCaption(kdev_caption);

  bKDevelop=true;
  this->setUpdatesEnabled(false);

  //////// change the mainview ////////
  kdlg_tabctl->hide();
  kdlg_top_panner->hide();
  s_tab_view->show();
  t_tab_view->show();

  top_panner->hide();
  top_panner->deactivate();
  top_panner->activate(t_tab_view,s_tab_view);// activate the top_panner
  top_panner->show();
  //////// change the bars ///////////
  kdlg_menubar->hide();
  kdev_menubar->show();
  setMenu(kdev_menubar);

  kdlg_statusbar->hide();
  kdev_statusbar->show();
  setStatusBar(kdev_statusbar);

  toolBar(ID_KDLG_TOOLBAR)->hide();
  toolBar()->show();
  toolBar(ID_BROWSER_TOOLBAR)->show();

  setKeyAccel();  // initialize Keys
  ///////// reset bar status ////////////
  if(view_menu->isItemChecked(ID_VIEW_STATUSBAR))
    kdev_statusbar->show();
  else
    kdev_statusbar->hide();

  if(view_menu->isItemChecked(ID_VIEW_TOOLBAR))
    enableToolBar(KToolBar::Show);
  else
    enableToolBar(KToolBar::Hide);

  if(view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR))
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  else
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);

  ///////// reset the views status ///////////////
  if(view_menu->isItemChecked(ID_VIEW_TREEVIEW))
    showTreeView();
  else
    showTreeView(false);
  if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW))
    showOutputView();
  else
    showOutputView(false);

  this->setUpdatesEnabled(true);
  this->repaint();

}
void CKDevelop::switchToKDlgEdit(){
  setCaption(kdlg_caption);
  bKDevelop=false;
  this->setUpdatesEnabled(false);
  //////// change the mainview ////////
  s_tab_view->hide();
  t_tab_view->hide();
  kdlg_tabctl->show();
  kdlg_top_panner->show();

  top_panner->hide();
  top_panner->deactivate();
  top_panner->activate(kdlg_tabctl,kdlg_top_panner);// activate the top_panner
  top_panner->show();

  //////// change the bars ///////////
  kdev_menubar->hide();
  kdlg_menubar->show();
  setMenu(kdlg_menubar);

  kdev_statusbar->hide();
  kdlg_statusbar->show();
  setStatusBar(kdlg_statusbar);

  toolBar()->hide();
  toolBar(ID_BROWSER_TOOLBAR)->hide();
  toolBar(ID_KDLG_TOOLBAR)->show();

  ///////// reset bar status ////////////
  if(kdlg_view_menu->isItemChecked(ID_VIEW_STATUSBAR))
    kdlg_statusbar->show();
  else
    kdlg_statusbar->hide();

  if(kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_TOOLBAR))
    enableToolBar(KToolBar::Show, ID_KDLG_TOOLBAR);
  else
    enableToolBar(KToolBar::Hide, ID_KDLG_TOOLBAR);

  setKeyAccel();  // initialize Keys

  // this toolbar toogle is for placing the panner devider correctly
  enableToolBar(KToolBar::Toggle, ID_KDLG_TOOLBAR);
  enableToolBar(KToolBar::Toggle, ID_KDLG_TOOLBAR);

  ///////// reset the views status ///////////////
  if(kdlg_view_menu->isItemChecked(ID_VIEW_TREEVIEW))
    showTreeView();
  else
    showTreeView(false);

  if(kdlg_view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW))
    showOutputView();
  else
    showOutputView(false);

  this->setUpdatesEnabled(true);
  this->repaint();
}

void CKDevelop::setToolMenuProcess(bool enable){

  if (enable){
    if(s_tab_view->getCurrentTab() == CPP){
      enableCommand(ID_BUILD_COMPILE_FILE);
    }
    enableCommand(ID_KDLG_BUILD_GENERATE);
    enableCommand(ID_BUILD_RUN);
    enableCommand(ID_BUILD_RUN_WITH_ARGS);
    enableCommand(ID_BUILD_SET_ARGS);
    enableCommand(ID_BUILD_DEBUG);
    enableCommand(ID_BUILD_MAKE);
    enableCommand(ID_BUILD_REBUILD_ALL);
    enableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    enableCommand(ID_BUILD_DISTCLEAN);
    enableCommand(ID_BUILD_AUTOCONF);
    enableCommand(ID_BUILD_CONFIGURE);
    if (prj->getProjectType() != "normal_kde" && prj->getProjectType() != "mini_kde"){
      disableCommand(ID_BUILD_MESSAGES);
    }
    else{
      enableCommand(ID_BUILD_MESSAGES);
    }
    disableCommand(ID_BUILD_STOP);
    enableCommand(ID_BUILD_MAKE_PROJECT_API);
    enableCommand(ID_BUILD_MAKE_USER_MANUAL);
	  if(bAutosave)
  	  saveTimer->start(saveTimeout); // restart autosaving if enabled after a process finished

  }
  else {
    
    // set the popupmenus enable or disable
    disableCommand(ID_KDLG_BUILD_GENERATE);
    disableCommand(ID_BUILD_COMPILE_FILE);
    disableCommand(ID_BUILD_RUN_WITH_ARGS);
    disableCommand(ID_BUILD_SET_ARGS);
    disableCommand(ID_BUILD_RUN);
    disableCommand(ID_BUILD_DEBUG);
    disableCommand(ID_BUILD_MAKE);
    disableCommand(ID_BUILD_REBUILD_ALL);
    disableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    disableCommand(ID_BUILD_DISTCLEAN);
    disableCommand(ID_BUILD_AUTOCONF);
    disableCommand(ID_BUILD_CONFIGURE);
    disableCommand(ID_BUILD_MESSAGES);
    enableCommand(ID_BUILD_STOP);
    disableCommand(ID_BUILD_MAKE_PROJECT_API);
    disableCommand(ID_BUILD_MAKE_USER_MANUAL);
    if(bAutosave)
    	saveTimer->stop();  // stop the autosaving if make or something is running
  }
}

void CKDevelop::switchToWorkspace(int id){
  workspace = id;
  if(id == 1){
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,true);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,false);
  }
  if(id == 2){
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,true);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,false);
  }
  if(id == 3){
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_1,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_2,false);
    project_menu->setItemChecked(ID_PROJECT_WORKSPACES_3,true);
  }
  TWorkspace ws = prj->getWorkspace(id);
  if(ws.show_output_view){
    showOutputView(true);
  }
  else{showOutputView(false);}
  
  if(ws.show_treeview){
    showTreeView(true);
  }
  else{showTreeView(false);}
}

void CKDevelop::showTreeView(bool show){
  if(bAutoswitch)
  {
    if(show){
      if(view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already visible){
      }
      else{
        top_panner->setSeparatorPos(tree_view_pos);
        view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already unvisible){
      }
      else{
        view_menu->setItemChecked(ID_VIEW_TREEVIEW,false);
        tree_view_pos=top_panner->separatorPos();
        top_panner->setSeparatorPos(0);
      }
    }
    QRect rMainGeom= top_panner->geometry();
    top_panner->resize(rMainGeom.width()+1,rMainGeom.height());
    top_panner->resize(rMainGeom.width(),rMainGeom.height());
  }
}
void CKDevelop::showOutputView(bool show){
  if(bAutoswitch){
    if(show){
      if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; // it's already visible
      }
      else{
        view->setSeparatorPos(output_view_pos);
        view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; //it's already unvisible
      }
      else{
        view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,false);
        output_view_pos=view->separatorPos();
        view->setSeparatorPos(100);
      }
    }
    QRect rMainGeom= view->geometry();
    view->resize(rMainGeom.width()+1,rMainGeom.height());
    view->resize(rMainGeom.width(),rMainGeom.height());
  }
}

void CKDevelop::closeEvent(QCloseEvent* e){
  config->setGroup("General Options");
  config->writeEntry("width",width());
  config->writeEntry("height",height());

  config->writeEntry("view_panner_pos",view->separatorPos());
  config->writeEntry("top_panner_pos",top_panner->separatorPos());
  config->writeEntry("kdlg_top_panner_pos",kdlg_top_panner->separatorPos());

  config->writeEntry("tree_view_pos",tree_view_pos);
  config->writeEntry("output_view_pos",output_view_pos);
  config->writeEntry("properties_view_pos", properties_view_pos);

  config->writeEntry("show_tree_view",view_menu->isItemChecked(ID_VIEW_TREEVIEW));
  config->writeEntry("show_output_view",view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW));
  config->writeEntry("show_properties_view",kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_PROPVIEW));

  config->writeEntry("show_std_toolbar",view_menu->isItemChecked(ID_VIEW_TOOLBAR));
  config->writeEntry("show_browser_toolbar",view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR));
  config->writeEntry("show_kdlg_toolbar",kdlg_view_menu->isItemChecked(ID_KDLG_VIEW_TOOLBAR));

  config->writeEntry("show_statusbar",view_menu->isItemChecked(ID_VIEW_STATUSBAR));
  config->writeEntry("LastActiveTab", s_tab_view->getCurrentTab());
  config->writeEntry("LastActiveTree", t_tab_view->getCurrentTab());

  config->writeEntry("show_kdevelop",bKDevelop);

  config->writeEntry("Autosave",bAutosave);
  config->writeEntry("Autosave Timeout",saveTimeout);

  config->writeEntry("Make",make_cmd);

  config->setGroup("Files");
  config->writeEntry("cpp_file",cpp_widget->getName());
  config->writeEntry("header_file",header_widget->getName());
  config->writeEntry("browser_file",history_list.current());

  config->setGroup("Files");
  config->writeEntry("project_file","");
  if(project){
    config->writeEntry("project_file",prj->getProjectFile());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
    if(!slotProjectClose()){ // if not ok,pressed cancel
      e->ignore();
      return; //not close!
    }
  }
  e->accept();
  swallow_widget->sWClose(false);

  config->sync();
  KDEBUG(KDEBUG_INFO,CKDEVELOP,"KTMainWindow::closeEvent()");
  KTMainWindow::closeEvent(e);
}



























































