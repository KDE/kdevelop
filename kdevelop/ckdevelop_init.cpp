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
//#include "ceditwidget.h"
#include "ckdevaccel.h"
#include "clogfileview.h"
#include "coutputwidget.h"
#include "crealfileview.h"
#include "ctoolclass.h"
#include "doctreeview.h"
#include "grepdialog.h"
#include "./ctags/ctagsdialog_impl.h"
#include "ckonsolewidget.h"

#include "./dbg/dbgcontroller.h"
#include "./dbg/vartree.h"
#include "./dbg/framestack.h"
#include "./dbg/brkptmanager.h"
#include "./dbg/disassemble.h"

#include "docviewman.h"
#include "kdevsession.h"

#include <kaction.h>
#include <kstdaction.h>		

#include <kaccel.h>
#include <kapp.h>
#include <kcursor.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <khtmlview.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kstdaccel.h>
#include <kaboutdata.h>

#include <qprogressbar.h>

//#include "./kwrite/kwdoc.h"

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
#include <krun.h>

CKDevelop::CKDevelop(): QextMdiMainFrm(0L,"CKDevelop")
  ,bStartupIsPending(true)
  ,view_menu(0L)
  ,process("/bin/sh")
  ,appl_process("/bin/sh")
  ,shell_process("/bin/sh")
  ,search_process("/bin/sh")
  ,class_tree(0L)
  ,log_file_tree(0L)
  ,real_file_tree(0L)
  ,doc_tree(0L)
  ,messages_widget(0L)
  ,stdin_stdout_widget(0L)
  ,stderr_widget(0L)
  ,konsole_widget(0L)
  ,dbgController(0L)
  ,dbgToolbar(0L)
  ,var_viewer(0L)
  ,brkptManager(0L)
  ,frameStack(0L)
  ,disassemble(0L)
  ,dbg_widget(0L)
  ,dbgInternal(false)
  ,m_docViewManager(0L)
  ,m_pKDevSession(0L)
  ,m_CTagsCmdLine()
  ,m_bToggleToolViewsIsPending(false)
{
  doctool = DT_KDOC;

  version = VERSION;
  project=false;// no project
  beep=false; // no beep
  cv_decl_or_impl=true;
  file_open_list.setAutoDelete(TRUE);

  config = KGlobal::config();

  // ********* MDI stuff (falk) *******
  m_docViewManager = new DocViewMan( this); // controls the kwrite documents, their views and the covering MDI views
  m_pKDevSession = new KDevSession( m_docViewManager, "testtest");
  connect(this, SIGNAL(dockWidgetHasUndocked(KDockWidget*)), SLOT(slotDockWidgetHasUndocked(KDockWidget*)) );

  m_FinishedToolProcesses.setAutoDelete( false );

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

  accel = new CKDevAccel( this);
  m_docViewManager->initKeyAccel(accel, this);

  initView();
  initConnections();
  initStatusBar();
  setKeyAccel();

  readOptions();
  setAutoSaveSettings("CKDevelop");

  slotViewRefresh();

  initDebugger();
  initWhatsThis();

  // default size if we start for the first time
  resize(kapp->desktop()->width() - 100, kapp->desktop()->height() - 100);
  // read the previous dock szenario from kdeveloprc
  // (this has to be after all creation of dockwidget-covered tool-views
  if (m_mdiMode == QextMdi::ToplevelMode) {
    m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
    readDockConfig(config, "docking_version_2_0");
    m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockNone);
  }
  else {
    readDockConfig(config, "docking_version_2_0");
  }

  createGUI(0L);

  show();

	adjustTTreesToolButtonState();
	adjustTOutputToolButtonState();

  setDebugMenuProcess(false);
  setToolmenuEntries();

  slotStatusMsg(i18n("Welcome to KDevelop!"));
}


CKDevelop::~CKDevelop()
{
  debug("queryClose()...");
  if (config)
  {
    config->setGroup("General Options");
    config->writeEntry("ShutdownOK", true);
    // i believe this is necessary to guarantee the config is written
    config->sync();
  }
}

