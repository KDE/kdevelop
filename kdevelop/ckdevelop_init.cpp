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
	: KDockMainWindow("CKDevelop_mainwindow")
  ,m_search_process("/bin/sh")
  ,m_dockbase_brkptManager_view(0L)
  ,m_bIsDebuggingInternal(false)
  ,m_bIsDebuggingExternal(false)
  ,m_bInternalDbgChosen(true)
{
  m_version = VERSION;
  m_project=false;// no project
  m_beep=false; // no beep
  m_cv_decl_or_impl=true;
  m_file_open_list.setAutoDelete(TRUE);
  
  m_config = kapp->config();
  m_kdev_caption=kapp->caption();

  initView();
  
  initConnections();

  initWhatsThis();
	
  readOptions();
	
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
   m_mdi_main_frame->setMenuForSDIModeSysButtons( 0L);   // tell QextMDI that the menubar is destructed
}

void CKDevelop::initView(){
  m_act_outbuffer_len=0;
  m_prj = 0;

  QFont font("Fixed",10);

  ////////////////////////
  // Outputwindow
  ////////////////////////
  KIconLoader *il = KGlobal::iconLoader();

  m_dockbase_messages_widget = createDockWidget(i18n("messages"), BarIcon(""), 0L, i18n("Messages"));
  m_messages_widget = new MakeView(0L, "messages_widget");
  m_dockbase_messages_widget->setWidget(m_messages_widget);

  m_dockbase_grepview = createDockWidget(i18n("search"), QPixmap( il->loadIcon( "find.png", KIcon::Small )), 0L, i18n("Search"), "" );
  m_grepview = new GrepView(0L, "grepview");
	m_dockbase_grepview->setWidget(m_grepview);
	
  m_dockbase_outputview = createDockWidget(i18n("output"), BarIcon(""), 0L, i18n("Output"));
  m_outputview = new OutputView(0L, "outputview");
	m_dockbase_outputview->setWidget(m_outputview);

  ////////////////////////
  // Treeviews
  ////////////////////////
  m_dockbase_class_tree = createDockWidget(i18n("CV"), QPixmap(locate("appdata", "pics/mini/CVclass.png")), 0L, i18n("Class view"), "" );
  m_class_tree = new CClassView(0L,"cv");
  m_dockbase_class_tree->setWidget(m_class_tree);
  m_dockbase_class_tree->setToolTipString(i18n("class tree view"));

  m_dockbase_log_file_tree = createDockWidget(i18n("LFV"), QPixmap( il->loadIcon( "kdevelop.png", KIcon::Small )), 0L, i18n("Logical file view"), "" );
  m_log_file_tree = new CLogFileView(0L,"lfv",m_config->readBoolEntry("lfv_show_path",false));
  m_dockbase_log_file_tree->setWidget(m_log_file_tree);
  m_dockbase_log_file_tree->setToolTipString(i18n("logical file tree view"));

  m_dockbase_real_file_tree = createDockWidget(i18n("RFV"), QPixmap( il->loadIcon( "folder.png", KIcon::Small )), 0L, i18n("Real file view"), "" );
  m_real_file_tree = new CRealFileView(0L,"RFV");
  m_dockbase_real_file_tree->setWidget(m_real_file_tree);
  m_dockbase_real_file_tree->setToolTipString(i18n("real file tree view"));

  m_dockbase_doc_tree = createDockWidget(i18n("DOC"), BarIcon("mini-book1"), 0L, i18n("Documentation"), "");
  m_doc_tree = new DocTreeView(0L,"DOC");
  m_dockbase_doc_tree->setWidget(m_doc_tree);
  m_dockbase_doc_tree->setToolTipString(i18n("documentation tree view"));

  m_dockbase_widprop_split_view = createDockWidget(i18n("DLG"), BarIcon("newwidget.xpm"), 0L, i18n("Dialog editor"), "");
  m_widprop_split_view = new WidgetsPropSplitView(0L,"DLG");
  m_dockbase_widprop_split_view->setWidget(m_widprop_split_view);
  m_dockbase_widprop_split_view->setToolTipString(i18n("dialog editor view"));

  initDlgEditor();

  ////////////////////////
  // Right main window
  ////////////////////////
  
  m_dockbase_mdi_main_frame = createDockWidget(i18n("MDI Mainframe"), BarIcon("filenew"));
  m_mdi_main_frame = new MdiFrame( 0, "mdi_frame");
  m_dockbase_mdi_main_frame->setWidget(m_mdi_main_frame);
  setView(m_dockbase_mdi_main_frame);
  setMainDockWidget( m_dockbase_mdi_main_frame );

  // maybe we should make this configurable :-)
  m_mdi_main_frame->setBackgroundPixmap(QPixmap(locate("wallpaper","Magneto_Bomb.jpg")));
  
  
#warning FIXME should we swallow tools in KDevelop 2??
  //  m_swallow_widget = new KSwallowWidget(t_tab_view);
  //  m_swallow_widget->setFocusPolicy(QWidget::StrongFocus);
  
  //  m_swallow_widget = new KSwallowWidget(s_tab_view);
  //  m_swallow_widget->setFocusPolicy(QWidget::StrongFocus);
  
  
  ComponentManager *manager = ComponentManager::self();
  manager->registerComponent(m_class_tree);
  manager->registerComponent(m_log_file_tree);
  manager->registerComponent(m_real_file_tree);
  manager->registerComponent(m_doc_tree);
  manager->registerComponent(m_outputview);
  manager->registerComponent(m_messages_widget);
  manager->registerComponent(m_grepview);


  //
  // dock the widgets
  //
  // ...the output views
  m_dockbase_messages_widget->manualDock(m_dockbase_mdi_main_frame, KDockWidget::DockBottom, 70/*size relation in %*/);
  m_dockbase_grepview->manualDock(m_dockbase_messages_widget, KDockWidget::DockCenter);
  m_dockbase_outputview->manualDock(m_dockbase_messages_widget, KDockWidget::DockCenter);
	// ...the tree views
  m_dockbase_class_tree->manualDock(m_dockbase_mdi_main_frame, KDockWidget::DockLeft, 35/*size relation in %*/);
  m_dockbase_log_file_tree->manualDock(m_dockbase_class_tree, KDockWidget::DockCenter);
  m_dockbase_real_file_tree->manualDock(m_dockbase_class_tree, KDockWidget::DockCenter);
  m_dockbase_doc_tree->manualDock(m_dockbase_class_tree, KDockWidget::DockCenter);
/*m_dockbase_t_tab_view = */m_dockbase_widprop_split_view->manualDock(m_dockbase_class_tree, KDockWidget::DockCenter);
//  QString nameOfTreeSuperDock = m_dockbase_t_tab_view->name();	

	// read all settings concerning to the dockwidget´s stuff
	// Note: this has to be done _before_ we disable some dock positions
  readDockConfig( m_config, "EditMode Dock-Settings");
	// recover broken pointers after the read of the configuration
//  m_dockbase_o_tab_view = dockManager->getDockWidgetFromName( nameOfOutputSuperDock);
//  m_dockbase_o_tab_view->setCaption(i18n("Output views"));
//  QObject::connect(m_dockbase_o_tab_view, SIGNAL(headerCloseButtonClicked()), this, SLOT(slotViewTOutputView()));
//  m_dockbase_t_tab_view = dockManager->getDockWidgetFromName( nameOfTreeSuperDock);
//  m_dockbase_t_tab_view->setCaption(i18n("Tree views"));
//  QObject::connect(m_dockbase_t_tab_view, SIGNAL(headerCloseButtonClicked()), this, SLOT(slotViewTTreeView()));

	m_dockbase_mdi_main_frame->setEnableDocking( KDockWidget::DockNone);
	m_dockbase_mdi_main_frame->setDockSite( KDockWidget::DockCorner);	

  initKeyAccel();

  initMenuBar();
  m_mdi_main_frame->setMenuForSDIModeSysButtons(m_kdev_menubar);

  initToolBar();
	// don´t show the dock-toolbar of KDockMainWindow, we don´t need it
  toolBar()->hide();

  initStatusBar();
  
  //
	// open the documentation browser view on default
	//   (just open it when the menubar is up)
  m_browser_view = new DocBrowserView(m_mdi_main_frame,"browser");
  m_browser_widget = m_browser_view->browser;
  m_mdi_main_frame->addWindow(m_browser_view, QextMdi::Maximize); // attached, shown and focused by default

  m_prev_was_search_result= false;
  //init
  m_browser_widget->setDocBrowserOptions();
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
  m_accel = new CKDevAccel( this );
  //file menu
  m_accel->connectItem( KStdAccel::Open , this, SLOT(slotFileOpen()), true, ID_FILE_OPEN);
  m_accel->connectItem( KStdAccel::Close , this, SLOT(slotFileClose()), true, ID_FILE_CLOSE);
  
  m_accel->connectItem( KStdAccel::Save , this, SLOT(slotFileSave()), true, ID_FILE_SAVE);
  
  m_accel->insertItem(i18n("Save As"), "SaveAs", 0);
  m_accel->connectItem( "SaveAs", this, SLOT(slotFileSaveAs()), true, ID_FILE_SAVE_AS);
  
  m_accel->insertItem(i18n("Save All"), "SaveAll", 0);
  m_accel->connectItem( "SaveAll", this, SLOT(slotFileSaveAll()), true, ID_FILE_SAVE_ALL);
  
  m_accel->connectItem( KStdAccel::Print , this, SLOT(slotFilePrint()), true, ID_FILE_PRINT);
  m_accel->connectItem( KStdAccel::Quit, this, SLOT(slotFileQuit()), true, ID_FILE_QUIT);

  //edit menu  

  m_accel->insertItem( i18n("Insert File"), "InsertFile", 0);
  m_accel->connectItem( "InsertFile", this, SLOT(slotEditInsertFile()), true, ID_EDIT_INSERT_FILE);

  m_accel->insertItem( i18n("Search in Files"), "Grep", IDK_EDIT_SEARCH_IN_FILES );
  m_accel->connectItem( "Grep", this, SLOT(slotEditSearchInFiles() ), true, ID_EDIT_SEARCH_IN_FILES );

  m_accel->insertItem( i18n("Search selection in Files"), "GrepSearch", IDK_EDIT_GREP_IN_FILES );
  m_accel->connectItem( "GrepSearch", this, SLOT(slotEditSearchText() ) );

  //view menu

  m_accel->insertItem( i18n("Next Error"), "NextError",IDK_VIEW_NEXT_ERROR);
  m_accel->connectItem( "NextError", this, SLOT( slotViewNextError()), true, ID_VIEW_NEXT_ERROR);
  
  m_accel->insertItem( i18n("Previous Error"), "PreviousError",IDK_VIEW_PREVIOUS_ERROR);
  m_accel->connectItem( "PreviousError", this, SLOT( slotViewPreviousError()), true, ID_VIEW_PREVIOUS_ERROR);
  
  m_accel->insertItem( i18n("Toogle Tree-View"), "Tree-View",IDK_VIEW_TREEVIEW);
  m_accel->connectItem( "Tree-View", this, SLOT(slotViewTTreeView()), true, ID_VIEW_TREEVIEW);
  
  m_accel->insertItem( i18n("Toogle Output-View"), "Output-View",IDK_VIEW_OUTPUTVIEW);
  m_accel->connectItem( "Output-View", this, SLOT(slotViewTOutputView()), true, ID_VIEW_OUTPUTVIEW);
  
  m_accel->insertItem( i18n("Toolbar"), "Toolbar", 0);
  m_accel->connectItem( "Toolbar", this, SLOT(slotViewTStdToolbar()), true, ID_VIEW_TOOLBAR);
  
  m_accel->insertItem( i18n("Browser-Toolbar"), "Browser-Toolbar", 0);
  m_accel->connectItem( "Browser-Toolbar", this, SLOT(slotViewTBrowserToolbar()), true, ID_VIEW_BROWSER_TOOLBAR);
	
  m_accel->insertItem( i18n("Statusbar"), "Statusbar", 0);
  m_accel->connectItem( "Statusbar", this, SLOT(slotViewTStatusbar()), true, ID_VIEW_STATUSBAR);
  
  m_accel->insertItem( i18n("MDI-View Taskbar"), "MDI-View Taskbar", 0);
  m_accel->connectItem( "MDI-View Taskbar", this, SLOT(slotViewTMDIViewTaskbar()), true, ID_VIEW_MDIVIEWTASKBAR);
  
  m_accel->insertItem( i18n("Preview dialog"), "Preview dialog",IDK_VIEW_PREVIEW);
  
  m_accel->insertItem( i18n("Refresh"), "Refresh", 0);
  m_accel->connectItem( "Refresh", this, SLOT(slotViewRefresh()), true, ID_VIEW_REFRESH);
  
  m_accel->insertItem( i18n("Graphical Classview"), "CVViewTree", 0);
  m_accel->connectItem( "CVViewTree", this, SLOT(slotClassbrowserViewTree()), true, ID_CV_GRAPHICAL_VIEW);
  
  // project menu
  m_accel->insertItem( i18n("New Project"), "NewProject",0);
  m_accel->connectItem( "NewProject", this, SLOT(slotProjectNewAppl()), true, ID_PROJECT_KAPPWIZARD);
  
  m_accel->insertItem( i18n("Open Project"), "OpenProject", 0);
  m_accel->connectItem( "OpenProject", this, SLOT(slotProjectOpen()), true, ID_PROJECT_OPEN);
  
  m_accel->insertItem( i18n("Close Project"), "CloseProject", 0);
  m_accel->connectItem("CloseProject", this, SLOT(slotProjectClose()), true, ID_PROJECT_CLOSE);
  
  m_accel->insertItem(i18n("New Class"), "NewClass", 0);
  m_accel->connectItem("NewClass", this, SLOT(slotProjectNewClass()), true, ID_PROJECT_NEW_CLASS);
  
  m_accel->insertItem(i18n("Add existing File(s)"), "AddExistingFiles", 0);
  m_accel->connectItem("AddExistingFiles",this, SLOT(slotProjectAddExistingFiles()), true, ID_PROJECT_ADD_FILE_EXIST);
  
  m_accel->insertItem(i18n("Add new Translation File"),"Add new Translation File", 0);
  m_accel->connectItem("Add new Translation File", this, SLOT(slotProjectAddNewTranslationFile()), true, ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  
  m_accel->insertItem(i18n("File Properties"), "FileProperties", IDK_PROJECT_FILE_PROPERTIES);
  m_accel->connectItem("FileProperties", this, SLOT(slotProjectFileProperties()), true, ID_PROJECT_FILE_PROPERTIES );
  
  m_accel->insertItem(i18n("Make messages and merge"), "MakeMessages", 0);
  m_accel->connectItem("MakeMessages", this, SLOT(slotProjectMessages()), true, ID_PROJECT_MESSAGES);
  
  m_accel->insertItem(i18n("Make API-Doc"), "ProjectAPI", 0);
  m_accel->connectItem("ProjectAPI", this, SLOT(slotProjectAPI()), true, ID_PROJECT_MAKE_PROJECT_API);
  
  m_accel->insertItem(i18n("Make User-Manual..."), "ProjectManual", 0);
  m_accel->connectItem("ProjectManual", this, SLOT(slotProjectManual()), true, ID_PROJECT_MAKE_USER_MANUAL);
  
  m_accel->insertItem(i18n("Make Source-tgz"), "Source-tgz", 0);
  m_accel->connectItem("Source-tgz", this, SLOT(slotProjectMakeDistSourceTgz()), true, ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ);
 	
  m_accel->insertItem(i18n("Project options"), "ProjectOptions", IDK_PROJECT_OPTIONS);
  m_accel->connectItem("ProjectOptions", this, SLOT(slotProjectOptions()), true, ID_PROJECT_OPTIONS);
  
  
  //build menu
  
  m_accel->insertItem( i18n("Compile File"), "CompileFile", IDK_BUILD_COMPILE_FILE );
  m_accel->connectItem( "CompileFile", this, SLOT( slotBuildCompileFile()), true, ID_BUILD_COMPILE_FILE);
  
  m_accel->insertItem( i18n("Make"), "Make", IDK_BUILD_MAKE );
  m_accel->connectItem( "Make", this, SLOT(slotBuildMake()), true, ID_BUILD_MAKE);
  
  m_accel->insertItem( i18n("Rebuild All"), "RebuildAll", 0);
  m_accel->connectItem( "RebuildAll", this, SLOT(slotBuildRebuildAll()), true, ID_BUILD_REBUILD_ALL);
  
  m_accel->insertItem( i18n("Clean/Rebuild all"), "CleanRebuildAll", 0);
  m_accel->connectItem( "CleanRebuildAll", this, SLOT(slotBuildCleanRebuildAll()), true, ID_BUILD_CLEAN_REBUILD_ALL);
  
  m_accel->insertItem( i18n("Stop process"), "Stop_proc", IDK_BUILD_STOP);
  m_accel->connectItem( "Stop_proc", this, SLOT(slotBuildStop()), true, ID_BUILD_STOP);
  
  m_accel->insertItem( i18n("Execute"), "Run", IDK_BUILD_RUN);
  m_accel->connectItem( "Run", this, SLOT(slotBuildRun() ), true, ID_BUILD_RUN);
  
  m_accel->insertItem( i18n("Execute with arguments"), "Run_with_args", IDK_BUILD_RUN_WITH_ARGS);
  m_accel->connectItem( "Run_with_args", this, SLOT(slotBuildRunWithArgs() ), true, ID_BUILD_RUN_WITH_ARGS);
  
  m_accel->insertItem( i18n("Debug"), "BuildDebug", 0);
  m_accel->connectItem("BuildDebug", this, SLOT(slotBuildDebug()), true, ID_BUILD_DEBUG);
  
  m_accel->insertItem( i18n("DistClean"), "BuildDistClean", 0);
  m_accel->connectItem("BuildDistClean",this, SLOT(slotBuildDistClean()), true, ID_BUILD_DISTCLEAN);
  
  m_accel->insertItem( i18n("Autoconf and automake"), "BuildAutoconf", 0);
  m_accel->connectItem("BuildAutoconf", this, SLOT(slotBuildAutoconf()), true, ID_BUILD_AUTOCONF);
  
  m_accel->insertItem( i18n("Configure..."), "BuildConfigure", 0);
  m_accel->connectItem( "BuildConfigure", this, SLOT(slotBuildConfigure()), true, ID_BUILD_CONFIGURE);
  
  
  //   m_accel->insertItem( i18n("Make with"), "MakeWith", IDK_BUILD_MAKE_WITH );
  //   m_accel->connectItem( "MakeWith", this, SLOT(slotBuildMakeWith() ), true, ID_BUILD_MAKE_WITH );
  
  // Tools-menu
  
  // Bookmarks-menu
  m_accel->insertItem( i18n("Add Bookmark"), "Add_Bookmarks", IDK_BOOKMARKS_ADD);
  m_accel->connectItem( "Add_Bookmarks", this, SLOT(slotBookmarksAdd()), true, ID_BOOKMARKS_ADD);

  m_accel->insertItem( i18n("Clear Bookmarks"), "Clear_Bookmarks", IDK_BOOKMARKS_CLEAR);
  m_accel->connectItem( "Clear_Bookmarks", this, SLOT(slotBookmarksClear()), true, ID_BOOKMARKS_CLEAR);

  //Help menu
  m_accel->connectItem( KStdAccel::Help , this, SLOT(slotHelpContents()), true, ID_HELP_CONTENTS);
  
  m_accel->insertItem( i18n("Search Marked Text"), "SearchMarkedText",IDK_HELP_SEARCH_TEXT);
  m_accel->connectItem( "SearchMarkedText", this, SLOT(slotHelpSearchText()), true, ID_HELP_SEARCH_TEXT);

  m_accel->insertItem( i18n("Search for Help on"), "HelpSearch", 0);
  m_accel->connectItem( "HelpSearch", this, SLOT(slotHelpSearch()), true, ID_HELP_SEARCH);
  
 


  m_accel->readSettings(0, false);
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
    m_kdev_dispatcher = new KGuiCmdDispatcher(this, &cmdMngr);
    m_kdev_dispatcher->connectCategory(ctCursorCommands, this, SLOT(doCursorCommand(int)));
    m_kdev_dispatcher->connectCategory(ctEditCommands, this, SLOT(doEditCommand(int)));
    m_kdev_dispatcher->connectCategory(ctStateCommands, this, SLOT(doStateCommand(int)));
    
    m_kdev_menubar=new KMenuBar(this,"KDevelop_menubar");
    
    ///////////////////////////////////////////////////////////////////
// File-menu entries
  m_file_menu = new QPopupMenu;
  m_file_menu->insertItem(BarIcon("filenew"),i18n("&New..."),this,SLOT(slotFileNew()),0,ID_FILE_NEW);

  m_file_menu->insertItem(BarIcon("open"),i18n("&Open..."), this, SLOT(slotFileOpen()),0 ,ID_FILE_OPEN);
  m_file_menu->insertItem(i18n("&Close"), this, SLOT(slotFileClose()),0,ID_FILE_CLOSE);
  m_file_menu->insertItem(i18n("Close All"), this, SLOT(slotFileCloseAll()), 0, ID_FILE_CLOSE_ALL);
  m_file_menu->insertSeparator();
  m_file_menu->insertItem(BarIcon("save"),i18n("&Save"), this, SLOT(slotFileSave()),0 ,ID_FILE_SAVE);
  m_file_menu->insertItem(i18n("Save &As..."), this, SLOT(slotFileSaveAs()),0 ,ID_FILE_SAVE_AS);
  m_file_menu->insertItem(BarIcon("save_all"),i18n("Save A&ll"), this, SLOT(slotFileSaveAll()),0,ID_FILE_SAVE_ALL);
  m_file_menu->insertSeparator();
  m_file_menu->insertItem(BarIcon("fileprint"),i18n("&Print..."), this, SLOT(slotFilePrint()),0 ,ID_FILE_PRINT);
  m_file_menu->insertSeparator();
  m_file_menu->insertItem(i18n("E&xit"),this, SLOT(slotFileQuit()),0 ,ID_FILE_QUIT);
  
  m_kdev_menubar->insertItem(i18n("&File"), m_file_menu);


///////////////////////////////////////////////////////////////////
// Edit-menu entries
  pixmap = loader->loadIcon("undo", KIcon::Small);
  m_edit_menu = new KGuiCmdPopup(m_kdev_dispatcher);//QPopupMenu;
  m_edit_menu->addCommand(ctEditCommands, cmUndo,pixmap, ID_EDIT_UNDO);

  pixmap = loader->loadIcon("redo", KIcon::Small);
  m_edit_menu->addCommand(ctEditCommands, cmRedo, pixmap, ID_EDIT_REDO);

  m_edit_menu->insertItem(i18n("Undo/Redo &History..."),this,SLOT(slotEditUndoHistory()),0,ID_EDIT_UNDO_HISTORY);

  m_edit_menu->insertSeparator();
  pixmap = loader->loadIcon("cut", KIcon::Small);
  m_edit_menu->addCommand(ctEditCommands, cmCut, pixmap, this,SLOT(slotEditCut()),ID_EDIT_CUT);
  pixmap = loader->loadIcon("copy", KIcon::Small);
  m_edit_menu->addCommand(ctEditCommands, cmCopy, pixmap, this, SLOT(slotEditCopy()), ID_EDIT_COPY);
  pixmap = loader->loadIcon("paste", KIcon::Small);
  m_edit_menu->addCommand(ctEditCommands, cmPaste, pixmap, this,SLOT(slotEditPaste()),ID_EDIT_PASTE);


  m_edit_menu->insertSeparator();
  pixmap = loader->loadIcon("indent", KIcon::Small);
  m_edit_menu->addCommand(ctEditCommands, cmIndent, pixmap, ID_EDIT_INDENT);
  pixmap = loader->loadIcon("unindent", KIcon::Small);
  m_edit_menu->addCommand(ctEditCommands, cmUnindent, pixmap, ID_EDIT_UNINDENT);


  m_edit_menu->insertSeparator();
  m_edit_menu->insertItem(i18n("&Insert File..."), this, SLOT(slotEditInsertFile()),0,ID_EDIT_INSERT_FILE);

  m_edit_menu->insertSeparator();
  pixmap = loader->loadIcon("search", KIcon::Small);
  m_edit_menu->addCommand(ctFindCommands, cmFind, pixmap, this, SLOT(slotEditSearch()), ID_EDIT_SEARCH);
  m_edit_menu->addCommand(ctFindCommands, cmFindAgain, this, SLOT(slotEditRepeatSearch()), ID_EDIT_REPEAT_SEARCH);

  
  m_edit_menu->addCommand(ctFindCommands, cmReplace, this, SLOT(slotEditReplace()), ID_EDIT_REPLACE);
  m_edit_menu->insertItem(BarIcon("grep"),i18n("&Search in Files..."), this, SLOT(slotEditSearchInFiles()),0,ID_EDIT_SEARCH_IN_FILES);
//  m_edit_menu->insertItem(i18n("Spell&check..."),this, SLOT(slotEditSpellcheck()),0,ID_EDIT_SPELLCHECK);

  m_edit_menu->insertSeparator();
  m_edit_menu->addCommand(ctEditCommands, cmSelectAll,pixmap, this,SLOT(slotEditSelectAll()),ID_EDIT_SELECT_ALL);
  m_edit_menu->addCommand(ctEditCommands, cmDeselectAll,pixmap,this,SLOT(slotEditDeselectAll()), ID_EDIT_DESELECT_ALL);
  m_edit_menu->addCommand(ctEditCommands, cmInvertSelection, ID_EDIT_INVERT_SELECTION);


  
  m_kdev_menubar->insertItem(i18n("&Edit"), m_edit_menu);

  ///////////////////////////////////////////////////////////////////
  // View-menu entries
  m_view_menu = new KGuiCmdPopup(m_kdev_dispatcher);//QPopupMenu;
  m_view_menu->addCommand(ctFindCommands, cmGotoLine, this, SLOT(slotViewGotoLine()), ID_VIEW_GOTO_LINE);
//  m_view_menu->insertItem(i18n("Goto &Line..."), this,
//			SLOT(slotViewGotoLine()),0,ID_VIEW_GOTO_LINE)
  m_view_menu->insertSeparator();
  m_view_menu->insertItem(i18n("&Next Error"),this,
			SLOT(slotViewNextError()),0,ID_VIEW_NEXT_ERROR);
  m_view_menu->insertItem(i18n("&Previous Error"),this,
			SLOT(slotViewPreviousError()),0,ID_VIEW_PREVIOUS_ERROR);
  m_view_menu->insertSeparator();
  
  m_view_menu->insertItem(i18n("&Tree-View"),this,
			SLOT(slotViewTTreeView()),0,ID_VIEW_TREEVIEW);
  m_view_menu->insertItem(i18n("&Output-View"),this,
			SLOT(slotViewTOutputView()),0,ID_VIEW_OUTPUTVIEW);
  m_view_menu->insertSeparator();
  m_view_menu->insertItem(i18n("&Toolbar"),this,
			   SLOT(slotViewTStdToolbar()),0,ID_VIEW_TOOLBAR);
  m_view_menu->insertItem(i18n("&Browser-Toolbar"),this,
			   SLOT(slotViewTBrowserToolbar()),0,ID_VIEW_BROWSER_TOOLBAR);
  m_view_menu->insertItem(i18n("Status&bar"),this,
			   SLOT(slotViewTStatusbar()),0,ID_VIEW_STATUSBAR);
  m_view_menu->insertItem(i18n("&MDI-View Taskbar"),this,
			   SLOT(slotViewTMDIViewTaskbar()),0,ID_VIEW_MDIVIEWTASKBAR);
  m_view_menu->insertSeparator();

  m_debugger_views_menu = new QPopupMenu();
  m_debugger_views_menu->setCheckable(true);
  connect(m_debugger_views_menu, SIGNAL(aboutToShow()), this, SLOT(slotUpdateDebuggerViewsMenu()) );
  m_debugger_views_menu->insertItem(BarIcon("debugger.xpm"), i18n("&Variables"), this,
         SLOT(slotViewDebuggerViewsVar()), 0, ID_VIEW_DEBUGGER_VIEWS_VAR);
  m_debugger_views_menu->insertItem(i18n("&Breakpoints"), this,
         SLOT(slotViewDebuggerViewsBreakpoints()), 0, ID_VIEW_DEBUGGER_VIEWS_BREAKPOINTS);
  m_debugger_views_menu->insertItem(i18n("&Frame Stack"), this,
         SLOT(slotViewDebuggerViewsFrameStack()), 0, ID_VIEW_DEBUGGER_VIEWS_FRAMESTACK);
  m_debugger_views_menu->insertItem(i18n("&Disassemble"), this,
         SLOT(slotViewDebuggerViewsDisassemble()), 0, ID_VIEW_DEBUGGER_VIEWS_DISASSEMBLE);
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
  m_debugger_views_menu->insertItem(i18n("Debu&gger"), this,
         SLOT(slotViewDebuggerViewsDebugger()), 0, ID_VIEW_DEBUGGER_VIEWS_DEBUGGER);
#endif
  m_view_menu->insertItem(i18n("&Debugger Views..."), m_debugger_views_menu, ID_VIEW_DEBUGGER_VIEWS );
  m_view_menu->insertSeparator();

  m_view_menu->insertItem(BarIcon("reload"),i18n("&Refresh"),this,
			   SLOT(slotViewRefresh()),0,ID_VIEW_REFRESH);

  m_view_menu->insertSeparator();
  m_view_menu->insertItem(i18n("&Preview dialog"),m_dlgedit,
			SLOT(slotViewPreview()),0,ID_VIEW_PREVIEW);
  m_view_menu->insertItem(i18n("&Grid..."),m_dlgedit,
			SLOT(slotViewGridDlg()),0,ID_VIEW_GRID_DLG);
  
  m_kdev_menubar->insertItem(i18n("&View"), m_view_menu);
  

  ///////////////////////////////////////////////////////////////////
  // Project-menu entries
  m_project_menu = new QPopupMenu;
  m_project_menu->insertItem(i18n("New..."), this, SLOT(slotProjectNewAppl()),0,ID_PROJECT_KAPPWIZARD);
  m_project_menu->insertItem(BarIcon("openprj"),i18n("&Open..."), this, SLOT(slotProjectOpen()),0,ID_PROJECT_OPEN);

  m_recent_projects_menu = new QPopupMenu();
  connect( m_recent_projects_menu, SIGNAL(activated(int)), SLOT(slotProjectOpenRecent(int)) );
  m_project_menu->insertItem(i18n("Open &recent project..."), m_recent_projects_menu, ID_PROJECT_OPEN_RECENT );
  m_project_menu->insertItem(i18n(":Import..."), this, SLOT(slotProjectImport()),0,ID_PROJECT_IMPORT );
  m_project_menu->insertItem(i18n("C&lose"),this, SLOT(slotProjectClose()),0,ID_PROJECT_CLOSE);

  m_project_menu->insertSeparator();
  m_project_menu->insertItem(i18n("&New Class..."), this,
			   SLOT(slotProjectNewClass()),0,ID_PROJECT_NEW_CLASS);
  m_project_menu->insertItem(i18n("&Add existing File(s)..."),this,SLOT(slotProjectAddExistingFiles()),0,ID_PROJECT_ADD_FILE_EXIST);
  
  m_project_menu->insertItem(loader->loadIcon("locale", KIcon::Small),i18n("Add new &Translation File..."), this,
			   SLOT(slotProjectAddNewTranslationFile()),0,ID_PROJECT_ADD_NEW_TRANSLATION_FILE);

  //  m_project_menu->insertItem(i18n("&Remove File from Project"), this,
  //			   SLOT(slotProjectRemoveFile()),0,ID_PROJECT_REMOVE_FILE);
 		
  m_project_menu->insertItem(BarIcon("file_properties"),i18n("&File Properties..."), this, SLOT(slotProjectFileProperties())
			   ,0,ID_PROJECT_FILE_PROPERTIES);
  m_project_menu->insertSeparator();
  
  m_project_menu->insertItem(i18n("Make &messages and merge"), this, SLOT(slotProjectMessages()),0, ID_PROJECT_MESSAGES);
  m_project_menu->insertItem(i18n("Make AP&I-Doc"), this,
			 SLOT(slotProjectAPI()),0,ID_PROJECT_MAKE_PROJECT_API);
  m_project_menu->insertItem(BarIcon("mini-book1"), i18n("Make &User-Manual..."), this,
			 SLOT(slotProjectManual()),0,ID_PROJECT_MAKE_USER_MANUAL);
  // submenu for making dists

  QPopupMenu*  p2 = new QPopupMenu;
  p2->insertItem(i18n("&Source-tgz"), this, SLOT(slotProjectMakeDistSourceTgz()),0,ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ);
  m_project_menu->insertItem(i18n("Make D&istribution"),p2,ID_PROJECT_MAKE_DISTRIBUTION);
  m_project_menu->insertSeparator();
  
  m_project_menu->insertItem(i18n("&Options..."), this, SLOT(slotProjectOptions()),0,ID_PROJECT_OPTIONS);


  m_kdev_menubar->insertItem(i18n("&Project"), m_project_menu);
  

  ///////////////////////////////////////////////////////////////////
  // Build-menu entries
  m_build_menu = new QPopupMenu;
  m_build_menu->insertItem(BarIcon("generate"),i18n("&Generate Dialog Sources..."),m_dlgedit,
			       SLOT(slotBuildGenerate()),0,ID_KDLG_BUILD_GENERATE);  
  m_build_menu->insertItem(BarIcon("compfile"),i18n("Compile &File"),
			 this,SLOT(slotBuildCompileFile()),0,ID_BUILD_COMPILE_FILE);
  m_build_menu->insertItem(BarIcon("make"),i18n("&Make"),this,
			 SLOT(slotBuildMake()),0,ID_BUILD_MAKE);

//   m_build_menu->insertItem(Icon("make.png"),i18n("Make &with"),this,
// 			 SLOT(slotBuildMakeWith()),0,ID_BUILD_MAKE_WITH);
//   m_accel->changeMenuAccel(m_build_menu,ID_BUILD_MAKE_WITH ,"MakeWith" );

  m_build_menu->insertItem(BarIcon("rebuild"),i18n("&Rebuild all"), this,
			 SLOT(slotBuildRebuildAll()),0,ID_BUILD_REBUILD_ALL);

  m_build_menu->insertItem(i18n("&Clean/Rebuild all"), this,
			 SLOT(slotBuildCleanRebuildAll()),0,ID_BUILD_CLEAN_REBUILD_ALL);
  m_build_menu->insertSeparator();
  m_build_menu->insertItem(BarIcon("stop_proc"),i18n("&Stop Build"), this, SLOT(slotBuildStop()),0,ID_BUILD_STOP);
  m_build_menu->insertSeparator();

  m_build_menu->insertItem(BarIcon("run"),i18n("&Execute"),this,SLOT(slotBuildRun()),0,ID_BUILD_RUN);
  m_build_menu->insertItem(BarIcon("run"),i18n("Execute &with Arguments..."),this,SLOT(slotBuildRunWithArgs()),0,ID_BUILD_RUN_WITH_ARGS);
  m_build_menu->insertItem(BarIcon("debugger"),i18n("&Debug..."),this,SLOT(slotBuildDebug()),0,ID_BUILD_DEBUG);
  m_build_menu->insertSeparator();
  m_build_menu->insertItem(i18n("DistC&lean"),this,SLOT(slotBuildDistClean()),0,ID_BUILD_DISTCLEAN);
  m_build_menu->insertItem(i18n("&Autoconf and automake"),this,SLOT(slotBuildAutoconf()),0,ID_BUILD_AUTOCONF);
  m_build_menu->insertItem(i18n("C&onfigure..."), this, SLOT(slotBuildConfigure()),0,ID_BUILD_CONFIGURE);

  m_kdev_menubar->insertItem(i18n("&Build"), m_build_menu);



  ///////////////////////////////////////////////////////////////////
  // Tools-menu entries
  m_tools_menu = new QPopupMenu;
  m_kdev_menubar->insertItem(i18n("&Tools"), m_tools_menu);

  ///////////////////////////////////////////////////////////////////
  // Options-menu entries
  // submenu for setting printprograms
  QPopupMenu* p3 = new QPopupMenu;
  p3->insertItem(i18n("&A2ps..."), this, SLOT(slotOptionsConfigureA2ps()),0,ID_OPTIONS_PRINT_A2PS);
  p3->insertItem(i18n("&Enscript..."), this,
		  SLOT(slotOptionsConfigureEnscript()),0,ID_OPTIONS_PRINT_ENSCRIPT);

  m_options_menu = new QPopupMenu;
  m_options_menu->insertItem(i18n("&Editor..."),this,
			   SLOT(slotOptionsEditor()),0,ID_OPTIONS_EDITOR);
  m_options_menu->insertItem(i18n("Editor &Colors..."),this,
			   SLOT(slotOptionsEditorColors()),0,ID_OPTIONS_EDITOR_COLORS);
  m_options_menu->insertItem(i18n("Editor &Defaults..."),this,
			   SLOT(slotOptionsSyntaxHighlightingDefaults())
			   ,0,ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS);
  m_options_menu->insertItem(i18n("&Syntax Highlighting..."),this,
			   SLOT(slotOptionsSyntaxHighlighting()),0,ID_OPTIONS_SYNTAX_HIGHLIGHTING);
  m_options_menu->insertSeparator();
  m_options_menu->insertItem(i18n("Documentation &Browser..."),this,
			   SLOT(slotOptionsDocBrowser()),0,ID_OPTIONS_DOCBROWSER);

  m_options_menu->insertItem(i18n("Configure &Printer..."),p3,ID_OPTIONS_PRINT);
	m_options_menu->insertItem(i18n("Tools..."),this,SLOT(slotOptionsToolsConfigDlg()),0,ID_OPTIONS_TOOLS_CONFIG_DLG);
//  m_options_menu->insertItem(i18n("&Spellchecker..."),this,SLOT(slotOptionsSpellchecker()),0,ID_OPTIONS_SPELLCHECKER);
  m_options_menu->insertSeparator();
  // Temporary hack
  m_options_menu->insertItem(i18n("Customize..."),this,
			   SLOT(slotOptionsCustomize()),0,ID_OPTIONS_CUSTOMIZE);
  m_options_menu->insertItem(i18n("&KDevelop Setup..."),this,
			   SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);

  m_kdev_menubar->insertItem(i18n("&Options"), m_options_menu);
  
  ///////////////////////////////////////////////////////////////////
  // Window-menu entries
  m_kdev_menubar->insertItem(i18n("&Window"), m_mdi_main_frame->windowMenu());
  m_kdev_menubar->insertSeparator();

  ///////////////////////////////////////////////////////////////////
  // PlugIns menu
  m_plugin_menu= new QPopupMenu;
  m_plugin_menu->insertItem(i18n("&Plugin Manager..."),this,SLOT(slotPluginPluginManager()),0,ID_PLUGIN_MANAGER);
  m_plugin_menu->insertSeparator();

  m_kdev_menubar->insertItem(i18n("&Plugins"),m_plugin_menu);
  
  ///////////////////////////////////////////////////////////////////
  // Bookmarks-menu entries
  m_bookmarks_menu=new QPopupMenu;
  m_bookmarks_menu->insertItem(i18n("&Set Bookmark..."),this,SLOT(slotBookmarksSet()),0,ID_BOOKMARKS_SET);
  m_bookmarks_menu->insertItem(i18n("&Add Bookmark..."),this,SLOT(slotBookmarksAdd()),0,ID_BOOKMARKS_ADD);
  m_bookmarks_menu->insertItem(i18n("&Clear Bookmarks"),this,SLOT(slotBookmarksClear()),0,ID_BOOKMARKS_CLEAR);
  m_bookmarks_menu->insertSeparator();

#warning FIXME Bookmarks need new implementation
  // KGuiCmdPopup* header_bookmarks = new KGuiCmdPopup(m_kdev_dispatcher);//new QPopupMenu();
//   header_widget->installBMPopup(header_bookmarks);
//   KGuiCmdPopup* cpp_bookmarks = new KGuiCmdPopup(m_kdev_dispatcher);//new QPopupMenu();
//   cpp_widget->installBMPopup(cpp_bookmarks);
	
  m_doc_bookmarks = new QPopupMenu();
  
 //  m_bookmarks_menu->insertItem(i18n("&Header Window"),header_bookmarks,31000);
//   m_bookmarks_menu->insertItem(i18n("C/C++ &Window"),cpp_bookmarks,31010);
//   m_bookmarks_menu->insertItem(i18n("&Browser Window"), m_doc_bookmarks,31020);
	
  m_kdev_menubar->insertItem(i18n("Book&marks"),m_bookmarks_menu);

  ///////////////////////////////////////////////////////////////////
  // Help-menu entries
  m_help_menu = new QPopupMenu();
  m_help_menu->insertItem(BarIcon("back"),i18n("&Back"),this, SLOT(slotHelpBack()),0,ID_HELP_BACK);
  m_help_menu->insertItem(BarIcon("forward"),i18n("&Forward"),this, SLOT(slotHelpForward()),0,ID_HELP_FORWARD);
  m_help_menu->insertSeparator();
  m_help_menu->insertItem(BarIcon("lookup"),i18n("&Search Marked Text"),this,
				 SLOT(slotHelpSearchText()),0,ID_HELP_SEARCH_TEXT);
  m_help_menu->insertItem(BarIcon("contents"),i18n("Search for Help on..."),this,SLOT(slotHelpSearch()),0,ID_HELP_SEARCH);
  
  m_help_menu->insertSeparator();
  m_help_menu->insertItem(BarIcon("mini-book1"),i18n("User Manual"),this,SLOT(slotHelpContents()),0 ,ID_HELP_CONTENTS);
  m_help_menu->insertItem(BarIcon("mini-book1"),i18n("Programming Handbook"),this,SLOT(slotHelpTutorial()),0 ,ID_HELP_TUTORIAL);
	m_help_menu->insertItem(BarIcon("idea"),i18n("Tip of the Day"), this, SLOT(slotHelpTipOfDay()), 0, ID_HELP_TIP_OF_DAY);
  m_help_menu->insertItem(i18n("KDevelop Homepage"),this, SLOT(slotHelpHomepage()),0,ID_HELP_HOMEPAGE);
  m_help_menu->insertItem(BarIcon("filemail"),i18n("Bug Report..."),this, SLOT(slotHelpBugReport()),0,ID_HELP_BUG_REPORT);
  m_help_menu->insertSeparator();
  m_help_menu->insertItem(i18n("C/C++-Reference"),this,SLOT(slotHelpReference()),0,ID_HELP_REFERENCE);
  m_help_menu->insertItem(BarIcon("mini-book1"),i18n("&Qt-Library"),this, SLOT(slotHelpQtLib()),0,ID_HELP_QT_LIBRARY);
  m_help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&Core-Library"),this,
                        SLOT(slotHelpKDECoreLib()),0,ID_HELP_KDE_CORE_LIBRARY);
  m_help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&GUI-Library"),this,
                        SLOT(slotHelpKDEGUILib()),0,ID_HELP_KDE_GUI_LIBRARY);
  m_help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&KFile-Library"),this,
                        SLOT(slotHelpKDEKFileLib()),0,ID_HELP_KDE_KFILE_LIBRARY);
  m_help_menu->insertItem(BarIcon("mini-book1"),i18n("KDE-&HTML-Library"),this,
                        SLOT(slotHelpKDEHTMLLib()),0,ID_HELP_KDE_HTML_LIBRARY);
  m_help_menu->insertSeparator();
  m_help_menu->insertItem(i18n("Project &API-Doc"),this,
                        SLOT(slotHelpAPI()),0,ID_HELP_PROJECT_API);

  m_help_menu->insertItem(i18n("Project &User-Manual"),this,
                        SLOT(slotHelpManual()),0,ID_HELP_USER_MANUAL);
  m_help_menu->insertSeparator();
  m_help_menu->insertItem(i18n("About KDevelop..."),this, SLOT(slotHelpAbout()),0,ID_HELP_ABOUT);
  m_kdev_menubar->insertItem(i18n("&Help"), m_help_menu);

  ////////////////////////////////////////////////
  // Popupmenu for the classbrowser wizard button
  ////////////////////////////////////////////////

  m_classbrowser_popup = new QPopupMenu();
  m_classbrowser_popup->insertItem( i18n("Goto declaration"), this,
                                  SLOT(slotClassbrowserViewDeclaration()),0, ID_CV_VIEW_DECLARATION );
  m_classbrowser_popup->insertItem( i18n("Goto definition"), this,
                                  SLOT(slotClassbrowserViewDefinition()), 0, ID_CV_VIEW_DEFINITION );
  m_classbrowser_popup->insertSeparator();
  m_classbrowser_popup->insertItem( i18n("Goto class declaration"), this,
                                  SLOT(slotClassbrowserViewClass()), 0, ID_CV_VIEW_CLASS_DECLARATION);
  m_classbrowser_popup->insertItem( i18n("New class..."), this,
                                  SLOT(slotProjectNewClass()), 0, ID_PROJECT_NEW_CLASS);
  m_classbrowser_popup->insertSeparator();
  m_classbrowser_popup->insertItem( i18n("Add member function..."), this,
                                  SLOT(slotClassbrowserNewMethod()), 0, ID_CV_METHOD_NEW);
  m_classbrowser_popup->insertItem( i18n("Add member variable..."), this,
                                  SLOT(slotClassbrowserNewAttribute()), 0, ID_CV_ATTRIBUTE_NEW );
  m_classbrowser_popup->insertItem( BarIcon("graphview"), i18n("Show graphical classview"), this,
  																SLOT(slotClassbrowserViewTree()),0, ID_CV_GRAPHICAL_VIEW);
  

  //  disableCommand(ID_FILE_NEW);
  disableCommand(ID_FILE_PRINT);
  
 
  // Why??  disableCommand(ID_VIEW_NEXT_ERROR);
  //  disableCommand(ID_VIEW_PREVIOUS_ERROR);

  disableCommand(ID_EDIT_UNDO);
  disableCommand(ID_EDIT_REDO);
  
  disableCommand(ID_VIEW_DEBUGGER_VIEWS_VAR);
  disableCommand(ID_VIEW_DEBUGGER_VIEWS_BREAKPOINTS);
  disableCommand(ID_VIEW_DEBUGGER_VIEWS_FRAMESTACK);
  disableCommand(ID_VIEW_DEBUGGER_VIEWS_DISASSEMBLE);
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
  disableCommand(ID_VIEW_DEBUGGER_VIEWS_DEBUGGER);
