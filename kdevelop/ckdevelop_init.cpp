/***************************************************************************
      init.cpp - the init specific part of CKDevelop (constructor,init() ...)
                             -------------------                                         

    begin                : 20 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier,(C) 1999 the KDevelop Team                         
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

#include <qdir.h>
#include <qtoolbutton.h>
#include <qprogressdialog.h>
#include <qframe.h>
#include <qclipbrd.h>
#include <qsplitter.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>
#include <dcopclient.h>
#include <kcursor.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kaction.h>


#include "./kwrite/kwdoc.h"
#include "./kwrite/kguicommand.h"
#include "ckdevelop.h"
#include "cclassview.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
#include "clogfileview.h"
#include "crealfileview.h"
#include "ceditwidget.h"
#include "ctoolclass.h"	
#include "ctabctl.h"
#include "grepview.h"
#include "makeview.h"
#include "outputview.h"
#include "ckdevaccel.h"
#include "componentmanager.h"
#include "processview.h"
#include "mdiframe.h"
#include "editorview.h"
#include "docbrowserview.h"
#include "kdevelopifaceimpl.h"
#include "dlgedit/widgetspropsplitview.h"
#include "dlgedit/dlgedit.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

extern KGuiCmdManager cmdMngr;

CKDevelop::CKDevelop()
	: DockMainWindow("CKDevelop_mainwindow")
  ,search_process("/bin/sh")
{
  version = VERSION;
  project=false;// no project
  beep=false; // no beep
  cv_decl_or_impl=true;
  file_open_list.setAutoDelete(TRUE);
  
  config = kapp->config();
  kdev_caption=kapp->caption();

  initView();
  
  initConnections();

  initWhatsThis();
	
  readOptions();
  readDockConfig();
	
  initProject();
  setToolmenuEntries();

  DCOPClient *client = kapp->dcopClient();
  client->attach();
  client->registerAs("KDevelop");
  (void) new KDevelopIfaceImpl(this);
  
//  error_parser = new CErrorMessageParser;
  slotStatusMsg(i18n("Welcome to KDevelop!"));
}

CKDevelop::~CKDevelop(){
  // from Constructur... delete everything which is not constructed
  //   with a binding to the application
//  delete error_parser;
}

void CKDevelop::initView(){
  act_outbuffer_len=0;
  prj = 0;

  QFont font("Fixed",10);

  ////////////////////////
  // Outputwindow
  ////////////////////////
  dockbase_o_tab_view = createDockWidget(i18n("Output messages"), BarIcon("filenew"));
  o_tab_view = new CTabCtl(dockbase_o_tab_view,"output_tabview","output_widget");
	
  messages_widget = new MakeView(o_tab_view, "messages_widget");
  grepview = new GrepView(o_tab_view, "grepview");
  outputview = new OutputView(o_tab_view, "outputview");
  

  o_tab_view->addTab(messages_widget,i18n("messages"));
  o_tab_view->addTab(grepview, i18n("search"));
  o_tab_view->addTab(outputview, i18n("output"));

  ////////////////////////
  // Top Panner
  ////////////////////////

  //  s_tab_current = 0;
  dockbase_t_tab_view = createDockWidget(i18n("Tree views"), BarIcon("filenew"));
  t_tab_view = new CTabCtl(dockbase_t_tab_view);
  t_tab_view->setFocusPolicy(QWidget::ClickFocus);

  ////////////////////////
  // Treeviews
  ////////////////////////

  class_tree = new CClassView(t_tab_view,"cv");
  class_tree->setFocusPolicy(QWidget::ClickFocus); //#

  log_file_tree = new CLogFileView(t_tab_view,"lfv",config->readBoolEntry("lfv_show_path",false));
  //  log_file_tree->setFocusPolicy(QWidget::ClickFocus); //#

  real_file_tree = new CRealFileView(t_tab_view,"RFV");
  //  real_file_tree->setFocusPolicy(QWidget::ClickFocus); //#

  doc_tree = new DocTreeView(t_tab_view,"DOC");
  //  doc_tree->setFocusPolicy(QWidget::ClickFocus); //#

  widprop_split_view = new WidgetsPropSplitView(t_tab_view,"DLG");
  
  t_tab_view->addTab(class_tree,i18n("CV"));
  t_tab_view->addTab(log_file_tree,i18n("LFV"));
  t_tab_view->addTab(real_file_tree,i18n("RFV"));
  t_tab_view->addTab(doc_tree,i18n("DOC"));
  t_tab_view->addTab(widprop_split_view,i18n("DLG"));

  initDlgEditor();

  ////////////////////////
  // Right main window
  ////////////////////////
  
  mdi_main_frame = new MdiFrame( this, "mdi_frame");
  // maybe we should make this configurable :-)
  mdi_main_frame-> m_pMdi->setBackgroundPixmap(QPixmap(locate("wallpaper","Magneto_Bomb.jpg")));
  
  
#warning FIXME should we swallow tools in KDevelop 2??
  //  swallow_widget = new KSwallowWidget(t_tab_view);
  //  swallow_widget->setFocusPolicy(QWidget::StrongFocus);
  
  //  swallow_widget = new KSwallowWidget(s_tab_view);
  //  swallow_widget->setFocusPolicy(QWidget::StrongFocus);
  
  
  ComponentManager *manager = ComponentManager::self();
  manager->registerComponent(class_tree);
  manager->registerComponent(log_file_tree);
  manager->registerComponent(real_file_tree);
  manager->registerComponent(doc_tree);
  manager->registerComponent(outputview);
  manager->registerComponent(messages_widget);
  manager->registerComponent(grepview);


  //
  // dock the 2 base widgets
  //
  
  dockIn(dockbase_t_tab_view,DockLeft);
  dockIn(dockbase_o_tab_view,DockBottom);
  // set the mainwidget
  setView(mdi_main_frame);
  initKeyAccel();

  initMenuBar();
  mdi_main_frame->setMenuForSDIModeSysButtons(kdev_menubar);

  initToolBar();
  initStatusBar();
  
  //
	// open the documentation browser view on default
	//   (just open it when the menubar is up)
  browser_view = new DocBrowserView(mdi_main_frame,"browser");
  browser_widget = browser_view->browser;
  mdi_main_frame->addWindow(browser_view, // the view pointer
                            false,        // show it
			    true);        // attach it
  browser_view->maximize();
  browser_view->show();

  prev_was_search_result= false;
  //init
  browser_widget->setDocBrowserOptions();
}

/*--------------------------------------- CKDevelop::initKeyAccel()
 * initKeyAccel()
 *   Create key accelerators.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::initKeyAccel(){
  accel = new CKDevAccel( this );
  //file menu
  accel->connectItem( KStdAccel::Open , this, SLOT(slotFileOpen()), true, ID_FILE_OPEN);
  accel->connectItem( KStdAccel::Close , this, SLOT(slotFileClose()), true, ID_FILE_CLOSE);
  
  accel->connectItem( KStdAccel::Save , this, SLOT(slotFileSave()), true, ID_FILE_SAVE);
  
  accel->insertItem(i18n("Save As"), "SaveAs", 0);
  accel->connectItem( "SaveAs", this, SLOT(slotFileSaveAs()), true, ID_FILE_SAVE_AS);
  
  accel->insertItem(i18n("Save All"), "SaveAll", 0);
  accel->connectItem( "SaveAll", this, SLOT(slotFileSaveAll()), true, ID_FILE_SAVE_ALL);
  
  accel->connectItem( KStdAccel::Print , this, SLOT(slotFilePrint()), true, ID_FILE_PRINT);
  accel->connectItem( KStdAccel::Quit, this, SLOT(slotFileQuit()), true, ID_FILE_QUIT);

  //edit menu  

  accel->insertItem( i18n("Insert File"), "InsertFile", 0);
  accel->connectItem( "InsertFile", this, SLOT(slotEditInsertFile()), true, ID_EDIT_INSERT_FILE);

  accel->insertItem( i18n("Search in Files"), "Grep", IDK_EDIT_SEARCH_IN_FILES );
  accel->connectItem( "Grep", this, SLOT(slotEditSearchInFiles() ), true, ID_EDIT_SEARCH_IN_FILES );

  accel->insertItem( i18n("Search selection in Files"), "GrepSearch", IDK_EDIT_GREP_IN_FILES );
  accel->connectItem( "GrepSearch", this, SLOT(slotEditSearchText() ) );

  //view menu

  accel->insertItem( i18n("Next Error"), "NextError",IDK_VIEW_NEXT_ERROR);
  accel->connectItem( "NextError", this, SLOT( slotViewNextError()), true, ID_VIEW_NEXT_ERROR);
  
  accel->insertItem( i18n("Previous Error"), "PreviousError",IDK_VIEW_PREVIOUS_ERROR);
  accel->connectItem( "PreviousError", this, SLOT( slotViewPreviousError()), true, ID_VIEW_PREVIOUS_ERROR);
  
  accel->insertItem( i18n("Toogle Tree-View"), "Tree-View",IDK_VIEW_TREEVIEW);
  accel->connectItem( "Tree-View", this, SLOT(slotViewTTreeView()), true, ID_VIEW_TREEVIEW);
  
  accel->insertItem( i18n("Toogle Output-View"), "Output-View",IDK_VIEW_OUTPUTVIEW);
  accel->connectItem( "Output-View", this, SLOT(slotViewTOutputView()), true, ID_VIEW_OUTPUTVIEW);
  
  accel->insertItem( i18n("Toolbar"), "Toolbar", 0);
  accel->connectItem( "Toolbar", this, SLOT(slotViewTStdToolbar()), true, ID_VIEW_TOOLBAR);
  
  accel->insertItem( i18n("Browser-Toolbar"), "Browser-Toolbar", 0);
  accel->connectItem( "Browser-Toolbar", this, SLOT(slotViewTBrowserToolbar()), true, ID_VIEW_BROWSER_TOOLBAR);
	
  accel->insertItem( i18n("Statusbar"), "Statusbar", 0);
  accel->connectItem( "Statusbar", this, SLOT(slotViewTStatusbar()), true, ID_VIEW_STATUSBAR);
  
  accel->insertItem( i18n("MDI-View Taskbar"), "MDI-View Taskbar", 0);
  accel->connectItem( "MDI-View Taskbar", this, SLOT(slotViewTMDIViewTaskbar()), true, ID_VIEW_MDIVIEWTASKBAR);
  
  accel->insertItem( i18n("Preview dialog"), "Preview dialog",IDK_VIEW_PREVIEW);
  
  accel->insertItem( i18n("Refresh"), "Refresh", 0);
  accel->connectItem( "Refresh", this, SLOT(slotViewRefresh()), true, ID_VIEW_REFRESH);
  
  accel->insertItem( i18n("Graphical Classview"), "CVViewTree", 0);
  accel->connectItem( "CVViewTree", this, SLOT(slotClassbrowserViewTree()), true, ID_CV_GRAPHICAL_VIEW);
  
  // project menu
  accel->insertItem( i18n("New Project"), "NewProject",0);
  accel->connectItem( "NewProject", this, SLOT(slotProjectNewAppl()), true, ID_PROJECT_KAPPWIZARD);
  
  accel->insertItem( i18n("Open Project"), "OpenProject", 0);
  accel->connectItem( "OpenProject", this, SLOT(slotProjectOpen()), true, ID_PROJECT_OPEN);
  
  accel->insertItem( i18n("Close Project"), "CloseProject", 0);
  accel->connectItem("CloseProject", this, SLOT(slotProjectClose()), true, ID_PROJECT_CLOSE);
  
  accel->insertItem(i18n("New Class"), "NewClass", 0);
  accel->connectItem("NewClass", this, SLOT(slotProjectNewClass()), true, ID_PROJECT_NEW_CLASS);
  
  accel->insertItem(i18n("Add existing File(s)"), "AddExistingFiles", 0);
  accel->connectItem("AddExistingFiles",this, SLOT(slotProjectAddExistingFiles()), true, ID_PROJECT_ADD_FILE_EXIST);
  
  accel->insertItem(i18n("Add new Translation File"),"Add new Translation File", 0);
  accel->connectItem("Add new Translation File", this, SLOT(slotProjectAddNewTranslationFile()), true, ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  
  accel->insertItem(i18n("File Properties"), "FileProperties", IDK_PROJECT_FILE_PROPERTIES);
  accel->connectItem("FileProperties", this, SLOT(slotProjectFileProperties()), true, ID_PROJECT_FILE_PROPERTIES );
  
  accel->insertItem(i18n("Make messages and merge"), "MakeMessages", 0);
  accel->connectItem("MakeMessages", this, SLOT(slotProjectMessages()), true, ID_PROJECT_MESSAGES);
  
  accel->insertItem(i18n("Make API-Doc"), "ProjectAPI", 0);
  accel->connectItem("ProjectAPI", this, SLOT(slotProjectAPI()), true, ID_PROJECT_MAKE_PROJECT_API);
  
  accel->insertItem(i18n("Make User-Manual..."), "ProjectManual", 0);
  accel->connectItem("ProjectManual", this, SLOT(slotProjectManual()), true, ID_PROJECT_MAKE_USER_MANUAL);
  
  accel->insertItem(i18n("Make Source-tgz"), "Source-tgz", 0);
  accel->connectItem("Source-tgz", this, SLOT(slotProjectMakeDistSourceTgz()), true, ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ);
 	
  accel->insertItem(i18n("Project options"), "ProjectOptions", IDK_PROJECT_OPTIONS);
  accel->connectItem("ProjectOptions", this, SLOT(slotProjectOptions()), true, ID_PROJECT_OPTIONS);
  
  
  //build menu
  
  accel->insertItem( i18n("Compile File"), "CompileFile", IDK_BUILD_COMPILE_FILE );
  accel->connectItem( "CompileFile", this, SLOT( slotBuildCompileFile()), true, ID_BUILD_COMPILE_FILE);
  
  accel->insertItem( i18n("Make"), "Make", IDK_BUILD_MAKE );
  accel->connectItem( "Make", this, SLOT(slotBuildMake()), true, ID_BUILD_MAKE);
  
  accel->insertItem( i18n("Rebuild All"), "RebuildAll", 0);
  accel->connectItem( "RebuildAll", this, SLOT(slotBuildRebuildAll()), true, ID_BUILD_REBUILD_ALL);
  
  accel->insertItem( i18n("Clean/Rebuild all"), "CleanRebuildAll", 0);
  accel->connectItem( "CleanRebuildAll", this, SLOT(slotBuildCleanRebuildAll()), true, ID_BUILD_CLEAN_REBUILD_ALL);
  
  accel->insertItem( i18n("Stop process"), "Stop_proc", IDK_BUILD_STOP);
  accel->connectItem( "Stop_proc", this, SLOT(slotBuildStop()), true, ID_BUILD_STOP);
  
  accel->insertItem( i18n("Execute"), "Run", IDK_BUILD_RUN);
  accel->connectItem( "Run", this, SLOT(slotBuildRun() ), true, ID_BUILD_RUN);
  
  accel->insertItem( i18n("Execute with arguments"), "Run_with_args", IDK_BUILD_RUN_WITH_ARGS);
  accel->connectItem( "Run_with_args", this, SLOT(slotBuildRunWithArgs() ), true, ID_BUILD_RUN_WITH_ARGS);
  
  accel->insertItem( i18n("Debug"), "BuildDebug", 0);
  accel->connectItem("BuildDebug", this, SLOT(slotBuildDebug()), true, ID_BUILD_DEBUG);
  
  accel->insertItem( i18n("DistClean"), "BuildDistClean", 0);
  accel->connectItem("BuildDistClean",this, SLOT(slotBuildDistClean()), true, ID_BUILD_DISTCLEAN);
  
  accel->insertItem( i18n("Autoconf and automake"), "BuildAutoconf", 0);
  accel->connectItem("BuildAutoconf", this, SLOT(slotBuildAutoconf()), true, ID_BUILD_AUTOCONF);
  
  accel->insertItem( i18n("Configure..."), "BuildConfigure", 0);
  accel->connectItem( "BuildConfigure", this, SLOT(slotBuildConfigure()), true, ID_BUILD_CONFIGURE);
  
  
  //   accel->insertItem( i18n("Make with"), "MakeWith", IDK_BUILD_MAKE_WITH );
  //   accel->connectItem( "MakeWith", this, SLOT(slotBuildMakeWith() ), true, ID_BUILD_MAKE_WITH );
  
  // Tools-menu
  
  // Bookmarks-menu
  accel->insertItem( i18n("Add Bookmark"), "Add_Bookmarks", IDK_BOOKMARKS_ADD);
  accel->connectItem( "Add_Bookmarks", this, SLOT(slotBookmarksAdd()), true, ID_BOOKMARKS_ADD);

  accel->insertItem( i18n("Clear Bookmarks"), "Clear_Bookmarks", IDK_BOOKMARKS_CLEAR);
  accel->connectItem( "Clear_Bookmarks", this, SLOT(slotBookmarksClear()), true, ID_BOOKMARKS_CLEAR);

  //Help menu
  accel->connectItem( KStdAccel::Help , this, SLOT(slotHelpContents()), true, ID_HELP_CONTENTS);
  
  accel->insertItem( i18n("Search Marked Text"), "SearchMarkedText",IDK_HELP_SEARCH_TEXT);
  accel->connectItem( "SearchMarkedText", this, SLOT(slotHelpSearchText()), true, ID_HELP_SEARCH_TEXT);

  accel->insertItem( i18n("Search for Help on"), "HelpSearch", 0);
  accel->connectItem( "HelpSearch", this, SLOT(slotHelpSearch()), true, ID_HELP_SEARCH);
  
 


  accel->readSettings(0, false);
}

/*--------------------------------------- CKDevelop::initMenuBar()
 * initMenuBar()
 *   Create the menubar with its' entries.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::initMenuBar(){
    //  KDEVELOP MENUBAR //
    // NOTE: ALL disableCommand(ID_XX) are placed in initKDlgMenuBar() at the end !!!
    // NEW DISABLES HAVE TO BE ADDED THERE AFTER BOTH MENUBARS ARE CREATED COMPLETELY,
    // OTHERWISE KDEVELOP CRASHES !!!
    
    
    // the command dispatcher for this instance of CKDevelop (i
    // think for KDevelop this is always only one) gets its information
    // from the command manager. at the moment only the kwrite related
    // commands are done with this (jochen)
    
    
    QPixmap pixmap;
    KIconLoader *loader = KGlobal::iconLoader();
    kdev_dispatcher = new KGuiCmdDispatcher(this, &cmdMngr);
    kdev_dispatcher->connectCategory(ctCursorCommands, this, SLOT(doCursorCommand(int)));
    kdev_dispatcher->connectCategory(ctEditCommands, this, SLOT(doEditCommand(int)));
    kdev_dispatcher->connectCategory(ctStateCommands, this, SLOT(doStateCommand(int)));
    
    kdev_menubar=new KMenuBar(this,"KDevelop_menubar");
    
    ///////////////////////////////////////////////////////////////////
// File-menu entries
  file_menu = new QPopupMenu;
  file_menu->insertItem(BarIcon("filenew"),i18n("&New..."),this,SLOT(slotFileNew()),0,ID_FILE_NEW);

  file_menu->insertItem(BarIcon("open"),i18n("&Open..."), this, SLOT(slotFileOpen()),0 ,ID_FILE_OPEN);
  file_menu->insertItem(i18n("&Close"), this, SLOT(slotFileClose()),0,ID_FILE_CLOSE);
  file_menu->insertItem(i18n("Close All"), this, SLOT(slotFileCloseAll()), 0, ID_FILE_CLOSE_ALL);
  file_menu->insertSeparator();
  file_menu->insertItem(BarIcon("save"),i18n("&Save"), this, SLOT(slotFileSave()),0 ,ID_FILE_SAVE);
  file_menu->insertItem(i18n("Save &As..."), this, SLOT(slotFileSaveAs()),0 ,ID_FILE_SAVE_AS);
  file_menu->insertItem(BarIcon("save_all"),i18n("Save A&ll"), this, SLOT(slotFileSaveAll()),0,ID_FILE_SAVE_ALL);
  file_menu->insertSeparator();
  file_menu->insertItem(BarIcon("fileprint"),i18n("&Print..."), this, SLOT(slotFilePrint()),0 ,ID_FILE_PRINT);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("E&xit"),this, SLOT(slotFileQuit()),0 ,ID_FILE_QUIT);
  
  kdev_menubar->insertItem(i18n("&File"), file_menu);


///////////////////////////////////////////////////////////////////
// Edit-menu entries
  pixmap = loader->loadIcon("undo");
  edit_menu = new KGuiCmdPopup(kdev_dispatcher);//QPopupMenu;
  edit_menu->addCommand(ctEditCommands, cmUndo,pixmap, ID_EDIT_UNDO);

  pixmap = loader->loadIcon("redo");
  edit_menu->addCommand(ctEditCommands, cmRedo, pixmap, ID_EDIT_REDO);

  edit_menu->insertItem(i18n("Undo/Redo &History..."),this,SLOT(slotEditUndoHistory()),0,ID_EDIT_UNDO_HISTORY);

  edit_menu->insertSeparator();
  pixmap = loader->loadIcon("cut");
  edit_menu->addCommand(ctEditCommands, cmCut, pixmap, this,SLOT(slotEditCut()),ID_EDIT_CUT);
  pixmap = loader->loadIcon("copy");
  edit_menu->addCommand(ctEditCommands, cmCopy, pixmap, this, SLOT(slotEditCopy()), ID_EDIT_COPY);
  pixmap = loader->loadIcon("paste");
  edit_menu->addCommand(ctEditCommands, cmPaste, pixmap, this,SLOT(slotEditPaste()),ID_EDIT_PASTE);


  edit_menu->insertSeparator();
  pixmap = loader->loadIcon("indent");
  edit_menu->addCommand(ctEditCommands, cmIndent, pixmap, ID_EDIT_INDENT);
  pixmap = loader->loadIcon("unindent");
  edit_menu->addCommand(ctEditCommands, cmUnindent, pixmap, ID_EDIT_UNINDENT);


  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("&Insert File..."), this, SLOT(slotEditInsertFile()),0,ID_EDIT_INSERT_FILE);

  edit_menu->insertSeparator();
  pixmap = loader->loadIcon("search");
  edit_menu->addCommand(ctFindCommands, cmFind, pixmap, this, SLOT(slotEditSearch()), ID_EDIT_SEARCH);
  edit_menu->addCommand(ctFindCommands, cmFindAgain, this, SLOT(slotEditRepeatSearch()), ID_EDIT_REPEAT_SEARCH);

  
  edit_menu->addCommand(ctFindCommands, cmReplace, this, SLOT(slotEditReplace()), ID_EDIT_REPLACE);
  edit_menu->insertItem(BarIcon("grep"),i18n("&Search in Files..."), this, SLOT(slotEditSearchInFiles()),0,ID_EDIT_SEARCH_IN_FILES);
//  edit_menu->insertItem(i18n("Spell&check..."),this, SLOT(slotEditSpellcheck()),0,ID_EDIT_SPELLCHECK);

  edit_menu->insertSeparator();
  edit_menu->addCommand(ctEditCommands, cmSelectAll,pixmap, this,SLOT(slotEditSelectAll()),ID_EDIT_SELECT_ALL);
  edit_menu->addCommand(ctEditCommands, cmDeselectAll,pixmap,this,SLOT(slotEditDelectAll()), ID_EDIT_DESELECT_ALL);
  edit_menu->addCommand(ctEditCommands, cmInvertSelection, ID_EDIT_INVERT_SELECTION);


  
  kdev_menubar->insertItem(i18n("&Edit"), edit_menu);

  ///////////////////////////////////////////////////////////////////
  // View-menu entries
  view_menu = new KGuiCmdPopup(kdev_dispatcher);//QPopupMenu;
  view_menu->addCommand(ctFindCommands, cmGotoLine, this, SLOT(slotViewGotoLine()), ID_VIEW_GOTO_LINE);
//  view_menu->insertItem(i18n("Goto &Line..."), this,
//			SLOT(slotViewGotoLine()),0,ID_VIEW_GOTO_LINE)
  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Next Error"),this,
			SLOT(slotViewNextError()),0,ID_VIEW_NEXT_ERROR);
  view_menu->insertItem(i18n("&Previous Error"),this,
			SLOT(slotViewPreviousError()),0,ID_VIEW_PREVIOUS_ERROR);
  view_menu->insertSeparator();
  
  view_menu->insertItem(i18n("&Tree-View"),this,
			SLOT(slotViewTTreeView()),0,ID_VIEW_TREEVIEW);
  view_menu->insertItem(i18n("&Output-View"),this,
			SLOT(slotViewTOutputView()),0,ID_VIEW_OUTPUTVIEW);
  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Toolbar"),this,
			   SLOT(slotViewTStdToolbar()),0,ID_VIEW_TOOLBAR);
  view_menu->insertItem(i18n("&Browser-Toolbar"),this,
			   SLOT(slotViewTBrowserToolbar()),0,ID_VIEW_BROWSER_TOOLBAR);
  view_menu->insertItem(i18n("Status&bar"),this,
			   SLOT(slotViewTStatusbar()),0,ID_VIEW_STATUSBAR);
  view_menu->insertItem(i18n("&MDI-View Taskbar"),this,
			   SLOT(slotViewTMDIViewTaskbar()),0,ID_VIEW_MDIVIEWTASKBAR);
  view_menu->insertSeparator();
  view_menu->insertItem(BarIcon("reload"),i18n("&Refresh"),this,
			   SLOT(slotViewRefresh()),0,ID_VIEW_REFRESH);

  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Preview dialog"),dlgedit,
			SLOT(slotViewPreview()),0,ID_VIEW_PREVIEW);
  view_menu->insertItem(i18n("&Grid..."),dlgedit,
			SLOT(slotViewGridDlg()),0,ID_VIEW_GRID_DLG);
  
  kdev_menubar->insertItem(i18n("&View"), view_menu);
  

  ///////////////////////////////////////////////////////////////////
  // Project-menu entries
  project_menu = new QPopupMenu;
  project_menu->insertItem(i18n("New..."), this, SLOT(slotProjectNewAppl()),0,ID_PROJECT_KAPPWIZARD);
  project_menu->insertItem(BarIcon("openprj"),i18n("&Open..."), this, SLOT(slotProjectOpen()),0,ID_PROJECT_OPEN);

  recent_projects_menu = new QPopupMenu();
  connect( recent_projects_menu, SIGNAL(activated(int)), SLOT(slotProjectOpenRecent(int)) );
  project_menu->insertItem(i18n("Open &recent project..."), recent_projects_menu, ID_PROJECT_OPEN_RECENT );
  project_menu->insertItem(i18n(":Import..."), this, SLOT(slotProjectImport()),0,ID_PROJECT_IMPORT );
  project_menu->insertItem(i18n("C&lose"),this, SLOT(slotProjectClose()),0,ID_PROJECT_CLOSE);

  project_menu->insertSeparator();
  project_menu->insertItem(i18n("&New Class..."), this,
			   SLOT(slotProjectNewClass()),0,ID_PROJECT_NEW_CLASS);
  project_menu->insertItem(i18n("&Add existing File(s)..."),this,SLOT(slotProjectAddExistingFiles()),0,ID_PROJECT_ADD_FILE_EXIST);
  
  project_menu->insertItem(loader->loadIcon("locale", KIconLoader::Small),i18n("Add new &Translation File..."), this,
			   SLOT(slotProjectAddNewTranslationFile()),0,ID_PROJECT_ADD_NEW_TRANSLATION_FILE);

  //  project_menu->insertItem(i18n("&Remove File from Project"), this,
  //			   SLOT(slotProjectRemoveFile()),0,ID_PROJECT_REMOVE_FILE);
 		
  project_menu->insertItem(BarIcon("file_properties"),i18n("&File Properties..."), this, SLOT(slotProjectFileProperties())
			   ,0,ID_PROJECT_FILE_PROPERTIES);
  project_menu->insertSeparator();
  
  project_menu->insertItem(i18n("Make &messages and merge"), this, SLOT(slotProjectMessages()),0, ID_PROJECT_MESSAGES);
  project_menu->insertItem(i18n("Make AP&I-Doc"), this,
			 SLOT(slotProjectAPI()),0,ID_PROJECT_MAKE_PROJECT_API);
  project_menu->insertItem(BarIcon("mini-book1"), i18n("Make &User-Manual..."), this,
			 SLOT(slotProjectManual()),0,ID_PROJECT_MAKE_USER_MANUAL);
  // submenu for making dists

  QPopupMenu*  p2 = new QPopupMenu;
  p2->insertItem(i18n("&Source-tgz"), this, SLOT(slotProjectMakeDistSourceTgz()),0,ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ);
  project_menu->insertItem(i18n("Make D&istribution"),p2,ID_PROJECT_MAKE_DISTRIBUTION);
  project_menu->insertSeparator();
  
  project_menu->insertItem(i18n("&Options..."), this, SLOT(slotProjectOptions()),0,ID_PROJECT_OPTIONS);


  kdev_menubar->insertItem(i18n("&Project"), project_menu);
  

  ///////////////////////////////////////////////////////////////////
  // Build-menu entries
  build_menu = new QPopupMenu;
  build_menu->insertItem(BarIcon("generate"),i18n("&Generate Dialog Sources..."),dlgedit,
			       SLOT(slotBuildGenerate()),0,ID_KDLG_BUILD_GENERATE);  
  build_menu->insertItem(BarIcon("compfile"),i18n("Compile &File"),
			 this,SLOT(slotBuildCompileFile()),0,ID_BUILD_COMPILE_FILE);
  build_menu->insertItem(BarIcon("make"),i18n("&Make"),this,
			 SLOT(slotBuildMake()),0,ID_BUILD_MAKE);

//   build_menu->insertItem(Icon("make.png"),i18n("Make &with"),this,
// 			 SLOT(slotBuildMakeWith()),0,ID_BUILD_MAKE_WITH);
//   accel->changeMenuAccel(build_menu,ID_BUILD_MAKE_WITH ,"MakeWith" );

  build_menu->insertItem(BarIcon("rebuild"),i18n("&Rebuild all"), this,
			 SLOT(slotBuildRebuildAll()),0,ID_BUILD_REBUILD_ALL);

  build_menu->insertItem(i18n("&Clean/Rebuild all"), this, 
			 SLOT(slotBuildCleanRebuildAll()),0,ID_BUILD_CLEAN_REBUILD_ALL);
  build_menu->insertSeparator();
  build_menu->insertItem(BarIcon("stop_proc"),i18n("&Stop Build"), this, SLOT(slotBuildStop()),0,ID_BUILD_STOP);
  build_menu->insertSeparator();

  build_menu->insertItem(BarIcon("run"),i18n("&Execute"),this,SLOT(slotBuildRun()),0,ID_BUILD_RUN);
  build_menu->insertItem(BarIcon("run"),i18n("Execute &with Arguments..."),this,SLOT(slotBuildRunWithArgs()),0,ID_BUILD_RUN_WITH_ARGS);
  build_menu->insertItem(BarIcon("debugger"),i18n("&Debug..."),this,SLOT(slotBuildDebug()),0,ID_BUILD_DEBUG);
  build_menu->insertSeparator();
  build_menu->insertItem(i18n("DistC&lean"),this,SLOT(slotBuildDistClean()),0,ID_BUILD_DISTCLEAN);
  build_menu->insertItem(i18n("&Autoconf and automake"),this,SLOT(slotBuildAutoconf()),0,ID_BUILD_AUTOCONF);
  build_menu->insertItem(i18n("C&onfigure..."), this, SLOT(slotBuildConfigure()),0,ID_BUILD_CONFIGURE);

  kdev_menubar->insertItem(i18n("&Build"), build_menu);



  ///////////////////////////////////////////////////////////////////
  // Tools-menu entries
  tools_menu = new QPopupMenu;
  kdev_menubar->insertItem(i18n("&Tools"), tools_menu);

  ///////////////////////////////////////////////////////////////////
  // Options-menu entries
  // submenu for setting printprograms
  QPopupMenu* p3 = new QPopupMenu;
  p3->insertItem(i18n("&A2ps..."), this, SLOT(slotOptionsConfigureA2ps()),0,ID_OPTIONS_PRINT_A2PS);
  p3->insertItem(i18n("&Enscript..."), this,
		  SLOT(slotOptionsConfigureEnscript()),0,ID_OPTIONS_PRINT_ENSCRIPT);

  options_menu = new QPopupMenu;
  options_menu->insertItem(i18n("&Editor..."),this,
			   SLOT(slotOptionsEditor()),0,ID_OPTIONS_EDITOR);
  options_menu->insertItem(i18n("Editor &Colors..."),this,
			   SLOT(slotOptionsEditorColors()),0,ID_OPTIONS_EDITOR_COLORS);
  options_menu->insertItem(i18n("Editor &Defaults..."),this,
			   SLOT(slotOptionsSyntaxHighlightingDefaults())
			   ,0,ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS);
  options_menu->insertItem(i18n("&Syntax Highlighting..."),this,
			   SLOT(slotOptionsSyntaxHighlighting()),0,ID_OPTIONS_SYNTAX_HIGHLIGHTING);
  options_menu->insertSeparator();
  options_menu->insertItem(i18n("Documentation &Browser..."),this,
			   SLOT(slotOptionsDocBrowser()),0,ID_OPTIONS_DOCBROWSER);

  options_menu->insertItem(i18n("Configure &Printer..."),p3,ID_OPTIONS_PRINT);
	options_menu->insertItem(i18n("Tools..."),this,SLOT(slotOptionsToolsConfigDlg()),0,ID_OPTIONS_TOOLS_CONFIG_DLG);
//  options_menu->insertItem(i18n("&Spellchecker..."),this,SLOT(slotOptionsSpellchecker()),0,ID_OPTIONS_SPELLCHECKER);
  options_menu->insertSeparator();
  // Temporary hack
  options_menu->insertItem(i18n("Customize..."),this,
			   SLOT(slotOptionsCustomize()),0,ID_OPTIONS_CUSTOMIZE);
  options_menu->insertItem(i18n("&KDevelop Setup..."),this,
			   SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);

  kdev_menubar->insertItem(i18n("&Options"), options_menu);
  
  ///////////////////////////////////////////////////////////////////
  // Window-menu entries
  kdev_menubar->insertItem(i18n("&Window"), mdi_main_frame->m_pMdi->m_pWindowMenu);
  kdev_menubar->insertSeparator();

  ///////////////////////////////////////////////////////////////////
  // PlugIns menu
  plugin_menu= new QPopupMenu;
  plugin_menu->insertItem(i18n("&Plugin Manager..."),this,SLOT(slotPluginPluginManager()),0,ID_PLUGIN_MANAGER);
  plugin_menu->insertSeparator();

  kdev_menubar->insertItem(i18n("&Plugins"),plugin_menu);
  
  ///////////////////////////////////////////////////////////////////
  // Bookmarks-menu entries
  bookmarks_menu=new QPopupMenu;
  bookmarks_menu->insertItem(i18n("&Set Bookmark..."),this,SLOT(slotBookmarksSet()),0,ID_BOOKMARKS_SET);
  bookmarks_menu->insertItem(i18n("&Add Bookmark..."),this,SLOT(slotBookmarksAdd()),0,ID_BOOKMARKS_ADD);
  bookmarks_menu->insertItem(i18n("&Clear Bookmarks"),this,SLOT(slotBookmarksClear()),0,ID_BOOKMARKS_CLEAR);
  bookmarks_menu->insertSeparator();

#warning FIXME Bookmarks need new implementation
  // KGuiCmdPopup* header_bookmarks = new KGuiCmdPopup(kdev_dispatcher);//new QPopupMenu();
//   header_widget->installBMPopup(header_bookmarks);
//   KGuiCmdPopup* cpp_bookmarks = new KGuiCmdPopup(kdev_dispatcher);//new QPopupMenu();
//   cpp_widget->installBMPopup(cpp_bookmarks);
	
  doc_bookmarks = new QPopupMenu();
  
 //  bookmarks_menu->insertItem(i18n("&Header Window"),header_bookmarks,31000);
//   bookmarks_menu->insertItem(i18n("C/C++ &Window"),cpp_bookmarks,31010);
//   bookmarks_menu->insertItem(i18n("&Browser Window"), doc_bookmarks,31020);
	
  kdev_menubar->insertItem(i18n("Book&marks"),bookmarks_menu);

  ///////////////////////////////////////////////////////////////////
  // Help-menu entries
  help_menu = new QPopupMenu();
  help_menu->insertItem(BarIcon("back"),i18n("&Back"),this, SLOT(slotHelpBack()),0,ID_HELP_BACK);
  help_menu->insertItem(BarIcon("forward"),i18n("&Forward"),this, SLOT(slotHelpForward()),0,ID_HELP_FORWARD);
  help_menu->insertSeparator();
  help_menu->insertItem(BarIcon("lookup"),i18n("&Search Marked Text"),this,
				 SLOT(slotHelpSearchText()),0,ID_HELP_SEARCH_TEXT);
  help_menu->insertItem(BarIcon("contents"),i18n("Search for Help on..."),this,SLOT(slotHelpSearch()),0,ID_HELP_SEARCH);
  
  help_menu->insertSeparator();
  help_menu->insertItem(BarIcon("mini-book1"),i18n("User Manual"),this,SLOT(slotHelpContents()),0 ,ID_HELP_CONTENTS);
  help_menu->insertItem(BarIcon("mini-book1"),i18n("Programming Handbook"),this,SLOT(slotHelpTutorial()),0 ,ID_HELP_TUTORIAL);
	help_menu->insertItem(BarIcon("idea"),i18n("Tip of the Day"), this, SLOT(slotHelpTipOfDay()), 0, ID_HELP_TIP_OF_DAY);
  help_menu->insertItem(i18n("KDevelop Homepage"),this, SLOT(slotHelpHomepage()),0,ID_HELP_HOMEPAGE);
  help_menu->insertItem(BarIcon("filemail"),i18n("Bug Report..."),this, SLOT(slotHelpBugReport()),0,ID_HELP_BUG_REPORT);
  help_menu->insertSeparator();
  help_menu->insertItem(i18n("C/C++-Reference"),this,SLOT(slotHelpReference()),0,ID_HELP_REFERENCE);
  help_menu->insertItem(BarIcon("mini-book1"),i18n("&Qt-Library"),this, SLOT(slotHelpQtLib()),0,ID_HELP_QT_LIBRARY);
  help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&Core-Library"),this,
                        SLOT(slotHelpKDECoreLib()),0,ID_HELP_KDE_CORE_LIBRARY);
  help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&GUI-Library"),this,
                        SLOT(slotHelpKDEGUILib()),0,ID_HELP_KDE_GUI_LIBRARY);
  help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&KFile-Library"),this,
                        SLOT(slotHelpKDEKFileLib()),0,ID_HELP_KDE_KFILE_LIBRARY);
  help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&HTML-Library"),this,
                        SLOT(slotHelpKDEHTMLLib()),0,ID_HELP_KDE_HTML_LIBRARY);
  help_menu->insertSeparator();
  help_menu->insertItem(i18n("Project &API-Doc"),this,
                        SLOT(slotHelpAPI()),0,ID_HELP_PROJECT_API);

  help_menu->insertItem(i18n("Project &User-Manual"),this,
                        SLOT(slotHelpManual()),0,ID_HELP_USER_MANUAL);
  help_menu->insertSeparator();
  help_menu->insertItem(i18n("About KDevelop..."),this, SLOT(slotHelpAbout()),0,ID_HELP_ABOUT);
  kdev_menubar->insertItem(i18n("&Help"), help_menu);

  ////////////////////////////////////////////////
  // Popupmenu for the classbrowser wizard button
  ////////////////////////////////////////////////

  classbrowser_popup = new QPopupMenu();
  classbrowser_popup->insertItem( i18n("Goto declaration"), this,
                                  SLOT(slotClassbrowserViewDeclaration()),0, ID_CV_VIEW_DECLARATION );
  classbrowser_popup->insertItem( i18n("Goto definition"), this,
                                  SLOT(slotClassbrowserViewDefinition()), 0, ID_CV_VIEW_DEFINITION );
  classbrowser_popup->insertSeparator();
  classbrowser_popup->insertItem( i18n("Goto class declaration"), this,
                                  SLOT(slotClassbrowserViewClass()), 0, ID_CV_VIEW_CLASS_DECLARATION);
  classbrowser_popup->insertItem( i18n("New class..."), this,
                                  SLOT(slotProjectNewClass()), 0, ID_PROJECT_NEW_CLASS);
  classbrowser_popup->insertSeparator();
  classbrowser_popup->insertItem( i18n("Add member function..."), this,
                                  SLOT(slotClassbrowserNewMethod()), 0, ID_CV_METHOD_NEW);
  classbrowser_popup->insertItem( i18n("Add member variable..."), this,
                                  SLOT(slotClassbrowserNewAttribute()), 0, ID_CV_ATTRIBUTE_NEW );
  classbrowser_popup->insertItem( BarIcon("graphview"), i18n("Show graphical classview"), this, 
  																SLOT(slotClassbrowserViewTree()),0, ID_CV_GRAPHICAL_VIEW);
  

  //  disableCommand(ID_FILE_NEW);
  disableCommand(ID_FILE_PRINT);
  
 
  // Why??  disableCommand(ID_VIEW_NEXT_ERROR);
  //  disableCommand(ID_VIEW_PREVIOUS_ERROR);

  disableCommand(ID_EDIT_UNDO);
  disableCommand(ID_EDIT_REDO);
  

  disableCommand(ID_PROJECT_CLOSE);
  disableCommand(ID_PROJECT_ADD_FILE_EXIST);

  //  disableCommand(ID_PROJECT_REMOVE_FILE);
  disableCommand(ID_PROJECT_NEW_CLASS);
  disableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  disableCommand(ID_PROJECT_FILE_PROPERTIES);
  disableCommand(ID_PROJECT_OPTIONS);
  disableCommand(ID_PROJECT_WORKSPACES);
  disableCommand(ID_PROJECT_MESSAGES);  	
  disableCommand(ID_PROJECT_MAKE_PROJECT_API);
  disableCommand(ID_PROJECT_MAKE_USER_MANUAL);
  disableCommand(ID_PROJECT_MAKE_DISTRIBUTION);
 
  disableCommand(ID_BUILD_RUN);
  disableCommand(ID_BUILD_RUN_WITH_ARGS);
  disableCommand(ID_BUILD_DEBUG);
  disableCommand(ID_BUILD_MAKE);
  disableCommand(ID_BUILD_REBUILD_ALL);
  disableCommand(ID_BUILD_STOP);
  disableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
  disableCommand(ID_BUILD_DISTCLEAN);
  disableCommand(ID_BUILD_AUTOCONF);
  disableCommand(ID_BUILD_CONFIGURE);
  
  disableCommand(ID_BUILD_COMPILE_FILE);
 
  disableCommand(ID_KDLG_BUILD_GENERATE);

  disableCommand(ID_CV_WIZARD);
  disableCommand(ID_CV_GRAPHICAL_VIEW);
  disableCommand(ID_CV_TOOLBAR_CLASS_CHOICE);
  disableCommand(ID_CV_TOOLBAR_METHOD_CHOICE);

  disableCommand(ID_HELP_BACK);
  disableCommand(ID_HELP_FORWARD);
  disableCommand(ID_HELP_PROJECT_API);
  disableCommand(ID_HELP_USER_MANUAL);

///////////////////////////////////////////////////////////////////
// connects for the statusbar help
  connect(file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p3,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(view_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p2,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(tools_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(options_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(bookmarks_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(help_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(classbrowser_popup,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

}

/*------------------------------------------ CKDevelop::initToolBar()
 * initToolBar()
 *   Initialize all toolbars.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::initToolBar(){
   
//  toolBar()->insertButton(Icon("filenew.png"),ID_FILE_NEW, false,i18n("New"));

  toolBar()->insertButton(BarIcon("openprj"),ID_PROJECT_OPEN, true,i18n("Open Project"));
  toolBar()->insertSeparator();
  toolBar()->insertButton(BarIcon("open"),ID_FILE_OPEN, true,i18n("Open File"));
  file_open_popup= new QPopupMenu();
  connect(file_open_popup, SIGNAL(activated(int)), SLOT(slotFileOpen(int)));
  toolBar()->setDelayedPopup(ID_FILE_OPEN, file_open_popup);

  toolBar()->insertButton(BarIcon("save"),ID_FILE_SAVE,true,i18n("Save File"));
//  toolBar()->insertButton(Icon("save_all.png"),ID_FILE_SAVE_ALL,true,i18n("Save All"));

  toolBar()->insertButton(BarIcon("print"),ID_FILE_PRINT,false,i18n("Print"));

  QFrame *separatorLine= new QFrame(toolBar());
  separatorLine->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar()->insertWidget(0,20,separatorLine);
	
  toolBar()->insertButton(BarIcon("undo"),ID_EDIT_UNDO,false,i18n("Undo"));
  toolBar()->insertButton(BarIcon("redo"),ID_EDIT_REDO,false,i18n("Redo"));
  toolBar()->insertSeparator();
  toolBar()->insertButton(BarIcon("cut"),ID_EDIT_CUT,true,i18n("Cut"));
  toolBar()->insertButton(BarIcon("copy"),ID_EDIT_COPY, true,i18n("Copy"));
  toolBar()->insertButton(BarIcon("paste"),ID_EDIT_PASTE, true,i18n("Paste"));
	
  QFrame *separatorLine1= new QFrame(toolBar());
  separatorLine1->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar()->insertWidget(0,20,separatorLine1);

  toolBar()->insertButton(BarIcon("compfile"),ID_BUILD_COMPILE_FILE, false,i18n("Compile file"));
  toolBar()->insertButton(BarIcon("make"),ID_BUILD_MAKE, false,i18n("Make"));
  toolBar()->insertButton(BarIcon("rebuild"),ID_BUILD_REBUILD_ALL, false,i18n("Rebuild"));
  toolBar()->insertSeparator();
  toolBar()->insertButton(BarIcon("debugger"),ID_BUILD_DEBUG, false, i18n("Debug"));
  toolBar()->insertButton(BarIcon("run"),ID_BUILD_RUN, false,i18n("Run"));
  toolBar()->insertSeparator();
  toolBar()->insertButton(BarIcon("stop_proc"),ID_BUILD_STOP, false,i18n("Stop"));

  QFrame *separatorLine2= new QFrame(toolBar());
  separatorLine2->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar()->insertWidget(0,20,separatorLine2);
  toolBar()->insertButton(BarIcon("tree_win"),ID_VIEW_TREEVIEW, true,i18n("Tree-View"));
  toolBar()->insertButton(BarIcon("output_win"),ID_VIEW_OUTPUTVIEW, true,i18n("Output-View"));
  toolBar()->setToggle(ID_VIEW_TREEVIEW);
  toolBar()->setToggle(ID_VIEW_OUTPUTVIEW);

 QFrame *separatorLine3= new QFrame(toolBar());
 separatorLine3->setFrameStyle(QFrame::VLine|QFrame::Sunken);
 toolBar()->insertWidget(0,20,separatorLine3);


  QToolButton *btnwhat = QWhatsThis::whatsThisButton(toolBar());
  QToolTip::add(btnwhat, i18n("What's this...?"));
  toolBar()->insertWidget(ID_HELP_WHATS_THIS, btnwhat->sizeHint().width(), btnwhat);
  btnwhat->setFocusPolicy(QWidget::NoFocus);

  connect(toolBar(), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  connect(toolBar(), SIGNAL(pressed(int)), SLOT(statusCallback(int)));
	
  /////////////////////
  // the second toolbar
  /////////////////////

  // Class combo
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo(i18n("Classes"),
                                           ID_CV_TOOLBAR_CLASS_CHOICE,true,
                                           SIGNAL(activated(int))
                                           ,this,
                                           SLOT(slotClassChoiceCombo(int)),
                                           true,i18n("Classes"),160 );

  QComboBox* class_combo = toolBar(1)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  class_combo->setFocusPolicy(QWidget::NoFocus);

  // Method combo
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo(i18n("Methods"),
                                           ID_CV_TOOLBAR_METHOD_CHOICE,true,
                                           SIGNAL(activated(int))
                                           ,this,SLOT(slotMethodChoiceCombo(int)),
                                           true,i18n("Methods"),240 );

  QComboBox* choice_combo = toolBar(1)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  choice_combo->setFocusPolicy(QWidget::NoFocus);

  // Classbrowserwizard click button
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("classwiz"),
                                            ID_CV_WIZARD, true,
                                            i18n("Declaration/Definition"));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_CV_WIZARD,
                                               classbrowser_popup);
  
  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("back"),ID_HELP_BACK, false,i18n("Back"));
  history_prev = new QPopupMenu();
  connect(history_prev, SIGNAL(activated(int)), SLOT(slotHelpHistoryBack(int)));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_HELP_BACK, history_prev);
	
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("forward"),ID_HELP_FORWARD, false,i18n("Forward"));
  history_next = new QPopupMenu();
  connect(history_next, SIGNAL(activated(int)), SLOT(slotHelpHistoryForward(int)));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_HELP_FORWARD, history_next);
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("stop"),ID_HELP_BROWSER_STOP, false,i18n("Stop"));
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("reload_page"),ID_HELP_BROWSER_RELOAD, true,i18n("Reload"));
	toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("start_page"), ID_HELP_CONTENTS, true, i18n("User Manual"));
	
  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("lookup"), ID_HELP_SEARCH_TEXT,
					    true,i18n("Search Text in Documenation"));
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("contents"),ID_HELP_SEARCH,
              true,i18n("Search for Help on..."));
	
  connect(toolBar(ID_BROWSER_TOOLBAR), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  connect(toolBar(ID_BROWSER_TOOLBAR), SIGNAL(pressed(int)), SLOT(statusCallback(int)));

}

/*------------------------------------------ CKDevelop::initStatusBar()
 * initStatusBar()
 *   Initialize the statusbar.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::initStatusBar(){
  kdev_statusbar= new KStatusBar(this,"KDevelop_statusbar");
/*
  statProg = new KProgress(0,100,0,KProgress::Horizontal,kdev_statusbar,"Progressbar");
  statProg->setBarColor("blue");
  statProg->setBarStyle(KProgress::Solid);  // Solid or Blocked
  statProg->setTextEnabled(false);
  statProg->setBackgroundMode(PaletteBackground);
  statProg->setLineWidth( 1 );                // hehe, this *is* tricky...
*/
  statProg = new QProgressBar(kdev_statusbar,"Progressbar");
  kdev_statusbar->insertItem(i18n("xxxxxxxxxxxxxxxxxxxx"), ID_STATUS_EMPTY);
  kdev_statusbar->insertItem(i18n("Line: 00000 Col: 000"), ID_STATUS_LN_CLM);
  kdev_statusbar->changeItem("", ID_STATUS_EMPTY);
  kdev_statusbar->changeItem("", ID_STATUS_LN_CLM);

  kdev_statusbar->insertItem(i18n(" INS "), ID_STATUS_INS_OVR);