void CKDevelop::initView()
{
  act_outbuffer_len=0;
  prj = 0;

  ////////////////////////
  // Treeviews
  ////////////////////////

  class_tree = new CClassView(0L,"cv");
  class_tree->setFocusPolicy(QWidget::NoFocus);//???

  log_file_tree = new CLogFileView(config->readBoolEntry("lfv_show_path",false),0L,"lfv");
  log_file_tree->setFocusPolicy(QWidget::NoFocus);//???

  real_file_tree = new CRealFileView(0L,"RFV");
  real_file_tree->setFocusPolicy(QWidget::NoFocus);//???

  doc_tree = new DocTreeView(0L,"DOC");
  doc_tree->setFocusPolicy(QWidget::NoFocus);//???

  QString class_tree_title = i18n("Classes");
  QString log_file_tree_title = i18n("Groups");
  QString real_file_tree_title = i18n("Files");
  QString doc_tree_title = i18n("Books");
  class_tree->setCaption(class_tree_title);
  log_file_tree->setCaption(log_file_tree_title);
  real_file_tree->setCaption(real_file_tree_title);
  doc_tree->setCaption(doc_tree_title);

  // set the mode of the tab headers
  config->setGroup("General Options");
  int mode=config->readNumEntry("tabviewmode", 3);
  switch (mode) {
    case 2:
      class_tree_title = "";
      log_file_tree_title = "";
      real_file_tree_title = "";
      doc_tree_title = "";
    case 3:
      class_tree->setIcon(SmallIcon("CVclass"));
      log_file_tree->setIcon(SmallIcon("attach"));
      real_file_tree->setIcon(SmallIcon("folder"));
      doc_tree->setIcon(SmallIcon("contents"));
  }
  addToolWindow(class_tree, KDockWidget::DockLeft, m_pMdi, 25, i18n("class tree"), class_tree_title);
  addToolWindow(log_file_tree, KDockWidget::DockCenter, class_tree, 25, i18n("files of project"), log_file_tree_title);
  addToolWindow(real_file_tree, KDockWidget::DockCenter, class_tree, 25, i18n("view on project directory"), real_file_tree_title);
  addToolWindow(doc_tree, KDockWidget::DockCenter, class_tree, 25, i18n("documentation"), doc_tree_title);

  prev_was_search_result= false;
  //init

  ////////////////////////
  // Outputwindow
  ////////////////////////
	
  messages_widget = new CMakeOutputWidget(0L,"messages");
  messages_widget->setFocusPolicy(QWidget::ClickFocus);
//  messages_widget->setReadOnly(TRUE);

  stdin_stdout_widget = new COutputWidget(0L,"stdin");
  stdin_stdout_widget->setReadOnly(TRUE);
  stdin_stdout_widget->setFocusPolicy(QWidget::ClickFocus);

  stderr_widget = new COutputWidget(0L,"stderr");
  stderr_widget->setReadOnly(TRUE);
  stderr_widget->setFocusPolicy(QWidget::ClickFocus);

  konsole_widget = new CKonsoleWidget(0L,"konsole");

  messages_widget->setCaption(i18n("messages"));
  addToolWindow(messages_widget, KDockWidget::DockBottom, m_pMdi, 70, i18n("output of KDevelop"), i18n("messages"));
  stdin_stdout_widget->setCaption(i18n("stdout"));
  addToolWindow(stdin_stdout_widget, KDockWidget::DockCenter, messages_widget, 70, i18n("messages of started program"), i18n("stdout"));
  stderr_widget->setCaption(i18n("stderr"));
  addToolWindow(stderr_widget, KDockWidget::DockCenter, messages_widget, 70, i18n("error messages of started program"), i18n("stderr"));
  konsole_widget->setCaption(i18n("Konsole"));
  addToolWindow(konsole_widget, KDockWidget::DockCenter, messages_widget, 70, i18n("embedded konsole window"), i18n("Konsole"));

///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

  m_docViewManager->initKeyAccel(accel, this);
  initMenuBar();
  initToolBar();

  // init some dialogs
  grep_dlg = new GrepDialog(QDir::homeDirPath(),0,"grepdialog");
  ctags_dlg = new searchTagsDialogImpl(this,"searchTagsDialog");
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
// File
	KAction* pAction = KStdAction::openNew(this,SLOT(slotFileNew()),actionCollection(),"file_new");
	pAction->setEnabled(false);
	pAction->setStatusText(i18n("Creates a new file"));
	pAction = KStdAction::open(this,SLOT(slotFileOpen()),actionCollection(),"file_open");
	pAction->setStatusText(i18n("Opens an existing file"));
	pAction = KStdAction::close(this,SLOT(slotFileClose()),actionCollection(),"file_close");
	pAction->setStatusText(i18n("Closes the current file"));
	pAction = new KAction(i18n("Close All"),0,this,SLOT(slotFileClose()),actionCollection(),"file_close_all");
	pAction->setStatusText(i18n("Closes all open files"));
	pAction = KStdAction::save(this,SLOT(slotFileSave()),actionCollection(),"file_save");
	pAction->setStatusText(i18n("Save the current document"));
	pAction = KStdAction::saveAs(this,SLOT(slotFileSaveAs()),actionCollection(),"file_save_as");
	pAction->setStatusText(i18n("Save the document as..."));
	pAction = new KAction(i18n("Save A&ll"),SmallIconSet("save_all"),0,this,SLOT(slotFileSaveAll()),actionCollection(),"file_save_all");
	pAction->setStatusText(i18n("Save all changed files"));
	pAction = KStdAction::print(this,SLOT(slotFilePrint()),actionCollection(),"file_print");
	pAction->setStatusText(i18n("Prints the current document"));
	pAction = KStdAction::quit(this,SLOT(slotFileQuit()),actionCollection(),"file_quit");
	pAction->setStatusText(i18n("Exits the program"));

///////////////////////////////////////////////////////////////////
// Edit-menu entries
	pAction = KStdAction::undo(m_docViewManager, SLOT(slotEditUndo()),actionCollection(),"edit_undo");
	pAction = KStdAction::redo(m_docViewManager, SLOT(slotEditRedo()),actionCollection(),"edit_redo");
	pAction = KStdAction::cut(m_docViewManager, SLOT(slotEditCut()),actionCollection(),"edit_cut");
	pAction = KStdAction::copy(m_docViewManager, SLOT(slotEditCopy()),actionCollection(),"edit_copy");
	pAction = KStdAction::paste(m_docViewManager, SLOT(slotEditPaste()),actionCollection(),"edit_paste");
	pAction = new KAction(i18n("In&dent"),SmallIconSet("increaseindent"),0,this,SLOT(slotEditIndent()),actionCollection(),"edit_indent");
	pAction = new KAction(i18n("Uninden&t"),SmallIconSet("decreaseindent"),0,this,SLOT(slotEditUnIndent()),actionCollection(),"edit_unindent");
	pAction = new KAction(i18n("C&omment"),0,this,SLOT(slotEditComment()),actionCollection(),"edit_comment");
	pAction = new KAction(i18n("Unco&mment"),0,this,SLOT(slotEditUncomment()),actionCollection(),"edit_uncomment");
	pAction = new KAction(i18n("&Insert File..."),0,m_docViewManager,SLOT(slotEditInsertFile()),actionCollection(),"edit_insert_file");
	pAction = KStdAction::find(m_docViewManager,SLOT(slotEditSearch()),actionCollection(),"edit_find");
	pAction = KStdAction::findNext(m_docViewManager, SLOT(slotEditSearch()),actionCollection(),"edit_find_next");
	pAction = KStdAction::replace(m_docViewManager, SLOT(slotEditSearch()),actionCollection(),"edit_replace");
	pAction = new KAction(i18n("Search in &Files..."),SmallIconSet("grep"),0,this,SLOT(slotEditSearchInFiles()),actionCollection(),"edit_find_files");
	// this is not inserted in the menu
	pAction = new KAction(i18n("Search &CTags Database..."),0,this,SLOT(slotTagSearch()),actionCollection(),"edit_find_tag");
	pAction = new KAction(i18n("Switch to Header/Source..."),0,this,SLOT(slotTagSwitchTo()),actionCollection(),"edit_switch_header");
	pAction = KStdAction::selectAll(m_docViewManager, SLOT(slotEditSelectAll()),actionCollection(),"edit_select_all");
	pAction = new KAction(i18n("Deselect All"),0,m_docViewManager,SLOT(slotEditDeselectAll()),actionCollection(),"edit_deslect_all");
	pAction = new KAction(i18n("Invert Selection"),0,m_docViewManager,SLOT(slotEditInvertSelection()),actionCollection(),"edit_invert_selection");

  ///////////////////////////////////////////////////////////////////
  // View-menu entries
	pAction = KStdAction::gotoLine(this,SLOT(slotViewGotoLine()),actionCollection(),"view_goto");
	pAction = new KAction(i18n("&Next Error"),0,this,SLOT(slotViewNextError()),actionCollection(),"view_next_error");
	pAction = new KAction(i18n("&Previous Error"),0,this,SLOT(slotViewPreviousError()),actionCollection(),"view_prev_error");
	pAction = new KAction(i18n("&Dialog Editor"),SmallIconSet("newwidget"),0,this,SLOT(startDesigner()),actionCollection(),"view_designer");

	KToggleAction* pToggleAction = new KToggleAction(i18n("All &Tree Tool-Views"),0,this,SLOT(slotViewTTreeView()),actionCollection(),"view_tree_all");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Classes"),0,this,SLOT(slotViewTClassesView()),actionCollection(),"view_tree_classes");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Groups"),0,this,SLOT(slotViewTGroupsView()),actionCollection(),"view_tree_groups");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Files"),0,this,SLOT(slotViewTFilesView()),actionCollection(),"view_tree_file");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Books"),0,this,SLOT(slotViewTBooksView()),actionCollection(),"view_tree_books");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Watch"),0,this,SLOT(slotViewTWatchView()),actionCollection(),"view_tree_watch");
	pToggleAction->setChecked(true);

	pToggleAction = new KToggleAction(i18n("All &Output Tool-Views"),0,this,SLOT(slotViewTOutputView()),actionCollection(),"view_out_all");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Messages"),0,this,SLOT(slotViewOMessagesView()),actionCollection(),"view_out_msg");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&StdOut"),0,this,SLOT(slotViewOStdOutView()),actionCollection(),"view_out_stdout");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("S&tdErr"),0,this,SLOT(slotViewOStdErrView()),actionCollection(),"view_out_stderr");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Konsole"),0,this,SLOT(slotViewOKonsoleView()),actionCollection(),"view_out_konsole");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Breakpoints"),0,this,SLOT(slotViewOBreakpointView()),actionCollection(),"view_out_break");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Disassemble"),0,this,SLOT(slotViewODisassembleView()),actionCollection(),"view_out_disasm");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("&Call Stack"),0,this,SLOT(slotViewOFrameStackView()),actionCollection(),"view_out_stack");
	pToggleAction->setChecked(true);
	pToggleAction = new KToggleAction(i18n("D&ebugger"),0,this,SLOT(slotViewODebuggerView()),actionCollection(),"view_out_dbg");
	pToggleAction->setChecked(true);

	pToggleAction = new KToggleAction(i18n("Toolb&ar"),0,this,SLOT(slotViewTStdToolbar()),actionCollection(),"view_toolbar");
	pToggleAction = new KToggleAction(i18n("&Browser-Toolbar"),0,this,SLOT(slotViewTBrowserToolbar()),actionCollection(),"view_browser");
	pToggleAction = new KToggleAction(i18n("&Statusbar"),0,this,SLOT(slotViewTStatusbar()),actionCollection(),"view_status");
	pToggleAction = new KToggleAction(i18n("&MDI-View-Taskbar"),0,this,SLOT(slotViewMdiViewTaskbar()),actionCollection(),"view_mdi");

	pToggleAction = new KToggleAction(i18n("&Text only"),0,this,SLOT(slotViewTabText()),actionCollection(),"view_tab_text");
	pToggleAction = new KToggleAction(i18n("&Icons only"),0,this,SLOT(slotViewTabIcons()),actionCollection(),"view_tab_icons");
	pToggleAction = new KToggleAction(i18n("&Text and Icons"),0,this,SLOT(slotViewTabTextIcons()),actionCollection(),"view_tab_texticons");

	pAction = new KAction(i18n("&Refresh"),SmallIconSet("reload"),0,this,SLOT(slotViewRefresh()),actionCollection(),"view_refresh");

  ///////////////////////////////////////////////////////////////////
  // Project-menu entries
	pAction = new KAction(i18n("New..."),SmallIconSet("window_new"),0,this,SLOT(slotProjectNewAppl()),actionCollection(),"project_new");

	pAction = new KAction(i18n("Generate Project File..."),SmallIconSet("wizard"),0,this,SLOT(slotProjectGenerate()),actionCollection(),"project_generate");
	pAction = new KAction(i18n("&Open..."),SmallIconSet("project_open"),0,this,SLOT(slotProjectOpen()),actionCollection(),"project_open");
