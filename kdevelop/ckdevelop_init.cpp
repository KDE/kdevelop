/***************************************************************************
      init.cpp - the init specific part of CKDevelop (construtor ...)
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
#include "./kwrite/kwdoc.h"

CKDevelop::CKDevelop(){
  QString filename;
  version = VERSION;
  project=false;// no project

  init();
  initConnections();
  initProject(); 
  
  // read the three last files

   config->setGroup("Files");
   filename = config->readEntry("header_file");
   if (QFile::exists(filename)){
     switchToFile(filename);
   }
   config->setGroup("Files");
   filename = config->readEntry("cpp_file");
   if (QFile::exists(filename)){
     switchToFile(filename);
   }
   config->setGroup("Files");
   filename = config->readEntry("browser_file");
   if(!filename.isEmpty()){
     slotURLSelected(browser_widget,filename,1,"test");
   }
   else{
   slotHelpContent();
   }
  config->setGroup("General Options");
  int lastActiveTab=config->readNumEntry("LastActiveTab", BROWSER);
  slotSTabSelected(lastActiveTab);

}

// all the init-stuff
void CKDevelop::initConnections(){
  // connections for the proc -processes
  connect(&search_process,SIGNAL(receivedStdout(KProcess*,char*,int)),
  	  this,SLOT(slotSearchReceivedStdout(KProcess*,char*,int)) );
  connect(&search_process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotSearchProcessExited(KProcess*) )) ;
  
  connect(&process,SIGNAL(receivedStdout(KProcess*,char*,int)),
  	  this,SLOT(slotReceivedStdout(KProcess*,char*,int)) );

  connect(&process,SIGNAL(receivedStderr(KProcess*,char*,int)),
	  this,SLOT(slotReceivedStderr(KProcess*,char*,int)) );

  connect(&process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotProcessExited(KProcess*) )) ;
  
  connect(&shell_process,SIGNAL(receivedStdout(KProcess*,char*,int)),
	  this,SLOT(slotReceivedStdout(KProcess*,char*,int)) );
  connect(&shell_process,SIGNAL(processExited(KProcess*)),
	  this,SLOT(slotProcessExited(KProcess*) )) ;
  
  // connect the windowsmenu with a method
  connect(menu_buffers,SIGNAL(activated(int)),this,SLOT(slotMenuBuffersSelected(int)));

  //connect the editor lookup function with slotDocSText
  connect(cpp_widget,SIGNAL(lookUp(QString)),this,SLOT(slotDocSText(QString)));
  connect(header_widget,SIGNAL(lookUp(QString)),this,SLOT(slotDocSText(QString)));
	  
  
}
void CKDevelop::init(){
  
  act_outbuffer_len=0;

  QFont font("Fixed",10);
  KApplication *app=KApplication::getKApplication();
  config = app->getConfig();
  config->setGroup("General Options");
  int w = config->readNumEntry("width", 800);
  int h = config->readNumEntry("height", 500);
  resize(w,h);

  // call bar functions to create bars

  // create the main view

  view = new KNewPanner(this,"view",KNewPanner::Horizontal,KNewPanner::Absolute,
  			config->readNumEntry("view_panner_pos", 337));

  output_widget = new COutputWidget(kapp,view);
//  output_widget->setFillColumnMode(80,true);
//  output_widget->setWordWrap(true);
  output_widget->setReadOnly(TRUE);
//  output_widget->setAutoIndentMode(true);
//  output_widget->setFocusPolicy(QWidget::ClickFocus);
  connect(output_widget,SIGNAL(clicked()),this,SLOT(slotClickedOnOutputWidget()));

  
  //  s_tab_current = 0;

  top_panner = new KNewPanner(view,"top_panner",KNewPanner::Vertical,KNewPanner::Absolute,
  			      config->readNumEntry("top_panner_pos", 213));

  t_tab_view = new CTabCtl(top_panner);

  log_file_tree = new CLogFileView(t_tab_view,"lfv");
  log_file_tree->setIndentSpacing(15);
  log_file_tree->setFocusPolicy(QWidget::ClickFocus);
  
  class_tree = new CClassView(t_tab_view,"cv");
  class_tree->setIndentSpacing(15);
  class_tree->setFocusPolicy(QWidget::ClickFocus);

  real_file_tree = new CRealFileView(t_tab_view,"RFV");
  real_file_tree->setFocusPolicy(QWidget::ClickFocus);
  real_file_tree->setIndentSpacing(15);

  doc_tree = new CDocTree(t_tab_view,"DOC");
  doc_tree->setFocusPolicy(QWidget::ClickFocus);
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
  connect(s_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotSTabSelected(int)));

  header_widget = new CEditWidget(kapp,s_tab_view,"header");
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
  cpp_widget->setFont(font);
  cpp_widget->setName("Untitled.cpp");
  config->setGroup("KWrite Options");
  cpp_widget->readConfig(config);
  cpp_widget->doc()->readConfig(config);
 
  connect(cpp_widget, SIGNAL(newCurPos()), this, SLOT(slotNewLineColumn()));
  connect(cpp_widget, SIGNAL(newStatus()),this, SLOT(slotNewStatus()));
  connect(cpp_widget, SIGNAL(newUndo()),this, SLOT(slotNewUndo()));


  // init the 2 first kedits
  TEditInfo* edit1 = new TEditInfo;
  TEditInfo* edit2 = new TEditInfo;
  edit1->filename = header_widget->getName();
  edit2->filename = cpp_widget->getName();
  


  browser_widget = new CDocBrowser(s_tab_view,"browser");  
  prev_was_search_result= false;
  //init
  browser_widget->setDocBrowserOptions();

  connect(browser_widget,SIGNAL(URLSelected(KHTMLView*,const char*,int,const char*)),
	  this,SLOT(slotURLSelected(KHTMLView*,const char*,int,const char*)));
  connect(browser_widget,SIGNAL(documentDone(KHTMLView*)),
	  this,SLOT(slotDocumentDone(KHTMLView*)));
  swallow_widget = new KSwallowWidget(s_tab_view);
  

  s_tab_view->addTab(header_widget,"Header/Resource Files");
  s_tab_view->addTab(cpp_widget,"C/C++ Files");
  s_tab_view->addTab(browser_widget,"Documentation-Browser");
  s_tab_view->addTab(swallow_widget,"Tools");

  top_panner->activate(t_tab_view,s_tab_view);// activate the top_panner
  view->activate(top_panner,output_widget); 
  
  output_view_pos=view->separatorPos();
  tree_view_pos=top_panner->separatorPos();

  // set the mainwidget
  setView(view);
  initMenu();
  initToolbar();
  initStatusBar();

  // the rest of the init for the kedits
  edit1->id = menu_buffers->insertItem(edit1->filename,-2,0);
  edit1->modified=false;
  edit2->id = menu_buffers->insertItem(edit2->filename,-2,0);
  edit2->modified=false;
  edit_infos.append(edit1);
  edit_infos.append(edit2);


}
void CKDevelop::initMenu(){
  // build a menubar
  QPixmap pix;
  // a menubar consists of popup menus
  file_menu = new QPopupMenu;
 
  file_menu->insertItem(Icon("filenew.xpm"),i18n("&New"), this, SLOT(slotFileNewFile()),
			CTRL+Key_N,ID_FILE_NEW_FILE);


  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  file_menu->insertItem(pix,i18n("&Open..."), this, SLOT(slotFileOpenFile()),
			CTRL+Key_O,ID_FILE_OPEN_FILE);

  file_menu->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save.xpm");
  file_menu->insertItem(pix,i18n("&Save"), this, SLOT(slotFileSave()), CTRL+Key_S,ID_FILE_SAVE);
  file_menu->insertItem(i18n("Save &As..."), this, SLOT(slotFileSaveAs()),0,ID_FILE_SAVE_AS); 
  file_menu->insertItem(i18n("Save All"), this, SLOT(slotFileSaveAll()),0,ID_FILE_SAVE_ALL);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("&Close"), this, SLOT(slotFileClose()), CTRL+Key_W,ID_FILE_CLOSE);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("&Print..."), this, SLOT(slotFilePrint()),0,ID_FILE_PRINT);
  file_menu->insertSeparator();
  file_menu->insertItem(i18n("&Quit"),this, SLOT(slotFileQuit()), CTRL+Key_Q,ID_FILE_QUIT);

  // put the popupmenu as File-menu into the menu bar
  menuBar()->insertItem(i18n("&File"), file_menu);

  disableCommand(ID_FILE_NEW_FILE);
  disableCommand(ID_FILE_CLOSE_ALL);
  menuBar()->insertSeparator();

  // edit menu
  
  edit_menu = new QPopupMenu;
  edit_menu->insertItem(i18n("U&ndo"), this, SLOT(slotEditUndo()), CTRL+Key_Z,ID_EDIT_UNDO);
  edit_menu->insertItem(i18n("R&edo"), this, SLOT(slotEditRedo()), CTRL+Key_Y,ID_EDIT_REDO);  
  edit_menu->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/cut.xpm");
  edit_menu->insertItem(pix,i18n("C&ut"), this, SLOT(slotEditCut()), CTRL+Key_X,ID_EDIT_CUT);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/copy.xpm");
  edit_menu->insertItem(pix,i18n("&Copy"), this, SLOT(slotEditCopy()), CTRL+Key_C,ID_EDIT_COPY);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/paste.xpm");
  edit_menu->insertItem(pix,i18n("&Paste"), this, SLOT(slotEditPaste()), CTRL+Key_V,ID_EDIT_PASTE);
  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("&Insert File..."),this, SLOT(slotEditInsertFile()),0,ID_EDIT_INSERT_FILE);
  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("&Search..."), this, SLOT(slotEditSearch()),0,ID_EDIT_SEARCH);
  edit_menu->insertItem(i18n("&Repeat Search..."), this, 
			SLOT(slotEditRepeatSearch()),Key_F3,ID_EDIT_REPEAT_SEARCH);
  edit_menu->insertItem(i18n("&Replace..."), this, SLOT(slotEditReplace()),0,ID_EDIT_REPLACE);
  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("Select &All"), this, SLOT(slotEditSelectAll()),0,ID_EDIT_SELECT_ALL);
  edit_menu->insertItem(i18n("Deselect All"), this, SLOT(slotEditDeselectAll()),0,ID_EDIT_DESELECT_ALL);
  edit_menu->insertItem(i18n("Invert Selection"), this, SLOT(slotEditInvertSelection()),0,ID_EDIT_INVERT_SELECTION);
  
  menuBar()->insertItem(i18n("&Edit"), edit_menu);
  disableCommand(ID_EDIT_UNDO);
  disableCommand(ID_EDIT_REDO);
  menuBar()->insertSeparator();

  view_menu = new QPopupMenu;
  view_menu->insertItem(i18n("&Goto Line..."), this, SLOT(slotEditGotoLine()),0,ID_VIEW_GOTO_LINE);
  view_menu->insertSeparator();
  config->setGroup("General Options");
  bViewStatusbar = config->readBoolEntry("show_statusbar",true);

  view_menu->insertItem(i18n("&Tree-View"),this, SLOT(slotOptionsTTreeView()),
 			   CTRL+Key_T,ID_VIEW_TREEVIEW);
  view_menu->setItemChecked(ID_VIEW_TREEVIEW,config->readBoolEntry("show_tree_view",true));

  view_menu->insertItem(i18n("&Output-View"),this, SLOT(slotOptionsTOutputView()),CTRL+Key_O,ID_VIEW_OUTPUTVIEW);
  view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,config->readBoolEntry("show_output_view",true));

  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Toolbar"),this,
			   SLOT(slotOptionsTStdToolbar()),0,ID_VIEW_TOOLBAR);
  view_menu->setItemChecked(ID_VIEW_TOOLBAR,config->readBoolEntry("show_std_toolbar", true));

  view_menu->insertItem(i18n("&Browser-Toolbar"),this,
			   SLOT(slotOptionsTBrowserToolbar()),0,ID_VIEW_BROWSER_TOOLBAR);
  view_menu->setItemChecked(ID_VIEW_BROWSER_TOOLBAR,config->readBoolEntry("show_browser_toolbar",true));

  view_menu->insertItem(i18n("Status&bar"),this,
			   SLOT(slotOptionsTStatusbar()),CTRL+Key_B,ID_VIEW_STATUSBAR);
  view_menu->setItemChecked(ID_VIEW_STATUSBAR,bViewStatusbar);

  view_menu->insertSeparator();
  view_menu->insertItem(Icon("reload.xpm"),i18n("&Refresh"),this,
			   SLOT(slotOptionsRefresh()),0,ID_VIEW_REFRESH);

  menuBar()->insertItem(i18n("&View"), view_menu);
  menuBar()->insertSeparator();
  



  // the bookmarks menu
  // p = new QPopupMenu;
//   p->insertItem(i18n("&Add Bookmark"), this, SLOT(slotBookmarksAdd()));
//   p->insertItem(i18n("&Edit Bookmarks..."), this, SLOT(slotBookmarksEdit()));
//   p->insertSeparator();
//   menuBar()->insertItem(i18n("&Bookmarks"),p);



  // project Menu
  // first the submenu for adding projectfiles
  QPopupMenu* p2 = new QPopupMenu;
  p2->insertItem(i18n("&New File..."), this, SLOT(slotProjectAddNewFile()),0,ID_PROJECT_ADD_FILE_NEW);
  p2->insertItem(i18n("&Existing File(s)..."), this,
		 SLOT(slotProjectAddExistingFiles()),0,ID_PROJECT_ADD_FILE_EXIST);


  project_menu = new QPopupMenu;
  project_menu->insertItem(i18n("KAppWizard..."), this, SLOT(slotFileNewAppl()),0,ID_FILE_NEW_PROJECT);
  project_menu->insertItem(i18n("New..."), this, SLOT(slotProjectNew()),0, ID_PROJECT_NEW);
  project_menu->insertItem(i18n("Open &Project..."), this, SLOT(slotFileOpenPrj()),0,ID_FILE_OPEN_PROJECT);
  project_menu->insertSeparator();		
  project_menu->insertItem(i18n("Compile File "),this, SLOT(slotProjectCompileFile()),CTRL+Key_F8,ID_PROJECT_COMPILE_FILE);
  
  project_menu->insertSeparator();		
  project_menu->insertItem(i18n("&New Class..."), this,
			   SLOT(slotProjectNewClass()),0,ID_PROJECT_NEW_CLASS);
  project_menu->insertItem("&Add File(s) to Project",p2,ID_PROJECT_ADD_FILE);
  //  project_menu->insertItem(i18n("&Remove File from Project"), this,
  //			   SLOT(slotProjectRemoveFile()),0,ID_PROJECT_REMOVE_FILE);
  //project_menu->insertSeparator();

  project_menu->insertSeparator();		
  project_menu->insertItem(i18n("&File Properties..."), this, SLOT(slotProjectFileProperties())
			   ,0,ID_PROJECT_FILE_PROPERTIES);
  project_menu->insertItem(i18n("&Options..."), this, SLOT(slotProjectOptions()),0,ID_PROJECT_OPTIONS);

  menuBar()->insertItem(i18n("&Project"), project_menu);
  disableCommand(ID_PROJECT_ADD_FILE_NEW);
  disableCommand(ID_PROJECT_NEW);
  disableCommand(ID_PROJECT_COMPILE_FILE);
  disableCommand(ID_PROJECT_ADD_FILE_EXIST);
  disableCommand(ID_PROJECT_ADD_FILE);
  //  disableCommand(ID_PROJECT_REMOVE_FILE);
  disableCommand(ID_PROJECT_NEW_CLASS);
  disableCommand(ID_PROJECT_FILE_PROPERTIES);
  disableCommand(ID_PROJECT_OPTIONS);

  // build menu
  
  build_menu = new QPopupMenu;

  build_menu->insertItem(Icon("run.xpm"),i18n("&Run"), this, 
			 SLOT(slotBuildRun()),CTRL+Key_F9,ID_BUILD_RUN);
  build_menu->insertItem(i18n("&Debug..."),this,SLOT(slotBuildDebug()),0,ID_BUILD_DEBUG);
  build_menu->insertItem(Icon("make.xpm"),i18n("&Make"), this, 
			 SLOT(slotBuildMake()),Key_F9,ID_BUILD_MAKE);
  build_menu->insertSeparator();

  build_menu->insertItem(i18n("&Rebuild all"), this, 
			 SLOT(slotBuildRebuildAll()),0,ID_BUILD_REBUILD_ALL);
  build_menu->insertItem(i18n("&Clean/Rebuild all"), this, 
			 SLOT(slotBuildCleanRebuildAll()),0,ID_BUILD_CLEAN_REBUILD_ALL);
  build_menu->insertItem(i18n("&DistClean"),this,SLOT(slotBuildDistClean()),0,ID_BUILD_DISTCLEAN);
  build_menu->insertItem(i18n("&Autoconf"),this,SLOT(slotBuildAutoconf()),0,ID_BUILD_AUTOCONF);
  build_menu->insertItem(i18n("C&onfigure"), this, SLOT(slotBuildConfigure()),0,ID_BUILD_CONFIGURE);
  build_menu->insertSeparator();
  build_menu->insertItem(Icon("stop.xpm"),i18n("&Stop"), this, SLOT(slotBuildStop()),0,ID_BUILD_STOP);
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

  //the tools menu
  tools_menu = new QPopupMenu;
  tools_menu->insertItem(i18n("&KDbg"),this, SLOT(slotToolsKDbg()),0,ID_TOOLS_KDBG);
  tools_menu->insertItem(i18n("&KIconedit"),this, SLOT(slotToolsKIconEdit()),0,ID_TOOLS_KICONEDIT);
  tools_menu->insertItem(i18n("KTranslator"),this, SLOT(slotToolsKTranslator()),0,ID_TOOLS_KTRANSLATOR);
  menuBar()->insertItem(i18n("&Tools"), tools_menu);

  

  // options menu
  options_menu = new QPopupMenu;
  options_menu->insertItem(i18n("&Editor..."),this,
			   SLOT(slotOptionsEditor()),0,ID_OPTIONS_EDITOR);
  options_menu->insertItem(i18n("Editor-&Colors..."),this,
			   SLOT(slotOptionsEditorColors()),0,ID_OPTIONS_EDITOR_COLORS);
  options_menu->insertItem(i18n("&Syntax Highlighting..."),this,
			   SLOT(slotOptionsSyntaxHighlighting()),0,ID_OPTIONS_SYNTAX_HIGHLIGHTING); 
  options_menu->insertSeparator();
  options_menu->insertItem(i18n("&Documentation Path..."),this,
		SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);    
  options_menu->insertItem(i18n("Documentation-&Browser..."),this,
		SLOT(slotOptionsDocBrowser()),0,ID_OPTIONS_DOCBROWSER);
  options_menu->insertSeparator();
  options_menu->insertItem(i18n("Update &KDE-Documentation..."),this,
				 SLOT(slotDocUpdateKDEDocumentation()),0,ID_OPTIONS_UPDATE_KDE_DOCUMENTATION);
  options_menu->insertItem(i18n("Create &Searchdatabase..."),this,
				 SLOT(slotCreateSearchDatabase()),0,ID_OPTIONS_CREATE_SEARCHDATABASE);


  menuBar()->insertItem(i18n("&Options"), options_menu);
  menuBar()->insertSeparator();

  // the buffers menu

  menu_buffers = new QPopupMenu;
  menuBar()->insertItem(i18n("&Window"), menu_buffers);
  


  // the helpmenu

  menuBar()->insertSeparator();
  // documentation menu
  documentation_menu = new QPopupMenu;
  documentation_menu->insertItem(i18n("Back"),this, SLOT(slotDocBack()),0,ID_DOC_BACK);
  documentation_menu->insertItem(i18n("Forward"),this, SLOT(slotDocForward()),0,ID_DOC_FORWARD);
  documentation_menu->insertSeparator();
  documentation_menu->insertItem(i18n("&Search Marked Text"),this,
				 SLOT(slotDocSText()),Key_F2,ID_DOC_SEARCH_TEXT);
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


  // generate the help menu
  QPopupMenu* help_menu = new QPopupMenu();
  help_menu->insertItem(i18n("Contents"),this, SLOT(slotHelpContent()),Key_F1,ID_HELP_CONTENT);
  //  help_menu->insertItem(i18n("KDevelop Homepage"),this, SLOT(slotHelpHomepage()),0,ID_HELP_HOMEPAGE);
  help_menu->insertSeparator();
  help_menu->insertItem(i18n("About KDevelop..."),this, SLOT(slotHelpAbout()),0,ID_HELP_ABOUT);
  
  menuBar()->insertItem(i18n("&Help"), help_menu);

  connect(file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(documentation_menu,SIGNAL(highlighted(int)),SLOT(statusCallback(int)));
  connect(build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p2,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(options_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(help_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

}

void CKDevelop::initToolbar(){
  QPixmap pix;
  QString  path;
  // build the first toolbar

  // insert some buttons 
  
  toolBar()->insertButton(Icon("filenew.xpm"),ID_FILE_NEW_FILE, false,i18n("New"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  toolBar()->insertButton(pix,ID_FILE_OPEN_FILE, true,i18n("Open File"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save.xpm");
  toolBar()->insertButton(pix,ID_FILE_SAVE,true,i18n("Save File"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save_all.xpm");
  toolBar()->insertButton(pix,ID_FILE_SAVE_ALL,true,i18n("Save All"));

  toolBar()->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/copy.xpm");
  toolBar()->insertButton(pix,ID_EDIT_COPY, true,i18n("Copy"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/paste.xpm");
  toolBar()->insertButton(pix,ID_EDIT_PASTE, true,i18n("Paste"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/cut.xpm");
  toolBar()->insertButton(pix,ID_EDIT_CUT,true,i18n("Cut"));
  
  toolBar()->insertSeparator();
  toolBar()->insertButton(Icon("reload.xpm"),ID_VIEW_REFRESH, true,i18n("Refresh"));

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/make.xpm");
  toolBar()->insertSeparator();
  toolBar()->insertButton(pix,ID_BUILD_MAKE, false,i18n("Make"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/run.xpm");
  toolBar()->insertButton(pix,ID_BUILD_RUN, false,i18n("Run"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/stop.xpm");
  toolBar()->insertButton(pix,ID_BUILD_STOP, false,i18n("Stop"));

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
			  ,this,SLOT(slotClassChoiceCombo(int)),true,i18n("choice the class"),140 );
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
    
    
  }  
  else{
    refreshTrees(); // this refresh only the documentation tab,tree
  }
  
}