//  kdev_statusbar->insertItem(i18n(" CAPS "), ID_STATUS_CAPS);
  kdev_statusbar->insertItem(i18n("yyyyyyyyyyyyyy"),ID_STATUS_EMPTY_2);
  kdev_statusbar->changeItem("", ID_STATUS_EMPTY_2);

  kdev_statusbar->insertWidget(statProg,150, ID_STATUS_PROGRESS);
  kdev_statusbar->insertItem("", ID_STATUS_MSG);
  kdev_statusbar->setInsertOrder(KStatusBar::RightToLeft);
  kdev_statusbar->setAlignment(ID_STATUS_INS_OVR, AlignCenter);

  setStatusBar(kdev_statusbar);
  enableStatusBar();
}

/*--------------------------------------- CKDevelop::initConnections()
 * initConnections()
 *   Create the signal<->slot mappings.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CKDevelop::initConnections(){
  // the clipboard change signal is not needed in kwrite (jochen)
  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(slotClipboardChanged()));

  connect(t_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotTTabSelected(int)));

  connect(class_tree,SIGNAL(setStatusbarProgressSteps(int)),statProg,SLOT(setTotalSteps(int)));
  connect(class_tree,SIGNAL(setStatusbarProgress(int)),statProg,SLOT(setProgress(int)));
  connect(class_tree,SIGNAL(resetStatusbarProgress()),statProg,SLOT(reset()));
  connect(class_tree, SIGNAL(selectedFileNew()), SLOT(slotProjectAddNewFile()));
  connect(class_tree, SIGNAL(selectedClassNew()), SLOT(slotProjectNewClass()));
  connect(class_tree, SIGNAL(selectedProjectOptions()), SLOT(slotProjectOptions()));
  connect(class_tree, SIGNAL(selectedViewDeclaration(const char *, const char *,THType)), SLOT(slotCVViewDeclaration(const char *, const char *,THType)));
  connect(class_tree, SIGNAL(selectedViewDefinition(const char *, const char *,THType)), SLOT(slotCVViewDefinition(const char *, const char *,THType)));
  connect(class_tree, SIGNAL(signalAddMethod(const char *)), SLOT(slotCVAddMethod(const char * )));
  connect(class_tree, SIGNAL(signalAddAttribute(const char *)), SLOT(slotCVAddAttribute(const char * )));
  connect(class_tree, SIGNAL(signalMethodDelete(const char *,const char *)), SLOT(slotCVDeleteMethod(const char *,const char *)));
  connect(class_tree, SIGNAL(popupHighlighted(int)), SLOT(statusCallback(int)));
  
  connect(log_file_tree, SIGNAL(logFileTreeSelected(QString)), SLOT(slotLogFileTreeSelected(QString)));
  connect(log_file_tree, SIGNAL(selectedNewClass()), SLOT(slotProjectNewClass()));
  connect(log_file_tree, SIGNAL(selectedNewFile()), SLOT(slotProjectAddNewFile()));
  connect(log_file_tree, SIGNAL(selectedFileRemove(QString)), SLOT(delFileFromProject(QString)));
  connect(log_file_tree, SIGNAL(showFileProperties(QString)),SLOT(slotShowFileProperties(QString)));
  connect(log_file_tree, SIGNAL(menuItemHighlighted(int)), SLOT(statusCallback(int)));
  connect(log_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(log_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString)));   

  connect(real_file_tree, SIGNAL(fileSelected(QString)), SLOT(slotRealFileTreeSelected(QString)));
  connect(real_file_tree, SIGNAL(showFileProperties(QString)),SLOT(slotShowFileProperties(QString)));
  connect(real_file_tree, SIGNAL(addFileToProject(QString)),SLOT(slotAddFileToProject(QString)));
  connect(real_file_tree, SIGNAL(removeFileFromProject(QString)),SLOT(delFileFromProject(QString)));
  connect(real_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(real_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString))); 
  connect(real_file_tree, SIGNAL(commitDirToVCS(QString)), SLOT(slotCommitDirToVCS(QString)));
  connect(real_file_tree, SIGNAL(updateDirFromVCS(QString)), SLOT(slotUpdateDirFromVCS(QString))); 


  connect(doc_tree, SIGNAL(fileSelected(const QString&)), SLOT(slotDocTreeSelected(const QString&)));
  connect(doc_tree, SIGNAL(projectAPISelected()), SLOT(slotHelpAPI()));
  connect(doc_tree, SIGNAL(projectManualSelected()), SLOT(slotHelpManual()));


  // connect Docbrowser rb menu

  connect(browser_widget, SIGNAL(urlClicked(const QString&,const QString&,int)),
          this, SLOT(slotURLSelected(const QString&,const QString&,int)));
  connect(browser_widget, SIGNAL(onURL(const QString&)),
          this, SLOT(slotURLonURL(const QString&)));
  connect(browser_widget, SIGNAL(textSelected(bool)),
          this, SLOT(slotBROWSERMarkStatus(bool)));
  connect(browser_widget, SIGNAL(completed()),
          this, SLOT(slotDocumentDone()));
  connect(browser_widget, SIGNAL(signalURLBack()),this,SLOT(slotHelpBack()));
  connect(browser_widget, SIGNAL(signalURLForward()),this,SLOT(slotHelpForward()));
  connect(browser_widget, SIGNAL(signalBookmarkAdd()),this,SLOT(slotBookmarksAdd()));
  connect(browser_widget, SIGNAL(signalSearchText()),this,SLOT(slotHelpSearchText()));
  connect(browser_widget, SIGNAL(enableStop(int)), this, SLOT(enableCommand(int)));	
  connect(browser_widget->popup(), SIGNAL(highlighted(int)), this, SLOT(statusCallback(int)));
  connect(browser_widget, SIGNAL(signalGrepText(QString)), this, SLOT(slotEditSearchInFiles(QString)));
  
  connect(browser_view,SIGNAL(focusInEventOccurs(QextMdiChildView*)),this,SLOT(slotMDIGetFocus(QextMdiChildView*)));

  // connect the windowsmenu with a method
  connect(doc_bookmarks, SIGNAL(activated(int)), this, SLOT(slotBoomarksBrowserSelected(int)));

  connect(messages_widget, SIGNAL(processExited(KProcess*)),
          this, SLOT(slotProcessExited(KProcess*)));
  connect(messages_widget, SIGNAL(itemSelected(const QString&,int)),
          SLOT(slotSwitchFileRequest(const QString&,int)));
  connect(grepview, SIGNAL(itemSelected(const QString&,int)),
          SLOT(slotSwitchFileRequest(const QString&,int)));

  // connections for the proc -processes
  connect(&search_process, SIGNAL(receivedStdout(KProcess*,char*,int)),
          this, SLOT(slotSearchReceivedStdout(KProcess*,char*,int)) );

#if 0
  // let's see
  connect(&search_process, SIGNAL(receivedStderr(KProcess*,char*,int)),
          this, SLOT(slotReceivedStderr(KProcess*,char*,int)) );
#endif

  connect(&search_process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotSearchProcessExited(KProcess*) )) ;

  connect(mdi_main_frame,SIGNAL(letKDevelopDoTheSwitchToChildframeMode()),
          this,SLOT(slotSwitchToChildframeMode()) );
  connect(mdi_main_frame,SIGNAL(letKDevelopDoTheSwitchToToplevelMode()),
          this,SLOT(slotSwitchToToplevelMode()) );
}
void CKDevelop::initDlgEditor(){
    // the dialog editor manager
    dlgedit = new DlgEdit(widprop_split_view->getWidgetsView(),widprop_split_view->getPropertyView());
    ComponentManager::self()->registerComponent(dlgedit);   
}


void CKDevelop::initProject()
{
    QString filename;
  config->setGroup("General Options");
  bool bLastProject =true;
  if(!kapp->argc() > 1)
    bLastProject= config->readBoolEntry("LastProject",true);
  else
    bLastProject=false;

  if(bLastProject){
    config->setGroup("Files");
    filename = config->readEntry("project_file");
  }

  QFile file(filename);
  
  if (file.exists()){
   
    if(!(readProjectFile(filename))){
      KMessageBox::sorry(0, "This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\n"
                         "Please use only new generated projects!");
      refreshTrees();
    }
    else{
      setCursor(KCursor::waitCursor());
      QProgressDialog *progressDlg= new QProgressDialog(NULL, "progressDlg", true );
      connect(class_tree,SIGNAL(setStatusbarProgressSteps(int)),progressDlg,SLOT(setTotalSteps(int)));
      connect(class_tree,SIGNAL(setStatusbarProgress(int)),progressDlg,SLOT(setProgress(int)));
      progressDlg->setCaption(i18n("Starting..."));
      progressDlg->setLabelText( i18n("Initializing last project...\nPlease wait...\n") );
      progressDlg->setCancelButtonText(i18n("Cancel"));
      progressDlg->setProgress(0);
      progressDlg->show();
      setCursor(KCursor::arrowCursor());
      refreshTrees();
      delete progressDlg;
      
    }
    config->setGroup("Files");
    filename = config->readEntry("header_file",i18n("Untitled.h"));
    QFile _file(filename);
    
    if (QFile::exists(filename)){
      switchToFile(filename);
      
    }

    filename = config->readEntry("cpp_file", i18n("Untitled.cpp"));
    if (QFile::exists(filename)){
      switchToFile(filename);
    }
  }
  else{
    refreshTrees(); // this refresh only the documentation tab,tree
  }
  
}


void CKDevelop::setKeyAccel(){    
    
    accel->connectItem( KStdAccel::Open , this, SLOT(slotFileOpen()), true, ID_FILE_OPEN);
    accel->connectItem( KStdAccel::Close , this, SLOT(slotFileClose()), true, ID_FILE_CLOSE);
    accel->connectItem( KStdAccel::Save , this, SLOT(slotFileSave()), true, ID_FILE_SAVE);
    
    
    accel->changeMenuAccel(file_menu, ID_FILE_NEW, KStdAccel::New );
    accel->changeMenuAccel(file_menu, ID_FILE_OPEN, KStdAccel::Open );
    accel->changeMenuAccel(file_menu, ID_FILE_CLOSE, KStdAccel::Close );
    accel->changeMenuAccel(file_menu, ID_FILE_SAVE, KStdAccel::Save );
    accel->changeMenuAccel(file_menu, ID_FILE_PRINT, KStdAccel::Print );
    accel->changeMenuAccel(file_menu, ID_FILE_QUIT, KStdAccel::Quit );

    accel->changeMenuAccel(edit_menu, ID_EDIT_SEARCH_IN_FILES,"Grep" );


    accel->changeMenuAccel(view_menu,ID_VIEW_NEXT_ERROR ,"NextError" );
    accel->changeMenuAccel(view_menu,ID_VIEW_PREVIOUS_ERROR ,"PreviousError" );
    accel->changeMenuAccel(view_menu,ID_VIEW_TREEVIEW ,"Tree-View" );
    accel->changeMenuAccel(view_menu,ID_VIEW_OUTPUTVIEW,"Output-View" );

    accel->changeMenuAccel(project_menu,ID_PROJECT_OPTIONS ,"ProjectOptions" );
    accel->changeMenuAccel(project_menu,ID_PROJECT_FILE_PROPERTIES ,"FileProperties" );

    accel->changeMenuAccel(build_menu,ID_BUILD_COMPILE_FILE ,"CompileFile" );
    accel->changeMenuAccel(build_menu,ID_BUILD_MAKE ,"Make" );
    accel->changeMenuAccel(build_menu,ID_BUILD_RUN ,"Run" );
    accel->changeMenuAccel(build_menu,ID_BUILD_RUN_WITH_ARGS,"Run_with_args");
    accel->changeMenuAccel(build_menu,ID_BUILD_STOP,"Stop_proc");
    
    accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_ADD ,"Add_Bookmarks" );
    accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_CLEAR ,"Clear_Bookmarks" );
    
    accel->changeMenuAccel(help_menu,ID_HELP_SEARCH_TEXT,"SearchMarkedText" );
    accel->changeMenuAccel(help_menu, ID_HELP_CONTENTS, KStdAccel::Help );
    
}

void CKDevelop::setToolmenuEntries(){
  config = kapp->config();
  config->setGroup("ToolsMenuEntries");
	config->readListEntry("Tools_exe",tools_exe);
	config->readListEntry("Tools_entry",tools_entry);
	config->readListEntry("Tools_argument",tools_argument);
	
	uint items;
	for(items=0;items<tools_entry.count();items++){
		tools_menu->insertItem(tools_entry.at(items));
	}
  
  
	
	connect(tools_menu,SIGNAL(activated(int)),SLOT(slotToolsTool(int)));
       

}