//
//  recent_projects_menu = new QPopupMenu();
//  connect( recent_projects_menu, SIGNAL(activated(int)), ) );
//  pAction = new KAction(i18n("Open &recent project..."), recent_projects_menu, ID_PROJECT_OPEN_RECENT );
//
	KStdAction::openRecent(this,SLOT(slotProjectOpenRecent(int)),actionCollection(),"project_open_recent");
	pAction = new KAction(i18n("C&lose"),SmallIconSet("fileclose"),0,this,SLOT(slotProjectClose()),actionCollection(),"project_close");
	pAction = new KAction(i18n("&New Class..."),SmallIconSet("classnew"),0,this,
	          SLOT(slotProjectNewClass()),actionCollection(),"project_new_class");
	pAction = new KAction(i18n("&Add existing File(s)..."),0,this,
	          SLOT(slotProjectAddExistingFiles()),actionCollection(),"project_add_file");
	pAction = new KAction(i18n("Add new &Translation File..."),SmallIconSet("locale"),0,this,
	          SLOT(slotProjectAddNewTranslationFile()),actionCollection(),"project_nwe_trans");
//  pAction = new KAction(i18n("&Remove File from Project"),0,this,
//  //			   SLOT(slotProjectRemoveFile()),actionCollection(),"project_rm_file");
	pAction = new KAction(i18n("&File Properties..."),SmallIconSet("file_properties"),0,this,
	          SLOT(slotProjectFileProperties()),actionCollection(),"project_file_prop");
	pAction = new KAction(i18n("Make &messages and merge"),0,this,
	          SLOT(slotProjectMessages()),actionCollection(),"project_make_msg");
	pToggleAction = new KToggleAction(i18n("kdoc"),0,this,
	          SLOT(slotSwitchDocTool()),actionCollection(),"project_api_kdoc");
	pToggleAction = new KToggleAction(i18n("doxygen"),0,this,
	          SLOT(slotSwitchDocTool()),actionCollection(),"project_api_doxygen");
	pAction = new KAction(i18n("Configure doxygen"),SmallIconSet("configure"),0,this,
	          SLOT(slotConfigureDoxygen()),actionCollection(),"project_api_doxyconf");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("Make AP&I-Doc"),0,this,
	          SLOT(slotProjectAPI()),actionCollection(),"project_make_api_doc");
	pAction = new KAction(i18n("Make &User-Manual..."),SmallIconSet("contents2"),0,this,
	          SLOT(slotProjectManual()),actionCollection(),"project_make_user_man");
	pAction = new KAction(i18n("&Source-tgz"),SmallIconSet("tgz"),0,this,
	          SLOT(slotProjectMakeDistSourceTgz()),actionCollection(),"project_dist_srctgz");
	pAction = new KAction(i18n("&Build RPM Package"),SmallIconSet("rpm"),0,this,
	          SLOT(slotProjectMakeDistRPM()),actionCollection(),"project_build_rpm");
	pAction = new KAction(i18n("&Configure RPM Package"),SmallIconSet("rpm"),0,this,
	          SLOT(slotConfigMakeDistRPM()),actionCollection(),"project_conf_rpm");
	pAction = new KAction(i18n("Load &tags file"),0,this,
	          SLOT(slotProjectLoadTags()),actionCollection(),"project_load_tags");
	pAction = new KAction(i18n("O&ptions..."),SmallIconSet("configure"),0,this,
	          SLOT(slotProjectOptions()),actionCollection(),"project_options");
	pAction->setEnabled(false);

  ///////////////////////////////////////////////////////////////////
  // Build-menu entries
	pAction = new KAction(i18n("Compile &File"),SmallIconSet("compfile"),0,this,
	          SLOT(slotBuildCompileFile()),actionCollection(),"build_compile");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("&Make"),SmallIconSet("make_kdevelop"),0,this,
	          SLOT(slotBuildMake()),actionCollection(),"build_make");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("Clea&n"),0,this,
	          SLOT(slotBuildMakeClean()),actionCollection(),"build_clean");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("&Rebuild"),SmallIconSet("rebuild"),0,this,
	          SLOT(slotBuildRebuildAll()),actionCollection(),"build_rebuild");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("Dist&Clean/Rebuild All"),0,this,
	          SLOT(slotBuildCleanRebuildAll()),actionCollection(),"build_rebuild_all");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("&Stop Build"),SmallIconSet("stop"),0,this,
	          SLOT(slotBuildStop()),actionCollection(),"build_stop");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("&Execute"),SmallIconSet("exec"),0,this,
	          SLOT(slotBuildRun()),actionCollection(),"build_exec");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("Execute &with Arguments..."),SmallIconSet("exec"),0,this,
	          SLOT(slotBuildRunWithArgs()),actionCollection(),"build_exec_args");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("DistC&lean"),0,this,
	          SLOT(slotBuildDistClean()),actionCollection(),"build_distclean");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("&Autoconf and automake"),0,this,
	          SLOT(slotBuildAutoconf()),actionCollection(),"build_autoconf");
	pAction->setEnabled(false);
	pAction = new KAction(i18n("C&onfigure..."),0,this,
	          SLOT(slotBuildConfigure()),actionCollection(),"build_configure");
	pAction->setEnabled(false);

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
  connect(tools_menu,SIGNAL(activated(int)),SLOT(slotToolsTool(int)));

  ///////////////////////////////////////////////////////////////////
  // Options-menu entries
  // submenu for setting printprograms
