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

#include <iostream.h>

#include <qprogressdialog.h>
#include <qsplitter.h>
#include <qlist.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <kmenubar.h>
#include <kglobal.h>
#include <kstddirs.h>

#include "ckdevelop.h"
#include "cclassview.h"
#include "ctoolclass.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
#include "clogfileview.h"
#include "crealfileview.h"
#include "ceditwidget.h"
#include "editorview.h"
#include "dlgedit/dialogview.h"
#include "dlgedit/dialogwidget.h"
#include "mdiframe.h"

#include "./kwrite/kwdoc.h"
#include "./kwrite/kguicommand.h"

void CKDevelop::addRecentProject(const char* file)
{
  if(recent_projects.find(file) == -1){
    if( recent_projects.count() < 5)
      recent_projects.insert(0,file);
    else{
      recent_projects.remove(4);
      recent_projects.insert(0,file);
    }
    recent_projects_menu->clear();
		uint i;
    for ( i =0 ; i < recent_projects.count(); i++){
      recent_projects_menu->insertItem(recent_projects.at(i), i);
    }
	}
}




/*---------------------------------------- CKDevelop::setMainCaption()
 * setMainCaption()
 *
 *  make the caption of the main window and sets the generated title
 *
 * Parameters:
 *
 * Returns:
 *    nothing
 *-----------------------------------------------------------------*/
void CKDevelop::setMainCaption(int tab_item){
    EditorView* editor_view = getCurrentEditorView();
    
    if (bKDevelop)
	{
	    switch(tab_item)
		{
		case BROWSER:
		    kdev_caption=browser_widget->currentTitle()+ " - KDevelop " + version ;
		    break;
		case TOOLS:
		    kdev_caption=QString(i18n("Tools")) +" - KDevelop " + version ;
		    break;
		default:
		    kdev_caption=(project) ? (const char *) (prj->getProjectName()+" - KDevelop ") : "KDevelop ";
		    if(editor_view != 0){
			kdev_caption+= version +
			    " - ["+ QFileInfo(editor_view->currentEditor()->getName()).fileName()+"] ";
			if (editor_view->currentEditor()->isModified())
			    {
				enableCommand(ID_FILE_SAVE);
				kdev_caption+= " *";
			    }
			else
			    {
				disableCommand(ID_FILE_SAVE);
			    }
			break;
		    }
		}
	}
    setCaption(kdev_caption);
    
}



/*---------------------------------------- CKDevelop::fileSaveAs()
 * fileSaveAs()
 *
 *  makes file "save As" handling
 *  and returns true if it succeeded
 *
 * Parameters:
 *  none
 * Returns:
 *   true if the file was saved
 *-----------------------------------------------------------------*/
bool CKDevelop::fileSaveAs(){

   QString name,oldName;
   int msg_result = KMessageBox::Ok;

   if (bAutosave) saveTimer->stop();
   
   oldName = editor_view->currentEditor()->getName();

   do {
       name = KFileDialog::getSaveFileName(oldName,0,this,oldName);
       if (name.isNull()){
	   if (bAutosave) {
	       saveTimer->start(saveTimeout);
	   }
	   return false;
       }
       if(QFile::exists(name)){
	   msg_result=KMessageBox::warningYesNoCancel(this,
						      i18n("\nThe file\n\n%1\n\nalready exists.\n"
							   "Do you want overwrite the old one?\n").arg(name));
	   
	   if (msg_result==KMessageBox::Cancel){
	       if (bAutosave)
		   saveTimer->start(saveTimeout);
	       return false;
	   } 
       }
   }
   while(msg_result == KMessageBox::No);
   
   editor_view->setWindowCaption(QFileInfo(name).fileName());
   editor_view->currentEditor()->setName(name);
   editor_view->currentEditor()->doSave();
  
   slotViewRefresh();
   if (bAutosave)
       saveTimer->start(saveTimeout);

   return true;
}


