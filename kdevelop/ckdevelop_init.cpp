/***************************************************************************
      init.cpp - the init specific part of CKDevelop (constructor,init() ...)
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

#include "cclassview.h"
#include "cdocbrowser.h"
#include "ceditwidget.h"
#include "cerrormessageparser.h"
#include "ckdevaccel.h"
#include "clogfileview.h"
#include "coutputwidget.h"
#include "crealfileview.h"
#include "ctoolclass.h"
#include "ctabctl.h"
#include "doctreeview.h"
#include "grepdialog.h"
#include "kstartuplogo.h"

#include "./dbg/dbgcontroller.h"
#include "./dbg/vartree.h"
#include "./dbg/framestack.h"
#include "./dbg/brkptmanager.h"
#include "./dbg/disassemble.h"

#include <kaccel.h>
#include <kapp.h>
#include <kcursor.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <khtmlview.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstdaccel.h>
#include <kaboutdata.h>

#include <qprogressbar.h>

#include "./kwrite/kwdoc.h"

#undef Unsorted
#include <qdir.h>
#include <qclipbrd.h>
#include <qframe.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>

CKDevelop::CKDevelop(): KDockMainWindow(0L,"CKDevelop"),
  process("/bin/sh"),
  appl_process("/bin/sh"),
  shell_process("/bin/sh"),
  search_process("/bin/sh"),

  dbgController(0),
  dbgToolbar(0),
  var_viewer(0),
  brkptManager(0),
  frameStack(0),
  disassemble(0),
  dbg_widget(0),
  dbgInternal(false)
{
  doctool = DT_KDOC;

  version = VERSION;
  project=false;// no project
  beep=false; // no beep
  cv_decl_or_impl=true;
  file_open_list.setAutoDelete(TRUE);

  config = KGlobal::config();

  config->setGroup("General Options");
  start_logo=0L;
  if (config->readBoolEntry("Logo",true) && (!kapp->isRestored() ) )
  {
    start_logo= new KStartupLogo(this);
    start_logo->show();
  }


  // ********* DEBUGGER stuff splattered everywhere (jbb) :-)
  // We need to know what debugger we are using to set the
  // system up appropriately
  config->setGroup("Debug");
  dbgInternal = !config->readBoolEntry("Use external debugger");
  dbgExternalCmd = config->readEntry("External debugger program","kdbg");
  dbgEnableFloatingToolbar = config->readBoolEntry("Enable floating toolbar", false);
  // ************ END DEBUGGER STUFF

  config->setGroup("General Options");
  lastShutdownOK = config->readBoolEntry("ShutdownOK",true);
  config->writeEntry("ShutdownOK", false);
  config->sync();   // force this to be written back

  initView();
  initConnections();
  initStatusBar();
  initKeyAccel();
  setKeyAccel();

  readOptions();
  setAutoSaveSettings();

  slotViewRefresh();
  if(start_logo)
    start_logo->raise();

  initDebugger();
  initWhatsThis();


  show();


  setDebugMenuProcess(false);
  setToolmenuEntries();

  error_parser = new CErrorMessageParser;

  slotStatusMsg(i18n("Welcome to KDevelop!"));
}


CKDevelop::~CKDevelop()
{
  if (config)
  {
    config->setGroup("General Options");
    config->writeEntry("ShutdownOK", true);
  }

  // from Constructur... delete everything which is not constructed
  //   with a binding to the application
  delete error_parser;
}

void CKDevelop::initView()
{
  act_outbuffer_len=0;
  prj = 0;

  maindock = createDockWidget( "Editor",SmallIcon("kdevelop") );
  setView(maindock);
  setMainDockWidget( maindock );
  maindock->setEnableDocking( KDockWidget::DockNone);   // We cannot remove this window

  treedock=createDockWidget( "Tree-View", SmallIcon("tree_win"), 0L, i18n("Tree-View"));
  outputdock=createDockWidget( "Output-View", SmallIcon("output_win"), 0L, i18n("Output-View"));
 
  s_tab_view = new CTabCtl(maindock);
  maindock->setWidget( s_tab_view );
	
  t_tab_view = new CTabCtl(treedock);
  treedock->setWidget(t_tab_view);

  o_tab_view = new CTabCtl(outputdock, "output_tabview","output_widget");
  outputdock->setWidget(o_tab_view);

  if (config->hasGroup("dock_setting_default"))
  {
		// use the last placements
    readDockConfig(config);
  }
  else
  {
    // Set the default window placement
    outputdock->manualDock(maindock, KDockWidget::DockBottom, 70/*size relation in %*/);
    treedock->manualDock(maindock, KDockWidget::DockLeft, 35/*size relation in %*/);
  }
  // do not allow to pull off the dockwidgets to desktop or to tab mode
  // the probs which would be introduced are too hard to handle (in KDevelop-1.x ;-)
  outputdock->setEnableDocking( KDockWidget::DockCorner);
  treedock->setEnableDocking( KDockWidget::DockCorner);
  outputdock->setDockSite( KDockWidget::DockCorner);
  treedock->setDockSite( KDockWidget::DockCorner);

  t_tab_view->setFocusPolicy(QWidget::ClickFocus);

  ////////////////////////
  // Treeviews
  ////////////////////////

  class_tree = new CClassView(t_tab_view,"cv");
  class_tree->setFocusPolicy(QWidget::NoFocus);

  log_file_tree = new CLogFileView(config->readBoolEntry("lfv_show_path",false),t_tab_view,"lfv");
  log_file_tree->setFocusPolicy(QWidget::NoFocus);

  real_file_tree = new CRealFileView(t_tab_view,"RFV");
  real_file_tree->setFocusPolicy(QWidget::NoFocus);

  doc_tree = new DocTreeView(t_tab_view,"DOC");
  doc_tree->setFocusPolicy(QWidget::NoFocus);

  // set the mode of the tab headers
  config->setGroup("General Options");
  int mode=config->readNumEntry("tabviewmode", 3);
  switch (mode){
    case 1:
      t_tab_view->addTab(class_tree,i18n("C&lasses"));
      t_tab_view->addTab(log_file_tree,i18n("G&roups"));
      t_tab_view->addTab(real_file_tree,i18n("File&s"));
      t_tab_view->addTab(doc_tree,i18n("Boo&ks"));
      break;
    case 2:
      t_tab_view->addTab(class_tree,SmallIcon("CVclass"),"");
      t_tab_view->addTab(log_file_tree,SmallIcon("attach"),"");
      t_tab_view->addTab(real_file_tree,SmallIcon("folder"),"");
      t_tab_view->addTab(doc_tree,SmallIcon("contents"),"");
      break;
    case 3:
      t_tab_view->addTab(class_tree,SmallIcon("CVclass"),i18n("C&lasses"));
      t_tab_view->addTab(log_file_tree,SmallIcon("attach"),i18n("G&roups"));
      t_tab_view->addTab(real_file_tree,SmallIcon("folder"),i18n("File&s"));
      t_tab_view->addTab(doc_tree,SmallIcon("contents"),i18n("Boo&ks"));
      break;
  }

  ////////////////////////////
  // editor and browser window
  ////////////////////////////
  s_tab_view->setFocusPolicy(QWidget::ClickFocus);

  header_widget = new CEditWidget(s_tab_view,"header");
  header_widget->setFocusPolicy(QWidget::StrongFocus);

  header_widget->setFont(KGlobalSettings::fixedFont());
  header_widget->setName(i18n("Untitled.h"));
  config->setGroup("KWrite Options");
  header_widget->readConfig(config);
  header_widget->doc()->readConfig(config);


  edit_widget=header_widget;
  cpp_widget = new CEditWidget(s_tab_view,"cpp");
  cpp_widget->setFocusPolicy(QWidget::StrongFocus);
  cpp_widget->setFont(KGlobalSettings::fixedFont());
  cpp_widget->setName(i18n("Untitled.cpp"));
  config->setGroup("KWrite Options");
  cpp_widget->readConfig(config);
  cpp_widget->doc()->readConfig(config);

  // init the 2 first kedits
  TEditInfo* edit1 = new TEditInfo;
  TEditInfo* edit2 = new TEditInfo;
  edit1->filename = header_widget->getName();
  edit2->filename = cpp_widget->getName();

  browser_widget = new CDocBrowser(s_tab_view,"browser");

  prev_was_search_result= false;
  //init
  browser_widget->setDocBrowserOptions();

  s_tab_view->addTab(header_widget,SmallIcon("source_h"),i18n("Header/Reso&urce Files"));
  s_tab_view->addTab(cpp_widget,SmallIcon("source_cpp"),i18n("&C/C++ Files"));
  s_tab_view->addTab(browser_widget->view(),SmallIcon("contents"),i18n("&Documentation-Browser"));

  ////////////////////////
  // Outputwindow
  ////////////////////////
	
  messages_widget = new COutputWidget(o_tab_view);
  messages_widget->setFocusPolicy(QWidget::ClickFocus);
  messages_widget->setReadOnly(TRUE);

  stdin_stdout_widget = new COutputWidget(o_tab_view);
  stdin_stdout_widget->setReadOnly(TRUE);
  stdin_stdout_widget->setFocusPolicy(QWidget::ClickFocus);

  stderr_widget = new COutputWidget(o_tab_view);
  stderr_widget->setReadOnly(TRUE);
  stderr_widget->setFocusPolicy(QWidget::ClickFocus);

  o_tab_view->addTab(messages_widget,i18n("messages"));
  o_tab_view->addTab(stdin_stdout_widget,i18n("stdout"));
  o_tab_view->addTab(stderr_widget,i18n("stderr"));

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

  initKeyAccel();
  initMenuBar();
  initToolBar();

  // the rest of the init for the kedits
  edit1->id = menu_buffers->insertItem(edit1->filename,-2,0);
  edit1->modified=false;
  edit2->id = menu_buffers->insertItem(edit2->filename,-2,0);
  edit2->modified=false;
  edit_infos.append(edit1);
  edit_infos.append(edit2);

  // init some dialogs
  grep_dlg = new GrepDialog(QDir::homeDirPath(),0,"grepdialog");
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
void CKDevelop::initKeyAccel()
{
  accel = new CKDevAccel( this );

  //file menu
  accel->connectItem( KStdAccel::New, this, SLOT(slotFileNew()), true, ID_FILE_NEW );
  accel->connectItem( KStdAccel::Open , this, SLOT(slotFileOpen()), true, ID_FILE_OPEN );
  accel->connectItem( KStdAccel::Close , this, SLOT(slotFileClose()), true, ID_FILE_CLOSE );

  accel->connectItem( KStdAccel::Save , this, SLOT(slotFileSave()), true, ID_FILE_SAVE );

  accel->insertItem(i18n("Save As"), "SaveAs", (unsigned int) 0);
  accel->connectItem( "SaveAs", this, SLOT(slotFileSaveAs()), true, ID_FILE_SAVE_AS);

  accel->insertItem(i18n("Save All"), "SaveAll", (unsigned int) 0);
  accel->connectItem( "SaveAll", this, SLOT(slotFileSaveAll()), true, ID_FILE_SAVE_ALL);

  accel->connectItem( KStdAccel::Print , this, SLOT(slotFilePrint()), true, ID_FILE_PRINT );
  accel->connectItem( KStdAccel::Quit, this, SLOT(slotFileQuit()), true, ID_FILE_QUIT );

  //edit menu
  accel->connectItem( KStdAccel::Undo , this, SLOT(slotEditUndo()), true, ID_EDIT_UNDO );

  accel->insertItem( i18n("Redo"), "Redo",IDK_EDIT_REDO );
  accel->connectItem( "Redo" , this, SLOT(slotEditRedo()), true, ID_EDIT_REDO  );

  accel->connectItem( KStdAccel::Cut , this, SLOT(slotEditCut()), true, ID_EDIT_CUT );
  accel->connectItem( KStdAccel::Copy , this, SLOT(slotEditCopy()), true, ID_EDIT_COPY );
  accel->connectItem( KStdAccel::Paste , this, SLOT(slotEditPaste()), true, ID_EDIT_PASTE );

  accel->insertItem( i18n("Indent"), "Indent",IDK_EDIT_INDENT );
  accel->connectItem( "Indent", this, SLOT(slotEditIndent() ), true, ID_EDIT_INDENT );

  accel->insertItem( i18n("Unindent"), "Unindent",IDK_EDIT_UNINDENT );
  accel->connectItem( "Unindent", this, SLOT(slotEditUnindent() ), true, ID_EDIT_UNINDENT );

  accel->insertItem( i18n("Comment"), "Comment",IDK_EDIT_COMMENT );
  accel->connectItem( "Comment", this, SLOT(slotEditComment() ), true, ID_EDIT_COMMENT );

  accel->insertItem( i18n("Uncomment"), "Uncomment",IDK_EDIT_UNCOMMENT );
  accel->connectItem( "Uncomment", this, SLOT(slotEditUncomment() ), true, ID_EDIT_UNCOMMENT );

  accel->insertItem( i18n("Insert File"), "InsertFile", (unsigned int) 0);
  accel->connectItem( "InsertFile", this, SLOT(slotEditInsertFile()), true, ID_EDIT_INSERT_FILE );

  accel->connectItem( KStdAccel::Find, this, SLOT(slotEditSearch() ), true, ID_EDIT_SEARCH );

  accel->insertItem( i18n("Repeat Search"), "RepeatSearch",IDK_EDIT_REPEAT_SEARCH );
  accel->connectItem( "RepeatSearch", this, SLOT(slotEditRepeatSearch() ), true, ID_EDIT_REPEAT_SEARCH );

  accel->connectItem( KStdAccel::Replace, this, SLOT(slotEditReplace() ), true, ID_EDIT_REPLACE );

  accel->insertItem( i18n("Search in Files"), "Grep", IDK_EDIT_GREP_IN_FILES );
  accel->connectItem( "Grep", this, SLOT(slotEditSearchInFiles() ), true, ID_EDIT_SEARCH_IN_FILES );

  accel->insertItem( i18n("Search selection in Files"), "GrepSearch", IDK_EDIT_SEARCH_GREP_IN_FILES );
  accel->connectItem( i18n("GrepSearch"), this, SLOT(slotEditSearchText() ) );

  accel->insertItem( i18n("Select All"), "SelectAll", IDK_EDIT_SELECT_ALL);
  accel->connectItem("SelectAll", this, SLOT(slotEditSelectAll() ), true, ID_EDIT_SELECT_ALL );

  accel->insertItem(i18n("Deselect All"), "DeselectAll", (unsigned int) 0);
  accel->connectItem("DeselectAll", this, SLOT(slotEditDeselectAll()), true, ID_EDIT_DESELECT_ALL);

  accel->insertItem(i18n("Invert Selection"), "Invert Selection", (unsigned int) 0);
  accel->connectItem("Invert Selection", this, SLOT(slotEditInvertSelection()), true, ID_EDIT_INVERT_SELECTION);

  //view menu
  accel->insertItem( i18n("Goto Line"), "GotoLine",IDK_VIEW_GOTO_LINE);
  accel->connectItem( "GotoLine", this, SLOT( slotViewGotoLine()), true, ID_VIEW_GOTO_LINE );

  accel->insertItem( i18n("Next Error"), "NextError",IDK_VIEW_NEXT_ERROR);
  accel->connectItem( "NextError", this, SLOT( slotViewNextError()), true, ID_VIEW_NEXT_ERROR );

  accel->insertItem( i18n("Previous Error"), "PreviousError",IDK_VIEW_PREVIOUS_ERROR);
  accel->connectItem( "PreviousError", this, SLOT( slotViewPreviousError()), true, ID_VIEW_PREVIOUS_ERROR  );

  accel->insertItem( i18n("Dialog Editor"), "Dialog Editor", (unsigned int) 0);
  accel->connectItem("Dialog Editor", this, SLOT(startDesigner()), true, ID_TOOLS_DESIGNER );

  accel->insertItem( i18n("Toogle Tree-View"), "Tree-View",IDK_VIEW_TREEVIEW);
  accel->connectItem( "Tree-View", this, SLOT(slotViewTTreeView()), true, ID_VIEW_TREEVIEW );

  accel->insertItem( i18n("Toogle Output-View"), "Output-View",IDK_VIEW_OUTPUTVIEW);
  accel->connectItem( "Output-View", this, SLOT(slotViewTOutputView()), true, ID_VIEW_OUTPUTVIEW );

  accel->insertItem( i18n("Toolbar"), "Toolbar", (unsigned int) 0);
  accel->connectItem( "Toolbar", this, SLOT(slotViewTStdToolbar()), true, ID_VIEW_TOOLBAR );

  accel->insertItem( i18n("Browser-Toolbar"), "Browser-Toolbar", (unsigned int) 0);
  accel->connectItem( "Browser-Toolbar", this, SLOT(slotViewTBrowserToolbar()), true, ID_VIEW_BROWSER_TOOLBAR );
	
  accel->insertItem( i18n("Statusbar"), "Statusbar", (unsigned int) 0);
  accel->connectItem( "Statusbar", this, SLOT(slotViewTStatusbar()), true, ID_VIEW_STATUSBAR );


  accel->insertItem( i18n("Preview dialog"), "Preview dialog",IDK_VIEW_PREVIEW);

  accel->insertItem( i18n("Refresh"), "Refresh", (unsigned int) 0);
  accel->connectItem( "Refresh", this, SLOT(slotViewRefresh()), true, ID_VIEW_REFRESH);

  accel->insertItem( i18n("Goto Declaration"), "CVGotoDeclaration", (unsigned int) 0);
  accel->connectItem( "CVGotoDeclaration", this,SLOT(slotClassbrowserViewDeclaration()),true, ID_CV_VIEW_DECLARATION); // project menu

  accel->insertItem( i18n("Goto Definition"), "CVGotoDefinition", (unsigned int) 0);
  accel->connectItem( "CVGotoDefinition", this, SLOT(slotClassbrowserViewDefinition()), true,ID_CV_VIEW_DEFINITION );

  accel->insertItem( i18n("Class Declaration"), "CVGotoClass", (unsigned int) 0);
  accel->connectItem( "CVGotoClass", this,SLOT(slotClassbrowserViewClass()),true, ID_CV_VIEW_CLASS_DECLARATION);

  accel->insertItem( i18n("Graphical Classview"), "CVViewTree", (unsigned int) 0);
  accel->connectItem( "CVViewTree", this, SLOT(slotClassbrowserViewTree()), true, ID_CV_GRAPHICAL_VIEW );


  // projectmenu
  accel->insertItem( i18n("New Project"), "NewProject",(unsigned int) 0);
  accel->connectItem( "NewProject", this, SLOT(slotProjectNewAppl()), true, ID_PROJECT_KAPPWIZARD );

  accel->insertItem( i18n("Open Project"), "OpenProject", (unsigned int) 0);
  accel->connectItem( "OpenProject", this, SLOT(slotProjectOpen()), true, ID_PROJECT_OPEN );

  accel->insertItem( i18n("Close Project"), "CloseProject", (unsigned int) 0);
  accel->connectItem("CloseProject", this, SLOT(slotProjectClose()), true, ID_PROJECT_CLOSE );

  accel->insertItem(i18n("New Class"), "NewClass", (unsigned int) 0);
  accel->connectItem("NewClass", this, SLOT(slotProjectNewClass()), true, ID_PROJECT_NEW_CLASS );

  accel->insertItem(i18n("Add existing File(s)"), "AddExistingFiles", (unsigned int) 0);
  accel->connectItem("AddExistingFiles",this, SLOT(slotProjectAddExistingFiles()), true, ID_PROJECT_ADD_FILE_EXIST );

  accel->insertItem(i18n("Add new Translation File"),"Add new Translation File", (unsigned int) 0);
  accel->connectItem("Add new Translation File", this, SLOT(slotProjectAddNewTranslationFile()), true, ID_PROJECT_ADD_NEW_TRANSLATION_FILE );

  accel->insertItem(i18n("File Properties"), "FileProperties", IDK_PROJECT_FILE_PROPERTIES);
  accel->connectItem("FileProperties", this, SLOT(slotProjectFileProperties() ), true, ID_PROJECT_FILE_PROPERTIES );

  accel->insertItem(i18n("Make messages and merge"), "MakeMessages", (unsigned int) 0);
  accel->connectItem("MakeMessages", this, SLOT(slotProjectMessages()), true, ID_PROJECT_MESSAGES  );

  accel->insertItem(i18n("Make API-Doc"), "ProjectAPI", (unsigned int) 0);
  accel->connectItem("ProjectAPI", this, SLOT(slotProjectAPI()), true, ID_PROJECT_MAKE_PROJECT_API );

  accel->insertItem(i18n("Make User-Manual..."), "ProjectManual", (unsigned int) 0);
  accel->connectItem("ProjectManual", this, SLOT(slotProjectManual()), true, ID_PROJECT_MAKE_USER_MANUAL);

  accel->insertItem(i18n("Make Source-tgz"), "Source-tgz", (unsigned int) 0);
  accel->connectItem("Source-tgz", this, SLOT(slotProjectMakeDistSourceTgz()), true, ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ );
 	
  accel->insertItem(i18n("Project options"), "ProjectOptions", IDK_PROJECT_OPTIONS);
  accel->connectItem("ProjectOptions", this, SLOT(slotProjectOptions() ), true, ID_PROJECT_OPTIONS );


  //build menu
  accel->insertItem( i18n("Compile File"), "CompileFile", IDK_BUILD_COMPILE_FILE );
  accel->connectItem( "CompileFile", this, SLOT( slotBuildCompileFile()), true, ID_BUILD_COMPILE_FILE );

  accel->insertItem( i18n("Make"), "Make", IDK_BUILD_MAKE );
  accel->connectItem( "Make", this, SLOT(slotBuildMake() ), true, ID_BUILD_MAKE );

  accel->insertItem( i18n("Rebuild All"), "RebuildAll", (unsigned int) 0);
  accel->connectItem( "RebuildAll", this, SLOT(slotBuildRebuildAll()), true, ID_BUILD_REBUILD_ALL );

  accel->insertItem( i18n("Clean/Rebuild all"), "CleanRebuildAll", (unsigned int) 0);
  accel->connectItem( "CleanRebuildAll", this, SLOT(slotBuildCleanRebuildAll()), true, ID_BUILD_CLEAN_REBUILD_ALL );

  accel->insertItem( i18n("Stop process"), "Stop_proc", IDK_BUILD_STOP);
  accel->connectItem( "Stop_proc", this, SLOT(slotBuildStop() ), true, ID_BUILD_STOP );

  accel->insertItem( i18n("Execute"), "Run", IDK_BUILD_RUN);
  accel->connectItem( "Run", this, SLOT(slotBuildRun() ), true, ID_BUILD_RUN );

  accel->insertItem( i18n("Execute with arguments"), "Run_with_args", IDK_BUILD_RUN_WITH_ARGS);
  accel->connectItem( "Run_with_args", this, SLOT(slotBuildRunWithArgs() ), true, ID_BUILD_RUN_WITH_ARGS );

  accel->insertItem( i18n("DistClean"), "BuildDistClean", (unsigned int) 0);
  accel->connectItem("BuildDistClean",this, SLOT(slotBuildDistClean()), true, ID_BUILD_DISTCLEAN );

  accel->insertItem( i18n("Make Clean"), "BuildMakeClean", (unsigned int) 0);
  accel->connectItem("BuildMakeClean",this, SLOT(slotBuildMakeClean()), true, ID_BUILD_MAKECLEAN );

  accel->insertItem( i18n("Autoconf and automake"), "BuildAutoconf", (unsigned int) 0);
  accel->connectItem("BuildAutoconf", this,SLOT(slotBuildAutoconf()), true, ID_BUILD_AUTOCONF );

  accel->insertItem( i18n("Configure..."), "BuildConfigure", (unsigned int) 0);
  accel->connectItem( "BuildConfigure", this, SLOT(slotBuildConfigure()), true, ID_BUILD_CONFIGURE );

  // Bookmarks-menu
  accel->insertItem( i18n("Toggle Bookmark"), "Toggle_Bookmarks", IDK_BOOKMARKS_TOGGLE);
  accel->connectItem( "Toggle_Bookmarks", this, SLOT(slotBookmarksToggle() ), true, ID_BOOKMARKS_TOGGLE );

  accel->insertItem( i18n("Next Bookmark"), "Next_Bookmarks", IDK_BOOKMARKS_NEXT);
  accel->connectItem( "Next_Bookmarks", this, SLOT(slotBookmarksNext() ), true, ID_BOOKMARKS_NEXT );

  accel->insertItem( i18n("Previous Bookmark"), "Previous_Bookmarks", IDK_BOOKMARKS_PREVIOUS);
  accel->connectItem( "Previous_Bookmarks", this, SLOT(slotBookmarksPrevious() ), true, ID_BOOKMARKS_PREVIOUS );

  accel->insertItem( i18n("Clear Bookmarks"), "Clear_Bookmarks", IDK_BOOKMARKS_CLEAR);
  accel->connectItem( "Clear_Bookmarks", this, SLOT(slotBookmarksClear() ), true, ID_BOOKMARKS_CLEAR );

  //Help menu
  accel->connectItem( KStdAccel::Help , this, SLOT(slotHelpContents()), true, ID_HELP_CONTENTS );

  accel->insertItem( i18n("Search Marked Text"), "SearchMarkedText",IDK_HELP_SEARCH_TEXT);
  accel->connectItem( "SearchMarkedText", this, SLOT(slotHelpSearchText() ), true, ID_HELP_SEARCH_TEXT );

  accel->insertItem( i18n("Search for Help on"), "HelpSearch", (unsigned int) 0);
  accel->connectItem( "HelpSearch", this, SLOT(slotHelpSearch()), true, ID_HELP_SEARCH );

  accel->insertItem( i18n("View Project API-Doc"), "HelpProjectAPI", (unsigned int) 0);
  accel->connectItem("HelpProjectAPI", this, SLOT(slotHelpAPI()), true, ID_HELP_PROJECT_API);

  accel->insertItem( i18n("View Project User-Manual"), "HelpProjectManual", (unsigned int) 0);
  accel->connectItem( "HelpProjectManual", this, SLOT(slotHelpManual()), true, ID_HELP_USER_MANUAL);   // Tab-Switch

  accel->insertItem( i18n("Toggle Last"), "ToggleLast",IDK_TOGGLE_LAST);
  accel->connectItem( "ToggleLast", this, SLOT(slotToggleLast()) );

  // Debugger startups
  accel->insertItem( i18n("Debug start"), "DebugStart", (unsigned int) 0);
  accel->connectItem( "DebugStart", this, SLOT(slotBuildDebugStart()), true, ID_DEBUG_START);

  accel->insertItem( i18n("Debug start other"), "DebugStartOther", (unsigned int) 0);
  accel->connectItem( "DebugStartOther", this, SLOT(slotDebugNamedFile()), true, ID_DEBUG_START_OTHER);

  accel->insertItem( i18n("Debug start with args"), "DebugRunWithArgs", (unsigned int) 0);
  accel->connectItem( "DebugRunWithArgs", this, SLOT(slotDebugRunWithArgs()), true, ID_DEBUG_SET_ARGS);

  accel->insertItem( i18n("Debug examine core"), "DebugExamineCore", (unsigned int) 0);
  accel->connectItem( "DebugExamineCore", this, SLOT(slotDebugExamineCore()), true, ID_DEBUG_CORE);

  accel->insertItem( i18n("Debug other executable"), "DebugOtherExec", (unsigned int) 0);
  accel->connectItem( "DebugOtherExec", this, SLOT(slotDebugNamedFile()), true, ID_DEBUG_NAMED_FILE);

  accel->insertItem( i18n("Debug attach"), "DebugAttach", (unsigned int) 0);
  accel->connectItem( "DebugAttach", this, SLOT(slotDebugAttach()), true, ID_DEBUG_ATTACH);

  // Debugger actions
  accel->insertItem( i18n("Debug run"), "DebugRun", (unsigned int) 0);
  accel->connectItem( "DebugRun", this, SLOT(slotDebugRun()), true, ID_DEBUG_RUN );

  accel->insertItem( i18n("Debug run to cursor"), "DebugRunCursor", (unsigned int) 0);
  accel->connectItem( "DebugRunCursor", this, SLOT(slotDebugRun()), true, ID_DEBUG_RUN_CURSOR );

  accel->insertItem( i18n("Debug stop"), "DebugStop", (unsigned int) 0);
  accel->connectItem( "DebugStop", this, SLOT(slotDebugStop()), true, ID_DEBUG_STOP);

  accel->insertItem( i18n("Debug step into"), "DebugStepInto", (unsigned int) 0);
  accel->connectItem( "DebugStepInto", this, SLOT(slotDebugStepInto()), true, ID_DEBUG_STEP);

  accel->insertItem( i18n("Debug step into instr"), "DebugStepIntoInstr", (unsigned int) 0);
  accel->connectItem( "DebugStepIntoInstr", this, SLOT(slotDebugStepIntoIns()), true, ID_DEBUG_STEP_INST);

  accel->insertItem( i18n("Debug step over"), "DebugStepOver", (unsigned int) 0);
  accel->connectItem( "DebugStepOver", this, SLOT(slotDebugStepOver()), true, ID_DEBUG_NEXT);

  accel->insertItem( i18n("Debug step over instr"), "DebugStepOverInstr", (unsigned int) 0);
  accel->connectItem( "DebugStepOverInstr", this, SLOT(slotDebugStepOverIns()), true, ID_DEBUG_NEXT_INST);

  accel->insertItem( i18n("Debug step out"), "DebugStepOut", (unsigned int) 0);
  accel->connectItem( "DebugStepOut", this, SLOT(slotDebugStepOutOff()), true, ID_DEBUG_FINISH);

  accel->insertItem( i18n("Debug viewers"), "DebugViewer", (unsigned int) 0);
  accel->connectItem( "DebugViewer", this, SLOT(slotDebugMemoryView()), true, ID_DEBUG_MEMVIEW);

  accel->insertItem( i18n("Debug interrupt"), "DebugInterrupt", (unsigned int) 0);
  accel->connectItem( "DebugInterrupt", this, SLOT(slotDebugInterrupt()), true, ID_DEBUG_BREAK_INTO);

  accel->insertItem( i18n("Debug toggle breakpoint"), "DebugToggleBreakpoint", (unsigned int) 0);
  accel->connectItem( "DebugToggleBreakpoint", this, SLOT(slotDebugToggleBreakpoint()), true, ID_DEBUG_TOGGLE_BP);

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


///////////////////////////////////////////////////////////////////
// File-menu entries
  file_menu = new QPopupMenu;
  file_menu->insertItem(SmallIconSet("filenew"),i18n("&New..."),this,SLOT(slotFileNew()),0,ID_FILE_NEW);
  file_menu->insertItem(SmallIconSet("fileopen"),i18n("&Open..."), this, SLOT(slotFileOpen()),0 ,ID_FILE_OPEN);
  file_menu->insertItem(SmallIconSet("fileclose"),i18n("&Close"), this, SLOT(slotFileClose()),0,ID_FILE_CLOSE);
  file_menu->insertItem(i18n("Close All"), this, SLOT(slotFileCloseAll()), 0, ID_FILE_CLOSE_ALL);
  file_menu->insertSeparator();
  file_menu->insertItem(SmallIconSet("filesave"),i18n("&Save"), this, SLOT(slotFileSave()),0 ,ID_FILE_SAVE);
  file_menu->insertItem(i18n("Save &As..."), this, SLOT(slotFileSaveAs()),0 ,ID_FILE_SAVE_AS);
  file_menu->insertItem(SmallIconSet("save_all"),i18n("Save A&ll"), this, SLOT(slotFileSaveAll()),0,ID_FILE_SAVE_ALL);
  file_menu->insertSeparator();
  file_menu->insertItem(SmallIconSet("fileprint"),i18n("&Print..."), this, SLOT(slotFilePrint()),0 ,ID_FILE_PRINT);
  file_menu->insertSeparator();
  file_menu->insertItem(SmallIconSet("exit"),i18n("E&xit"),this, SLOT(slotFileQuit()),0 ,ID_FILE_QUIT);

  menuBar()->insertItem(i18n("&File"), file_menu);


///////////////////////////////////////////////////////////////////
// Edit-menu entries

  edit_menu = new QPopupMenu;
  edit_menu->insertItem(SmallIconSet("undo"), i18n("U&ndo"), this, SLOT(slotEditUndo()),0 ,ID_EDIT_UNDO);
  edit_menu->insertItem(SmallIconSet("redo"), i18n("R&edo"), this, SLOT(slotEditRedo()),0 ,ID_EDIT_REDO);
  edit_menu->insertSeparator();
  edit_menu->insertItem(SmallIconSet("editcut"),i18n("C&ut"), this, SLOT(slotEditCut()),0 ,ID_EDIT_CUT);
  edit_menu->insertItem(SmallIconSet("editcopy"),i18n("&Copy"), this, SLOT(slotEditCopy()),0 ,ID_EDIT_COPY);
  edit_menu->insertItem(SmallIconSet("editpaste"),i18n("&Paste"), this, SLOT(slotEditPaste()),0 , ID_EDIT_PASTE);
  edit_menu->insertSeparator();
	edit_menu->insertItem(SmallIconSet("increaseindent"),i18n("In&dent"), this,SLOT(slotEditIndent()),0,ID_EDIT_INDENT);
	edit_menu->insertItem(SmallIconSet("decreaseindent"),i18n("Uninden&t"), this, SLOT(slotEditUnindent()),0,ID_EDIT_UNINDENT);
  edit_menu->insertSeparator();
	edit_menu->insertItem(i18n("C&omment"), this,SLOT(slotEditComment()),0,ID_EDIT_COMMENT);
	edit_menu->insertItem(i18n("Unco&mment"), this, SLOT(slotEditUncomment()),0,ID_EDIT_UNCOMMENT);

  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("&Insert File..."),this, SLOT(slotEditInsertFile()),0,ID_EDIT_INSERT_FILE);

  edit_menu->insertSeparator();
  edit_menu->insertItem(SmallIconSet("find"),i18n("&Search..."), this, SLOT(slotEditSearch()),0,ID_EDIT_SEARCH);
  edit_menu->insertItem(SmallIconSet("next"),i18n("Repeat Searc&h"), this, SLOT(slotEditRepeatSearch()),0,ID_EDIT_REPEAT_SEARCH);

  edit_menu->insertItem(i18n("&Replace..."), this, SLOT(slotEditReplace()),0,ID_EDIT_REPLACE);
  edit_menu->insertItem(SmallIconSet("grep"),i18n("Search in &Files..."), this, SLOT(slotEditSearchInFiles()),0,ID_EDIT_SEARCH_IN_FILES);