//  QPopupMenu* p3 = new QPopupMenu;
//  p3->insertItem(i18n("&Enscript..."), this,
//		  SLOT(slotOptionsConfigureEnscript()),0,ID_OPTIONS_PRINT_ENSCRIPT);

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

//  options_menu->insertItem(SmallIconSet("fileprint"),i18n("Configure &Printer..."),p3,ID_OPTIONS_PRINT);
	options_menu->insertItem(SmallIconSet("run"),i18n("Tools..."),this,SLOT(slotOptionsToolsConfigDlg()),0,ID_OPTIONS_TOOLS_CONFIG_DLG);
//  options_menu->insertItem(i18n("&Spellchecker..."),this,SLOT(slotOptionsSpellchecker()),0,ID_OPTIONS_SPELLCHECKER);
  options_menu->insertSeparator();
  options_menu->insertItem(SmallIconSet("configure"),i18n("&KDevelop Setup..."),this,
			   SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);

  menuBar()->insertItem(i18n("&Options"), options_menu);

  ///////////////////////////////////////////////////////////////////
  // Window-menu entries
  //   menu_buffers = new QPopupMenu;
  menuBar()->insertItem(i18n("&Window"), windowMenu());
  menuBar()->insertSeparator();

  ///////////////////////////////////////////////////////////////////
  // Bookmarks-menu entries
  bookmarks_menu=new QPopupMenu;
  bookmarks_menu->insertItem(SmallIconSet("bookmark_add"),i18n("&Toggle Bookmark"),this,SLOT(slotBookmarksToggle()),0,ID_BOOKMARKS_TOGGLE);
  bookmarks_menu->insertItem(i18n("&Next Bookmark"),this,SLOT(slotBookmarksNext()),0,ID_BOOKMARKS_NEXT);
  bookmarks_menu->insertItem(i18n("&Previous Bookmark"),this,SLOT(slotBookmarksPrevious()),0,ID_BOOKMARKS_PREVIOUS);
  bookmarks_menu->insertItem(i18n("&Clear Bookmarks"),this,SLOT(slotBookmarksClear()),0,ID_BOOKMARKS_CLEAR);
  bookmarks_menu->insertSeparator();

  // Changed by Christian
  m_docViewManager->installBMPopup(bookmarks_menu);