/*------------------------------------------ CKDevelop::refreshTrees()
 * refreshTrees()
 *   Refresh the all trees. Includes re-parsing all files.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::refreshTrees(){
  if (!project){
    return; // no project
  }

  // Update the classview.
  slotStatusMsg(i18n("Scanning project files..."));
	setCursor(KCursor::waitCursor());
  statProg->show();
  class_tree->refresh(prj);
  statProg->reset();
  statProg->hide();

  // Update the classcombo.
  CVRefreshClassCombo();

  // Update LFV.
  log_file_tree->storeState(prj);
  log_file_tree->refresh(prj);

  // Update RFV.
  real_file_tree->refresh(prj);

  kdev_statusbar->repaint();
  setCursor(KCursor::arrowCursor());	
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
	  CVRefreshClassCombo();
	}
      
      // Update LFV.
      log_file_tree->storeState(prj);
      log_file_tree->refresh(prj);
      
      // Update RFV.
      real_file_tree->refresh(prj);
      // update dialogs tree
      // -> Component
      //      kdlg_dialogs_view->refresh(prj);
      
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

/** one of the most important methods in KDevelop, 
    it creats new dialog/editor views if needed, otherwise set the focus
*/
void CKDevelop::switchToFile(QString filename, bool bForceReload, bool bShowModifiedBox){
    
    cerr << "enter switchtofile\n";
    filename =  QFileInfo(filename).absFilePath();

  // check if the file exists
  if(!QFile::exists(filename)){
    KMessageBox::sorry(this, i18n("File %1 does not exist!").arg(filename));
    return;
  }

  
  
  // load kiconedit if clicked/loaded  an icon
  if((filename).right(4) == ".xpm"){
    if(!CToolClass::searchProgram("kiconedit")){
      return;
    }
    showOutputView(false);
    //    s_tab_view->setCurrentTab(TOOLS);
   //  swallow_widget->sWClose(false);
//     swallow_widget->setExeString("kiconedit " + filename);
//     swallow_widget->sWExecute();
//     swallow_widget->init();
    return;
  }
  //load ktranslator if clicked/loaded an po file
  if((filename).right(3) == ".po"){
    if(CToolClass::searchInstProgram("ktranslator")){
    showOutputView(false);
    //    s_tab_view->setCurrentTab(TOOLS);
    // swallow_widget->sWClose(false);
//     swallow_widget->setExeString("ktranslator "+ filename);
//     swallow_widget->sWExecute();
//     swallow_widget->init();
    return;
    }
  }
  //load ktranslator if clicked/loaded an po file
  if((filename).right(4) == ".gif" || (filename).right(4) == ".bmp" || (filename).right(4) == ".xbm"){
    if(!CToolClass::searchInstProgram("kpaint")){
			return;
    }
    showOutputView(false);
    //    s_tab_view->setCurrentTab(TOOLS);
    // swallow_widget->sWClose(false);
//     swallow_widget->setExeString("kpaint "+ filename);
//     swallow_widget->sWExecute();
//     swallow_widget->init();
    return;
  }
  if((filename).right(3) == ".ps"){
    if(!CToolClass::searchInstProgram("kghostview")){
			return;
		}
    showOutputView(false);
    //    s_tab_view->setCurrentTab(TOOLS);
    // swallow_widget->sWClose(false);
//     swallow_widget->setExeString("kghostview "+ filename);
//     swallow_widget->sWExecute();
//     swallow_widget->init();
    return;
  }
  
  // dialog view (MDI)--------------------------------------------
  cerr << "filename:" << filename << endl;
  if((filename).right(8) == ".kdevdlg"){
      
      QList<QextMdiChildView> dlgviews = mdi_main_frame->childrenOfType("DialogView");
      QListIterator<QextMdiChildView> it(dlgviews);
      for (; it.current(); ++it) {
	  DialogView *tmp_dlg_view = static_cast<DialogView*>(it.current());
	  if (tmp_dlg_view->dialogWidget()->fileName() == filename ) {
	      tmp_dlg_view->setFocus();
	      return;
	  }
      }

      // new dialog view
      QextMdiChildView*act_win = mdi_main_frame->activeWindow () ;
      bool maximize = false;
      if(act_win != 0){
	  maximize = act_win->isMaximized();
      }
      QFileInfo fileinfo(filename);
      DialogView* new_dialogview = new DialogView(mdi_main_frame,QFileInfo(filename).fileName());
      new_dialogview->dialogWidget()->openDialog(filename);
      
      //connections
      connect(new_dialogview,SIGNAL(focusInEventOccurs(QextMdiChildView*)),this,SLOT(slotMDIGetFocus(QextMdiChildView*)));
      // default view size is 3/4 of mdi mainframe
      new_dialogview->resize(int(double(mdi_main_frame->width()*3)/4.0),
			     int(double(mdi_main_frame->height()*3)/4.0) );  
  
#warning FIXME size handling when adding MDI views
      mdi_main_frame->addWindow( new_dialogview,  // the view pointer
				 true,            // show it
				 true,            // attach it
				 true,            // show it maximized
				 0);              // initial geometry rectangle, 0 means minimumSize()
      if(maximize){
	  new_dialogview->maximize(true);
      }
      new_dialogview->setFocus();
      return;
  }
  
 


  // and now editor MDI----------------------------------------------------
  cerr << "filename:" << filename << endl;
  QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
  QListIterator<QextMdiChildView> it(editorviews);
  for (; it.current(); ++it) {
      EditorView *tmp_editor_view = static_cast<EditorView*>(it.current());
      if (tmp_editor_view->currentEditor()->isEditing(filename) ) { 
	  
          cerr << "Already edited with name:" << tmp_editor_view->currentEditor()->fileName() << endl;          
	  tmp_editor_view->setFocus();
	  
	  //ok, and now the modified outside the editor stuff
	  
	  //   // handle file if it was modified on disk by another editor/cvs
	  
	  
	  if(editor_view->currentEditor()->modifiedOnDisk() && bShowModifiedBox){
	      if(KMessageBox::questionYesNo(this,
					    i18n("The file %1 was modified outside this editor.\n"
						 "Open the file from disk and delete the current Buffer?").arg(filename)) == KMessageBox::Yes){
		  bForceReload = true;
	      }
	  }
	      
	  if(bForceReload){
	      tmp_editor_view->currentEditor()->loadFile(filename,1);  
	      tmp_editor_view->currentEditor()->setName(filename);
	  }
	  return;
      }
  }

  //  //MDI
  cerr << "new mdi window";
  
  QextMdiChildView*act_win = mdi_main_frame->activeWindow () ;
  bool maximize = false;
  if(act_win != 0){
      maximize = act_win->isMaximized();
  }
  QFileInfo fileinfo(filename);
  EditorView* new_editorview = new EditorView(mdi_main_frame,QFileInfo(filename).fileName());
  config->setGroup("KWrite Options");
  new_editorview->currentEditor()->readConfig(config);
  new_editorview->currentEditor()->doc()->readConfig(config);
  new_editorview->currentEditor()->loadFile(filename,1);  
  new_editorview->currentEditor()->setName(filename);

  //connections
  connect(new_editorview,SIGNAL(focusInEventOccurs(QextMdiChildView*)),this,SLOT(slotMDIGetFocus(QextMdiChildView*)));

  // firstview
  connect(new_editorview, SIGNAL(lookUp(QString)),this, SLOT(slotHelpSearchText(QString)));
  connect(new_editorview, SIGNAL(newCurPos()), this, SLOT(slotNewLineColumn()));
  connect(new_editorview, SIGNAL(newStatus()),this, SLOT(slotNewStatus()));
  connect(new_editorview, SIGNAL(newMarkStatus()), this, SLOT(slotMarkStatus()));
  connect(new_editorview, SIGNAL(newUndo()),this, SLOT(slotNewUndo()));
  connect(new_editorview, SIGNAL(grepText(QString)), this, SLOT(slotEditSearchInFiles(QString)));
  connect(new_editorview->editorfirstview->popup(), SIGNAL(highlighted(int)), this, SLOT(statusCallback(int)));
  connect(new_editorview,SIGNAL(closing(EditorView*)),SLOT(slotFileWillBeClosed(EditorView*)));

  // default view size is 3/4 of mdi mainframe
  new_editorview->resize(int(double(mdi_main_frame->width()*3)/4.0),
                         int(double(mdi_main_frame->height()*3)/4.0) );  
  
#warning FIXME size handling when adding MDI views
  mdi_main_frame->addWindow( new_editorview,  // the view pointer
                             true,            // show it
			     true,            // attach it
			     true,            // show it maximized
			     0);              // initial geometry rectangle, 0 means minimumSize()
  if(maximize){
    new_editorview->maximize(true);
  }
  new_editorview->setFocus();
  
}