//  edit_menu->insertItem(i18n("Spell&check..."),this, SLOT(slotEditSpellcheck()),0,ID_EDIT_SPELLCHECK);

  edit_menu->insertSeparator();
  edit_menu->insertItem(i18n("Select &All"), this, SLOT(slotEditSelectAll()),0,ID_EDIT_SELECT_ALL);
  edit_menu->insertItem(i18n("Deselect All"), this, SLOT(slotEditDeselectAll()),0,ID_EDIT_DESELECT_ALL);
  edit_menu->insertItem(i18n("Invert Selection"), this, SLOT(slotEditInvertSelection()),0,ID_EDIT_INVERT_SELECTION);

  menuBar()->insertItem(i18n("&Edit"), edit_menu);

  ///////////////////////////////////////////////////////////////////
  // View-menu entries
  // set the mode of the tab headers
  view_tab_menu = new QPopupMenu;
  view_tab_menu->insertItem(i18n("Text only"), this, SLOT(slotViewTabText()), 0, ID_VIEW_TAB_TEXT);
  view_tab_menu->insertItem(i18n("Icons only"), this, SLOT(slotViewTabIcons()), 0, ID_VIEW_TAB_ICONS);
  view_tab_menu->insertItem(i18n("Text and Icons"), this, SLOT(slotViewTabTextIcons()),0, ID_VIEW_TAB_TEXT_ICONS);
  config->setGroup("General Options");
  int tabtext=config->readNumEntry("tabviewmode",3);
  switch(tabtext){
    case 1:
      view_tab_menu->setItemChecked(ID_VIEW_TAB_TEXT,true);
      break;
    case 2:
      view_tab_menu->setItemChecked(ID_VIEW_TAB_ICONS,true);
      break;
    case 3:
      view_tab_menu->setItemChecked(ID_VIEW_TAB_TEXT_ICONS,true);
      break;
    default:
      view_tab_menu->setItemChecked(ID_VIEW_TAB_TEXT_ICONS,true);
      break;
  }

  view_menu = new QPopupMenu;
  view_menu->insertItem(SmallIconSet("goto"),i18n("Goto &Line..."), this,
			SLOT(slotViewGotoLine()),0,ID_VIEW_GOTO_LINE);
  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Next Error"),this,
			SLOT(slotViewNextError()),0,ID_VIEW_NEXT_ERROR);
  view_menu->insertItem(i18n("&Previous Error"),this,
			SLOT(slotViewPreviousError()),0,ID_VIEW_PREVIOUS_ERROR);
  view_menu->insertSeparator();
  view_menu->insertItem(SmallIconSet("newwidget"),i18n("&Dialog Editor"),this,SLOT(startDesigner()),0,ID_TOOLS_DESIGNER);
  view_menu->insertSeparator();
  view_menu->insertItem(i18n("&Tree-View"),this,
			SLOT(slotViewTTreeView()),0,ID_VIEW_TREEVIEW);
  view_menu->insertItem(i18n("&Output-View"),this,
			SLOT(slotViewTOutputView()),0,ID_VIEW_OUTPUTVIEW);
  view_menu->insertSeparator();
  view_menu->insertItem(i18n("Toolb&ar"),this,
			   SLOT(slotViewTStdToolbar()),0,ID_VIEW_TOOLBAR);
  view_menu->insertItem(i18n("&Browser-Toolbar"),this,
			   SLOT(slotViewTBrowserToolbar()),0,ID_VIEW_BROWSER_TOOLBAR);
  view_menu->insertItem(i18n("&Statusbar"),this,
			   SLOT(slotViewTStatusbar()),0,ID_VIEW_STATUSBAR);
  view_menu->insertItem(i18n("Tab-Te&xt"), view_tab_menu,ID_VIEW_TABS);
  view_menu->insertSeparator();
  view_menu->insertItem(SmallIconSet("reload"),i18n("&Refresh"),this,
			   SLOT(slotViewRefresh()),0,ID_VIEW_REFRESH);

  menuBar()->insertItem(i18n("&View"), view_menu);


  ///////////////////////////////////////////////////////////////////
  // Project-menu entries
  project_menu = new QPopupMenu;
  project_menu->insertItem(SmallIconSet("window_new"),i18n("New..."), this, SLOT(slotProjectNewAppl()),0,ID_PROJECT_KAPPWIZARD);
  project_menu->insertItem(SmallIconSet("wizard"),i18n("Generate Project File..."), this, SLOT(slotProjectGenerate()),0,ID_PROJECT_GENERATE);

  project_menu->insertItem(SmallIconSet("project_open"),i18n("&Open..."), this, SLOT(slotProjectOpen()),0,ID_PROJECT_OPEN);

  recent_projects_menu = new QPopupMenu();
  connect( recent_projects_menu, SIGNAL(activated(int)), SLOT(slotProjectOpenRecent(int)) );
  project_menu->insertItem(i18n("Open &recent project..."), recent_projects_menu, ID_PROJECT_OPEN_RECENT );

  project_menu->insertItem(SmallIconSet("fileclose"),i18n("C&lose"),this, SLOT(slotProjectClose()),0,ID_PROJECT_CLOSE);

  project_menu->insertSeparator();
  project_menu->insertItem(SmallIconSet("classnew"),i18n("&New Class..."), this,
			   SLOT(slotProjectNewClass()),0,ID_PROJECT_NEW_CLASS);
  project_menu->insertItem(i18n("&Add existing File(s)..."),this,SLOT(slotProjectAddExistingFiles()),0,ID_PROJECT_ADD_FILE_EXIST);

  project_menu->insertItem(SmallIconSet("locale"),i18n("Add new &Translation File..."), this,
			   SLOT(slotProjectAddNewTranslationFile()),0,ID_PROJECT_ADD_NEW_TRANSLATION_FILE);

  //  project_menu->insertItem(i18n("&Remove File from Project"), this,
  //			   SLOT(slotProjectRemoveFile()),0,ID_PROJECT_REMOVE_FILE);
 		
  project_menu->insertItem(SmallIconSet("file_properties"),i18n("&File Properties..."), this, SLOT(slotProjectFileProperties())
			   ,0,ID_PROJECT_FILE_PROPERTIES);
			
  project_menu->insertSeparator();

  project_menu->insertItem(i18n("Make &messages and merge"), this, SLOT(slotProjectMessages()),0, ID_PROJECT_MESSAGES);
  //MB
  doctool_menu = new QPopupMenu();
  doctool_menu->insertItem(i18n("kdoc"), this, SLOT(slotSwitchDocTool()),0,ID_PROJECT_DOC_TOOL_KDOC);
  doctool_menu->insertItem(i18n("doxygen"), this, SLOT(slotSwitchDocTool()),0,ID_PROJECT_DOC_TOOL_DOXYGEN);
  doctool_menu->insertSeparator();
  doctool_menu->insertItem(SmallIconSet("configure"),i18n("Configure doxygen"), this, SLOT(slotConfigureDoxygen()),0,ID_PROJECT_DOC_TOOL_CONF_DOXYGEN);
  project_menu->insertItem(i18n("API Doc Tool..."), doctool_menu, ID_PROJECT_DOC_TOOL );
  //MB end
  project_menu->insertItem(i18n("Make AP&I-Doc"), this,
			 SLOT(slotProjectAPI()),0,ID_PROJECT_MAKE_PROJECT_API);
  project_menu->insertItem(SmallIconSet("contents2"), i18n("Make &User-Manual..."), this,
			 SLOT(slotProjectManual()),0,ID_PROJECT_MAKE_USER_MANUAL);
  // submenu for making dists

  QPopupMenu*  p2 = new QPopupMenu;
  p2->insertItem(SmallIconSet("tgz"),i18n("&Source-tgz"), this, SLOT(slotProjectMakeDistSourceTgz()),0,ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ);
  project_menu->insertItem(i18n("Make &Distribution"),p2,ID_PROJECT_MAKE_DISTRIBUTION);
  project_menu->insertSeparator();

  project_menu->insertItem(SmallIconSet("configure"),i18n("O&ptions..."), this, SLOT(slotProjectOptions()),0,ID_PROJECT_OPTIONS);

  menuBar()->insertItem(i18n("&Project"), project_menu);

  ///////////////////////////////////////////////////////////////////
  // Build-menu entries
  build_menu = new QPopupMenu;
  build_menu->insertItem(SmallIconSet("compfile"),i18n("Compile &File"),
			 this,SLOT(slotBuildCompileFile()),0,ID_BUILD_COMPILE_FILE);
  build_menu->insertItem(SmallIconSet("make_kdevelop"),i18n("&Make"),this,
			 SLOT(slotBuildMake()),0,ID_BUILD_MAKE);