/*
  QPopupMenu* code_bookmarks = new QPopupMenu();
  m_docViewManager->installBMPopup(code_bookmarks);
  //   QPopupMenu* cpp_bookmarks = new QPopupMenu();
  //  cpp_widget->installBMPopup(cpp_bookmarks);
	
  bookmarks_menu->insertItem(SmallIconSet("bookmark_folder"),i18n("Code &Window"),code_bookmarks,31000);
  // bookmarks_menu->insertItem(SmallIconSet("bookmark_folder"),i18n("C/C++ &Window"),cpp_bookmarks,31010);

  doc_bookmarks = new QPopupMenu();

  bookmarks_menu->insertItem(SmallIconSet("bookmark_folder"),i18n("&Browser Window"), doc_bookmarks,31010);
*/
	
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
  help->insertItem(SmallIconSet("help"),i18n("Show Manpage on..."),this,SLOT(slotManpage()),0,ID_HELP_MANPAGE);
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
	help->insertItem(SmallIconSet("idea"),i18n("Tip of the Day"), this, SLOT(slotHelpTipOfDay(bool)), 0, ID_HELP_TIP_OF_DAY);
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
  classbrowser_popup->insertItem( SmallIconSet("CVpublic_signal"),i18n("Add signal..."), this,
                                  SLOT(slotClassbrowserNewSignal()), 0, ID_CV_SIGNAL_NEW );
  classbrowser_popup->insertItem( SmallIconSet("CVpublic_slot"),i18n("Add slot..."), this,
                                  SLOT(slotClassbrowserNewSlot()), 0, ID_CV_SLOT_NEW );

  classbrowser_popup->insertSeparator();
  classbrowser_popup->insertItem( SmallIconSet("graphview"), i18n("Show graphical classview"), this,
  																SLOT(slotClassbrowserViewTree()),0, ID_CV_GRAPHICAL_VIEW);


///////////////////////////////////////////////////////////////////
// connects for the statusbar help
//  connect(file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(p3,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(view_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(p2,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(debug_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(tools_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(options_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
//  connect(bookmarks_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  //connect(help_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(classbrowser_popup,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

  // QextMDI wants to know which one the menubar is, needs it for maximized mode
  setMenuForSDIModeSysButtons( menuBar());
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
//  toolBar()->insertSeparator();
  toolBar()->insertButton("project_open",ID_PROJECT_OPEN, true,i18n("Open Project"));
//  toolBar()->insertSeparator();
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
//  toolBar()->insertSeparator();

  toolBar()->insertButton("undo",ID_EDIT_UNDO,false,i18n("Undo"));
  toolBar()->insertButton("redo",ID_EDIT_REDO,false,i18n("Redo"));
//  toolBar()->insertSeparator();
  toolBar()->insertButton("editcut",ID_EDIT_CUT,true,i18n("Cut"));
  toolBar()->insertButton("editcopy",ID_EDIT_COPY, true,i18n("Copy"));
  toolBar()->insertButton("editpaste",ID_EDIT_PASTE, true,i18n("Paste"));
	
//  toolBar()->insertSeparator();

  toolBar()->insertButton("compfile",ID_BUILD_COMPILE_FILE, false,i18n("Compile file"));
  toolBar()->insertButton("make_kdevelop",ID_BUILD_MAKE, false,i18n("Make"));
  toolBar()->insertButton("rebuild",ID_BUILD_REBUILD_ALL, false,i18n("Rebuild"));
