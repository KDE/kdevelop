/***************************************************************************
      init.cpp - the init specific part of CKDevelop (construtor ...)
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
#include "ckdevelop.h"
#include <iostream.h>
#include <kmsgbox.h>
#include "./kwrite/kwdoc.h"
#include "ctoolclass.h"
#include <kaccel.h>
#include <qtoolbutton.h>
			
CKDevelop::CKDevelop(){
  QString filename;
  version = VERSION;
  project=false;// no project
  

  init();
  initConnections();
  initProject(); 
  
  config->setGroup("Files");
  filename = config->readEntry("browser_file");
  if(!filename.isEmpty()){
    slotURLSelected(browser_widget,filename,1,"test");
  }
  else{
    slotURLSelected(browser_widget,"file:" + KApplication::kde_htmldir() +
		    "/en/kdevelop/index.html",1,"test");
    
  }
  // if first start, the onlinehelp is shown, so outputview is false
  // else show output view according to value in config file  -Ralf

  config->setGroup("General Options");
  bool showOutput=config->readBoolEntry("show_output_view",false);
  if(showOutput)
      { slotViewTOutputView();}

 
  
  
}


void CKDevelop::init(){
  
  act_outbuffer_len=0;
  prj = 0;

  QFont font("Fixed",10);
  KApplication *app=KApplication::getKApplication();
  config = app->getConfig();
  config->setGroup("General Options");
  int w = config->readNumEntry("width", 800);
  int h = config->readNumEntry("height", 500);
  resize(w,h);

  // call bar functions to create bars

  // create the main view

  view = new KNewPanner(this,"view",KNewPanner::Horizontal,KNewPanner::Percent,
  			config->readNumEntry("view_panner_pos",80));


  o_tab_view = new CTabCtl(view,"output_tabview","output_widget");
  
  messages_widget = new COutputWidget(kapp,o_tab_view);
//  messages_widget->setFocusPolicy(QWidget::ClickFocus);
  messages_widget->setFocusPolicy(QWidget::NoFocus);

//  output_widget->setFillColumnMode(80,true);
//  output_widget->setWordWrap(true);
  messages_widget->setReadOnly(TRUE);
//  output_widget->setAutoIndentMode(true);
//  output_widget->setFocusPolicy(QWidget::ClickFocus);
  connect(messages_widget,SIGNAL(clicked()),this,SLOT(slotClickedOnMessagesWidget()));

  stdin_stdout_widget = new COutputWidget(kapp,o_tab_view);
//  stdin_stdout_widget->setFocusPolicy(QWidget::StrongFocus);
  stdin_stdout_widget->setFocusPolicy(QWidget::ClickFocus);

  connect(stdin_stdout_widget,SIGNAL(keyPressed(int)),this,SLOT(slotKeyPressedOnStdinStdoutWidget(int)));
  stderr_widget = new COutputWidget(kapp,o_tab_view);
  stderr_widget->setReadOnly(TRUE);
//  stderr_widget->setFocusPolicy(QWidget::ClickFocus);
  stderr_widget->setFocusPolicy(QWidget::NoFocus);

  o_tab_view->addTab(messages_widget,"messages");
  o_tab_view->addTab(stdin_stdout_widget,"stdin/stdout");
  o_tab_view->addTab(stderr_widget,"stderr");
  
  
  //  s_tab_current = 0;

  top_panner = new KNewPanner(view,"top_panner",KNewPanner::Vertical,KNewPanner::Absolute,
  			      config->readNumEntry("top_panner_pos", 213));

  t_tab_view = new CTabCtl(top_panner);
  t_tab_view->setFocusPolicy(QWidget::ClickFocus);
  connect(t_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotTTabSelected(int)));

  log_file_tree = new CLogFileView(t_tab_view,"lfv");
  log_file_tree->setIndentSpacing(15);
  log_file_tree->setFocusPolicy(QWidget::NoFocus);
  
  class_tree = new CClassView(t_tab_view,"cv");
  class_tree->setIndentSpacing(15);
  class_tree->setFocusPolicy(QWidget::NoFocus);

  real_file_tree = new CRealFileView(t_tab_view,"RFV");
  real_file_tree->setFocusPolicy(QWidget::NoFocus);
  real_file_tree->setIndentSpacing(15);

  doc_tree = new CDocTree(t_tab_view,"DOC",config);
  doc_tree->setFocusPolicy(QWidget::NoFocus);
  doc_tree->setIndentSpacing(15);

  t_tab_view->addTab(class_tree,"CV");
  t_tab_view->addTab(log_file_tree,"LFV");
  t_tab_view->addTab(real_file_tree,"RFV");
  t_tab_view->addTab(doc_tree,"DOC");

  connect(class_tree, SIGNAL(singleSelected(int)), SLOT(slotClassTreeSelected(int)));
  connect(class_tree, SIGNAL(selectedFileNew()), SLOT(slotProjectAddNewFile()));
  connect(class_tree, SIGNAL(selectedClassNew()), SLOT(slotProjectNewClass()));
  connect(class_tree, SIGNAL(selectedProjectOptions()), SLOT(slotProjectOptions()));
  connect(class_tree, SIGNAL(selectedViewDeclaration(int)), SLOT(slotCVViewDeclaration(int)));
  connect(class_tree, SIGNAL(selectedViewDefinition(int)), SLOT(slotCVViewDefinition(int)));

  connect(log_file_tree, SIGNAL(singleSelected(int)), SLOT(slotLogFileTreeSelected(int)));
  connect(log_file_tree, SIGNAL(selectedNewClass()), SLOT(slotProjectNewClass()));
  connect(log_file_tree, SIGNAL(selectedNewFile()), SLOT(slotProjectAddNewFile()));
  connect(log_file_tree, SIGNAL(selectedFileRemove()), SLOT(slotProjectRemoveFile()));
  connect(log_file_tree, SIGNAL(selectedFileProp()),SLOT(slotProjectFileProperties()));

  connect(real_file_tree, SIGNAL(singleSelected(int)), SLOT(slotRealFileTreeSelected(int)));

  connect(doc_tree, SIGNAL(singleSelected(int)), SLOT(slotDocTreeSelected(int)));


  // the tabbar + tabwidgets for edit and browser
  s_tab_view = new CTabCtl(top_panner);
  s_tab_view->setFocusPolicy(QWidget::ClickFocus);

  connect(s_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotSTabSelected(int)));



  header_widget = new CEditWidget(kapp,s_tab_view,"header");
  header_widget->setFocusPolicy(QWidget::StrongFocus);

  header_widget->setFont(font);
  header_widget->setName("Untitled.h");
  config->setGroup("KWrite Options");
  header_widget->readConfig(config);
  header_widget->doc()->readConfig(config);

  connect(header_widget, SIGNAL(newCurPos()), this, SLOT(slotNewLineColumn()));
  connect(header_widget, SIGNAL(newStatus()),this, SLOT(slotNewStatus()));
  connect(header_widget, SIGNAL(newUndo()),this, SLOT(slotNewUndo()));


  edit_widget=header_widget;
  cpp_widget = new CEditWidget(kapp,s_tab_view,"cpp");
  cpp_widget->setFocusPolicy(QWidget::StrongFocus);
  cpp_widget->setFont(font);
  cpp_widget->setName("Untitled.cpp");
  config->setGroup("KWrite Options");
  cpp_widget->readConfig(config);
  cpp_widget->doc()->readConfig(config);

//  edit_widget->setFocusPolicy(QWidget::StrongFocus);

  connect(cpp_widget, SIGNAL(newCurPos()), this, SLOT(slotNewLineColumn()));
  connect(cpp_widget, SIGNAL(newStatus()),this, SLOT(slotNewStatus()));
  connect(cpp_widget, SIGNAL(newUndo()),this, SLOT(slotNewUndo()));

  // init the 2 first kedits
  TEditInfo* edit1 = new TEditInfo;
  TEditInfo* edit2 = new TEditInfo;
  edit1->filename = header_widget->getName();
  edit2->filename = cpp_widget->getName();

  browser_widget = new CDocBrowser(s_tab_view,"browser");
  browser_widget->setFocusPolicy(QWidget::StrongFocus);

  prev_was_search_result= false;
  //init
  browser_widget->setDocBrowserOptions();

  connect(browser_widget,SIGNAL(URLSelected(KHTMLView*,const char*,int,const char*)),
	  this,SLOT(slotURLSelected(KHTMLView*,const char*,int,const char*)));
  connect(browser_widget,SIGNAL(documentDone(KHTMLView*)),
	  this,SLOT(slotDocumentDone(KHTMLView*)));
  swallow_widget = new KSwallowWidget(s_tab_view);
  swallow_widget->setFocusPolicy(QWidget::StrongFocus);
//  swallow_widget->setFocusPolicy(QWidget::NoFocus);

  

  s_tab_view->addTab(header_widget,i18n("Header/Resource Files"));
  s_tab_view->addTab(cpp_widget,i18n("C/C++ Files"));
  s_tab_view->addTab(browser_widget,i18n("Documentation-Browser"));
  s_tab_view->addTab(swallow_widget,i18n("Tools"));

  top_panner->activate(t_tab_view,s_tab_view);// activate the top_panner
  view->activate(top_panner,o_tab_view); 
  
  output_view_pos=view->separatorPos();
  tree_view_pos=top_panner->separatorPos();

  // set the mainwidget
  setView(view);
  initKeyAccel();
  initMenu();
  initToolbar();
  initStatusBar();
	initWhatsThis();

  // initialize output_view_pos
  if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW)){
    output_view_pos=view->separatorPos();
  }
  else{
    config->setGroup("General Options");
    output_view_pos=config->readNumEntry("output_view_pos", 80);
  }
  
  // initialize tree_view_pos
  
  if(view_menu->isItemChecked(ID_VIEW_TREEVIEW)){
    tree_view_pos=top_panner->separatorPos();
  }
  else{
    config->setGroup("General Options");
    tree_view_pos=config->readNumEntry("tree_view_pos", 213);
  }

   // the rest of the init for the kedits
 	edit1->id = menu_buffers->insertItem(edit1->filename,-2,0);
  edit1->modified=false;
  edit2->id = menu_buffers->insertItem(edit2->filename,-2,0);
  edit2->modified=false;
  edit_infos.append(edit1);
  edit_infos.append(edit2);
}
void CKDevelop::initKeyAccel(){
  accel = new KAccel( this );
  
  //edit menu
  accel->insertItem( i18n("Repeat Search"), "RepeatSearch",IDK_EDIT_REPEAT_SEARCH );
  accel->connectItem( "RepeatSearch", this, SLOT(slotEditRepeatSearch() ) );
  
  //view menu
  accel->insertItem( i18n("Goto Line"), "GotoLine",IDK_VIEW_GOTO_LINE);
  accel->connectItem( "GotoLine", this, SLOT( slotViewGotoLine()) );

  accel->insertItem( i18n("Toogle Tree-View"), "Tree-View",IDK_VIEW_TREEVIEW);
  accel->connectItem( "Tree-View", this, SLOT(slotViewTTreeView()) );
  
  accel->insertItem( i18n("Toogle Output-View"), "Output-View",IDK_VIEW_OUTPUTVIEW);
  accel->connectItem( "Output-View", this, SLOT(slotViewTOutputView()) );
  
  accel->insertItem( i18n("Toogle Statusbar"), "Statusbar",IDK_VIEW_STATUSBAR);
  accel->connectItem( "Statusbar", this, SLOT(slotViewTStatusbar()) );
   
  //build menu 
  accel->insertItem( i18n("Compile File"), "CompileFile", IDK_BUILD_COMPILE_FILE );
  accel->connectItem( "CompileFile", this, SLOT( slotBuildCompileFile()) );

  accel->insertItem( i18n("Make"), "Make", IDK_BUILD_MAKE );
  accel->connectItem( "Make", this, SLOT(slotBuildMake() ) );

  accel->insertItem( i18n("Run"), "Run", IDK_BUILD_RUN);
  accel->connectItem( "Run", this, SLOT(slotBuildRun() ) );

  //doc menu
  accel->insertItem( i18n("Search Marked Text"), "SearchMarkedText",IDK_DOC_SEARCH_TEXT);
  accel->connectItem( "SearchMarkedText", this, SLOT(slotDocSText() ) );
  
  accel->readSettings();
}
void CKDevelop::initMenu(){

///////////////////////////////////////////////////////////////////
// File-menu entries

  QPixmap pix;
  file_menu = new QPopupMenu;
 
  file_menu->insertItem(Icon("filenew.xpm"),i18n("&New"),this,SLOT(slotFileNew()),IDK_FILE_NEW,ID_FILE_NEW);

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  file_menu->insertItem(pix,i18n("&Open..."), this, SLOT(slotFileOpenFile()),
			IDK_FILE_OPEN,ID_FILE_OPEN);

  file_menu->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save.xpm");
  file_menu->insertItem(pix,i18n("&Save"), this, SLOT(slotFileSave()), IDK_FILE_SAVE,ID_FILE_SAVE);
  file_menu->insertItem(i18n("Save &As..."), this, SLOT(slotFileSaveAs()),0,ID_FILE_SAVE_AS); 
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/saveall.xpm");
  file_menu->insertItem(pix,i18n("Save All"), this, SLOT(slotFileSaveAll()),0,ID_FILE_SAVE_ALL);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("&Close"), this, SLOT(slotFileClose()), IDK_FILE_CLOSE,ID_FILE_CLOSE);
  file_menu->insertSeparator();
  file_menu->insertItem(Icon("fileprint.xpm"),i18n("&Print..."), this, SLOT(slotFilePrint()),IDK_FILE_PRINT,ID_FILE_PRINT);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("&Quit"),this, SLOT(slotFileQuit()), IDK_FILE_QUIT,ID_FILE_QUIT);

  menuBar()->insertItem(i18n("&File"), file_menu);
  disableCommand(ID_FILE_NEW);


///////////////////////////////////////////////////////////////////
// Edit-menu entries
  
  edit_menu = new QPopupMenu;
  edit_menu->insertItem(i18n("U&ndo"), this, SLOT(slotEditUndo()), IDK_EDIT_UNDO,ID_EDIT_UNDO);
  edit_menu->insertItem(i18n("R&edo"), this, SLOT(slotEditRedo()), IDK_EDIT_REDO,ID_EDIT_REDO);  
  edit_menu->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/cut.xpm");
  edit_menu->insertItem(pix,i18n("C&ut"), this, SLOT(slotEditCut()), IDK_EDIT_CUT,ID_EDIT_CUT);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/copy.xpm");
  edit_menu->insertItem(pix,i18n("&Copy"), this, SLOT(slotEditCopy()), IDK_EDIT_COPY,ID_EDIT_COPY);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/paste.xpm");
  edit_menu->insertItem(pix,i18n("&Paste"), this, SLOT(slotEditPaste()), IDK_EDIT_PASTE,ID_EDIT_PASTE);
  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("&Insert File..."),this, SLOT(slotEditInsertFile()),0,ID_EDIT_INSERT_FILE);
  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("&Search..."), this, SLOT(slotEditSearch()),IDK_EDIT_SEARCH,ID_EDIT_SEARCH);
  edit_menu->insertItem(i18n("&Repeat Search..."), this, 
			SLOT(slotEditRepeatSearch()),0,ID_EDIT_REPEAT_SEARCH);
  accel->changeMenuAccel(edit_menu, ID_EDIT_REPEAT_SEARCH,"RepeatSearch" );
  
  
  edit_menu->insertItem(i18n("&Replace..."), this, SLOT(slotEditReplace()),IDK_EDIT_REPLACE,ID_EDIT_REPLACE);
  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("Select &All"), this, SLOT(slotEditSelectAll()),0,ID_EDIT_SELECT_ALL);
  edit_menu->insertItem(i18n("Deselect All"), this, SLOT(slotEditDeselectAll()),0,ID_EDIT_DESELECT_ALL);
  edit_menu->insertItem(i18n("Invert Selection"), this, SLOT(slotEditInvertSelection()),0,ID_EDIT_INVERT_SELECTION);
  
  menuBar()->insertItem(i18n("&Edit"), edit_menu);

  disableCommand(ID_EDIT_UNDO);
  disableCommand(ID_EDIT_REDO);


///////////////////////////////////////////////////////////////////
// View-menu entries

  config->setGroup("General Options");
  bViewStatusbar = config->readBoolEntry("show_statusbar",true);

  view_menu = new QPopupMenu;
  view_menu->insertItem(i18n("&Line..."), this,
			SLOT(slotViewGotoLine()),0,ID_VIEW_GOTO_LINE);
  accel->changeMenuAccel(view_menu,ID_VIEW_GOTO_LINE ,"GotoLine" );


  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Tree-View"),this,
			SLOT(slotViewTTreeView()),0,ID_VIEW_TREEVIEW);
  accel->changeMenuAccel(view_menu,ID_VIEW_TREEVIEW ,"Tree-View" );
  view_menu->setItemChecked(ID_VIEW_TREEVIEW,config->readBoolEntry("show_tree_view",true));

  view_menu->insertItem(i18n("&Output-View"),this,
			SLOT(slotViewTOutputView()),0,ID_VIEW_OUTPUTVIEW);
  accel->changeMenuAccel(view_menu,ID_VIEW_OUTPUTVIEW,"Output-View" );
  view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,config->readBoolEntry("show_output_view",true));

  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Toolbar"),this,
			   SLOT(slotViewTStdToolbar()),0,ID_VIEW_TOOLBAR);
  view_menu->setItemChecked(ID_VIEW_TOOLBAR,config->readBoolEntry("show_std_toolbar", true));

  view_menu->insertItem(i18n("&Browser-Toolbar"),this,
			   SLOT(slotViewTBrowserToolbar()),0,ID_VIEW_BROWSER_TOOLBAR);
  view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR,config->readBoolEntry("show_browser_toolbar",true));

  view_menu->insertItem(i18n("Status&bar"),this,
			   SLOT(slotViewTStatusbar()),0,ID_VIEW_STATUSBAR);
  accel->changeMenuAccel(view_menu,ID_VIEW_STATUSBAR,"Statusbar");

  view_menu->setItemChecked(ID_VIEW_STATUSBAR,bViewStatusbar);

  view_menu->insertSeparator();
  view_menu->insertItem(Icon("reload.xpm"),i18n("&Refresh"),this,
			   SLOT(slotViewRefresh()),0,ID_VIEW_REFRESH);

  menuBar()->insertItem(i18n("&View"), view_menu);
  

  // the bookmarks menu
  // p = new QPopupMenu;
//   p->insertItem(i18n("&Add Bookmark"), this, SLOT(slotBookmarksAdd()));
//   p->insertItem(i18n("&Edit Bookmarks..."), this, SLOT(slotBookmarksEdit()));
//   p->insertSeparator();
//   menuBar()->insertItem(i18n("&Bookmarks"),p);


///////////////////////////////////////////////////////////////////
// Project-menu entries

  // submenu for adding projectfiles
  QPopupMenu* p2 = new QPopupMenu;
  p2->insertItem(i18n("&New File..."), this, SLOT(slotProjectAddNewFile()),0,ID_PROJECT_ADD_FILE_NEW);
  p2->insertItem(i18n("&Existing File(s)..."), this,
		 SLOT(slotProjectAddExistingFiles()),0,ID_PROJECT_ADD_FILE_EXIST);

  // project-menu
  project_menu = new QPopupMenu;
  project_menu->insertItem(i18n("KAppWizard..."), this, SLOT(slotProjectNewAppl()),0,ID_PROJECT_KAPPWIZARD);
  project_menu->insertItem(i18n("New"), this, SLOT(slotProjectNew()),0, ID_PROJECT_NEW);
  project_menu->insertItem(i18n("&Open..."), this, SLOT(slotProjectOpen()),0,ID_PROJECT_OPEN);
  project_menu->insertItem(i18n("C&lose"),this, SLOT(slotProjectClose()),0,ID_PROJECT_CLOSE);
  project_menu->insertSeparator();		
  
  project_menu->insertItem(i18n("&New Class..."), this,
			   SLOT(slotProjectNewClass()),0,ID_PROJECT_NEW_CLASS);
  project_menu->insertItem(i18n("&Add File(s) to Project"),p2,ID_PROJECT_ADD_FILE);
  //  project_menu->insertItem(i18n("&Remove File from Project"), this,
  //			   SLOT(slotProjectRemoveFile()),0,ID_PROJECT_REMOVE_FILE);
  //project_menu->insertSeparator();

  project_menu->insertSeparator();		
  project_menu->insertItem(i18n("&File Properties..."), this, SLOT(slotProjectFileProperties())
			   ,0,ID_PROJECT_FILE_PROPERTIES);
  project_menu->insertItem(i18n("&Options..."), this, SLOT(slotProjectOptions()),0,ID_PROJECT_OPTIONS);

  menuBar()->insertItem(i18n("&Project"), project_menu);

  disableCommand(ID_PROJECT_ADD_FILE_NEW);
  disableCommand(ID_PROJECT_CLOSE);
  disableCommand(ID_PROJECT_ADD_FILE_EXIST);
  disableCommand(ID_PROJECT_ADD_FILE);
  //  disableCommand(ID_PROJECT_REMOVE_FILE);
  disableCommand(ID_PROJECT_NEW_CLASS);
  disableCommand(ID_PROJECT_FILE_PROPERTIES);
  disableCommand(ID_PROJECT_OPTIONS);

  
///////////////////////////////////////////////////////////////////
// Build-menu entries

  build_menu = new QPopupMenu;
  build_menu->insertItem(Icon("compfile.xpm"),i18n("Compile File"),
			 this,SLOT(slotBuildCompileFile()),0,ID_BUILD_COMPILE_FILE);
  accel->changeMenuAccel(build_menu,ID_BUILD_COMPILE_FILE ,"CompileFile" );

  build_menu->insertItem(Icon("make.xpm"),i18n("&Make"),this,
			 SLOT(slotBuildMake()),0,ID_BUILD_MAKE);
  accel->changeMenuAccel(build_menu,ID_BUILD_MAKE ,"Make" );

  build_menu->insertItem(Icon("rebuild.xpm"),i18n("&Rebuild all"), this,
			 SLOT(slotBuildRebuildAll()),0,ID_BUILD_REBUILD_ALL);

  build_menu->insertItem(i18n("&Clean/Rebuild all"), this, 
			 SLOT(slotBuildCleanRebuildAll()),0,ID_BUILD_CLEAN_REBUILD_ALL);
  build_menu->insertSeparator();
  build_menu->insertItem(Icon("stop.xpm"),i18n("&Stop Build"), this, SLOT(slotBuildStop()),0,ID_BUILD_STOP);
  build_menu->insertSeparator();

  build_menu->insertItem(Icon("run.xpm"),i18n("&Run"),this,SLOT(slotBuildRun()),0,ID_BUILD_RUN);
  accel->changeMenuAccel(build_menu,ID_BUILD_RUN ,"Run" );

  build_menu->insertItem(i18n("&Debug..."),this,SLOT(slotBuildDebug()),0,ID_BUILD_DEBUG);
  build_menu->insertSeparator();
  build_menu->insertItem(i18n("&DistClean"),this,SLOT(slotBuildDistClean()),0,ID_BUILD_DISTCLEAN);
  build_menu->insertItem(i18n("&Autoconf"),this,SLOT(slotBuildAutoconf()),0,ID_BUILD_AUTOCONF);
  build_menu->insertItem(i18n("C&onfigure"), this, SLOT(slotBuildConfigure()),0,ID_BUILD_CONFIGURE);
  build_menu->insertSeparator();
  build_menu->insertItem(i18n("Make &API-Doc"), this, 
			 SLOT(slotBuildAPI()),0,ID_BUILD_MAKE_PROJECT_API);
  build_menu->insertItem(i18n("Make &User-Manual"), this, 
			 SLOT(slotBuildManual()),0,ID_BUILD_MAKE_USER_MANUAL);
  
  menuBar()->insertItem(i18n("&Build"), build_menu);

  disableCommand(ID_BUILD_RUN);
  disableCommand(ID_BUILD_DEBUG);
  disableCommand(ID_BUILD_MAKE);
  disableCommand(ID_BUILD_REBUILD_ALL);
  disableCommand(ID_BUILD_CLEAN_REBUILD_ALL);
  disableCommand(ID_BUILD_DISTCLEAN);
  disableCommand(ID_BUILD_AUTOCONF);
  disableCommand(ID_BUILD_CONFIGURE);
  disableCommand(ID_BUILD_STOP);
  disableCommand(ID_BUILD_MAKE_PROJECT_API);
  disableCommand(ID_BUILD_MAKE_USER_MANUAL);
  disableCommand(ID_BUILD_COMPILE_FILE);


///////////////////////////////////////////////////////////////////
// Tools-menu entries

  tools_menu = new QPopupMenu;
  tools_menu->insertItem(i18n("&KDbg"),this, SLOT(slotToolsKDbg()),0,ID_TOOLS_KDBG);
  tools_menu->insertItem(i18n("&KIconedit"),this, SLOT(slotToolsKIconEdit()),0,ID_TOOLS_KICONEDIT);
  tools_menu->insertItem(i18n("KTranslator"),this, SLOT(slotToolsKTranslator()),0,ID_TOOLS_KTRANSLATOR);
  menuBar()->insertItem(i18n("&Tools"), tools_menu);

  
///////////////////////////////////////////////////////////////////
// Options-menu entries

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

  options_menu->insertItem(i18n("Configure &Keys..."),this,
			   SLOT(slotOptionsKeys()),0,ID_OPTIONS_KEYS);
  options_menu->insertSeparator();
  
  options_menu->insertItem(i18n("Documentation &Browser..."),this,
		SLOT(slotOptionsDocBrowser()),0,ID_OPTIONS_DOCBROWSER);
  options_menu->insertItem(i18n("&Documentation Path..."),this,
		SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);
  options_menu->insertSeparator();
  options_menu->insertItem(i18n("Update &KDE-Documentation..."),this,
				 SLOT(slotDocUpdateKDEDocumentation()),0,ID_OPTIONS_UPDATE_KDE_DOCUMENTATION);
  options_menu->insertItem(i18n("Create &Searchdatabase..."),this,
				 SLOT(slotCreateSearchDatabase()),0,ID_OPTIONS_CREATE_SEARCHDATABASE);
  options_menu->insertSeparator();


  QPopupMenu* make = new QPopupMenu;
  make->insertItem("&Make",ID_OPTIONS_MAKE_MAKE);
  make->insertItem("&Gmake",ID_OPTIONS_MAKE_GMAKE);
  make->insertItem("&Dmake",ID_OPTIONS_MAKE_DMAKE);
  options_menu->insertItem("Make-&Command..",make,ID_OPTIONS_MAKE);
  connect(make, SIGNAL(activated(int)), SLOT(slotOptionsMake(int)));
  
  config->setGroup("General Options");
  make_cmd=config->readEntry("Make","make");
	if(make_cmd=="make")
		options_menu->setItemChecked(ID_OPTIONS_MAKE_MAKE, true);
	if(make_cmd=="gmake")
		options_menu->setItemChecked(ID_OPTIONS_MAKE_GMAKE, true);
	if(make_cmd=="dmake")
		options_menu->setItemChecked(ID_OPTIONS_MAKE_DMAKE, true);

  menuBar()->insertItem(i18n("&Options"), options_menu);

///////////////////////////////////////////////////////////////////
// Window-menu entries

  menu_buffers = new QPopupMenu;
  menuBar()->insertItem(i18n("&Window"), menu_buffers);
  menuBar()->insertSeparator();
  
///////////////////////////////////////////////////////////////////
// Documentation-menu entries

  documentation_menu = new QPopupMenu;
  documentation_menu->insertItem(i18n("Back"),this, SLOT(slotDocBack()),0,ID_DOC_BACK);
  documentation_menu->insertItem(i18n("Forward"),this, SLOT(slotDocForward()),0,ID_DOC_FORWARD);
  documentation_menu->insertSeparator();
  documentation_menu->insertItem(i18n("&Search Marked Text"),this,
				 SLOT(slotDocSText()),0,ID_DOC_SEARCH_TEXT);
  accel->changeMenuAccel(documentation_menu,ID_DOC_SEARCH_TEXT,"SearchMarkedText" );
  documentation_menu->insertSeparator();
  documentation_menu->insertItem(i18n("&Qt-Library"),this, SLOT(slotDocQtLib()),0,ID_DOC_QT_LIBRARY);
  documentation_menu->insertItem(i18n("KDE-&Core-Library"),this,
				 SLOT(slotDocKDECoreLib()),0,ID_DOC_KDE_CORE_LIBRARY);
  documentation_menu->insertItem(i18n("KDE-&GUI-Library"),this,
				 SLOT(slotDocKDEGUILib()),0,ID_DOC_KDE_GUI_LIBRARY);
  documentation_menu->insertItem(i18n("KDE-&KFile-Library"),this,
				 SLOT(slotDocKDEKFileLib()),0,ID_DOC_KDE_KFILE_LIBRARY);
  documentation_menu->insertItem(i18n("KDE-&HTML-Library"),this,
				 SLOT(slotDocKDEHTMLLib()),0,ID_DOC_KDE_HTML_LIBRARY);
  documentation_menu->insertSeparator();
  documentation_menu->insertItem(i18n("Project &API-Doc"),this,
				      SLOT(slotDocAPI()),0,ID_DOC_PROJECT_API_DOC);

  documentation_menu->insertItem(i18n("Project &User-Manual"),this,
				      SLOT(slotDocManual()),0,ID_DOC_USER_MANUAL);

  menuBar()->insertItem(i18n("&Documentation"), documentation_menu);


  disableCommand(ID_DOC_BACK);
  disableCommand(ID_DOC_FORWARD);
  disableCommand(ID_DOC_PROJECT_API_DOC);
  disableCommand(ID_DOC_USER_MANUAL);

///////////////////////////////////////////////////////////////////
// Help-menu entries
  QPopupMenu* help_menu = new QPopupMenu();
  help_menu->insertItem(i18n("Contents"),this,SLOT(slotHelpContent()),IDK_HELP_CONTENT,ID_HELP_CONTENT);

  //  help_menu->insertItem(i18n("KDevelop Homepage"),this, SLOT(slotHelpHomepage()),0,ID_HELP_HOMEPAGE);
  help_menu->insertSeparator();
  help_menu->insertItem(i18n("About KDevelop..."),this, SLOT(slotHelpAbout()),0,ID_HELP_ABOUT);
  
  menuBar()->insertItem(i18n("&Help"), help_menu);

///////////////////////////////////////////////////////////////////
// connects for the statusbar help
  connect(file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(view_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p2,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(tools_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(options_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(documentation_menu,SIGNAL(highlighted(int)),SLOT(statusCallback(int)));
  connect(help_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

}

void CKDevelop::initToolbar(){
  QPixmap pix;
  QString  path;
 
//  toolBar()->insertButton(Icon("filenew.xpm"),ID_FILE_NEW, false,i18n("New"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/openprj.xpm");
  toolBar()->insertButton(pix,ID_PROJECT_OPEN, true,i18n("Open Project"));
  toolBar()->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  toolBar()->insertButton(pix,ID_FILE_OPEN, true,i18n("Open File"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save.xpm");
  toolBar()->insertButton(pix,ID_FILE_SAVE,true,i18n("Save File"));
/*  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save_all.xpm");
  toolBar()->insertButton(pix,ID_FILE_SAVE_ALL,true,i18n("Save All"));
*/

  QFrame *separatorLine= new QFrame(toolBar());
  separatorLine->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar()->insertWidget(0,10,separatorLine);