//   build_menu->insertItem(SmallIconSet("make_kdevelop"),i18n("Make &with"),this,
// 			 SLOT(slotBuildMakeWith()),0,ID_BUILD_MAKE_WITH);
//   accel->changeMenuAccel(build_menu,ID_BUILD_MAKE_WITH ,"MakeWith" );

  build_menu->insertItem(i18n("Clea&n"),this,SLOT(slotBuildMakeClean()),0,ID_BUILD_MAKECLEAN);
  build_menu->insertItem(SmallIconSet("rebuild"),i18n("&Rebuild"), this,
			 SLOT(slotBuildRebuildAll()),0,ID_BUILD_REBUILD_ALL);

  build_menu->insertItem(i18n("Dist&Clean/Rebuild All"), this,
			 SLOT(slotBuildCleanRebuildAll()),0,ID_BUILD_CLEAN_REBUILD_ALL);
  build_menu->insertSeparator();
  build_menu->insertItem(SmallIconSet("stop"),i18n("&Stop Build"), this, SLOT(slotBuildStop()),0,ID_BUILD_STOP);
  build_menu->insertSeparator();

  build_menu->insertItem(SmallIconSet("exec"),i18n("&Execute"),this,SLOT(slotBuildRun()),0,ID_BUILD_RUN);
  build_menu->insertItem(SmallIconSet("exec"),i18n("Execute &with Arguments..."),this,SLOT(slotBuildRunWithArgs()),0,ID_BUILD_RUN_WITH_ARGS);

  build_menu->insertSeparator();
  build_menu->insertItem(i18n("DistC&lean"),this,SLOT(slotBuildDistClean()),0,ID_BUILD_DISTCLEAN);
  build_menu->insertItem(i18n("&Autoconf and automake"),this,SLOT(slotBuildAutoconf()),0,ID_BUILD_AUTOCONF);
  build_menu->insertItem(i18n("C&onfigure..."), this, SLOT(slotBuildConfigure()),0,ID_BUILD_CONFIGURE);

  menuBar()->insertItem(i18n("&Build"), build_menu);

  ///////////////////////////////////////////////////////////////////
  // Debug-menu entries

  debugPopup = new QPopupMenu();
  debugPopup->insertItem(SmallIconSet("core"),i18n("Examine core file"),this,SLOT(slotDebugExamineCore()),0,ID_DEBUG_CORE);
  debugPopup->insertItem(SmallIconSet("exec"),i18n("Debug another executable"),this,SLOT(slotDebugNamedFile()),0,ID_DEBUG_NAMED_FILE);
  debugPopup->insertItem(SmallIconSet("connect_creating"),i18n("Attach to process"),this,SLOT(slotDebugAttach()),0,ID_DEBUG_ATTACH);
  debugPopup->insertItem(SmallIconSet("exec"),i18n("Debug with arguments"),this,SLOT(slotDebugRunWithArgs()),0,ID_DEBUG_SET_ARGS);
  connect(debugPopup,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

  debug_menu = new QPopupMenu;
  debug_menu->insertItem(SmallIconSet("1rightarrow"),    i18n("&Start"),           ID_DEBUG_START);
  //this, SLOT(slotBuildDebug()),0,ID_DEBUG_NORMAL);

  debug_menu->insertItem(SmallIconSet("1rightarrow"),    i18n("Start (other)..."), debugPopup, ID_DEBUG_START_OTHER);
  debug_menu->insertSeparator();

  debug_menu->insertItem(SmallIconSet("dbgrun"),      i18n("Run"),              ID_DEBUG_RUN);
  debug_menu->insertItem(SmallIconSet("dbgrunto"),    i18n("Run to cursor"),    ID_DEBUG_RUN_CURSOR);
  debug_menu->insertItem(SmallIconSet("dbgnext"),     i18n("Step over"),        ID_DEBUG_NEXT);
  debug_menu->insertItem(SmallIconSet("dbgnextinst"), i18n("Step over instr."), ID_DEBUG_NEXT_INST);
  debug_menu->insertItem(SmallIconSet("dbgstep"),     i18n("Step into"),        ID_DEBUG_STEP);
  debug_menu->insertItem(SmallIconSet("dbgstepinst"), i18n("Step into instr."), ID_DEBUG_STEP_INST);
  debug_menu->insertItem(SmallIconSet("dbgstepout"),  i18n("Step out"),         ID_DEBUG_FINISH);
  debug_menu->insertSeparator();

  debug_menu->insertItem(SmallIconSet("dbgmemview"),  i18n("Viewers"),          this, SLOT(slotDebugMemoryView()), 0, ID_DEBUG_MEMVIEW);
  debug_menu->insertSeparator();

  debug_menu->insertItem(SmallIconSet("player_pause"),    i18n("Interrupt"),        ID_DEBUG_BREAK_INTO);
  debug_menu->insertItem(SmallIconSet("stop"),   i18n("Stop"),             ID_DEBUG_STOP);

  menuBar()->insertItem(i18n("Debu&g"), debug_menu);
  connect(debug_menu,SIGNAL(activated(int)), SLOT(slotDebugActivator(int)));

  ///////////////////////////////////////////////////////////////////
  // Tools-menu entries
  tools_menu = new QPopupMenu;
  menuBar()->insertItem(i18n("&Tools"), tools_menu);

  ///////////////////////////////////////////////////////////////////
  // Options-menu entries
  // submenu for setting printprograms
  QPopupMenu* p3 = new QPopupMenu;
  p3->insertItem(i18n("&Enscript..."), this,
		  SLOT(slotOptionsConfigureEnscript()),0,ID_OPTIONS_PRINT_ENSCRIPT);

  options_menu = new QPopupMenu;
  options_menu->insertItem(SmallIconSet("edit"),i18n("&Editor..."),this,
			   SLOT(slotOptionsEditor()),0,ID_OPTIONS_EDITOR);
  options_menu->insertItem(i18n("Editor &Colors..."),this,
			   SLOT(slotOptionsEditorColors()),0,ID_OPTIONS_EDITOR_COLORS);
  options_menu->insertItem(i18n("Editor &Defaults..."),this,
			   SLOT(slotOptionsSyntaxHighlightingDefaults())
			   ,0,ID_OPTIONS_SYNTAX_HIGHLIGHTING_DEFAULTS);
  options_menu->insertItem(i18n("&Syntax Highlighting..."),this,
			   SLOT(slotOptionsSyntaxHighlighting()),0,ID_OPTIONS_SYNTAX_HIGHLIGHTING);
  options_menu->insertSeparator();
  options_menu->insertItem(SmallIconSet("www"),i18n("Documentation &Browser..."),this,
			   SLOT(slotOptionsDocBrowser()),0,ID_OPTIONS_DOCBROWSER);

  options_menu->insertItem(SmallIconSet("fileprint"),i18n("Configure &Printer..."),p3,ID_OPTIONS_PRINT);
	options_menu->insertItem(SmallIconSet("run"),i18n("Tools..."),this,SLOT(slotOptionsToolsConfigDlg()),0,ID_OPTIONS_TOOLS_CONFIG_DLG);
//  options_menu->insertItem(i18n("&Spellchecker..."),this,SLOT(slotOptionsSpellchecker()),0,ID_OPTIONS_SPELLCHECKER);
  options_menu->insertSeparator();
  options_menu->insertItem(SmallIconSet("configure"),i18n("&KDevelop Setup..."),this,
			   SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);

  menuBar()->insertItem(i18n("&Options"), options_menu);

  ///////////////////////////////////////////////////////////////////
  // Window-menu entries
  menu_buffers = new QPopupMenu;
  menuBar()->insertItem(i18n("&Window"), menu_buffers);
  menuBar()->insertSeparator();

  ///////////////////////////////////////////////////////////////////
  // Bookmarks-menu entries
  bookmarks_menu=new QPopupMenu;
  bookmarks_menu->insertItem(SmallIconSet("bookmark_add"),i18n("&Toggle Bookmark"),this,SLOT(slotBookmarksToggle()),0,ID_BOOKMARKS_TOGGLE);
  bookmarks_menu->insertItem(i18n("&Next Bookmark"),this,SLOT(slotBookmarksNext()),0,ID_BOOKMARKS_NEXT);
  bookmarks_menu->insertItem(i18n("&Previous Bookmark"),this,SLOT(slotBookmarksPrevious()),0,ID_BOOKMARKS_PREVIOUS);
  bookmarks_menu->insertItem(i18n("&Clear Bookmarks"),this,SLOT(slotBookmarksClear()),0,ID_BOOKMARKS_CLEAR);
  bookmarks_menu->insertSeparator();

  QPopupMenu* header_bookmarks = new QPopupMenu();
  header_widget->installBMPopup(header_bookmarks);
  QPopupMenu* cpp_bookmarks = new QPopupMenu();
  cpp_widget->installBMPopup(cpp_bookmarks);
	
  doc_bookmarks = new QPopupMenu();

  bookmarks_menu->insertItem(SmallIconSet("bookmark_folder"),i18n("&Header Window"),header_bookmarks,31000);
  bookmarks_menu->insertItem(SmallIconSet("bookmark_folder"),i18n("C/C++ &Window"),cpp_bookmarks,31010);
  bookmarks_menu->insertItem(SmallIconSet("bookmark_folder"),i18n("&Browser Window"), doc_bookmarks,31020);
	
  menuBar()->insertItem(i18n("Book&marks"),bookmarks_menu);

  ///////////////////////////////////////////////////////////////////
  // Help-menu entries
  QString manual=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("index.html"));
  QString programming=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("programming/index.html"));
  QString tutorial=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("tutorial/index.html"));
  QString kdelibref=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("kde_libref/index.html"));
  QString addendum=DocTreeKDevelopBook::readIndexTitle(DocTreeKDevelopBook::locatehtml("addendum/index.html"));

  const KAboutData *aboutData = KGlobal::instance()->aboutData();
  help_menu = new KHelpMenu( this, aboutData);
  KPopupMenu *help = help_menu->menu();
  help->clear(); //clear that damn stuff in there
  help->insertItem(SmallIconSet("back"),i18n("&Back"),this, SLOT(slotHelpBack()),0,ID_HELP_BACK);
  help->insertItem(SmallIconSet("forward"),i18n("&Forward"),this, SLOT(slotHelpForward()),0,ID_HELP_FORWARD);
  help->insertSeparator();
  help->insertItem(SmallIconSet("help"),i18n("&Search Marked Text"),this,
				 SLOT(slotHelpSearchText()),0,ID_HELP_SEARCH_TEXT);
  help->insertItem(SmallIconSet("filefind"),i18n("Search for Help on..."),this,SLOT(slotHelpSearch()),0,ID_HELP_SEARCH);
  help->insertSeparator();
  QToolButton* wtb = QWhatsThis::whatsThisButton(0);
  help->insertItem( wtb->iconSet(),i18n( "What's &This" ), help_menu, SLOT(contextHelpActivated()),SHIFT + Key_F1);
  delete wtb;
  help->insertSeparator();
  help->insertItem(SmallIconSet("contents"),manual,this,SLOT(slotHelpContents()),0 ,ID_HELP_CONTENTS);
  help->insertItem(SmallIconSet("contents"),programming,this,SLOT(slotHelpProgramming()),0 ,ID_HELP_PROGRAMMING);
  help->insertItem(SmallIconSet("contents"),tutorial,this,SLOT(slotHelpTutorial()),0 ,ID_HELP_TUTORIAL);
  help->insertItem(SmallIconSet("contents"),kdelibref,this,SLOT(slotHelpKDELibRef()),0 ,ID_HELP_KDELIBREF);
  help->insertItem(SmallIconSet("contents"),i18n("C/C++-Reference"),this,SLOT(slotHelpReference()),0,ID_HELP_REFERENCE);
  help->insertSeparator();	
  help->insertItem(SmallIconSet("contents"),i18n("Project &API-Doc"),this,
                        SLOT(slotHelpAPI()),0,ID_HELP_PROJECT_API);

  help->insertItem(SmallIconSet("contents"),i18n("Project &User-Manual"),this,
                        SLOT(slotHelpManual()),0,ID_HELP_USER_MANUAL);
  help->insertSeparator();	
	help->insertItem(SmallIconSet("idea"),i18n("Tip of the Day"), this, SLOT(slotHelpTipOfDay()), 0, ID_HELP_TIP_OF_DAY);
  help->insertItem(SmallIconSet("www"), i18n("KDevelop Homepage"),this, SLOT(slotHelpHomepage()),0,ID_HELP_HOMEPAGE);
  help->insertItem( i18n( "&Report Bug..." ),help_menu, SLOT(reportBug()),0,ID_HELP_BUG_REPORT);
  help->insertSeparator();	
  QString appName = (aboutData)? aboutData->programName() : QString::fromLatin1(kapp->name());
  help->insertItem( kapp->miniIcon(), i18n( "&About %1" ).arg(appName), help_menu, SLOT( aboutApplication() ));
  help->insertItem( SmallIcon("go"), i18n( "About &KDE" ),help_menu, SLOT( aboutKDE() ) );

  menuBar()->insertItem(i18n("&Help"), help_menu->menu());

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
  classbrowser_popup->insertItem( SmallIconSet("classnew"),i18n("New class..."), this,
                                  SLOT(slotProjectNewClass()), 0, ID_PROJECT_NEW_CLASS);
  classbrowser_popup->insertSeparator();
  classbrowser_popup->insertItem( SmallIconSet("methodnew"),i18n("Add member function..."), this,
                                  SLOT(slotClassbrowserNewMethod()), 0, ID_CV_METHOD_NEW);
  classbrowser_popup->insertItem( SmallIconSet("variablenew"),i18n("Add member variable..."), this,
                                  SLOT(slotClassbrowserNewAttribute()), 0, ID_CV_ATTRIBUTE_NEW );
  classbrowser_popup->insertItem( SmallIconSet("graphview"), i18n("Show graphical classview"), this,
  																SLOT(slotClassbrowserViewTree()),0, ID_CV_GRAPHICAL_VIEW);