//  toolBar()->insertSeparator();
  toolBar()->insertButton("debugger",ID_DEBUG_START, false, i18n("Debug"));
  QPopupMenu* debugToolPopup = new QPopupMenu();
  debugToolPopup->insertItem(SmallIconSet("core"),i18n("Examine core file"),this,SLOT(slotDebugExamineCore()),0,ID_DEBUG_CORE);
  debugToolPopup->insertItem(SmallIconSet("exec"),i18n("Debug another executable"),this,SLOT(slotDebugNamedFile()),0,ID_DEBUG_NAMED_FILE);
  debugToolPopup->insertItem(SmallIconSet("connect_creating"),i18n("Attach to process"),this,SLOT(slotDebugAttach()),0,ID_DEBUG_ATTACH);
  debugToolPopup->insertItem(SmallIconSet("exec"),i18n("Debug with arguments"),this,SLOT(slotDebugRunWithArgs()),0,ID_DEBUG_SET_ARGS);
  connect(debugToolPopup,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  toolBar()->setDelayedPopup(ID_DEBUG_START, debugToolPopup);

  toolBar()->insertButton("exec",ID_BUILD_RUN, false,i18n("Run"));
//  toolBar()->insertSeparator();
  toolBar()->insertButton("stop",ID_BUILD_STOP, false,i18n("Stop"));

//  toolBar()->insertSeparator();

  toolBar()->insertButton("newwidget",ID_TOOLS_DESIGNER, true,i18n("Switch to QT's designer (dialog editor)"));
  toolBar()->insertButton("tree_win",ID_VIEW_TREEVIEW, true,i18n("Tree-View"));
  toolBar()->insertButton("output_win",ID_VIEW_OUTPUTVIEW, true,i18n("Output-View"));
  toolBar()->setToggle(ID_VIEW_TREEVIEW);
  toolBar()->setToggle(ID_VIEW_OUTPUTVIEW);

//  toolBar()->insertSeparator();

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
  toolBar(ID_BROWSER_TOOLBAR)->insertCombo(i18n("Compile Configuration"),
                                           ID_CV_TOOLBAR_COMPILE_CHOICE,true,
                                           SIGNAL(activated(const QString&))
                                           ,this,
                                           SLOT(slotCompileCombo(const QString&)),
                                           true,i18n("Compile Configuration"),110 );
  KComboBox* compile_combo = toolBar(ID_BROWSER_TOOLBAR)->getCombo(ID_CV_TOOLBAR_COMPILE_CHOICE);
  compile_combo->setFocusPolicy(QWidget::ClickFocus);
  compile_combo->setAutoCompletion(true);
  compile_combo->setInsertionPolicy(QComboBox::NoInsertion);
  compile_combo->setEnabled(false);
  compile_combo->useGlobalKeyBindings();
  compile_combo->setCompletionMode ( KGlobalSettings::CompletionPopup );
  compile_comp = compile_combo->completionObject();
  compile_combo->setAutoDeleteCompletionObject( true );

  toolBar(ID_BROWSER_TOOLBAR)->insertCombo(i18n("Classes"),
                                           ID_CV_TOOLBAR_CLASS_CHOICE,true,
                                           SIGNAL(activated(const QString&))
                                           ,this,
                                           SLOT(slotClassChoiceCombo(const QString&)),
                                           true,i18n("Classes"),130 );

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
//  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

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
	
//  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

  toolBar(ID_BROWSER_TOOLBAR)->insertButton("help", ID_HELP_SEARCH_TEXT,
					    true,i18n("Search Text in Documenation"));
  toolBar(ID_BROWSER_TOOLBAR)->insertButton("filefind",ID_HELP_SEARCH,
              true,i18n("Search for Help on..."));
	
//  toolBar(ID_BROWSER_TOOLBAR)->insertSeparator();

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
void CKDevelop::initConnections()
{
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
  connect(class_tree,
          SIGNAL(sigAddAttribute( const char*, CParsedAttribute*)),
		     SLOT( slotCVAddAttribute( const char*, CParsedAttribute*)));
  connect(class_tree, SIGNAL(signalMethodDelete(const char *,const char *)), SLOT(slotCVDeleteMethod(const char *,const char *)));
  connect(class_tree, SIGNAL(popupHighlighted(int)), SLOT(statusCallback(int)));
  connect(class_tree, SIGNAL(selectFile(const QString &, int)), SLOT(slotSwitchToFile(const QString &, int)));
  connect(class_tree, SIGNAL(signalGrepText(QString)), SLOT(slotEditSearchInFiles(QString)));
  connect(class_tree,
          SIGNAL(sigAddMethod(const char *, CParsedMethod* )),
          SLOT(slotCVAddMethod(const char * , CParsedMethod*)));
  connect(class_tree,
            SIGNAL(sigSigSlotMapImplement(CParsedClass*, const QString&, CParsedMethod* )),
            SLOT(slotCVSigSlotMapImplement(CParsedClass*, const QString&, CParsedMethod* )));
  connect(log_file_tree, SIGNAL(logFileTreeSelected(QString)), SLOT(slotLogFileTreeSelected(QString)));
  connect(log_file_tree, SIGNAL(selectedNewClass()), SLOT(slotProjectNewClass()));
  connect(log_file_tree, SIGNAL(selectedNewFile()), SLOT(slotProjectAddNewFile()));
  connect(log_file_tree, SIGNAL(selectedFileRemove(QString)), SLOT(delFileFromProject(QString)));
  connect(log_file_tree, SIGNAL(removeFileFromEditlist(const QString &)), m_docViewManager, SLOT(slotRemoveFileFromEditlist(const QString &)));
  connect(log_file_tree, SIGNAL(showFileProperties(const QString&)),SLOT(slotShowFileProperties(const QString&)));
  connect(log_file_tree, SIGNAL(menuItemHighlighted(int)), SLOT(statusCallback(int)));
  connect(log_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(log_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString)));

  connect(real_file_tree, SIGNAL(fileSelected(QString)), SLOT(slotRealFileTreeSelected(QString)));
  connect(real_file_tree, SIGNAL(showFileProperties(const QString&)),SLOT(slotShowFileProperties(const QString&)));
  connect(real_file_tree, SIGNAL(addFileToProject(QString)),SLOT(slotAddFileToProject(QString)));
  connect(real_file_tree, SIGNAL(removeFileFromProject(QString)),SLOT(delFileFromProject(QString)));
  connect(real_file_tree, SIGNAL(removeFileFromEditlist(const QString &)), m_docViewManager, SLOT(slotRemoveFileFromEditlist(const QString &)));
  connect(real_file_tree, SIGNAL(commitFileToVCS(QString)), SLOT(slotCommitFileToVCS(QString)));
  connect(real_file_tree, SIGNAL(updateFileFromVCS(QString)), SLOT(slotUpdateFileFromVCS(QString)));
  connect(real_file_tree, SIGNAL(commitDirToVCS(QString)), SLOT(slotCommitDirToVCS(QString)));
  connect(real_file_tree, SIGNAL(updateDirFromVCS(QString)), SLOT(slotUpdateDirFromVCS(QString)));
  connect(real_file_tree, SIGNAL(menuItemHighlighted(int)), SLOT(statusCallback(int)));
  connect(real_file_tree, SIGNAL(selectedFileNew(const char*)), SLOT(slotFileNew(const char*)));
  connect(real_file_tree, SIGNAL(selectedClassNew(const char*)), SLOT(slotProjectNewClass(const char*)));
  connect(real_file_tree, SIGNAL(makeDir(const QString&)), SLOT(slotBuildCompileDir(const QString&)));


  connect(doc_tree, SIGNAL(fileSelected(QString)), SLOT(slotDocTreeSelected(QString)));
  connect(doc_tree, SIGNAL(signalUpdateAPI()), SLOT(slotProjectAPI()));
  connect(doc_tree, SIGNAL(signalUpdateUserManual()), SLOT(slotProjectManual()));

  connect(messages_widget, SIGNAL(switchToFile(const QString&, int)),this,SLOT(slotSwitchToFile(const QString&, int)));

  // connect the windowsmenu with a method
  //  connect(menu_buffers,SIGNAL(activated(int)),this,SLOT(slotMenuBuffersSelected(int)));

  // m_docViewManager->connectBMPopup();

/*
	  connect(doc_bookmarks, SIGNAL(activated(int)), this, SLOT(slotBookmarksBrowserSelected(int)));
*/

  connect(grep_dlg,SIGNAL(itemSelected(QString,int)),this,SLOT(slotGrepDialogItemSelected(QString,int)));
  // connect ctag search dialog signal
  // connect(ctags_dlg,SIGNAL(gotoTag(const CTag*)),this,SLOT(slotTagGotoFile(const CTag*)));
  connect(ctags_dlg,SIGNAL(switchToFile(const QString&, int)),this,SLOT(slotSwitchToFile(const QString&, int)));

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
  CProject* pProj = initProject(ignoreLastProject);
  QApplication::sendPostedEvents();

  if (pProj != 0L) {
    projectOpenCmdl_Part2(pProj);
  }

  config->setGroup("TipOfTheDay");
  if( !kapp->isRestored())
    slotHelpTipOfDay(false);

  // set the right default position for the MDI view taskbar
  config->setGroup("CKDevelop Toolbar QextMdiTaskBar");
  QString dockEdgeStr = config->readEntry("Position","Bottom");
  QMainWindow::ToolBarDock taskBarEdge = Bottom;
  if (dockEdgeStr == "Top")
    taskBarEdge = Top;
  else if (dockEdgeStr == "Left")
    taskBarEdge = Left;
  else if (dockEdgeStr == "Right")
    taskBarEdge = Right;
  moveToolBar( m_pTaskBar, taskBarEdge);
}

