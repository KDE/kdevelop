/***************************************************************************
                    noslot.cpp - some no slot functions in CKDevelop
                             -------------------                                         

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
#include "./kdlgedit/kdlgedit.h"
#include <iostream.h>
#include <kmsgbox.h>
#include <qprogressdialog.h>
#include "ckdevelop.h"
#include "debug.h"
#include "cclassview.h"
#include "kswallow.h"
#include "ctoolclass.h"
#include "./kdlgedit/kdlgdialogs.h"
#include "cdocbrowser.h"
#include "./kdlgedit/kdlgreadmedlg.h"


void CKDevelop::refreshTrees(){
  doc_tree->refresh(prj);
  if (!project){
    return; // no project
  }

  // Update the classview.
  slotStatusMsg(i18n("Scanning project files..."));
  statProg->show();
  class_tree->refresh(prj);
  statProg->reset();
  statProg->hide();

  // Update the classcombo.
  refreshClassCombo();

  // Update LFV.
  log_file_tree->storeState(prj);
  log_file_tree->refresh(prj);

  // Update RFV.
  real_file_tree->refresh(prj);

  kdlg_dialogs_view->refresh(prj);

  kdev_statusbar->repaint();
	
  // update the file_open_menu
  file_open_list=prj->getHeaders();
  QStrList sources=prj->getSources();
	uint j;
	for( j=0; j< sources.count(); j++){
		file_open_list.append(sources.at(j));
	}
	// create the file_open_popup for the toolbar
	file_open_popup->clear();
	uint i;
 	for ( i =0 ; i < file_open_list.count(); i++){
 		QFileInfo fileInfo (file_open_list.at(i));
  	file_open_popup->insertItem(fileInfo.fileName());
  }

  slotStatusMsg(i18n("Ready."));
}
 
/*------------------------------------------ CKDevelop::refreshTrees()
 * refreshTrees()
 *   Refresh the file trees with regard to a certain file.
 *
 * Parameters:
 *   aFilename   The relative filename.
 *   aType       Type of file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::refreshTrees(TFileInfo *info)
{
  if( project )
  {
    // If this is a sourcefile we parse it and update the classview.
    if( info->type == CPP_SOURCE || info->type == CPP_HEADER )
    {
      class_tree->addFile( prj->getProjectDir() + info->rel_name );
      refreshClassCombo();
    }

    // Update LFV.
    log_file_tree->storeState(prj);
    log_file_tree->refresh(prj);
    
    // Update RFV.
    real_file_tree->refresh(prj);
  }
  // refresh the file_open_list
  file_open_list=prj->getHeaders();
  QStrList sources=prj->getSources();
	uint j;
	for( j=0; j< sources.count(); j++){
		file_open_list.append(sources.at(j));
	}
	// create the file_open_popup for the toolbar
	file_open_popup->clear();
	uint i;
 	for ( i =0 ; i < file_open_list.count(); i++){
 		QFileInfo fileInfo (file_open_list.at(i));
  	file_open_popup->insertItem(fileInfo.fileName());
  }
}

void CKDevelop::switchToFile(QString filename, bool bForceReload){
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
    if(!CToolClass::searchInstProgram("ktranslator")){
			return;
		}
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("ktranslator "+ filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
  }
  //load ktranslator if clicked/loaded an po file
  if((filename).right(4) == ".gif" || (filename).right(4) == ".bmp"){
    if(!CToolClass::searchInstProgram("kpaint")){
			return;
		}
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("kpaint "+ filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
  }
  if((filename).right(3) == ".ps"){
    if(!CToolClass::searchInstProgram("kghostview")){
			return;
		}
    showOutputView(false);
    s_tab_view->setCurrentTab(TOOLS);
    swallow_widget->sWClose(false);
    swallow_widget->setExeString("kghostview "+ filename);
    swallow_widget->sWExecute();
    swallow_widget->init();
    return;
  }

  if(filename.right(8) == ".kdevdlg"){
    switchToKDlgEdit();
    kdlgedit->slotOpenDialog(filename);
    return;
  }
  
  // set the correct edit_widget
  if (CProject::getType(filename) == CPP_SOURCE){
    edit_widget = cpp_widget;
    s_tab_view->setCurrentTab(CPP);
    
    if(build_menu->isItemEnabled(ID_BUILD_MAKE))			
      enableCommand(ID_BUILD_COMPILE_FILE);
  }
  else{
    edit_widget = header_widget;
    s_tab_view->setCurrentTab(HEADER);
    disableCommand(ID_BUILD_COMPILE_FILE);
  }
  
  edit_widget->setFocus();
  if (!bForceReload && filename == edit_widget->getName()){
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
//    KDEBUG(KDEBUG_FATAL,CKDEVELOP,"actual_info in switchtoFile() is NULL!!!!!");
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
      if (bForceReload)
      {
	  QFileInfo fileinfo(filename);
               edit_widget->clear();
               edit_widget->loadFile(filename,1);
               info->modified=false;
               info->cursor_line=info->cursor_col=0;
               info->text = edit_widget->text();
      }
      else
      {
         edit_widget->setText(info->text);
      }
      edit_widget->setName(filename);
      edit_widget->toggleModified(info->modified);
      edit_widget->setCursorPosition(info->cursor_line,info->cursor_col);

      //      output_widget->append ("File: was was already there");
      setCaption(prj->getProjectName() +" - KDevelop " + version + " - [" + QFileInfo(filename).fileName()+"]");
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
//  KDEBUG1(KDEBUG_INFO,CKDEVELOP,"switchToFile: %s",filename.data());
  edit_widget->clear();
  edit_widget->loadFile(filename,1);
  edit_widget->setName(filename);
  edit_widget->setFocus();
  info->text = edit_widget->text();
  edit_infos.append(info); // add to the list
  if(project){
  	setCaption(prj->getProjectName()+" - KDevelop " + version + " - ["+ QFileInfo(filename).fileName()+"]");
 	}
 	else{
  	setCaption("KDevelop " + version + " - ["+ QFileInfo(filename).fileName()+"]");
  }

}

void CKDevelop::switchToFile(QString filename, int lineNo){
  switchToFile( filename, false);
  edit_widget->setCursorPosition( lineNo, 0 );
}

void CKDevelop::switchToKDevelop(){

  kdlg_caption = caption();
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
//   KConfig *config = kapp->getConfig();


  config->setGroup("KDlgEdit");
  if (config->readEntry("KDlgEdit_ShowReadme").lower() != "false")
    {
      KDlgReadmeDlg *readmedlg = new KDlgReadmeDlg(this);
      readmedlg->exec();

      if (!readmedlg->isShowAgain())
        config->writeEntry("KDlgEdit_ShowReadme","false");

      delete readmedlg;
    }


  kdev_caption = caption();
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
  
  if(!kdlg_tabctl->isTabEnabled("widgets_view")){
    kdlg_tabctl->setCurrentTab(1); // set Dialogs enabled if no dialog was choosen
  }
  this->setUpdatesEnabled(true);
  this->repaint();
}

void CKDevelop::setToolMenuProcess(bool enable){

  if (enable){
    if(s_tab_view->getCurrentTab() == CPP){
      enableCommand(ID_BUILD_COMPILE_FILE);
    }
    enableCommand(ID_BUILD_RUN);
    enableCommand(ID_BUILD_RUN_WITH_ARGS);
    enableCommand(ID_BUILD_DEBUG);
    enableCommand(ID_BUILD_MAKE);
    enableCommand(ID_BUILD_REBUILD_ALL);
    enableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    enableCommand(ID_BUILD_DISTCLEAN);
    enableCommand(ID_BUILD_AUTOCONF);
    enableCommand(ID_BUILD_CONFIGURE);
    if (prj->getProjectType() != "normal_kde" && prj->getProjectType() != "mini_kde"){
      disableCommand(ID_PROJECT_MESSAGES);
    }
    else{
      enableCommand(ID_PROJECT_MESSAGES);
    }
    disableCommand(ID_BUILD_STOP);
    enableCommand(ID_PROJECT_MAKE_PROJECT_API);
    enableCommand(ID_PROJECT_MAKE_USER_MANUAL);
		enableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
    if(bAutosave)
      saveTimer->start(saveTimeout); // restart autosaving if enabled after a process finished

  }
  else {
    
    // set the popupmenus enable or disable
    disableCommand(ID_BUILD_COMPILE_FILE);
    disableCommand(ID_BUILD_RUN_WITH_ARGS);
    disableCommand(ID_BUILD_RUN);
    disableCommand(ID_BUILD_DEBUG);
    disableCommand(ID_BUILD_MAKE);
    disableCommand(ID_BUILD_REBUILD_ALL);
    disableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
    disableCommand(ID_BUILD_DISTCLEAN);
    disableCommand(ID_BUILD_AUTOCONF);
    disableCommand(ID_BUILD_CONFIGURE);
    disableCommand(ID_PROJECT_MESSAGES);
    enableCommand(ID_BUILD_STOP);
    disableCommand(ID_PROJECT_MAKE_PROJECT_API);
    disableCommand(ID_PROJECT_MAKE_USER_MANUAL);
    disableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
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
				slotViewTTreeView();
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already unvisible){
      }
      else{
				slotViewTTreeView();
      }
    }
  }
}
void CKDevelop::showOutputView(bool show){
  if(bAutoswitch){
  	if(show){
	  	if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
  	 		return; // it's already visible
      }
      else{
				slotViewTOutputView();
      }
    }
    else{
      if(!view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; //it's already unvisible
      }
      else{
				slotViewTOutputView();
      }
    }
  }
}
void CKDevelop::readOptions(){
  config->setGroup("General Options");

	/////////////////////////////////////////
	// GEOMETRY
  QSize size=config->readSizeEntry("Geometry");
	if(!size.isEmpty())
		resize(size);
	else
		setGeometry(QApplication::desktop()->width()/2-400, QApplication::desktop()->height()/2-300, 800, 600);

	/////////////////////////////////////////
	// BAR STATUS
	KMenuBar::menuPosition kdev_menu_bar_pos=(KMenuBar::menuPosition)config->readNumEntry("KDevelop MenuBar Position", KMenuBar::Top);
	kdev_menubar->setMenuBarPos(kdev_menu_bar_pos);
	KMenuBar::menuPosition kdlg_menu_bar_pos=(KMenuBar::menuPosition)config->readNumEntry("KDlgEdit MenuBar Position", KMenuBar::Top);
	kdev_menubar->setMenuBarPos(kdlg_menu_bar_pos);


  KToolBar::BarPosition tool_bar_pos=(KToolBar::BarPosition)config->readNumEntry("ToolBar Position", KToolBar::Top);
  toolBar()->setBarPos(tool_bar_pos);
	bool std_toolbar=	config->readBoolEntry("show_std_toolbar", true);
	if(std_toolbar){
	  view_menu->setItemChecked(ID_VIEW_TOOLBAR, true);
    enableToolBar(KToolBar::Show,0);
  }
  else{
    enableToolBar(KToolBar::Hide,0);
  }
	// Browser Toolbar
  KToolBar::BarPosition browser_tool_bar_pos=(KToolBar::BarPosition)config->readNumEntry("Browser ToolBar Position", KToolBar::Top);
  toolBar(ID_BROWSER_TOOLBAR)->setBarPos(browser_tool_bar_pos);
	bool browser_toolbar=config->readBoolEntry("show_browser_toolbar",true);
	if(browser_toolbar){
	  view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR, true);
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);
  }
	
	// Dialogedit Toolbar	
  KToolBar::BarPosition kdlg_tool_bar_pos=(KToolBar::BarPosition)config->readNumEntry("KDlgEdit ToolBar Position", KToolBar::Top);
  toolBar(ID_KDLG_TOOLBAR)->setBarPos(kdlg_tool_bar_pos);
	bool kdlg_toolbar=config->readBoolEntry("show_kdlg_toolbar", true);
  if(kdlg_toolbar){
		kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_TOOLBAR,true);
    enableToolBar(KToolBar::Show,ID_KDLG_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide,ID_KDLG_TOOLBAR);
  }
	// Statusbar
	bool statusbar=config->readBoolEntry("show_statusbar",true);
	if(statusbar){
	  view_menu->setItemChecked(ID_VIEW_STATUSBAR, true);
    kdlg_view_menu->setItemChecked(ID_VIEW_STATUSBAR,true);
	}
	else{
		enableStatusBar();
	}
	
	/////////////////////////////////////////
	// Outputwindow, TreeView, KDevelop/KDlgEdit
	view->setSeparatorPos(config->readNumEntry("view_panner_pos",80));
	bool outputview= config->readBoolEntry("show_output_view", true);
	if(outputview){
	  view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW, true);
    kdlg_view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);
		toolBar()->setButton(ID_VIEW_OUTPUTVIEW, true);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_OUTPUTVIEW, true);
    output_view_pos=view->separatorPos();
	}
	else{
    output_view_pos=config->readNumEntry("output_view_pos", 80);
	}
	
  top_panner->setSeparatorPos(config->readNumEntry("top_panner_pos", 213));
	bool treeview=config->readBoolEntry("show_tree_view", true);
	if(treeview){
	  view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
    kdlg_view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
		toolBar()->setButton(ID_VIEW_TREEVIEW, true);
		toolBar(ID_KDLG_TOOLBAR)->setButton(ID_VIEW_TREEVIEW, true);
    tree_view_pos=top_panner->separatorPos();
	}
  else{
    tree_view_pos=config->readNumEntry("tree_view_pos", 213);
  }
	

  kdlg_top_panner->setSeparatorPos(config->readNumEntry("kdlg_top_panner_pos", 80));
	if(config->readBoolEntry("show_properties_view",true)){
	  kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_PROPVIEW,true);
    properties_view_pos=kdlg_top_panner->separatorPos();
	}	
  else{
    properties_view_pos=config->readNumEntry("properties_view_pos", 80);
  }


	/////////////////////////////////////////
	// RUNTIME VALUES AND FILES
  bAutosave=config->readBoolEntry("Autosave",true);
  saveTimeout=config->readNumEntry("Autosave Timeout",5*60*1000);
  saveTimer=new QTimer(this);
  connect(saveTimer,SIGNAL(timeout()),SLOT(slotFileSaveAll()));
  if(bAutosave){
    saveTimer->start(saveTimeout);
  }
  else{
    saveTimer->stop();
  }
  bAutoswitch=config->readBoolEntry("Autoswitch",true);
  bDefaultCV=config->readBoolEntry("DefaultClassView",true);
  make_cmd=config->readEntry("Make","make");
  //  make_with_cmd=config->readEntry("MakeWith","");

  config->setGroup("Files");
	doc_bookmarks_list.setAutoDelete(TRUE);
	doc_bookmarks_title_list.setAutoDelete(TRUE);
	
	config->readListEntry("doc_bookmarks",doc_bookmarks_list);
	config->readListEntry("doc_bookmarks_title",doc_bookmarks_title_list);
		
	uint i;
	for ( i =0 ; i < doc_bookmarks_title_list.count(); i++){
    doc_bookmarks->insertItem(Icon("mini/htlm.xpm"),doc_bookmarks_title_list.at(i));
  }
	
  QString filename;
  filename = config->readEntry("browser_file","");
  if(!filename.isEmpty()){
    slotURLSelected(browser_widget,filename,1,"test");
  }
  else{
		slotHelpContents();
  }

  bool switchKDevelop=config->readBoolEntry("show_kdevelop",true);  // if true, kdevelop, else kdialogedit
  if(switchKDevelop){
    switchToKDevelop();
  }
  else{
    switchToKDlgEdit();
  }
}

void CKDevelop::saveOptions(){
	
	config->setGroup("General Options");
	config->writeEntry("Geometry", size() );

  config->writeEntry("KDevelop MenuBar Position", (int)kdev_menubar->menuBarPos());
  config->writeEntry("KDlgEdit MenuBar Position", (int)kdlg_menubar->menuBarPos());
  config->writeEntry("ToolBar Position",  (int)toolBar()->barPos());
	config->writeEntry("Browser ToolBar Position", (int)toolBar(ID_BROWSER_TOOLBAR)->barPos());
	config->writeEntry("KDlgEdit ToolBar Position", (int)toolBar(ID_KDLG_TOOLBAR)->barPos());

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

  config->writeEntry("lfv_show_path",log_file_tree->showPath());

  config->writeEntry("Autosave",bAutosave);
  config->writeEntry("Autosave Timeout",saveTimeout);

  config->writeEntry("Make",make_cmd);

  config->setGroup("Files");
  config->writeEntry("browser_file",history_list.current());
	config->writeEntry("doc_bookmarks", doc_bookmarks_list);
	config->writeEntry("doc_bookmarks_title", doc_bookmarks_title_list);

	config->sync();
}

bool CKDevelop::queryExit(){
	saveOptions();
	return true;
}

bool CKDevelop::queryClose(){
  swallow_widget->sWClose(false);
  if(project){
	  config->setGroup("Files");
    config->writeEntry("project_file",prj->getProjectFile());
		config->writeEntry("cpp_file",cpp_widget->getName());
  	config->writeEntry("header_file",header_widget->getName());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
    if(!slotProjectClose()){ // if not ok,pressed cancel
      return false; //not close!
    }
  }
	return true;
}

void CKDevelop::readProperties(KConfig* sess_config){
  QString filename;
  filename = sess_config->readEntry("project_file","");

  QFile file(filename);
  if (file.exists()){
    if(!(readProjectFile(filename))){
      KMsgBox::message(0,filename,"This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\nPlease use only new generated projects!");
      refreshTrees();
    }
		else{
   	  QProgressDialog *progressDlg= new QProgressDialog(NULL, "progressDlg", true );
		  connect(class_tree,SIGNAL(setStatusbarProgressSteps(int)),progressDlg,SLOT(setTotalSteps(int)));
  		connect(class_tree,SIGNAL(setStatusbarProgress(int)),progressDlg,SLOT(setProgress(int)));
			progressDlg->setCaption(i18n("Starting..."));
   		progressDlg->setLabelText( i18n("Initializing last project...\nPlease wait...\n") );
   	  progressDlg->setProgress(0);
   	  progressDlg->show();
      refreshTrees();
			delete progressDlg;
		}
    filename = sess_config->readEntry("header_file",i18n("Untitled.h"));
    QFile _file(filename);

    if (QFile::exists(filename)){
      switchToFile(filename);

    }

    filename = sess_config->readEntry("cpp_file", i18n("Untitled.cpp"));
    if (QFile::exists(filename)){
      switchToFile(filename);
    }
  }
  else{
    refreshTrees(); // this refresh only the documentation tab,tree
  }
}

void CKDevelop::saveProperties(KConfig* sess_config){
	
  if(project){
		sess_config->writeEntry("project_file",prj->getProjectFile());
		sess_config->writeEntry("cpp_file",cpp_widget->getName());
  	sess_config->writeEntry("header_file",header_widget->getName());
    prj->setCurrentWorkspaceNumber(workspace);
    saveCurrentWorkspaceIntoProject();
    prj->writeProject();
	}	
	if(bAutosave)
		slotFileSaveAll();
	else{
	  TEditInfo* info;
	  for(info=edit_infos.first();info != 0;info=edit_infos.next()){
			if(info->modified){
				setUnsavedData ( true );
				break;
			}
		}
	}
}

bool  CKDevelop::isFileInBuffer(QString abs_filename){
  TEditInfo* info;
  for(info=edit_infos.first();info != 0;info=edit_infos.next()){
    if (info->filename == abs_filename ){
      return true;
    }
  }
  return false;
}












