#endif

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
  connect(m_file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p3,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_view_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p2,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_tools_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_options_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_bookmarks_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_help_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(m_classbrowser_popup,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

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
   
//  toolBar(ID_MAIN_TOOLBAR)->insertButton(Icon("filenew.png"),ID_FILE_NEW, false,i18n("New"));

  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("openprj"),ID_PROJECT_OPEN, true,i18n("Open Project"));
  toolBar(ID_MAIN_TOOLBAR)->insertSeparator();
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("open"),ID_FILE_OPEN, true,i18n("Open File"));
  m_file_open_popup= new QPopupMenu();
  connect(m_file_open_popup, SIGNAL(activated(int)), SLOT(slotFileOpen(int)));
  toolBar(ID_MAIN_TOOLBAR)->setDelayedPopup(ID_FILE_OPEN, m_file_open_popup);

  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("save"),ID_FILE_SAVE,true,i18n("Save File"));
//  toolBar(ID_MAIN_TOOLBAR)->insertButton(Icon("save_all.png"),ID_FILE_SAVE_ALL,true,i18n("Save All"));

  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("print"),ID_FILE_PRINT,false,i18n("Print"));

  QFrame *separatorLine= new QFrame(toolBar(ID_MAIN_TOOLBAR));
  separatorLine->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_MAIN_TOOLBAR)->insertWidget(0,20,separatorLine);
	
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("undo"),ID_EDIT_UNDO,false,i18n("Undo"));
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("redo"),ID_EDIT_REDO,false,i18n("Redo"));
  toolBar(ID_MAIN_TOOLBAR)->insertSeparator();
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("cut"),ID_EDIT_CUT,true,i18n("Cut"));
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("copy"),ID_EDIT_COPY, true,i18n("Copy"));
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("paste"),ID_EDIT_PASTE, true,i18n("Paste"));
	
  QFrame *separatorLine1= new QFrame(toolBar(ID_MAIN_TOOLBAR));
  separatorLine1->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_MAIN_TOOLBAR)->insertWidget(0,20,separatorLine1);

  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("compfile"),ID_BUILD_COMPILE_FILE, false,i18n("Compile file"));
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("make"),ID_BUILD_MAKE, false,i18n("Make"));
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("rebuild"),ID_BUILD_REBUILD_ALL, false,i18n("Rebuild"));
  toolBar(ID_MAIN_TOOLBAR)->insertSeparator();
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("debugger"),ID_BUILD_DEBUG, false, i18n("Debug"));
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("run"),ID_BUILD_RUN, false,i18n("Run"));
  toolBar(ID_MAIN_TOOLBAR)->insertSeparator();
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("stop_proc"),ID_BUILD_STOP, false,i18n("Stop"));

  QFrame *separatorLine2= new QFrame(toolBar(ID_MAIN_TOOLBAR));
  separatorLine2->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_MAIN_TOOLBAR)->insertWidget(0,20,separatorLine2);
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("tree_win"),ID_VIEW_TREEVIEW, true,i18n("Tree-View"));
  toolBar(ID_MAIN_TOOLBAR)->insertButton(BarIcon("output_win"),ID_VIEW_OUTPUTVIEW, true,i18n("Output-View"));
  toolBar(ID_MAIN_TOOLBAR)->setToggle(ID_VIEW_TREEVIEW);
  toolBar(ID_MAIN_TOOLBAR)->setToggle(ID_VIEW_OUTPUTVIEW);

 QFrame *separatorLine3= new QFrame(toolBar(ID_MAIN_TOOLBAR));
 separatorLine3->setFrameStyle(QFrame::VLine|QFrame::Sunken);
 toolBar(ID_MAIN_TOOLBAR)->insertWidget(0,20,separatorLine3);


  QToolButton *btnwhat = QWhatsThis::whatsThisButton(toolBar(ID_MAIN_TOOLBAR));
  QToolTip::add(btnwhat, i18n("What's this...?"));
  toolBar(ID_MAIN_TOOLBAR)->insertWidget(ID_HELP_WHATS_THIS, btnwhat->sizeHint().width(), btnwhat);
  btnwhat->setFocusPolicy(QWidget::NoFocus);

  connect(toolBar(ID_MAIN_TOOLBAR), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  connect(toolBar(ID_MAIN_TOOLBAR), SIGNAL(pressed(int)), SLOT(statusCallback(int)));
	
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

  QComboBox* class_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  class_combo->setFocusPolicy(QWidget::NoFocus);

  // Method combo
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo(i18n("Methods"),
                                           ID_CV_TOOLBAR_METHOD_CHOICE,true,
                                           SIGNAL(activated(int))
                                           ,this,SLOT(slotMethodChoiceCombo(int)),
                                           true,i18n("Methods"),240 );

  QComboBox* choice_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  choice_combo->setFocusPolicy(QWidget::NoFocus);

  // Classbrowserwizard click button
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("classwiz"),
                                            ID_CV_WIZARD, true,
                                            i18n("Declaration/Definition"));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_CV_WIZARD,
                                               m_classbrowser_popup);
  
  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("back"),ID_HELP_BACK, false,i18n("Back"));
  m_history_prev = new QPopupMenu();
  connect(m_history_prev, SIGNAL(activated(int)), SLOT(slotHelpHistoryBack(int)));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_HELP_BACK, m_history_prev);
	
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(BarIcon("forward"),ID_HELP_FORWARD, false,i18n("Forward"));
  m_history_next = new QPopupMenu();
  connect(m_history_next, SIGNAL(activated(int)), SLOT(slotHelpHistoryForward(int)));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_HELP_FORWARD, m_history_next);
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
void CKDevelop::initStatusBar()
{
  m_kdev_statusbar= new KStatusBar(this,"KDevelop_statusbar");
  m_statProg = new QProgressBar(m_kdev_statusbar,"Progressbar");
//  m_statProg->setFixedWidth(200);

  m_kdev_statusbar->insertFixedItem(" x ", ID_STATUS_DBG);
  m_kdev_statusbar->insertItem("", ID_STATUS_MSG, 1);       // stretch to fit
//  m_kdev_statusbar->addWidget(m_statProg, 0, true);
  m_kdev_statusbar->insertFixedItem(i18n(" INS "), ID_STATUS_INS_OVR, true);
  m_kdev_statusbar->insertFixedItem(i18n("Line: 00000 Col: 000"), ID_STATUS_LN_CLM, true);

  m_kdev_statusbar->changeItem("", ID_STATUS_LN_CLM);
  m_kdev_statusbar->changeItem("", ID_STATUS_DBG);

  m_kdev_statusbar->setItemAlignment (ID_STATUS_MSG, AlignLeft);

  setStatusBar(m_kdev_statusbar);
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

//REMOVE  connect(t_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotTabSelected(int)));

  connect(m_class_tree,SIGNAL(setStatusbarProgressSteps(int)),m_statProg,SLOT(setTotalSteps(int)));
  connect(m_class_tree,SIGNAL(setStatusbarProgress(int)),m_statProg,SLOT(setProgress(int)));
  connect(m_class_tree,SIGNAL(resetStatusbarProgress()),m_statProg,SLOT(reset()));
  connect(m_class_tree, SIGNAL(selectedFileNew()), SLOT(slotProjectAddNewFile()));
  connect(m_class_tree, SIGNAL(selectedClassNew()), SLOT(slotProjectNewClass()));
  connect(m_class_tree, SIGNAL(selectedProjectOptions()), SLOT(slotProjectOptions()));
  connect(m_class_tree, SIGNAL(selectedViewDeclaration(const char *, const char *,THType)), SLOT(slotCVViewDeclaration(const char *, const char *,THType)));
  connect(m_class_tree, SIGNAL(selectedViewDefinition(const char *, const char *,THType)), SLOT(slotCVViewDefinition(const char *, const char *,THType)));
  connect(m_class_tree, SIGNAL(signalAddMethod(const char *)), SLOT(slotCVAddMethod(const char * )));
  connect(m_class_tree, SIGNAL(signalAddAttribute(const char *)), SLOT(slotCVAddAttribute(const char * )));
  connect(m_class_tree, SIGNAL(signalMethodDelete(const char *,const char *)), SLOT(slotCVDeleteMethod(const char *,const char *)));
  connect(m_class_tree, SIGNAL(popupHighlighted(int)), SLOT(statusCallback(int)));
  
  connect(m_log_file_tree, SIGNAL(logFileTreeSelected(QString)), SLOT(slotLogFileTreeSelected(QString)));
  connect(m_log_file_tree, SIGNAL(selectedNewClass()), SLOT(slotProjectNewClass()));
  connect(m_log_file_tree, SIGNAL(selectedNewFile()), SLOT(slotProjectAddNewFile()));
  connect(m_log_file_tree, SIGNAL(selectedFileRemove(QString)), SLOT(delFileFromProject(QString)));
  connect(m_log_file_tree, SIGNAL(showFileProperties(QString)),SLOT(slotShowFileProperties(QString)));
  connect(m_log_file_tree, SIGNAL(menuItemHighlighted(int)), SLOT(statusCallback(int)));
  connect(m_log_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(m_log_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString)));

  connect(m_real_file_tree, SIGNAL(fileSelected(QString)), SLOT(slotRealFileTreeSelected(QString)));
  connect(m_real_file_tree, SIGNAL(showFileProperties(QString)),SLOT(slotShowFileProperties(QString)));
  connect(m_real_file_tree, SIGNAL(addFileToProject(QString)),SLOT(slotAddFileToProject(QString)));
  connect(m_real_file_tree, SIGNAL(removeFileFromProject(QString)),SLOT(delFileFromProject(QString)));
  connect(m_real_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(m_real_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString)));
  connect(m_real_file_tree, SIGNAL(commitDirToVCS(QString)), SLOT(slotCommitDirToVCS(QString)));
  connect(m_real_file_tree, SIGNAL(updateDirFromVCS(QString)), SLOT(slotUpdateDirFromVCS(QString)));


  connect(m_doc_tree, SIGNAL(fileSelected(const QString&)), SLOT(slotDocTreeSelected(const QString&)));
  connect(m_doc_tree, SIGNAL(projectAPISelected()), SLOT(slotHelpAPI()));
  connect(m_doc_tree, SIGNAL(projectManualSelected()), SLOT(slotHelpManual()));


  // connect Docbrowser rb menu

  connect(m_browser_widget, SIGNAL(urlClicked(const QString&,const QString&,int)),
          this, SLOT(slotURLSelected(const QString&,const QString&,int)));
  connect(m_browser_widget, SIGNAL(onURL(const QString&)),
          this, SLOT(slotURLonURL(const QString&)));
  connect(m_browser_widget, SIGNAL(textSelected(bool)),
          this, SLOT(slotBROWSERMarkStatus(bool)));
  connect(m_browser_widget, SIGNAL(completed()),
          this, SLOT(slotDocumentDone()));
  connect(m_browser_widget, SIGNAL(signalURLBack()),this,SLOT(slotHelpBack()));
  connect(m_browser_widget, SIGNAL(signalURLForward()),this,SLOT(slotHelpForward()));
  connect(m_browser_widget, SIGNAL(signalBookmarkAdd()),this,SLOT(slotBookmarksAdd()));
  connect(m_browser_widget, SIGNAL(signalSearchText()),this,SLOT(slotHelpSearchText()));
  connect(m_browser_widget, SIGNAL(enableStop(int)), this, SLOT(enableCommand(int)));	
  connect(m_browser_widget->popup(), SIGNAL(highlighted(int)), this, SLOT(statusCallback(int)));
  connect(m_browser_widget, SIGNAL(signalGrepText(QString)), this, SLOT(slotEditSearchInFiles(QString)));
  
  connect(m_browser_view,SIGNAL(focusInEventOccurs(QextMdiChildView*)),this,SLOT(slotMDIGetFocus(QextMdiChildView*)));

  // connect the windowsmenu with a method
  connect(m_doc_bookmarks, SIGNAL(activated(int)), this, SLOT(slotBoomarksBrowserSelected(int)));

  connect(m_messages_widget, SIGNAL(processExited(KProcess*)),
          this, SLOT(slotProcessExited(KProcess*)));
  connect(m_messages_widget, SIGNAL(itemSelected(const QString&,int)),
          SLOT(slotSwitchFileRequest(const QString&,int)));
  connect(m_grepview, SIGNAL(itemSelected(const QString&,int)),
          SLOT(slotSwitchFileRequest(const QString&,int)));

  // connections for the proc -processes
  connect(&m_search_process, SIGNAL(receivedStdout(KProcess*,char*,int)),
          this, SLOT(slotSearchReceivedStdout(KProcess*,char*,int)) );