CProject* CKDevelop::initProject(bool ignoreLastProject)
{
  CProject* pProj = 0L;

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
      bLastProject = false;
    }

    if (bLastProject)
    {
      config->setGroup("Files");
      filename = config->readEntry("project_file","");
      pProj = projectOpenCmdl_Part1(filename);
    }
  }
  return pProj;
}

void CKDevelop::setKeyAccel()
{

  accel->setItemEnabled("Dialog Editor", true );


//  accel->changeMenuAccel(file_menu, ID_FILE_NEW, KStdAccel::New );
//  accel->changeMenuAccel(file_menu, ID_FILE_OPEN, KStdAccel::Open );
//  accel->changeMenuAccel(file_menu, ID_FILE_CLOSE, KStdAccel::Close );
//  accel->changeMenuAccel(file_menu, ID_FILE_SAVE, KStdAccel::Save );
//  accel->changeMenuAccel(edit_menu, ID_FILE_SAVE_ALL,"SaveAll" );
//  accel->changeMenuAccel(edit_menu, ID_FILE_SAVE_AS,"SaveAs" );
//  accel->changeMenuAccel(file_menu, ID_FILE_PRINT, KStdAccel::Print );
//  accel->changeMenuAccel(file_menu, ID_FILE_QUIT, KStdAccel::Quit );

//  accel->changeMenuAccel(edit_menu, ID_EDIT_UNDO, KStdAccel::Undo );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_REDO,"Redo" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_CUT, KStdAccel::Cut );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_COPY, KStdAccel::Copy );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_PASTE, KStdAccel::Paste );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_SEARCH, KStdAccel::Find );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_REPEAT_SEARCH,"RepeatSearch" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_REPEAT_SEARCH_BACK,"RepeatSearchBack" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_REPLACE,KStdAccel::Replace );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_SEARCH_IN_FILES,"Grep" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_TAGS_SEARCH,"CTagsSearch" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_TAGS_SWITCH,"CTagsSwitch" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_INDENT,"Indent" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_UNINDENT,"Unindent" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_COMMENT,"Comment" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_UNCOMMENT,"Uncomment" );
//  accel->changeMenuAccel(edit_menu, ID_EDIT_SELECT_ALL, "SelectAll");


//  accel->changeMenuAccel(view_menu,ID_TOOLS_DESIGNER ,"Dialog Editor" );
//  accel->changeMenuAccel(view_menu,ID_VIEW_GOTO_LINE ,"GotoLine" );
//  accel->changeMenuAccel(view_menu,ID_VIEW_NEXT_ERROR ,"NextError" );
//  accel->changeMenuAccel(view_menu,ID_VIEW_PREVIOUS_ERROR ,"PreviousError" );
//  accel->changeMenuAccel(view_menu,ID_VIEW_TREEVIEW ,"Tree-View" );
//  accel->changeMenuAccel(view_menu,ID_VIEW_OUTPUTVIEW,"Output-View" );
//
//  accel->changeMenuAccel(project_menu, ID_PROJECT_KAPPWIZARD,"NewProject" );
//  accel->changeMenuAccel(project_menu, ID_PROJECT_OPEN,"OpenProject" );
//  accel->changeMenuAccel(project_menu, ID_PROJECT_CLOSE,"CloseProject" );
//  accel->changeMenuAccel(project_menu,ID_PROJECT_NEW_CLASS , "NewClass");
//  accel->changeMenuAccel(project_menu, ID_PROJECT_ADD_FILE_EXIST, "AddExistingFiles");
//  accel->changeMenuAccel(project_menu,ID_PROJECT_ADD_NEW_TRANSLATION_FILE ,"Add new Translation File" );
//  accel->changeMenuAccel(project_menu,ID_PROJECT_FILE_PROPERTIES ,"FileProperties" );
//  accel->changeMenuAccel(project_menu,ID_PROJECT_MESSAGES , "MakeMessages");
//  accel->changeMenuAccel(project_menu, ID_PROJECT_MAKE_PROJECT_API,"ProjectAPI" );
//  accel->changeMenuAccel(project_menu,ID_PROJECT_MAKE_USER_MANUAL , "ProjectManual");
////  accel->changeMenuAccel(project_menu, ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ,"Source-tgz" );
//  accel->changeMenuAccel(project_menu,ID_PROJECT_OPTIONS ,"ProjectOptions" );
//
//  accel->changeMenuAccel(build_menu,ID_BUILD_COMPILE_FILE ,"CompileFile" );
//  accel->changeMenuAccel(build_menu,ID_BUILD_MAKE ,"Make" );
//  accel->changeMenuAccel(build_menu,ID_BUILD_REBUILD_ALL , "RebuildAll");
//  accel->changeMenuAccel(build_menu,ID_BUILD_REBUILD_ALL ,"CleanRebuildAll" );
//  accel->changeMenuAccel(build_menu,ID_BUILD_STOP,"Stop_proc");
//  accel->changeMenuAccel(build_menu,ID_BUILD_RUN ,"Run" );
//  accel->changeMenuAccel(build_menu,ID_BUILD_RUN_WITH_ARGS,"Run_with_args");
//  accel->changeMenuAccel(build_menu,ID_BUILD_DISTCLEAN ,"BuildDistClean" );
//  accel->changeMenuAccel(build_menu, ID_BUILD_MAKECLEAN, "BuildMakeClean");
//  accel->changeMenuAccel(build_menu, ID_BUILD_AUTOCONF, "BuildAutoconf");
//  accel->changeMenuAccel(build_menu, ID_BUILD_CONFIGURE, "BuildConfigure");
//
//  accel->changeMenuAccel(classbrowser_popup, ID_CV_VIEW_DECLARATION, "CVGotoDeclaration");
//  accel->changeMenuAccel(classbrowser_popup, ID_CV_VIEW_DEFINITION, "CVGotoDefinition");
//  accel->changeMenuAccel(classbrowser_popup, ID_CV_VIEW_CLASS_DECLARATION, "CVGotoClass");
//  accel->changeMenuAccel(classbrowser_popup, ID_CV_GRAPHICAL_VIEW, "CVViewTree");
//  accel->changeMenuAccel(classbrowser_popup,ID_PROJECT_NEW_CLASS , "NewClass");
//
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_START , "DebugStart");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_RUN ,"DebugRun" );
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_RUN_CURSOR, "DebugRunCursor");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_STOP, "DebugStop");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_STEP, "DebugStepInto");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_STEP_INST, "DebugStepIntoInstr");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_NEXT, "DebugStepOver");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_NEXT_INST , "DebugStepOverInstr");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_FINISH ,  "DebugStepOut");
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_MEMVIEW ,"DebugViewer" );
//  accel->changeMenuAccel(debug_menu,ID_DEBUG_BREAK_INTO ,"DebugInterrupt" );
//
//  accel->changeMenuAccel(debugPopup, ID_DEBUG_CORE, "DebugExamineCore");
//  accel->changeMenuAccel(debugPopup, ID_DEBUG_NAMED_FILE, "DebugOtherExec");
//  accel->changeMenuAccel(debugPopup, ID_DEBUG_ATTACH, "DebugAttach");
//  accel->changeMenuAccel(debugPopup, ID_DEBUG_SET_ARGS, "DebugRunWithArgs");
//
//  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_TOGGLE ,"Toggle_Bookmarks" );
//  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_NEXT ,"Next_Bookmarks" );
//  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_PREVIOUS ,"Previous_Bookmarks" );
//  accel->changeMenuAccel(bookmarks_menu,ID_BOOKMARKS_CLEAR ,"Clear_Bookmarks" );
//
//  accel->changeMenuAccel(help_menu->menu(),ID_HELP_SEARCH_TEXT,"SearchMarkedText" );
//  accel->changeMenuAccel(help_menu->menu(), ID_HELP_SEARCH, "HelpSearch" );
//  accel->changeMenuAccel(help_menu->menu(), ID_HELP_CONTENTS, KStdAccel::Help );
//
//  accel->changeMenuAccel(help_menu->menu(),ID_HELP_PROJECT_API , "HelpProjectAPI" );
//  accel->changeMenuAccel(help_menu->menu(),ID_HELP_USER_MANUAL ,  "HelpProjectManual");

}