///////////////////////////////////////////////////////////////////
// connects for the statusbar help
  connect(file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p3,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(view_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(p2,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(debug_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
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

  toolBar()->insertButton("filenew",ID_FILE_NEW, false,i18n("New"));
  toolBar()->insertSeparator();
  toolBar()->insertButton("project_open",ID_PROJECT_OPEN, true,i18n("Open Project"));
  toolBar()->insertSeparator();
  toolBar()->insertButton("fileopen",ID_FILE_OPEN, true,i18n("Open File"));
  file_open_popup= new QPopupMenu();
  connect(file_open_popup, SIGNAL(activated(int)), SLOT(slotFileOpen(int)));
  toolBar()->setDelayedPopup(ID_FILE_OPEN, file_open_popup);

  toolBar()->insertButton("filesave",ID_FILE_SAVE,true,i18n("Save File"));
//  toolBar()->insertButton("save_all"),ID_FILE_SAVE_ALL,true,i18n("Save All"));

  toolBar()->insertButton("fileprint",ID_FILE_PRINT,false,i18n("Print"));

//  QFrame *sepUndo= new QFrame(toolBar());
//  sepUndo->setFrameStyle(QFrame::VLine|QFrame::Sunken);
//  toolBar()->insertWidget(0,20,sepUndo);
  toolBar()->insertSeparator();

  toolBar()->insertButton("undo",ID_EDIT_UNDO,false,i18n("Undo"));
  toolBar()->insertButton("redo",ID_EDIT_REDO,false,i18n("Redo"));
  toolBar()->insertSeparator();
  toolBar()->insertButton("editcut",ID_EDIT_CUT,true,i18n("Cut"));
  toolBar()->insertButton("editcopy",ID_EDIT_COPY, true,i18n("Copy"));
  toolBar()->insertButton("editpaste",ID_EDIT_PASTE, true,i18n("Paste"));
	
  toolBar()->insertSeparator();

  toolBar()->insertButton("compfile",ID_BUILD_COMPILE_FILE, false,i18n("Compile file"));
  toolBar()->insertButton("make_kdevelop",ID_BUILD_MAKE, false,i18n("Make"));
  toolBar()->insertButton("rebuild",ID_BUILD_REBUILD_ALL, false,i18n("Rebuild"));
  toolBar()->insertSeparator();
  toolBar()->insertButton("debugger",ID_DEBUG_START, false, i18n("Debug"));
  QPopupMenu* debugToolPopup = new QPopupMenu();
  debugToolPopup->insertItem(SmallIconSet("core"),i18n("Examine core file"),this,SLOT(slotDebugExamineCore()),0,ID_DEBUG_CORE);
  debugToolPopup->insertItem(SmallIconSet("exec"),i18n("Debug another executable"),this,SLOT(slotDebugNamedFile()),0,ID_DEBUG_NAMED_FILE);
  debugToolPopup->insertItem(SmallIconSet("connect_creating"),i18n("Attach to process"),this,SLOT(slotDebugAttach()),0,ID_DEBUG_ATTACH);
  debugToolPopup->insertItem(SmallIconSet("exec"),i18n("Debug with arguments"),this,SLOT(slotDebugRunWithArgs()),0,ID_DEBUG_SET_ARGS);
  connect(debugToolPopup,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  toolBar()->setDelayedPopup(ID_DEBUG_START, debugToolPopup);

  toolBar()->insertButton("exec",ID_BUILD_RUN, false,i18n("Run"));
  toolBar()->insertSeparator();
  toolBar()->insertButton("stop",ID_BUILD_STOP, false,i18n("Stop"));

  toolBar()->insertSeparator();

  toolBar()->insertButton("newwidget",ID_TOOLS_DESIGNER, true,i18n("Switch to QT's designer (dialog editor)"));
  toolBar()->insertButton("tree_win",ID_VIEW_TREEVIEW, true,i18n("Tree-View"));
  toolBar()->insertButton("output_win",ID_VIEW_OUTPUTVIEW, true,i18n("Output-View"));
  toolBar()->setToggle(ID_VIEW_TREEVIEW);
  toolBar()->setToggle(ID_VIEW_OUTPUTVIEW);

  toolBar()->insertSeparator();

  toolBar()->insertButton("dbgrun",ID_DEBUG_RUN, false, i18n("Continue with app execution. May start the app"));
  toolBar()->insertButton("dbgnext",ID_DEBUG_NEXT, false,i18n("Execute one line of code, but run through functions"));
  toolBar()->insertButton("dbgstep",ID_DEBUG_STEP, false,i18n("Execute one line of code, stepping into fn if appropriate"));
  toolBar()->insertButton("dbgstepout",ID_DEBUG_FINISH, false,i18n("Execute to end of current stack frame"));

  QPopupMenu* stepOverMenu = new QPopupMenu();
  stepOverMenu->insertItem(SmallIconSet("dbgnextinst"),i18n("Step over instr."),this,SLOT(slotDebugStepOverIns()),0,ID_DEBUG_NEXT_INST);
  connect(stepOverMenu, SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  toolBar()->setDelayedPopup(ID_DEBUG_NEXT, stepOverMenu);

  QPopupMenu* stepIntoMenu = new QPopupMenu();
  stepIntoMenu->insertItem(SmallIconSet("dbgstepinst"),i18n("Step into instr."),this,SLOT(slotDebugStepIntoIns()),0,ID_DEBUG_STEP_INST);
  connect(stepIntoMenu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  toolBar()->setDelayedPopup(ID_DEBUG_STEP, stepIntoMenu);

  connect(toolBar(), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  connect(toolBar(), SIGNAL(pressed(int)), SLOT(statusCallback(int)));

  /////////////////////
  // the second toolbar
  /////////////////////

  // Class combo
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo(i18n("Classes"),
                                           ID_CV_TOOLBAR_CLASS_CHOICE,true,
                                           SIGNAL(activated(const QString&))
                                           ,this,
                                           SLOT(slotClassChoiceCombo(const QString&)),
                                           true,i18n("Classes"),160 );

  KComboBox* class_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_CLASS_CHOICE);
  class_combo->setFocusPolicy(QWidget::ClickFocus);
  class_combo->setAutoCompletion(true);
  class_combo->setInsertionPolicy(QComboBox::NoInsertion);
  class_combo->setEnabled(false);
  class_combo->useGlobalKeyBindings();
  class_combo->setCompletionMode ( KGlobalSettings::CompletionPopup );
  class_comp = class_combo->completionObject();
  class_combo->setAutoDeleteCompletionObject( true );
  connect(class_combo, SIGNAL(returnPressed(const QString&)), this,
                      SLOT(slotClassChoiceCombo(const QString&)));

  // Method combo
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo(i18n("Methods"),
                                           ID_CV_TOOLBAR_METHOD_CHOICE,true,
                                           SIGNAL(activated(const QString&))
                                           ,this,SLOT(slotMethodChoiceCombo(const QString&)),
                                           true,i18n("Methods"),240 );

  KComboBox* choice_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_METHOD_CHOICE);
  choice_combo->setFocusPolicy(QWidget::ClickFocus);
  choice_combo->setAutoCompletion(true);
  choice_combo->setInsertionPolicy(QComboBox::NoInsertion);
  choice_combo->setEnabled(false);
  choice_combo->useGlobalKeyBindings();
  choice_combo->setCompletionMode ( KGlobalSettings::CompletionPopup );
  method_comp = choice_combo->completionObject();
  choice_combo->setAutoDeleteCompletionObject( true );
  connect(choice_combo, SIGNAL(returnPressed(const QString&)), this,
                      SLOT(slotMethodChoiceCombo(const QString&)));

  // Classbrowserwizard click button
  toolBar(ID_BROWSER_TOOLBAR)->insertButton("classwiz",
                                            ID_CV_WIZARD, true,
                                            i18n("Declaration/Definition"));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_CV_WIZARD,
                                               classbrowser_popup);
  disableCommand(ID_CV_WIZARD);
  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

  toolBar(ID_BROWSER_TOOLBAR)->insertButton("back",ID_HELP_BACK, false,i18n("Back"));
  history_prev = new QPopupMenu();
  connect(history_prev, SIGNAL(activated(int)), SLOT(slotHelpHistoryBack(int)));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_HELP_BACK, history_prev);
	
  toolBar(ID_BROWSER_TOOLBAR)->insertButton("forward",ID_HELP_FORWARD, false,i18n("Forward"));
  history_next = new QPopupMenu();
  connect(history_next, SIGNAL(activated(int)), SLOT(slotHelpHistoryForward(int)));
  toolBar(ID_BROWSER_TOOLBAR)->setDelayedPopup(ID_HELP_FORWARD, history_next);
  toolBar(ID_BROWSER_TOOLBAR)->insertButton("stop",ID_HELP_BROWSER_STOP, false,i18n("Stop"));
  toolBar(ID_BROWSER_TOOLBAR)->insertButton("reload_page",ID_HELP_BROWSER_RELOAD, true,i18n("Reload"));
	toolBar(ID_BROWSER_TOOLBAR)->insertButton("contents", ID_HELP_CONTENTS, true, i18n("User Manual"));
	
  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

  toolBar(ID_BROWSER_TOOLBAR)->insertButton("help", ID_HELP_SEARCH_TEXT,
					    true,i18n("Search Text in Documenation"));
  toolBar(ID_BROWSER_TOOLBAR)->insertButton("filefind",ID_HELP_SEARCH,
              true,i18n("Search for Help on..."));
	
  toolBar()->insertSeparator();

  toolBar(ID_BROWSER_TOOLBAR)->insertButton("contexthelp",ID_HELP_WHATS_THIS,
              true,i18n("What's this...?"));

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

  statProg = new QProgressBar(statusBar(),"Progressbar");
  statProg->setFixedWidth( 100 );             // arbitrary width
  statProg->setCenterIndicator(true);
  statProg->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  statProg->setMargin( 0 );
  statProg->setLineWidth(0);
  statProg->setBackgroundMode( QWidget::PaletteBackground );

  connect(class_tree,SIGNAL(setStatusbarProgressSteps(int)),statProg,SLOT(setTotalSteps(int)));
  connect(class_tree,SIGNAL(setStatusbarProgress(int)),statProg,SLOT(setProgress(int)));
  connect(class_tree,SIGNAL(resetStatusbarProgress()),statProg,SLOT(reset()));

  m_statusLabel = new KStatusBarLabel( "", 0, statusBar() );
  m_statusLabel->setFixedHeight( m_statusLabel->sizeHint().height() );
  m_statusLabel->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
  m_statusLabel->setMargin( 0 );
  m_statusLabel->setLineWidth(0);


  statusBar()->addWidget( m_statusLabel, 1, false );
  statusBar()->insertFixedItem("     ",                             ID_STATUS_DBG,          true);
  statusBar()->addWidget(statProg,                                                      0,  true);
  statProg->setFixedHeight( statProg->sizeHint().height() - 8 );
  statusBar()->insertFixedItem("              ",                    ID_STATUS_EMPTY_2,      true);
  statusBar()->insertFixedItem("        ",                          ID_STATUS_INS_OVR,      true);
  statusBar()->insertFixedItem("                                ",  ID_STATUS_LN_CLM,       true);
  statusBar()->insertFixedItem("                      ",            ID_STATUS_EMPTY,        true);

  m_statusLabel->setAlignment(AlignLeft|AlignVCenter);
  statusBar()->setItemAlignment(ID_STATUS_DBG, AlignCenter|AlignVCenter);
  statusBar()->setItemAlignment(ID_STATUS_INS_OVR, AlignCenter|AlignVCenter);

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

  connect(t_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotTTabSelected(int)));
  connect(s_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotSTabSelected(int)));
  connect(o_tab_view,SIGNAL(tabSelected(int)),this,SLOT(slotOTabSelected(int)));

  connect(class_tree, SIGNAL(selectedFileNew()), SLOT(slotProjectAddNewFile()));
  connect(class_tree, SIGNAL(selectedFileNew(const char*)), SLOT(slotFileNew(const char*)));
  connect(class_tree, SIGNAL(selectedClassNew()), SLOT(slotProjectNewClass()));
  connect(class_tree, SIGNAL(selectedClassNew(const char*)), SLOT(slotProjectNewClass(const char*)));
  connect(class_tree, SIGNAL(selectedProjectOptions()), SLOT(slotProjectOptions()));
  connect(class_tree,
          SIGNAL(selectedViewDeclaration(const char *, const char *,THType,THType)),
          SLOT(slotCVViewDeclaration(const char *, const char *,THType,THType)));
  connect(class_tree,
          SIGNAL(selectedViewDefinition(const char *, const char *,THType,THType)),
          SLOT(slotCVViewDefinition(const char *, const char *,THType,THType)));
  connect(class_tree, SIGNAL(signalAddMethod(const char *)), SLOT(slotCVAddMethod(const char * )));
  connect(class_tree, SIGNAL(signalAddAttribute(const char *)), SLOT(slotCVAddAttribute(const char * )));
  connect(class_tree, SIGNAL(signalMethodDelete(const char *,const char *)), SLOT(slotCVDeleteMethod(const char *,const char *)));
  connect(class_tree, SIGNAL(popupHighlighted(int)), SLOT(statusCallback(int)));
  connect(class_tree, SIGNAL(selectFile(const QString &, int)), SLOT(slotSwitchToFile(const QString &, int)));
  connect(class_tree, SIGNAL(signalGrepText(QString)), SLOT(slotEditSearchInFiles(QString)));

  connect(log_file_tree, SIGNAL(logFileTreeSelected(QString)), SLOT(slotLogFileTreeSelected(QString)));
  connect(log_file_tree, SIGNAL(selectedNewClass()), SLOT(slotProjectNewClass()));
  connect(log_file_tree, SIGNAL(selectedNewFile()), SLOT(slotProjectAddNewFile()));
  connect(log_file_tree, SIGNAL(selectedFileRemove(QString)), SLOT(delFileFromProject(QString)));
  connect(log_file_tree, SIGNAL(removeFileFromEditlist(const QString &)), SLOT(slotRemoveFileFromEditlist(const QString &)));
  connect(log_file_tree, SIGNAL(showFileProperties(QString)),SLOT(slotShowFileProperties(QString)));
  connect(log_file_tree, SIGNAL(menuItemHighlighted(int)), SLOT(statusCallback(int)));
  connect(log_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(log_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString)));

  connect(real_file_tree, SIGNAL(fileSelected(QString)), SLOT(slotRealFileTreeSelected(QString)));
  connect(real_file_tree, SIGNAL(showFileProperties(QString)),SLOT(slotShowFileProperties(QString)));
  connect(real_file_tree, SIGNAL(addFileToProject(QString)),SLOT(slotAddFileToProject(QString)));
  connect(real_file_tree, SIGNAL(removeFileFromProject(QString)),SLOT(delFileFromProject(QString)));
  connect(real_file_tree, SIGNAL(removeFileFromEditlist(const QString &)), SLOT(slotRemoveFileFromEditlist(const QString &)));
  connect(real_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(real_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString)));
  connect(real_file_tree, SIGNAL(commitDirToVCS(QString)), SLOT(slotCommitDirToVCS(QString)));
  connect(real_file_tree, SIGNAL(updateDirFromVCS(QString)), SLOT(slotUpdateDirFromVCS(QString)));
  connect(real_file_tree, SIGNAL(menuItemHighlighted(int)), SLOT(statusCallback(int)));
  connect(real_file_tree, SIGNAL(selectedFileNew(const char*)), SLOT(slotFileNew(const char*)));
  connect(real_file_tree, SIGNAL(selectedClassNew(const char*)), SLOT(slotProjectNewClass(const char*)));


  connect(doc_tree, SIGNAL(fileSelected(QString)), SLOT(slotDocTreeSelected(QString)));
  connect(doc_tree, SIGNAL(signalUpdateAPI()), SLOT(slotProjectAPI()));
  connect(doc_tree, SIGNAL(signalUpdateUserManual()), SLOT(slotProjectManual()));


  //connect the editor lookup function with slotHelpSText
  connect(cpp_widget, SIGNAL(lookUp(QString)),this, SLOT(slotHelpSearchText(QString)));
  connect(cpp_widget, SIGNAL(newCurPos()), this, SLOT(slotNewLineColumn()));
  connect(cpp_widget, SIGNAL(newStatus()),this, SLOT(slotNewStatus()));
  connect(cpp_widget, SIGNAL(markStatus(KWriteView *, bool)),
      this, SLOT(slotCPPMarkStatus(KWriteView *, bool)));
  connect(cpp_widget, SIGNAL(clipboardStatus(KWriteView *, bool)),
      this, SLOT(slotClipboardChanged(KWriteView *, bool)));
  connect(cpp_widget, SIGNAL(newUndo()),this, SLOT(slotNewUndo()));
  connect(cpp_widget, SIGNAL(bufferMenu(const QPoint&)),this, SLOT(slotBufferMenu(const QPoint&)));
  connect(cpp_widget, SIGNAL(grepText(QString)), this, SLOT(slotEditSearchInFiles(QString)));
  connect(cpp_widget->popup(), SIGNAL(highlighted(int)), this, SLOT(statusCallback(int)));

  connect(header_widget, SIGNAL(lookUp(QString)),this, SLOT(slotHelpSearchText(QString)));
  connect(header_widget, SIGNAL(newCurPos()), this, SLOT(slotNewLineColumn()));
  connect(header_widget, SIGNAL(newStatus()),this, SLOT(slotNewStatus()));
  connect(header_widget, SIGNAL(markStatus(KWriteView *, bool)),
      this, SLOT(slotHEADERMarkStatus(KWriteView *, bool)));
  connect(header_widget, SIGNAL(clipboardStatus(KWriteView *, bool)),
      this, SLOT(slotClipboardChanged(KWriteView *, bool)));
  connect(header_widget, SIGNAL(newUndo()),this, SLOT(slotNewUndo()));
  connect(header_widget, SIGNAL(bufferMenu(const QPoint&)),this, SLOT(slotBufferMenu(const QPoint&)));
  connect(header_widget, SIGNAL(grepText(QString)), this, SLOT(slotEditSearchInFiles(QString)));
  connect(header_widget->popup(), SIGNAL(highlighted(int)), this, SLOT(statusCallback(int)));

  connect(browser_widget, SIGNAL(completed()),this, SLOT(slotDocumentDone()));
  connect(browser_widget, SIGNAL(signalURLBack()),this,SLOT(slotHelpBack()));
  connect(browser_widget, SIGNAL(signalURLForward()),this,SLOT(slotHelpForward()));
  connect(browser_widget, SIGNAL(signalBookmarkToggle()),this,SLOT(slotBookmarksToggle()));

  connect(browser_widget, SIGNAL(onURL(const QString&)),this,SLOT(slotURLonURL(const QString&)));
  connect(browser_widget, SIGNAL(signalSearchText()),this,SLOT(slotHelpSearchText()));