#if 0
  // let's see
  connect(&m_search_process, SIGNAL(receivedStderr(KProcess*,char*,int)),
          this, SLOT(slotReceivedStderr(KProcess*,char*,int)) );
#endif

  connect(&m_search_process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotSearchProcessExited(KProcess*) )) ;

  connect(m_mdi_main_frame,SIGNAL(letKDevelopDoTheSwitchToChildframeMode()),
          this,SLOT(slotSwitchToChildframeMode()) );
  connect(m_mdi_main_frame,SIGNAL(letKDevelopDoTheSwitchToToplevelMode()),
          this,SLOT(slotSwitchToToplevelMode()) );
}
void CKDevelop::initDlgEditor(){
    // the dialog editor manager
    m_dlgedit = new DlgEdit(m_widprop_split_view->getWidgetsView(),m_widprop_split_view->getPropertyView());
    ComponentManager::self()->registerComponent(m_dlgedit);
}


void CKDevelop::initProject()
{
    QString filename;
  m_config->setGroup("General Options");
  bool bLastProject =true;
  if(!kapp->argc() > 1)
    bLastProject= m_config->readBoolEntry("LastProject",true);
  else
    bLastProject=false;

  if(bLastProject){
    m_config->setGroup("Files");
    filename = m_config->readEntry("project_file");
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
      connect(m_class_tree,SIGNAL(setStatusbarProgressSteps(int)),progressDlg,SLOT(setTotalSteps(int)));
      connect(m_class_tree,SIGNAL(setStatusbarProgress(int)),progressDlg,SLOT(setProgress(int)));
      progressDlg->setCaption(i18n("Starting..."));
      progressDlg->setLabelText( i18n("Initializing last project...\nPlease wait...\n") );
      progressDlg->setCancelButtonText(i18n("Cancel"));
      progressDlg->setProgress(0);
      progressDlg->show();
      setCursor(KCursor::arrowCursor());
      refreshTrees();
      delete progressDlg;
      
    }
    m_config->setGroup("Files");
    filename = m_config->readEntry("header_file",i18n("Untitled.h"));
    QFile _file(filename);
    
    if (QFile::exists(filename)){
      switchToFile(filename);
      
    }

    filename = m_config->readEntry("cpp_file", i18n("Untitled.cpp"));
    if (QFile::exists(filename)){
      switchToFile(filename);
    }
  }
  else{
    refreshTrees(); // this refresh only the documentation tab,tree
  }
  
}