void CKDevelop::setToolmenuEntries(){

  CToolClass::readToolConfig(toolList);

  ToolAppList::Iterator it;
  int count = 0;
  for( it = toolList.begin(); it != toolList.end(); ++it ) {
    tools_menu->insertItem((*it).getLabel(),count++);
  }
	
//	connect(tools_menu,SIGNAL(activated(int)),SLOT(slotToolsTool(int)));
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
    brkptManager  = new BreakpointManager(0L, "BPManagerTab");
    frameStack    = new FrameStack(0L, "FStackTab");
    disassemble   = new Disassemble(0L, "DisassembleTab");
    brkptManager->setCaption(i18n("breakpoint"));
    addToolWindow(brkptManager, KDockWidget::DockBottom, messages_widget, 70, i18n("debugger breakpoints"), i18n("breakpoint"));
    frameStack->setCaption(i18n("call stack"));
    addToolWindow(frameStack, KDockWidget::DockBottom, messages_widget, 70, i18n("debugger function call stack"), i18n("call stack"));
    disassemble->setCaption(i18n("disassemble"));
    addToolWindow(disassemble, KDockWidget::DockBottom, messages_widget, 70, i18n("debugger disassemble view"), i18n("disassemble"));

    var_viewer    = new VarViewer(0L,"VARTab");

    brkptManager->setFocusPolicy(QWidget::ClickFocus);
    frameStack->setFocusPolicy(QWidget::ClickFocus);
    disassemble->setFocusPolicy(QWidget::ClickFocus);
    var_viewer->setFocusPolicy(QWidget::NoFocus);

    QString var_viewer_title = i18n("Watch");
    config->setGroup("General Options");
    int mode=config->readNumEntry("tabviewmode", 3);
    switch (mode){
      case 2:
        var_viewer_title = "";
      case 3:
        var_viewer->setIcon(SmallIcon("brace"));
    }
    var_viewer->setCaption(var_viewer_title);
    addToolWindow(var_viewer, KDockWidget::DockLeft, class_tree, 35, i18n("debugger variable watch view"), var_viewer_title);


#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
    dbg_widget = new COutputWidget(0L, "debuggerTab");
    dbg_widget->setCaption(i18n("debugger"));
    addToolWindow(dbg_widget, KDockWidget::DockBottom, messages_widget, 70, i18n("debugger control view"), i18n("debugger"));
    dbg_widget->setFocusPolicy(QWidget::ClickFocus);
    dbg_widget->setReadOnly(TRUE);
#endif

  	// Connect the breakpoint manager to monitor the bp setting - even
  	// when the debugging isn't running
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
    connect(  var_viewer->varTree(),  SIGNAL(selectFrame(int, int)),
              frameStack,             SLOT(slotSelectFrame(int, int)));
  }

  // Enable or disable the tabs, if they exist...
  if (var_viewer)
  {
    dockManager->findWidgetParentDock(var_viewer->parentWidget())->undock();
    dockManager->findWidgetParentDock(disassemble->parentWidget())->undock();
    dockManager->findWidgetParentDock(frameStack->parentWidget())->undock();
    brkptManager->setEnabled(dbgInternal);
    frameStack->setEnabled(dbgInternal && dbgController);
    disassemble->setEnabled(dbgInternal && dbgController);
    var_viewer->setEnabled(dbgInternal && dbgController);
#if defined(GDB_MONITOR) || defined(DBG_MONITOR)
    dockManager->findWidgetParentDock(dbg_widget->parentWidget())->undock();
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