//  connect(browser_widget, SIGNAL(goRight()), this, SLOT(slotHelpForward()));
//  connect(browser_widget, SIGNAL(goLeft()), this, SLOT(slotHelpBack()));
  connect(browser_widget, SIGNAL(enableStop(int)), this, SLOT(enableCommand(int)));	
  connect(browser_widget->popup(), SIGNAL(highlighted(int)), this, SLOT(statusCallback(int)));
  connect(browser_widget, SIGNAL(signalGrepText(QString)), this, SLOT(slotEditSearchInFiles(QString)));
  connect(browser_widget, SIGNAL(textSelected(KHTMLPart *, bool)),this,SLOT(slotBROWSERMarkStatus(KHTMLView *, bool)));

  connect(messages_widget, SIGNAL(clicked()),this,SLOT(slotClickedOnMessagesWidget()));
  // connect the windowsmenu with a method
  connect(menu_buffers,SIGNAL(activated(int)),this,SLOT(slotMenuBuffersSelected(int)));
  connect(doc_bookmarks, SIGNAL(activated(int)), this, SLOT(slotBookmarksBrowserSelected(int)));

  connect(grep_dlg,SIGNAL(itemSelected(QString,int)),SLOT(slotGrepDialogItemSelected(QString,int)));


  // connections for the proc -processes
  connect(&search_process, SIGNAL(receivedStdout(KProcess*,char*,int)),
          this, SLOT(slotSearchReceivedStdout(KProcess*,char*,int)) );

  connect(&search_process, SIGNAL(receivedStderr(KProcess*,char*,int)),
          this, SLOT(slotReceivedStderr(KProcess*,char*,int)) );

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
}