//  toolBar()->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/cut.xpm");
  toolBar()->insertButton(pix,ID_EDIT_CUT,true,i18n("Cut"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/copy.xpm");
  toolBar()->insertButton(pix,ID_EDIT_COPY, true,i18n("Copy"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/paste.xpm");
  toolBar()->insertButton(pix,ID_EDIT_PASTE, true,i18n("Paste"));
  toolBar()->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/undo.xpm");
	toolBar()->insertButton(pix,ID_EDIT_UNDO,false,i18n("Undo"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/redo.xpm");
	toolBar()->insertButton(pix,ID_EDIT_REDO,false,i18n("Undo"));

//  toolBar()->insertSeparator();
  QFrame *separatorLine1= new QFrame(toolBar());
  separatorLine1->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar()->insertWidget(0,20,separatorLine1);

// toolBar()->insertButton(Icon("reload.xpm"),ID_VIEW_REFRESH, true,i18n("Refresh"));

//  toolBar()->insertSeparator();
//  toolBar()->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/compfile.xpm");
  toolBar()->insertButton(pix,ID_BUILD_COMPILE_FILE, false,i18n("Compile file"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/make.xpm");
  toolBar()->insertButton(pix,ID_BUILD_MAKE, false,i18n("Make"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/rebuild.xpm");
  toolBar()->insertButton(pix,ID_BUILD_REBUILD_ALL, false,i18n("Rebuild"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/run.xpm");
  toolBar()->insertButton(pix,ID_BUILD_RUN, false,i18n("Run"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/stop.xpm");
  toolBar()->insertSeparator();
  toolBar()->insertButton(pix,ID_BUILD_STOP, false,i18n("Stop"));
//  toolBar()->insertSeparator();

  QFrame *separatorLine2= new QFrame(toolBar());
  separatorLine2->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar()->insertWidget(0,30,separatorLine2);

  whats_this = new QWhatsThis;  
  QToolButton *btnwhat = whats_this->whatsThisButton(toolBar());
  QToolTip::add(btnwhat, i18n("What's this...?"));
  toolBar()->insertWidget(ID_HELP_WHATS_THIS, btnwhat->sizeHint().width(), btnwhat);
  btnwhat->setFocusPolicy(QWidget::NoFocus);

  connect(toolBar(), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  config->setGroup("General Options");
  if(config->readBoolEntry("show_std_toolbar", true)){
    enableToolBar(KToolBar::Show,0);
  }
  else{
    enableToolBar(KToolBar::Hide,0);
  }

  // the second toolbar
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo("Classes",TOOLBAR_CLASS_CHOICE,true,SIGNAL(activated(int))
			  ,this,SLOT(slotClassChoiceCombo(int)),true,i18n("choice the class"),160 );
  KCombo* class_combo = toolBar(1)->getCombo(TOOLBAR_CLASS_CHOICE);
  class_combo->setFocusPolicy(QWidget::NoFocus);
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo("Methods",TOOLBAR_METHOD_CHOICE,true,SIGNAL(activated(int))
			  ,this,SLOT(slotMethodChoiceCombo(int)),true,i18n("choice the methods"),240 );
  KCombo* choice_combo = toolBar(1)->getCombo(TOOLBAR_METHOD_CHOICE);
  choice_combo->setFocusPolicy(QWidget::NoFocus);
  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(Icon("back.xpm"),ID_DOC_BACK, false,i18n("Back"));
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(Icon("forward.xpm"),ID_DOC_FORWARD, false,i18n("Forward"));
    
  toolBar(ID_BROWSER_TOOLBAR)->insertButton(Icon("contents.xpm"),ID_DOC_SEARCH_TEXT, 
					    true,i18n("Search Text in Documenation"));
  
  connect(toolBar(ID_BROWSER_TOOLBAR), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  if(config->readBoolEntry("show_browser_toolbar", true)){
    enableToolBar(KToolBar::Show,ID_BROWSER_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide,ID_BROWSER_TOOLBAR);
  }
}

void CKDevelop::initStatusBar(){
  statusBar()->insertItem(i18n("xxxxxxxxxxxxxxxxxxxx"), ID_STATUS_EMPTY);
  statusBar()->insertItem(i18n("Line: 00000 Col: 000"), ID_STATUS_LN_CLM);
  statusBar()->changeItem(i18n(""), ID_STATUS_EMPTY);
  statusBar()->changeItem(i18n(""), ID_STATUS_LN_CLM);
  statusBar()->insertItem(i18n(" INS "), ID_STATUS_INS_OVR);
  statusBar()->insertItem(i18n("Welcome to KDevelop!"), ID_STATUS_MSG);
  statusBar()->setInsertOrder(KStatusBar::RightToLeft);
  statusBar()->setAlignment(ID_STATUS_INS_OVR, AlignCenter);
  enableStatusBar();
  if(!bViewStatusbar)
  enableStatusBar();
}

// all the init-stuff
void CKDevelop::initConnections(){
  // connections for the proc -processes
  connect(&search_process,SIGNAL(receivedStdout(KProcess*,char*,int)),
  	  this,SLOT(slotSearchReceivedStdout(KProcess*,char*,int)) );

  connect(&search_process,SIGNAL(receivedStderr(KProcess*,char*,int)),
  	  this,SLOT(slotReceivedStderr(KProcess*,char*,int)) );

  connect(&search_process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotSearchProcessExited(KProcess*) )) ;


  connect(&process,SIGNAL(receivedStdout(KProcess*,char*,int)),
  	  this,SLOT(slotReceivedStdout(KProcess*,char*,int)) );

  connect(&process,SIGNAL(receivedStderr(KProcess*,char*,int)),
	  this,SLOT(slotReceivedStderr(KProcess*,char*,int)) );

  connect(&process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotProcessExited(KProcess*) )) ;

  // shellprocess
  connect(&shell_process,SIGNAL(receivedStdout(KProcess*,char*,int)),
	  this,SLOT(slotReceivedStdout(KProcess*,char*,int)) );

  connect(&shell_process,SIGNAL(receivedStderr(KProcess*,char*,int)),
	  this,SLOT(slotReceivedStderr(KProcess*,char*,int)) );

  connect(&shell_process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotProcessExited(KProcess*) )) ;

  //application process

  connect(&appl_process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotProcessExited(KProcess*) ));

  connect(&appl_process,SIGNAL(receivedStdout(KProcess*,char*,int)),
	  this,SLOT(slotApplReceivedStdout(KProcess*,char*,int)) );

  connect(&appl_process,SIGNAL(receivedStderr(KProcess*,char*,int)),
	  this,SLOT(slotApplReceivedStderr(KProcess*,char*,int)) );


  // connect the windowsmenu with a method
  connect(menu_buffers,SIGNAL(activated(int)),this,SLOT(slotMenuBuffersSelected(int)));

  //connect the editor lookup function with slotDocSText
  connect(cpp_widget,SIGNAL(lookUp(QString)),this,SLOT(slotDocSText(QString)));
  connect(header_widget,SIGNAL(lookUp(QString)),this,SLOT(slotDocSText(QString)));
	
}
void CKDevelop::initProject(){
  config->setGroup("Files");
  QString filename = config->readEntry("project_file","");
  QFile file(filename);
  // cerr << "INITPROJECT: " << filename << endl;
  if (file.exists()){
    if(!(readProjectFile(filename))){
      KMsgBox::message(0,filename,"This is a Project-File from KDevelop 0.1\nSorry,but it's incompatible with KDevelop >= 0.2.\nPlease use only new generated projects!");
      refreshTrees();
    }
	  config->setGroup("Files");
  	QString filename = config->readEntry("project_file","");
  	QFile file(filename);
  	filename = config->readEntry("header_file",i18n("Untitled.h"));
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









































