void CKDevelop::setKeyAccel(){    
    
    m_accel->connectItem( KStdAccel::Open , this, SLOT(slotFileOpen()), true, ID_FILE_OPEN);
    m_accel->connectItem( KStdAccel::Close , this, SLOT(slotFileClose()), true, ID_FILE_CLOSE);
    m_accel->connectItem( KStdAccel::Save , this, SLOT(slotFileSave()), true, ID_FILE_SAVE);
    
    
    m_accel->changeMenuAccel(m_file_menu, ID_FILE_NEW, KStdAccel::New );
    m_accel->changeMenuAccel(m_file_menu, ID_FILE_OPEN, KStdAccel::Open );
    m_accel->changeMenuAccel(m_file_menu, ID_FILE_CLOSE, KStdAccel::Close );
    m_accel->changeMenuAccel(m_file_menu, ID_FILE_SAVE, KStdAccel::Save );
    m_accel->changeMenuAccel(m_file_menu, ID_FILE_PRINT, KStdAccel::Print );
    m_accel->changeMenuAccel(m_file_menu, ID_FILE_QUIT, KStdAccel::Quit );

    m_accel->changeMenuAccel(m_edit_menu, ID_EDIT_SEARCH_IN_FILES,"Grep" );


    m_accel->changeMenuAccel(m_view_menu,ID_VIEW_NEXT_ERROR ,"NextError" );
    m_accel->changeMenuAccel(m_view_menu,ID_VIEW_PREVIOUS_ERROR ,"PreviousError" );
    m_accel->changeMenuAccel(m_view_menu,ID_VIEW_TREEVIEW ,"Tree-View" );
    m_accel->changeMenuAccel(m_view_menu,ID_VIEW_OUTPUTVIEW,"Output-View" );

    m_accel->changeMenuAccel(m_project_menu,ID_PROJECT_OPTIONS ,"ProjectOptions" );
    m_accel->changeMenuAccel(m_project_menu,ID_PROJECT_FILE_PROPERTIES ,"FileProperties" );

    m_accel->changeMenuAccel(m_build_menu,ID_BUILD_COMPILE_FILE ,"CompileFile" );
    m_accel->changeMenuAccel(m_build_menu,ID_BUILD_MAKE ,"Make" );
    m_accel->changeMenuAccel(m_build_menu,ID_BUILD_RUN ,"Run" );
    m_accel->changeMenuAccel(m_build_menu,ID_BUILD_RUN_WITH_ARGS,"Run_with_args");
    m_accel->changeMenuAccel(m_build_menu,ID_BUILD_STOP,"Stop_proc");
    
    m_accel->changeMenuAccel(m_bookmarks_menu,ID_BOOKMARKS_ADD ,"Add_Bookmarks" );
    m_accel->changeMenuAccel(m_bookmarks_menu,ID_BOOKMARKS_CLEAR ,"Clear_Bookmarks" );
    
    m_accel->changeMenuAccel(m_help_menu,ID_HELP_SEARCH_TEXT,"SearchMarkedText" );
    m_accel->changeMenuAccel(m_help_menu, ID_HELP_CONTENTS, KStdAccel::Help );
    
}

void CKDevelop::setToolmenuEntries(){
  m_config = kapp->config();
  m_config->setGroup("ToolsMenuEntries");
	m_config->readListEntry("Tools_exe",m_tools_exe);
	m_config->readListEntry("Tools_entry",m_tools_entry);
	m_config->readListEntry("Tools_argument",m_tools_argument);
	
	uint items;
	for(items=0;items<m_tools_entry.count();items++){
		m_tools_menu->insertItem(m_tools_entry.at(items));
	}
  
  
	
	connect(m_tools_menu,SIGNAL(activated(int)),SLOT(slotToolsTool(int)));
       

}