void CKDevelop::completeStartup(bool ignoreLastProject)
{
  initProject(ignoreLastProject);

  if (start_logo)
    delete start_logo;

  config->setGroup("TipOfTheDay");
  if(config->readBoolEntry("show_tod",true) && !kapp->isRestored())
    slotHelpTipOfDay();
}

void CKDevelop::initProject(bool ignoreLastProject)
{
  config->setGroup("General Options");

  bool bLastProject;
  if(!ignoreLastProject)
    bLastProject=false;
  else
    bLastProject= config->readBoolEntry("LastProject",true);

  QString filename="";
  if(bLastProject)
  {
    if (!lastShutdownOK)
    {
      if ( KMessageBox::No == KMessageBox::questionYesNo(this,
                                i18n( "KDevelop failed to shutdown correctly previously"
                                      "\nWould you like to start with the last loaded project?"),
                          i18n("KDevelop failed to shutdown correctly previously")))
      {
        bLastProject = false;
      }
    }

    if (bLastProject)
    {
      config->setGroup("Files");
      filename = config->readEntry("project_file","");
      slotProjectOpenCmdl(filename);
    }
  }
}

void CKDevelop::setKeyAccel()
{

  accel->setItemEnabled("Dialog Editor", true );


  accel->changeMenuAccel(file_menu, ID_FILE_NEW, KStdAccel::New );
  accel->changeMenuAccel(file_menu, ID_FILE_OPEN, KStdAccel::Open );
  accel->changeMenuAccel(file_menu, ID_FILE_CLOSE, KStdAccel::Close );
  accel->changeMenuAccel(file_menu, ID_FILE_SAVE, KStdAccel::Save );
  accel->changeMenuAccel(edit_menu, ID_FILE_SAVE_ALL,"SaveAll" );
  accel->changeMenuAccel(edit_menu, ID_FILE_SAVE_AS,"SaveAs" );
  accel->changeMenuAccel(file_menu, ID_FILE_PRINT, KStdAccel::Print );
  accel->changeMenuAccel(file_menu, ID_FILE_QUIT, KStdAccel::Quit );

  accel->changeMenuAccel(edit_menu, ID_EDIT_UNDO, KStdAccel::Undo );
  accel->changeMenuAccel(edit_menu, ID_EDIT_REDO,"Redo" );
  accel->changeMenuAccel(edit_menu, ID_EDIT_CUT, KStdAccel::Cut );
  accel->changeMenuAccel(edit_menu, ID_EDIT_COPY, KStdAccel::Copy );
  accel->changeMenuAccel(edit_menu, ID_EDIT_PASTE, KStdAccel::Paste );
  accel->changeMenuAccel(edit_menu, ID_EDIT_SEARCH, KStdAccel::Find );
  accel->changeMenuAccel(edit_menu, ID_EDIT_REPEAT_SEARCH,"RepeatSearch" );
  accel->changeMenuAccel(edit_menu, ID_EDIT_REPLACE,KStdAccel::Replace );
  accel->changeMenuAccel(edit_menu, ID_EDIT_SEARCH_IN_FILES,"Grep" );
  accel->changeMenuAccel(edit_menu, ID_EDIT_INDENT,"Indent" );
  accel->changeMenuAccel(edit_menu, ID_EDIT_UNINDENT,"Unindent" );
  accel->changeMenuAccel(edit_menu, ID_EDIT_COMMENT,"Comment" );
  accel->changeMenuAccel(edit_menu, ID_EDIT_UNCOMMENT,"Uncomment" );
  accel->changeMenuAccel(edit_menu, ID_EDIT_SELECT_ALL, "SelectAll");


  accel->changeMenuAccel(view_menu,ID_TOOLS_DESIGNER ,"Dialog Editor" );
  accel->changeMenuAccel(view_menu,ID_VIEW_GOTO_LINE ,"GotoLine" );
  accel->changeMenuAccel(view_menu,ID_VIEW_NEXT_ERROR ,"NextError" );
  accel->changeMenuAccel(view_menu,ID_VIEW_PREVIOUS_ERROR ,"PreviousError" );
  accel->changeMenuAccel(view_menu,ID_VIEW_TREEVIEW ,"Tree-View" );
  accel->changeMenuAccel(view_menu,ID_VIEW_OUTPUTVIEW,"Output-View" );

  accel->changeMenuAccel(project_menu, ID_PROJECT_KAPPWIZARD,"NewProject" );
  accel->changeMenuAccel(project_menu, ID_PROJECT_OPEN,"OpenProject" );
  accel->changeMenuAccel(project_menu, ID_PROJECT_CLOSE,"CloseProject" );
  accel->changeMenuAccel(project_menu,ID_PROJECT_NEW_CLASS , "NewClass");
  accel->changeMenuAccel(project_menu, ID_PROJECT_ADD_FILE_EXIST, "AddExistingFiles");
  accel->changeMenuAccel(project_menu,ID_PROJECT_ADD_NEW_TRANSLATION_FILE ,"Add new Translation File" );
  accel->changeMenuAccel(project_menu,ID_PROJECT_FILE_PROPERTIES ,"FileProperties" );
  accel->changeMenuAccel(project_menu,ID_PROJECT_MESSAGES , "MakeMessages");
  accel->changeMenuAccel(project_menu, ID_PROJECT_MAKE_PROJECT_API,"ProjectAPI" );
  accel->changeMenuAccel(project_menu,ID_PROJECT_MAKE_USER_MANUAL , "ProjectManual");
//  accel->changeMenuAccel(project_menu, ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ,"Source-tgz" );
  accel->changeMenuAccel(project_menu,ID_PROJECT_OPTIONS ,"ProjectOptions" );

  accel->changeMenuAccel(build_menu,ID_BUILD_COMPILE_FILE ,"CompileFile" );
  accel->changeMenuAccel(build_menu,ID_BUILD_MAKE ,"Make" );
  accel->changeMenuAccel(build_menu,ID_BUILD_REBUILD_ALL , "RebuildAll");
  accel->changeMenuAccel(build_menu,ID_BUILD_REBUILD_ALL ,"CleanRebuildAll" );
  accel->changeMenuAccel(build_menu,ID_BUILD_STOP,"Stop_proc");
  accel->changeMenuAccel(build_menu,ID_BUILD_RUN ,"Run" );
  accel->changeMenuAccel(build_menu,ID_BUILD_RUN_WITH_ARGS,"Run_with_args");
  accel->changeMenuAccel(build_menu,ID_BUILD_DISTCLEAN ,"BuildDistClean" );
  accel->changeMenuAccel(build_menu, ID_BUILD_MAKECLEAN, "BuildMakeClean");
  accel->changeMenuAccel(build_menu, ID_BUILD_AUTOCONF, "BuildAutoconf");
  accel->changeMenuAccel(build_menu, ID_BUILD_CONFIGURE, "BuildConfigure");

  accel->changeMenuAccel(classbrowser_popup, ID_CV_VIEW_DECLARATION, "CVGotoDeclaration");
  accel->changeMenuAccel(classbrowser_popup, ID_CV_VIEW_DEFINITION, "CVGotoDefinition");
  accel->changeMenuAccel(classbrowser_popup, ID_CV_VIEW_CLASS_DECLARATION, "CVGotoClass");
  accel->changeMenuAccel(classbrowser_popup, ID_CV_GRAPHICAL_VIEW, "CVViewTree");
  accel->changeMenuAccel(classbrowser_popup,ID_PROJECT_NEW_CLASS , "NewClass");

  accel->changeMenuAccel(debug_menu,ID_DEBUG_START , "DebugStart");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_RUN ,"DebugRun" );
  accel->changeMenuAccel(debug_menu,ID_DEBUG_RUN_CURSOR, "DebugRunCursor");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_STOP, "DebugStop");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_STEP, "DebugStepInto");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_STEP_INST, "DebugStepIntoInstr");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_NEXT, "DebugStepOver");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_NEXT_INST , "DebugStepOverInstr");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_FINISH ,  "DebugStepOut");
  accel->changeMenuAccel(debug_menu,ID_DEBUG_MEMVIEW ,"DebugViewer" );
  accel->changeMenuAccel(debug_menu,ID_DEBUG_BREAK_INTO ,"DebugInterrupt" );

  accel->changeMenuAccel(debugPopup, ID_DEBUG_CORE, "DebugExamineCore");
  accel->changeMenuAccel(debugPopup, ID_DEBUG_NAMED_FILE, "DebugOtherExec");
  accel->changeMenuAccel(debugPopup, ID_DEBUG_ATTACH, "DebugAttach");
  accel->changeMenuAccel(debugPopup, ID_DEBUG_SET_ARGS, "DebugRunWithArgs");

  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_TOGGLE ,"Toggle_Bookmarks" );
  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_NEXT ,"Next_Bookmarks" );
  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_PREVIOUS ,"Previous_Bookmarks" );
  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_CLEAR ,"Clear_Bookmarks" );

  accel->changeMenuAccel(help_menu->menu(),ID_HELP_SEARCH_TEXT,"SearchMarkedText" );
  accel->changeMenuAccel(help_menu->menu(), ID_HELP_SEARCH, "HelpSearch" );
  accel->changeMenuAccel(help_menu->menu(), ID_HELP_CONTENTS, KStdAccel::Help );

  accel->changeMenuAccel(help_menu->menu(),ID_HELP_PROJECT_API , "HelpProjectAPI" );
  accel->changeMenuAccel(help_menu->menu(),ID_HELP_USER_MANUAL ,  "HelpProjectManual");

}