void CKDevelop::switchToFile(QString filename, int lineNo){
  switchToFile( filename, false);
  if(editor_view !=0){
      editor_view->currentEditor()->setCursorPosition( lineNo, 0 );
  }
}

void CKDevelop::switchToKDevelop(){
  setCaption(kdev_caption);

  bKDevelop=true;
  this->setUpdatesEnabled(false);

  //////// change the mainview ////////
  mdi_main_frame->show();
  t_tab_view->show();

//F.B.  top_panner->hide();
  //  top_panner->deactivate();
  //  top_panner->activate(t_tab_view,s_tab_view);// activate the top_panner
//F.B.  top_panner->show();

  //////// change the event dispatchers ///////////
  kdev_dispatcher->setEnabled(true);

  //////// change the bars ///////////
  kdev_menubar->show();
  setMenu(kdev_menubar);

  kdev_statusbar->show();
  setStatusBar(kdev_statusbar);


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


void CKDevelop::setToolMenuProcess(bool enable){
    if (enable)
	disableCommand(ID_BUILD_STOP);
    else
	enableCommand(ID_BUILD_STOP);
    
    if (enable && project){
	if(editor_view !=0){
	  if (CProject::getType(editor_view->currentEditor()->getName()) == CPP_SOURCE){
	      enableCommand(ID_BUILD_COMPILE_FILE);
	  }
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
	enableCommand(ID_PROJECT_MAKE_PROJECT_API);
	enableCommand(ID_PROJECT_MAKE_USER_MANUAL);
	enableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
	
    }
    
    if (!enable)
	{
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
	    disableCommand(ID_PROJECT_MAKE_PROJECT_API);
	    disableCommand(ID_PROJECT_MAKE_USER_MANUAL);
	    disableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
	}
    
    if(bAutosave)
	{
	    if(enable)
		saveTimer->start(saveTimeout); // restart autosaving if enabled after a process finished
	    else
		saveTimer->stop();  // stop the autosaving if make or something is running
	}
}


void CKDevelop::showTreeView(bool show){

 
     if(view_menu->isItemChecked(ID_VIEW_TREEVIEW))
       t_tab_view->show();
     else
       t_tab_view->hide();
   
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
   {
     size=QSize(780, 560);
     setGeometry(QApplication::desktop()->width()/2-390, QApplication::desktop()->height()/2-280,
		size.width(), size.height());
   }

	/////////////////////////////////////////
	// BAR STATUS


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
	

	// Statusbar
  bool statusbar=config->readBoolEntry("show_statusbar",true);
  if(statusbar){
    view_menu->setItemChecked(ID_VIEW_STATUSBAR, true);
  }
  else{
    enableStatusBar();
  }
	// MDI view taskbar
  bool mdiViewTaskbar=config->readBoolEntry("show_mdiviewtaskbar",true);
  if(mdiViewTaskbar){
    view_menu->setItemChecked(ID_VIEW_MDIVIEWTASKBAR, true);
  }
  else{
    mdi_main_frame->hideViewTaskBar();
  }
	
	/////////////////////////////////////////
	// Outputwindow, TreeView, KDevelop/KDlgEdit
  bool outputview= config->readBoolEntry("show_output_view", true);
  int output_view_pos=config->readNumEntry("OutputViewHeight", size.height()*20/100);
  int edit_view_pos=config->readNumEntry("EditViewHeight", size.height()-output_view_pos);

  // set initial heights of trees, mdi and output views
  t_tab_view->resize(t_tab_view->width(),edit_view_pos);
  mdi_main_frame->resize(mdi_main_frame->width(),edit_view_pos);
  o_tab_view->resize(o_tab_view->width(),output_view_pos);

  if(outputview){
    view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW, true);
    toolBar()->setButton(ID_VIEW_OUTPUTVIEW, true);
  }
  else
    o_tab_view->hide();

  bool treeview=config->readBoolEntry("show_tree_view", true);
  int tree_view_pos=config->readNumEntry("ClassViewWidth", size.width()*20/100);
  edit_view_pos=config->readNumEntry("EditViewWidth", size.width()-tree_view_pos);

  // set initial widths of trees and  mdi views
  t_tab_view->resize(tree_view_pos,t_tab_view->height());
  mdi_main_frame->resize(edit_view_pos,mdi_main_frame->height());

  if(treeview){
    view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
    toolBar()->setButton(ID_VIEW_TREEVIEW, true);
  }
  else
    t_tab_view->hide();

	
  
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

  config->setGroup("Files");
	recent_projects.setAutoDelete(TRUE);
	config->readListEntry("Recent Projects",recent_projects);
	
	uint i;
	for ( i =0 ; i < recent_projects.count(); i++){
    recent_projects_menu->insertItem(recent_projects.at(i), i);
  }

	doc_bookmarks_list.setAutoDelete(TRUE);
	doc_bookmarks_title_list.setAutoDelete(TRUE);
	
	config->readListEntry("doc_bookmarks",doc_bookmarks_list);
	config->readListEntry("doc_bookmarks_title",doc_bookmarks_title_list);
	for ( i =0 ; i < doc_bookmarks_title_list.count(); i++){
    doc_bookmarks->insertItem(BarIcon("html"),doc_bookmarks_title_list.at(i));
  }
/*	
  QString filename;
  filename = config->readEntry("browser_file","");
  if(!filename.isEmpty()){
    slotURLSelected(browser_widget,filename,1,"test");
  }
  else{
    QString file = CToolClass::locatehtml("kdevelop/welcome/index.html");
    slotURLSelected(browser_widget,"file:" + file,1,"test");
  }
*/
    switchToKDevelop();
 
}

void CKDevelop::saveOptions(){
	
  config->setGroup("General Options");
  config->writeEntry("Geometry", size() );
  

  config->writeEntry("ToolBar Position",  (int)toolBar()->barPos());
  config->writeEntry("Browser ToolBar Position", (int)toolBar(ID_BROWSER_TOOLBAR)->barPos());


  config->writeEntry("EditViewHeight",mdi_main_frame->height());
  config->writeEntry("OutputViewHeight",o_tab_view->height());

  config->writeEntry("ClassViewWidth",t_tab_view->width());
  config->writeEntry("EditViewWidth",mdi_main_frame->width());

  config->writeEntry("show_tree_view",view_menu->isItemChecked(ID_VIEW_TREEVIEW));
  config->writeEntry("show_output_view",view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW));

  config->writeEntry("show_std_toolbar",view_menu->isItemChecked(ID_VIEW_TOOLBAR));
  config->writeEntry("show_browser_toolbar",view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR));

  config->writeEntry("show_statusbar",view_menu->isItemChecked(ID_VIEW_STATUSBAR));
  config->writeEntry("show_mdiviewtaskbar",view_menu->isItemChecked(ID_VIEW_MDIVIEWTASKBAR));
  config->writeEntry("LastActiveTree", t_tab_view->getCurrentTab());

  config->writeEntry("lfv_show_path",log_file_tree->showPath());

  config->writeEntry("Autosave",bAutosave);
  config->writeEntry("Autosave Timeout",saveTimeout);

  config->writeEntry("Make",make_cmd);

  config->setGroup("Files");
  config->writeEntry("browser_file",history_list.current());
  config->writeEntry("doc_bookmarks", doc_bookmarks_list);
  config->writeEntry("doc_bookmarks_title", doc_bookmarks_title_list);
  config->writeEntry("Recent Projects", recent_projects);
  
  writeDockConfig();
  config->sync();
}

