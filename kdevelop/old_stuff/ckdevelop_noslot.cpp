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
  if(m_recent_projects.find(file) == -1){
    if( m_recent_projects.count() < 5)
      m_recent_projects.insert(0,file);
    else{
      m_recent_projects.remove(4);
      m_recent_projects.insert(0,file);
    }
    m_recent_projects_menu->clear();
		uint i;
    for ( i =0 ; i < m_recent_projects.count(); i++){
      m_recent_projects_menu->insertItem(m_recent_projects.at(i), i);
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
    
    if (m_bKDevelop)
	{
	    switch(tab_item)
		{
		case BROWSER:
		    m_kdev_caption=m_browser_widget->currentTitle()+ " - KDevelop " + m_version ;
		    break;
		case TOOLS:
		    m_kdev_caption=QString(i18n("Tools")) +" - KDevelop " + m_version ;
		    break;
		default:
		    m_kdev_caption=(m_project) ? (const char *) (m_prj->getProjectName()+" - KDevelop ") : "KDevelop ";
		    if(editor_view != 0){
			m_kdev_caption+= m_version +
			    " - ["+ QFileInfo(editor_view->currentEditor()->getName()).fileName()+"] ";
			if (editor_view->currentEditor()->isModified())
			    {
				enableCommand(ID_FILE_SAVE);
				m_kdev_caption+= " *";
			    }
			else
			    {
				disableCommand(ID_FILE_SAVE);
			    }
			break;
		    }
		}
	}
    setCaption(m_kdev_caption);
    
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

   QString oldName;
   KURL name;
   int msg_result = KMessageBox::Ok;

   if (m_bAutosave) m_saveTimer->stop();
   
   oldName = m_editor_view->currentEditor()->getName();

   do {
       name = KFileDialog::getSaveURL(oldName,0,this,oldName);
       if( !name.isLocalFile() )
       {
         KMessageBox::sorry( 0L, i18n( "Only local files are supported" ) );
         return false;
       }
       if (name.isEmpty()){
	    if (m_bAutosave) {
	        m_saveTimer->start(m_saveTimeout);
	    }
	   return false;
       }
       if(QFile::exists(name.path())){
	   msg_result=KMessageBox::warningYesNoCancel(this,
						      i18n("\nThe file\n\n%1\n\nalready exists.\n"
							   "Do you want overwrite the old one?\n").arg(name.path()));
	   
	   if (msg_result==KMessageBox::Cancel){
	       if (m_bAutosave)
		   m_saveTimer->start(m_saveTimeout);
	       return false;
	   } 
       }
   }
   while(msg_result == KMessageBox::No);
   
   m_editor_view->setMDICaption(QFileInfo(name.path()).fileName());
   m_editor_view->currentEditor()->setName(name.path());
   m_editor_view->currentEditor()->doSave();
  
   slotViewRefresh();
   if (m_bAutosave)
       m_saveTimer->start(m_saveTimeout);

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
  if (!m_project){
    return; // no project
  }

  // Update the classview.
  slotStatusMsg(i18n("Scanning project files..."));
	setCursor(KCursor::waitCursor());
  m_statProg->show();
  m_class_tree->refresh(m_prj);
  m_statProg->reset();
  m_statProg->hide();

  // Update the classcombo.
  CVRefreshClassCombo();

  // Update LFV.
  m_log_file_tree->storeState(m_prj);
  m_log_file_tree->refresh(m_prj);

  // Update RFV.
  m_real_file_tree->refresh(m_prj);

  m_kdev_statusbar->repaint();
  setCursor(KCursor::arrowCursor());	
  // update the m_file_open_menu
  m_file_open_list=m_prj->getHeaders();
  QStrList sources=m_prj->getSources();
  uint j;
  for( j=0; j< sources.count(); j++){
    m_file_open_list.append(sources.at(j));
  }
  // create the m_file_open_popup for the toolbar
  m_file_open_popup->clear();
  uint i;
  for ( i =0 ; i < m_file_open_list.count(); i++){
    QFileInfo fileInfo (m_file_open_list.at(i));
    m_file_open_popup->insertItem(fileInfo.fileName());
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
  if( m_project )
    {
      // If this is a sourcefile we parse it and update the classview.
      if( info->type == CPP_SOURCE || info->type == CPP_HEADER )
	{
	  m_class_tree->addFile( m_prj->getProjectDir() + info->rel_name );
	  CVRefreshClassCombo();
	}
      
      // Update LFV.
      m_log_file_tree->storeState(m_prj);
      m_log_file_tree->refresh(m_prj);
      
      // Update RFV.
      m_real_file_tree->refresh(m_prj);
      // update dialogs tree
      // -> Component
      //      kdlg_dialogs_view->refresh(m_prj);
      
    }
  // refresh the m_file_open_list
  m_file_open_list=m_prj->getHeaders();
  QStrList sources=m_prj->getSources();
  uint j;
  for( j=0; j< sources.count(); j++){
    m_file_open_list.append(sources.at(j));
  }
  // create the m_file_open_popup for the toolbar
  m_file_open_popup->clear();
  uint i;
  for ( i =0 ; i < m_file_open_list.count(); i++){
    QFileInfo fileInfo (m_file_open_list.at(i));
    m_file_open_popup->insertItem(fileInfo.fileName());
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
      
      QList<QextMdiChildView> dlgviews = m_mdi_main_frame->childrenOfType("DialogView");
      QListIterator<QextMdiChildView> it(dlgviews);
      for (; it.current(); ++it) {
	  DialogView *tmp_dlg_view = static_cast<DialogView*>(it.current());
	  if (tmp_dlg_view->dialogWidget()->fileName() == filename ) {
	      tmp_dlg_view->setFocus();
	      return;
	  }
      }

      // new dialog view
      QextMdiChildView*act_win = m_mdi_main_frame->activeWindow () ;
      bool maximize = false;
      if(act_win != 0){
	  maximize = act_win->isMaximized();
      }
      QFileInfo fileinfo(filename);
      DialogView* new_dialogview = new DialogView(m_mdi_main_frame,QFileInfo(filename).fileName());
      new_dialogview->dialogWidget()->openDialog(filename);
      
      //connections
      connect(new_dialogview,SIGNAL(focusInEventOccurs(QextMdiChildView*)),this,SLOT(slotMDIGetFocus(QextMdiChildView*)));
      // default view size is 3/4 of mdi mainframe
      new_dialogview->resize(int(double(m_mdi_main_frame->width()*3)/4.0),
			     int(double(m_mdi_main_frame->height()*3)/4.0) );
  
      if( maximize)
        m_mdi_main_frame->addWindow( new_dialogview, QextMdi::Maximize); // attached, shown and focused by default
      else
        m_mdi_main_frame->addWindow( new_dialogview); // attached, shown and focused by default

      return;
  }
  
 


  // and now editor MDI----------------------------------------------------
  cerr << "filename:" << filename << endl;
  QList<QextMdiChildView> editorviews = m_mdi_main_frame->childrenOfType("EditorView");
  QListIterator<QextMdiChildView> it(editorviews);
  for (; it.current(); ++it) {
      EditorView *tmp_editor_view = static_cast<EditorView*>(it.current());
      if (tmp_editor_view->currentEditor()->isEditing(filename) ) { 
	  
          cerr << "Already edited with name:" << tmp_editor_view->currentEditor()->fileName() << endl;          
	  tmp_editor_view->setFocus();
	  
	  //ok, and now the modified outside the editor stuff
	  
	  //   // handle file if it was modified on disk by another editor/cvs
	  
	  
	  if(m_editor_view->currentEditor()->modifiedOnDisk() && bShowModifiedBox){
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
  
  QextMdiChildView*act_win = m_mdi_main_frame->activeWindow () ;
  bool maximize = false;
  if(act_win != 0){
      maximize = act_win->isMaximized();
  }
  QFileInfo fileinfo(filename);
  EditorView* new_editorview = new EditorView(m_mdi_main_frame,QFileInfo(filename).fileName());
  m_config->setGroup("KWrite Options");
  new_editorview->currentEditor()->readConfig(m_config);
  new_editorview->currentEditor()->doc()->readConfig(m_config);
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
  new_editorview->resize(int(double(m_mdi_main_frame->width()*3)/4.0),
                         int(double(m_mdi_main_frame->height()*3)/4.0) );
  
  if( maximize)
    m_mdi_main_frame->addWindow( new_editorview, QextMdi::Maximize); // attached, shown and focused by default
  else  
    m_mdi_main_frame->addWindow( new_editorview); // attached, shown and focused by default
}

void CKDevelop::switchToFile(QString filename, int lineNo){
  switchToFile( filename, false);
  if(m_editor_view !=0){
      m_editor_view->currentEditor()->setCursorPosition( lineNo, 0 );
  }
}

void CKDevelop::switchToKDevelop(){
  setCaption(m_kdev_caption);

  m_bKDevelop=true;
  this->setUpdatesEnabled(false);

  //////// change the mainview ////////
  m_mdi_main_frame->show();
//FALK  dockbase_t_tab_view->show();

//F.B.  top_panner->hide();
  //  top_panner->deactivate();
  //  top_panner->activate(t_tab_view,s_tab_view);// activate the top_panner
//F.B.  top_panner->show();

  //////// change the event dispatchers ///////////
  m_kdev_dispatcher->setEnabled(true);

  //////// change the bars ///////////
  m_kdev_menubar->show();
  setMenu(m_kdev_menubar);

  m_kdev_statusbar->show();
  setStatusBar(m_kdev_statusbar);


  toolBar(ID_MAIN_TOOLBAR)->show();
  toolBar(ID_BROWSER_TOOLBAR)->show();

  setKeyAccel();  // initialize Keys
  ///////// reset bar status ////////////
  if(m_view_menu->isItemChecked(ID_VIEW_STATUSBAR))
    m_kdev_statusbar->show();
  else
    m_kdev_statusbar->hide();

  if(m_view_menu->isItemChecked(ID_VIEW_TOOLBAR))
    enableToolBar(KToolBar::Show, ID_MAIN_TOOLBAR);
  else
    enableToolBar(KToolBar::Hide, ID_MAIN_TOOLBAR);

  if(m_view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR))
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  else
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);

  ///////// reset the views status ///////////////
  if(m_view_menu->isItemChecked(ID_VIEW_TREEVIEW))
    showTreeView();
  else
    showTreeView(false);
  if(m_view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW))
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
    
    if (enable && m_project){
	if(m_editor_view !=0){
	  if (CProject::getType(m_editor_view->currentEditor()->getName()) == CPP_SOURCE){
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
	if (m_prj->getProjectType() != "normal_kde" && m_prj->getProjectType() != "mini_kde"){
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
    
    if(m_bAutosave)
	{
	    if(enable)
		m_saveTimer->start(m_saveTimeout); // restart autosaving if enabled after a process finished
	    else
		m_saveTimer->stop();  // stop the autosaving if make or something is running
	}
}


void CKDevelop::showTreeView(bool show){

 
     if(m_view_menu->isItemChecked(ID_VIEW_TREEVIEW))
;//FALK       dockbase_t_tab_view->show();
     else
;//FALK       dockbase_t_tab_view->hide();
   
  if(m_bAutoswitch)
  {
    if(show){
      if(m_view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already visible){
      }
      else{
				slotViewTTreeView();
      }
    }
    else{
      if(!m_view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
        return; // it's already unvisible){
      }
      else{
				slotViewTTreeView();
      }
    }
  }
}


void CKDevelop::showOutputView(bool show){
  if(m_bAutoswitch){
  	if(show){
	  	if(m_view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
  	 		return; // it's already visible
      }
      else{
				slotViewTOutputView();
      }
    }
    else{
      if(!m_view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
        return; //it's already unvisible
      }
      else{
				slotViewTOutputView();
      }
    }
  }
}


void CKDevelop::readOptions(){
  m_config->setGroup("General Options");

	/////////////////////////////////////////
	// GEOMETRY
  QSize size=m_config->readSizeEntry("Geometry");
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


  KToolBar::BarPosition tool_bar_pos=(KToolBar::BarPosition)m_config->readNumEntry("ToolBar Position", KToolBar::Top);
  toolBar(ID_MAIN_TOOLBAR)->setBarPos(tool_bar_pos);
	bool std_toolbar=	m_config->readBoolEntry("show_std_toolbar", true);
	if(std_toolbar){
	  m_view_menu->setItemChecked(ID_VIEW_TOOLBAR, true);
    enableToolBar(KToolBar::Show, ID_MAIN_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide, ID_MAIN_TOOLBAR);
  }
	// Browser Toolbar
  KToolBar::BarPosition browser_tool_bar_pos=(KToolBar::BarPosition)m_config->readNumEntry("Browser ToolBar Position", KToolBar::Top);
  toolBar(ID_BROWSER_TOOLBAR)->setBarPos(browser_tool_bar_pos);
	bool browser_toolbar=m_config->readBoolEntry("show_browser_toolbar",true);
	if(browser_toolbar){
	  m_view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR, true);
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);
  }
	

	// Statusbar
  bool statusbar=m_config->readBoolEntry("show_statusbar",true);
  if(statusbar){
    m_view_menu->setItemChecked(ID_VIEW_STATUSBAR, true);
  }
  else{
    enableStatusBar();
  }
	// MDI view taskbar
  bool mdiViewTaskbar=m_config->readBoolEntry("show_mdiviewtaskbar",true);
  if(mdiViewTaskbar){
    m_view_menu->setItemChecked(ID_VIEW_MDIVIEWTASKBAR, true);
  }
  else{
    m_mdi_main_frame->hideViewTaskBar();
  }
	
	/////////////////////////////////////////
	// Outputwindow, TreeView, KDevelop/KDlgEdit
  bool outputview= m_config->readBoolEntry("show_output_view", true);
  if(outputview){
    m_view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW, true);
    toolBar(ID_MAIN_TOOLBAR)->setButton(ID_VIEW_OUTPUTVIEW, true);
  }
  else;
//FALK    dockbase_o_tab_view->hide();

  bool treeview=m_config->readBoolEntry("show_tree_view", true);
  if(treeview){
    m_view_menu->setItemChecked(ID_VIEW_TREEVIEW, true);
    toolBar(ID_MAIN_TOOLBAR)->setButton(ID_VIEW_TREEVIEW, true);
  }
  else
;//FALK    dockbase_t_tab_view->hide();

	
  
	/////////////////////////////////////////
	// RUNTIME VALUES AND FILES
  m_bAutosave=m_config->readBoolEntry("Autosave",true);
  m_saveTimeout=m_config->readNumEntry("Autosave Timeout",5*60*1000);
  m_saveTimer=new QTimer(this);
  connect(m_saveTimer,SIGNAL(timeout()),SLOT(slotFileSaveAll()));
  if(m_bAutosave){
    m_saveTimer->start(m_saveTimeout);
  }
  else{
    m_saveTimer->stop();
  }
  m_bAutoswitch=m_config->readBoolEntry("Autoswitch",true);
  m_bDefaultCV=m_config->readBoolEntry("DefaultClassView",true);
  m_make_cmd=m_config->readEntry("Make","make");

  m_config->setGroup("Files");
	m_recent_projects.setAutoDelete(TRUE);
	m_config->readListEntry("Recent Projects",m_recent_projects);
	
	uint i;
	for ( i =0 ; i < m_recent_projects.count(); i++){
    m_recent_projects_menu->insertItem(m_recent_projects.at(i), i);
  }

	m_doc_bookmarks_list.setAutoDelete(TRUE);
	m_doc_bookmarks_title_list.setAutoDelete(TRUE);
	
	m_config->readListEntry("doc_bookmarks",m_doc_bookmarks_list);
	m_config->readListEntry("doc_bookmarks_title",m_doc_bookmarks_title_list);
	for ( i =0 ; i < m_doc_bookmarks_title_list.count(); i++){
    m_doc_bookmarks->insertItem(BarIcon("html"),m_doc_bookmarks_title_list.at(i));
  }
/*	
  QString filename;
  filename = m_config->readEntry("browser_file","");
  if(!filename.isEmpty()){
    slotURLSelected(m_browser_widget,filename,1,"test");
  }
  else{
    QString file = CToolClass::locatehtml("kdevelop/welcome/index.html");
    slotURLSelected(m_browser_widget,"file:" + file,1,"test");
  }
*/
	
	switchToKDevelop();
 
}

void CKDevelop::saveOptions(){
	
  m_config->setGroup("General Options");
  m_config->writeEntry("Geometry", size() );
  
  m_config->writeEntry("ToolBar Position",  (int)toolBar(ID_MAIN_TOOLBAR)->barPos());
  m_config->writeEntry("Browser ToolBar Position", (int)toolBar(ID_BROWSER_TOOLBAR)->barPos());

  m_config->writeEntry("show_tree_view",m_view_menu->isItemChecked(ID_VIEW_TREEVIEW));
  m_config->writeEntry("show_output_view",m_view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW));

  m_config->writeEntry("show_std_toolbar",m_view_menu->isItemChecked(ID_VIEW_TOOLBAR));
  m_config->writeEntry("show_browser_toolbar",m_view_menu->isItemChecked(ID_VIEW_BROWSER_TOOLBAR));

  m_config->writeEntry("show_statusbar",m_view_menu->isItemChecked(ID_VIEW_STATUSBAR));
  m_config->writeEntry("show_mdiviewtaskbar",m_view_menu->isItemChecked(ID_VIEW_MDIVIEWTASKBAR));

  m_config->writeEntry("lfv_show_path",m_log_file_tree->showPath());

  m_config->writeEntry("Autosave",m_bAutosave);
  m_config->writeEntry("Autosave Timeout",m_saveTimeout);

  m_config->writeEntry("Make",m_make_cmd);

  m_config->setGroup("Files");
  m_config->writeEntry("browser_file",m_history_list.current());
  m_config->writeEntry("doc_bookmarks", m_doc_bookmarks_list);
  m_config->writeEntry("doc_bookmarks_title", m_doc_bookmarks_title_list);
  m_config->writeEntry("Recent Projects", m_recent_projects);
  
	// write all settings concerning to the dockwidget´s stuff
  writeDockConfig( m_config, "EditMode Dock-Settings");

  m_config->sync();
}

bool CKDevelop::queryExit(){
  saveOptions();
  return true;
}

bool CKDevelop::queryClose(){
    //  swallow_widget->sWClose(false); // close the tools in the tools-tab
  m_config->setGroup("Files");
  if(m_project){
    m_config->writeEntry("project_file",m_prj->getProjectFile());
    m_prj->writeProject();
    if(!slotProjectClose()){ // if not ok,pressed cancel
      return false; //not close!
    }
  }
  else{
    int msg_result = KMessageBox::Yes;
    int save=true;

    m_config->writeEntry("project_file","");

    QList<QextMdiChildView> editorviews = m_mdi_main_frame->childrenOfType("EditorView");
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
      connect(m_class_tree,SIGNAL(setStatusbarProgressSteps(int)),progressDlg,SLOT(setTotalSteps(int)));
      connect(m_class_tree,SIGNAL(setStatusbarProgress(int)),progressDlg,SLOT(setProgress(int)));
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

void CKDevelop::writeProperties(KConfig* sess_config){
	
  if(m_project){
    sess_config->writeEntry("project_file",m_prj->getProjectFile());
    m_prj->writeProject();
  }	
  if(m_bAutosave)
    slotFileSaveAll();
  else{
      QList<QextMdiChildView> editorviews = m_mdi_main_frame->childrenOfType("EditorView");
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
    QextMdiChildView * win = m_mdi_main_frame->activeWindow();
    if (win !=0 && win->inherits("EditorView"))
      return static_cast<EditorView*>(win);

    return 0;
}

void CKDevelop::resizeEvent(QResizeEvent* rse){
  // if necessary, adjust the 4 system buttons of a maximized MDI view to the right-hand side
  m_mdi_main_frame->setSysButtonsAtMenuPosition();
}