void CKDevelop::setToolmenuEntries(){
  config = KGlobal::config();
  config->setGroup("ToolsMenuEntries");
	config->readListEntry("Tools_exe",tools_exe);
	config->readListEntry("Tools_entry",tools_entry);
	config->readListEntry("Tools_argument",tools_argument);
	
	uint items;
	for(items=0;items<tools_entry.count();items++)
		tools_menu->insertItem(tools_entry.at(items),items);
	
	connect(tools_menu,SIGNAL(activated(int)),SLOT(slotToolsTool(int)));
}

void CKDevelop::initDebugger()
{
  bool oldDbg = dbgInternal;

  config = KGlobal::config();
  config->setGroup("Debug");
  dbgInternal = !config->readBoolEntry("Use external debugger");
  dbgExternalCmd = config->readEntry("External debugger program","kdbg");
  dbgEnableFloatingToolbar = config->readBoolEntry("Enable floating toolbar", false);

  // once we've set these tabs up we don't seem to be able to remove them
  // without crashing. So for internal debuggers set them up and if they
  // change their mind they may be disabled later... All rather ugly...
  if (dbgInternal && !var_viewer)
  {
    ASSERT(!frameStack && !brkptManager && !var_viewer && !dbgController);
    brkptManager  = new BreakpointManager(o_tab_view, "BPManagerTab");
    frameStack    = new FrameStack(o_tab_view, "FStackTab");
    disassemble   = new Disassemble(o_tab_view, "DisassembleTab");

    var_viewer    = new VarViewer(t_tab_view,"VARTab");

    brkptManager->setFocusPolicy(QWidget::ClickFocus);
    frameStack->setFocusPolicy(QWidget::ClickFocus);
    disassemble->setFocusPolicy(QWidget::ClickFocus);
    var_viewer->setFocusPolicy(QWidget::NoFocus);

    config->setGroup("General Options");
    int mode=config->readNumEntry("tabviewmode", 3);
    switch (mode){
      case 1:
        t_tab_view->addTab(var_viewer,i18n("W&atch"));
        break;
      case 2:
        t_tab_view->addTab(var_viewer,SmallIcon("brace"),"");
        break;
      case 3:
        t_tab_view->addTab(var_viewer,SmallIcon("brace"),i18n("W&atch"));
        break;
    }

    o_tab_view->addTab(brkptManager,i18n("breakpoint"));
    o_tab_view->addTab(frameStack,i18n("frame stack"));
    o_tab_view->addTab(disassemble,i18n("disassemble"));


#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
    dbg_widget = new COutputWidget(o_tab_view, "debuggerTab");
    o_tab_view->addTab(dbg_widget,i18n("debugger"));
    dbg_widget->setFocusPolicy(QWidget::ClickFocus);
    dbg_widget->setReadOnly(TRUE);
#endif

  	// Connect the breakpoint manager to monitor the bp setting - even
  	// when the debugging isn't running
    // ... Must connect up both editors!!!!!
    connect(  header_widget,  SIGNAL(editBreakpoint(const QString&,int)),
              brkptManager,   SLOT(slotEditBreakpoint(const QString&,int)));
    connect(  cpp_widget,     SIGNAL(editBreakpoint(const QString&,int)),
              brkptManager,   SLOT(slotEditBreakpoint(const QString&,int)));

    connect(  header_widget,  SIGNAL(toggleBPEnabled(const QString&,int)),
              brkptManager,   SLOT(slotToggleBPEnabled(const QString&,int)));
    connect(  cpp_widget,     SIGNAL(toggleBPEnabled(const QString&,int)),
              brkptManager,   SLOT(slotToggleBPEnabled(const QString&,int)));

    connect(  header_widget,  SIGNAL(toggleBreakpoint(const QString&,int)),
              brkptManager,   SLOT(slotToggleStdBreakpoint(const QString&,int)));
    connect(  cpp_widget,     SIGNAL(toggleBreakpoint(const QString&,int)),
              brkptManager,   SLOT(slotToggleStdBreakpoint(const QString&,int)));

    connect(  header_widget,  SIGNAL(clearAllBreakpoints()),
              brkptManager,   SLOT(slotClearAllBreakpoints()));
    connect(  cpp_widget,     SIGNAL(clearAllBreakpoints()),
              brkptManager,   SLOT(slotClearAllBreakpoints()));

    connect(  var_viewer->varTree(),  SIGNAL(toggleWatchpoint(const QString&)),
              brkptManager,           SLOT(slotToggleWatchpoint(const QString&)));

    // Use this one as it just goes to the file/line pos
    connect(  brkptManager,   SIGNAL(gotoSourcePosition(const QString&,int)),
              this,           SLOT(slotDebugGoToSourcePosition(const QString&,int)));

    // used when the BP _only_ needs to be displayed (i.e file (re)loaded into editor)
    connect(  brkptManager,   SIGNAL(refreshBPState(const Breakpoint*)),
              this,           SLOT(slotDebugRefreshBPState(const Breakpoint*)));

    // used when the BP gets displayed or the state has changed so the display may
    // need changing.
    connect(  brkptManager,   SIGNAL(publishBPState(Breakpoint*)),
              this,           SLOT(slotDebugBPState(Breakpoint*)));

    // connect adding watch variable from the rmb in the editors
    connect(  header_widget,          SIGNAL(addWatchVariable(const QString&)),
              var_viewer->varTree(),  SLOT(slotAddWatchVariable(const QString&)));
    connect(  cpp_widget,             SIGNAL(addWatchVariable(const QString&)),
              var_viewer->varTree(),  SLOT(slotAddWatchVariable(const QString&)));

    connect(  var_viewer->varTree(),  SIGNAL(selectFrame(int)),
              frameStack,             SLOT(slotSelectFrame(int)));
  }

  // Enable or disable the tabs, if they exist...
  if (var_viewer)
  {
    // Figure out whether the tabs should be enabled or not.
    o_tab_view->setTabEnabled("BPManagerTab", dbgInternal);
    o_tab_view->setTabEnabled("FStackTab", dbgInternal && dbgController);
    o_tab_view->setTabEnabled("DisassembleTab", dbgInternal && dbgController);

//    t_tab_view->setTabEnabled("VARTab", dbgInternal && dbgController);
    brkptManager->setEnabled(dbgInternal);
    frameStack->setEnabled(dbgInternal && dbgController);
    disassemble->setEnabled(dbgInternal && dbgController);
    var_viewer->setEnabled(dbgInternal && dbgController);
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
    o_tab_view->setTabEnabled("debuggerTab", dbgInternal);
    dbg_widget->setEnabled(dbgInternal);
#endif
  }

  // If we've switch external/internal debuggers then stop any
  // internal debugger running.
  if (oldDbg != dbgInternal)
    slotDebugStop();

  // If we are running an internal debugger they (may have) changed
  // the config, so poke the current values into the controller
  //
  if (dbgController)
    dbgController->reConfig();
}

void CKDevelop::slotSwitchDocTool(){
  // kdoc used, can we switch to doxygen ?
  if(doctool_menu->isItemChecked(ID_PROJECT_DOC_TOOL_KDOC))
  {
  	if(!CToolClass::searchInstProgram("doxygen"))
    {
   	  KMessageBox::error(0,
			  i18n(" This option requires Doxygen to work. Look for it at:\n\n http://www.stack.nl/~dimitri/doxygen/download.html\n"),
				i18n("Program not found -- doxygen"));
  	  // no doxygen found
  	  return;
    }
    // yes, we have it
    doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_KDOC,false);
    doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_DOXYGEN,true);
  	doctool_menu->setItemEnabled(ID_PROJECT_DOC_TOOL_CONF_DOXYGEN,true);
  	doctool = DT_DOX;
   	return;
  }
	// kdoc
  doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_KDOC,true);
  doctool_menu->setItemChecked(ID_PROJECT_DOC_TOOL_DOXYGEN,false);
	doctool_menu->setItemEnabled(ID_PROJECT_DOC_TOOL_CONF_DOXYGEN,false);
  doctool = DT_KDOC;
}
//MB end