bool CKDevelop::queryExit(){
  saveOptions();
  return true;
}

bool CKDevelop::queryClose(){
    //  swallow_widget->sWClose(false); // close the tools in the tools-tab
  config->setGroup("Files");
  if(project){
    config->writeEntry("project_file",prj->getProjectFile());
    prj->writeProject();
    if(!slotProjectClose()){ // if not ok,pressed cancel
      return false; //not close!
    }
  }
  else{
    int msg_result = KMessageBox::Yes;
    int save=true;

    config->writeEntry("project_file","");

    QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
    QListIterator<QextMdiChildView> it(editorviews);
    for (; it.current(); ++it) {
	EditorView *tmp_editor_view = static_cast<EditorView*>(it.current());
	if(tmp_editor_view->currentEditor()->isModified()){
	    save=false;     
	}
	
    }
    if (!save)  {
	msg_result = KMessageBox::questionYesNo(this, 
						i18n("There is unsaved data.\n"
						     "Do you really want to quit?"));
    }
    return msg_result==KMessageBox::Yes;
  }
  return true;
}

void CKDevelop::readProperties(KConfig* sess_config){
  QString filename;
  filename = sess_config->readEntry("project_file","");
  
  QFile file(filename);
  if (file.exists()){
    if(!(readProjectFile(filename))){
      KMessageBox::sorry(0, "This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\nPlease use only new generated projects!");
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
    prj->writeProject();
  }	
  if(bAutosave)
    slotFileSaveAll();
  else{
      QList<QextMdiChildView> editorviews = mdi_main_frame->childrenOfType("EditorView");
      QListIterator<QextMdiChildView> it(editorviews);
      for (; it.current(); ++it) {
	  EditorView *tmp_editor_view = static_cast<EditorView*>(it.current());
	  if(tmp_editor_view->currentEditor()->isModified()){
#warning FIXME new session managment?
	      //	setUnsavedData ( true );
	      break;
	  }
      }  
  }
}





EditorView* CKDevelop::getCurrentEditorView(){
    QextMdiChildView * win = mdi_main_frame->activeWindow();
    if (win !=0 && win->inherits("EditorView"))
      return static_cast<EditorView*>(win);

    return 0;
}

void CKDevelop::resizeEvent(QResizeEvent* rse){
  // if necessary, adjust the 4 system buttons of a maximized MDI view to the right-hand side
  mdi_main_frame->setSysButtonsAtMenuPosition();
}

