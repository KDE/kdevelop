/***************************************************************************
                          ckdevelop_kdlgedinit.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


// This file contains the construction of the kdialogedit views including menubar and toolbar
#include <qtoolbutton.h>
#include "ckdevelop.h"

#include "./kdlgedit/kdlgedit.h"
#include "./kdlgedit/kdlgeditwidget.h"
#include "./kdlgedit/kdlgpropwidget.h"
#include "./kdlgedit/kdlgwidgets.h"
#include "./kdlgedit/kdlgdialogs.h"
#include "./kdlgedit/kdlgitems.h"


void CKDevelop::initKDlg(){
  kdlg_caption="KDevelop Dialogeditor:";

  kdlg_tabctl= new CTabCtl(top_panner);
  kdlg_tabctl->setFocusPolicy(QWidget::ClickFocus);

  kdlg_widgets_view= new KDlgWidgets(this,kdlg_tabctl,"widgets_view");
  kdlg_dialogs_view = new KDlgDialogs(kdlg_tabctl,"dialogs_view");
  kdlg_items_view = new KDlgItems(this,kdlg_tabctl,"items_view");
  kdlgedit=new KDlgEdit(this,"dialogeditor");

  kdlg_tabctl->addTab(kdlg_widgets_view,i18n("Widgets"));
  kdlg_tabctl->addTab(kdlg_dialogs_view,i18n("Dialogs"));
  kdlg_tabctl->addTab(kdlg_items_view,i18n("Items"));

  kdlg_top_panner = new KNewPanner(top_panner,"kdlg_top_panner",KNewPanner::Vertical);

  kdlg_prop_widget = new KDlgPropWidget(this,kdlg_top_panner,"KDlg_properties_widget"); // the properties window of kdlg
  kdlg_edit_widget = new KDlgEditWidget(this,kdlg_top_panner,"KDlg_edit_widget"); // the editing view of kdlg

  kdlg_top_panner->showLabels(true);
  kdlg_top_panner->setLabels(i18n("Widget Editor"),i18n("Widget Properties"));

  kdlg_top_panner->activate(kdlg_edit_widget,kdlg_prop_widget);// activate the top_panner

  kdlg_edit_widget->hide();
  kdlg_prop_widget->hide();
  kdlg_tabctl->setTabEnabled("widgets_view",false);
  kdlg_tabctl->setTabEnabled("items_view",false);
  kdlg_tabctl->setTabEnabled("dialogs_view",false);
  kdlg_tabctl->setCurrentTab(1);// dialogs
  
  initKDlgMenuBar();
//  initKDlgKeyAccel();
  initKDlgToolBar();
  
  initKDlgStatusBar();

  connect(kdlg_dialogs_view,SIGNAL(newDialog()),SLOT(slotFileNew()));
}

void CKDevelop::initKDlgMenuBar(){
  kdlg_menubar=new KMenuBar(this,"KDlg_menubar");
  QPixmap pix;
  QString  path;

  ///////////////////////////////////////////////////////////////////
  // File-menu entries
  kdlg_file_menu= new QPopupMenu;
  kdlg_file_menu->insertItem(Icon("newwidget.xpm"),i18n("&New..."), this, SLOT(slotFileNew()), 0,ID_FILE_NEW);
  kdlg_file_menu->insertItem(Icon("open.xpm"), i18n("&Open..."), this, SLOT(slotFileOpen()), 0, ID_FILE_OPEN);
  kdlg_file_menu->insertItem(i18n("&Close"), kdlgedit, SLOT(slotFileClose()),0, ID_KDLG_FILE_CLOSE);
  kdlg_file_menu->insertSeparator();
  kdlg_file_menu->insertItem(Icon("save.xpm"), i18n("&Save"), kdlgedit, SLOT(slotFileSave()),0, ID_KDLG_FILE_SAVE);
  kdlg_file_menu->insertItem(i18n("Save &As..."), kdlgedit, SLOT(slotFileSaveAs()),0 ,ID_KDLG_FILE_SAVE_AS);
  kdlg_file_menu->insertSeparator();
  kdlg_file_menu->insertItem(i18n("E&xit"), this, SLOT(slotFileQuit()), 0, ID_FILE_QUIT);

  kdlg_menubar->insertItem(i18n("&File"), kdlg_file_menu);

  ///////////////////////////////////////////////////////////////////
  // Edit-menu entries

  kdlg_edit_menu= new QPopupMenu;
  kdlg_edit_menu->insertItem(Icon("undo.xpm"), i18n("U&ndo"), kdlgedit, SLOT(slotEditUndo()), 0, ID_KDLG_EDIT_UNDO);
  kdlg_edit_menu->insertItem(Icon("redo.xpm"), i18n("R&edo"), kdlgedit, SLOT(slotEditRedo()), 0, ID_KDLG_EDIT_REDO);
  kdlg_edit_menu->insertSeparator();
  kdlg_edit_menu->insertItem(Icon("cut.xpm"), i18n("C&ut"), kdlgedit, SLOT(slotEditCut()), 0, ID_KDLG_EDIT_CUT);
  kdlg_edit_menu->insertItem(Icon("copy.xpm"), i18n("&Copy"), kdlgedit, SLOT(slotEditCopy()), 0, ID_KDLG_EDIT_COPY);
  kdlg_edit_menu->insertItem(Icon("paste.xpm"), i18n("&Paste"), kdlgedit, SLOT(slotEditPaste()), 0, ID_KDLG_EDIT_PASTE);
  kdlg_edit_menu->insertSeparator();
  kdlg_edit_menu->insertItem(Icon("delete.xpm"), i18n("&Delete"), kdlgedit, SLOT(slotEditDelete()), 0, ID_KDLG_EDIT_DELETE);

  kdlg_menubar->insertItem(i18n("&Edit"), kdlg_edit_menu);

  ///////////////////////////////////////////////////////////////////
  // View-menu entries
  kdlg_view_menu= new QPopupMenu;
  kdlg_view_menu->insertItem(i18n("&Sourcecode Editor"),this,SLOT(switchToKDevelop()),0,ID_KDLG_TOOLS_KDEVELOP);
  kdlg_view_menu->insertSeparator();
  kdlg_view_menu->insertItem(i18n("&Widgets-View"),this, SLOT(slotViewTTreeView()),0,ID_VIEW_TREEVIEW);
  kdlg_view_menu->insertItem(i18n("&Output-View"),this, SLOT(slotViewTOutputView()),0,ID_VIEW_OUTPUTVIEW);
  kdlg_view_menu->insertItem(i18n("&Properties-View"), this, SLOT(slotKDlgViewPropView()), 0, ID_KDLG_VIEW_PROPVIEW);
  kdlg_view_menu->insertSeparator();
  kdlg_view_menu->insertItem(i18n("&Toolbar"), this, SLOT(slotKDlgViewToolbar()),0,ID_KDLG_VIEW_TOOLBAR);
  kdlg_view_menu->insertItem(i18n("Status&bar"),this, SLOT(slotViewTStatusbar()),0,ID_VIEW_STATUSBAR);
  kdlg_view_menu->insertSeparator();
  kdlg_view_menu->insertItem(Icon("reload.xpm"),i18n("&Refresh"),kdlgedit,
			   SLOT(slotViewRefresh()),0,ID_KDLG_VIEW_REFRESH);
  kdlg_view_menu->insertSeparator();
  kdlg_view_menu->insertItem(i18n("&Preview dialog"),kdlgedit,
			   SLOT(slotViewPreview()),0,ID_VIEW_PREVIEW);
  kdlg_view_menu->insertItem(i18n("&Grid..."),kdlgedit,
			   SLOT(slotViewGrid()),0,ID_KDLG_VIEW_GRID);

  kdlg_menubar->insertItem(i18n("&View"), kdlg_view_menu);

  ///////////////////////////////////////////////////////////////////
  // Project-menu entries
  kdlg_project_menu = new QPopupMenu;
  kdlg_project_menu->insertItem(i18n("New..."), this, SLOT(slotProjectNewAppl()),0,ID_PROJECT_KAPPWIZARD);
  kdlg_project_menu->insertItem(Icon("openprj.xpm"), i18n("&Open..."), this, SLOT(slotProjectOpen()),0,ID_PROJECT_OPEN);
  kdlg_project_menu->insertItem(i18n("C&lose"),this, SLOT(slotProjectClose()),0,ID_PROJECT_CLOSE);

  kdlg_project_menu->insertSeparator();
  kdlg_project_menu->insertItem(i18n("&New Class..."), this,
			   SLOT(slotProjectNewClass()),0,ID_PROJECT_NEW_CLASS);
  kdlg_project_menu->insertItem(i18n("&Add existing File(s)..."),this,SLOT(slotProjectAddExistingFiles()),0,ID_PROJECT_ADD_FILE_EXIST);
  //  kdlg_project_menu->insertItem(i18n("&Remove File from Project"), this,
  //			   SLOT(slotProjectRemoveFile()),0,ID_PROJECT_REMOVE_FILE);

  kdlg_project_menu->insertItem(Icon("mini/locale.xpm"),i18n("Add new &Translation File..."), this,
			   SLOT(slotProjectAddNewTranslationFile()),0,ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  kdlg_project_menu->insertItem(i18n("&File Properties..."), this, SLOT(slotProjectFileProperties())
			   ,0,ID_PROJECT_FILE_PROPERTIES);
  kdlg_project_menu->insertSeparator();	
  kdlg_project_menu->insertItem(i18n("Make &messages"), this, SLOT(slotProjectMessages()),0, ID_PROJECT_MESSAGES);
  kdlg_project_menu->insertItem(i18n("Make &API-Doc"), this,
				SLOT(slotProjectAPI()),0,ID_PROJECT_MAKE_PROJECT_API);
  kdlg_project_menu->insertItem(Icon("mini/mini-book1.xpm"),i18n("Make &User-Manual"), this,
				SLOT(slotProjectManual()),0,ID_PROJECT_MAKE_USER_MANUAL);
  // submenu for making dists

  QPopupMenu*  p2 = new QPopupMenu;
  p2->insertItem(i18n("&Source-tgz"), this, SLOT(slotProjectMakeDistSourceTgz()),0,ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ);
  kdlg_project_menu->insertItem(i18n("Make D&istribution"),p2,ID_PROJECT_MAKE_DISTRIBUTION);
  
  kdlg_project_menu->insertSeparator();	
  kdlg_project_menu->insertItem(i18n("&Options..."), this, SLOT(slotProjectOptions()),0,ID_PROJECT_OPTIONS);
  //  kdlg_project_menu->insertSeparator();		


  //  workspaces_submenu = new QPopupMenu;
  //workspaces_submenu->insertItem(i18n("Workspace 1"),ID_PROJECT_WORKSPACES_1);
  //  workspaces_submenu->insertItem(i18n("Workspace 2"),ID_PROJECT_WORKSPACES_2);
  //  workspaces_submenu->insertItem(i18n("Workspace 3"),ID_PROJECT_WORKSPACES_3);
  //  kdlg_project_menu->insertItem(i18n("Workspaces"),workspaces_submenu,ID_PROJECT_WORKSPACES);
  //  connect(workspaces_submenu, SIGNAL(activated(int)), SLOT(slotProjectWorkspaces(int)));
  
  
  kdlg_menubar->insertItem(i18n("&Project"), kdlg_project_menu);
  
///////////////////////////////////////////////////////////////////
// Build-menu entries

  kdlg_build_menu = new QPopupMenu;
  kdlg_build_menu->insertItem(Icon("generate.xpm"),i18n("&Generate Sources..."),kdlgedit,
			      SLOT(slotBuildGenerate()),0,ID_KDLG_BUILD_GENERATE);  	
  
  kdlg_build_menu->insertItem(Icon("generate.xpm"),i18n("&Generate Complete Sources..."),kdlgedit,
			      SLOT(slotBuildCompleteGenerate()),0,ID_KDLG_BUILD_COMPLETE_GENERATE);  
  kdlg_build_menu->insertSeparator(); 
  
  kdlg_build_menu->insertItem(Icon("make.xpm"),i18n("&Make"),this, 			 SLOT(slotBuildMake()),0,ID_BUILD_MAKE);
  kdlg_build_menu->insertItem(Icon("rebuild.xpm"),i18n("&Rebuild all"), this, 			 SLOT(slotBuildRebuildAll()),0,ID_BUILD_REBUILD_ALL);

  kdlg_build_menu->insertItem(i18n("&Clean/Rebuild all"), this,
			 SLOT(slotBuildCleanRebuildAll()),0,ID_BUILD_CLEAN_REBUILD_ALL);
  kdlg_build_menu->insertSeparator();
  kdlg_build_menu->insertItem(Icon("stop_proc.xpm"),i18n("&Stop Build"), this, SLOT(slotBuildStop()),0,ID_BUILD_STOP);
  kdlg_build_menu->insertSeparator();

  kdlg_build_menu->insertItem(Icon("run.xpm"),i18n("&Execute  "),this,SLOT(slotBuildRun()),0,ID_BUILD_RUN);
	kdlg_build_menu->insertItem(Icon("run.xpm"),i18n("Execute &with Arguments"),this,SLOT(slotBuildRunWithArgs()),0,ID_BUILD_RUN_WITH_ARGS);
  kdlg_build_menu->insertItem(Icon("debugger.xpm"),i18n("&Debug..."),this,SLOT(slotBuildDebug()),0,ID_DEBUG_START);

  kdlg_build_menu->insertSeparator();
  kdlg_build_menu->insertItem(i18n("&DistClean"),this,SLOT(slotBuildDistClean()),0,ID_BUILD_DISTCLEAN);
  kdlg_build_menu->insertItem(i18n("&Autoconf"),this,SLOT(slotBuildAutoconf()),0,ID_BUILD_AUTOCONF);
  kdlg_build_menu->insertItem(i18n("C&onfigure..."), this, SLOT(slotBuildConfigure()),0,ID_BUILD_CONFIGURE);
	
	

  kdlg_menubar->insertItem(i18n("&Build"), kdlg_build_menu);

  ///////////////////////////////////////////////////////////////////
  // Debug-menu entries - It's exactly the same as the other menu

  kdlg_menubar->insertItem(i18n("&Debug"), debug_menu);

  ///////////////////////////////////////////////////////////////////
  // Tools-menu entries
  kdlg_tools_menu= new QPopupMenu;
  kdlg_menubar->insertItem(i18n("&Tools"), kdlg_tools_menu);


  ///////////////////////////////////////////////////////////////////
  // Options-menu entries
  kdlg_options_menu = new QPopupMenu;
  kdlg_options_menu->insertItem(i18n("Documentation &Browser..."),this,
			   SLOT(slotOptionsDocBrowser()),0,ID_OPTIONS_DOCBROWSER);
  kdlg_options_menu->insertSeparator();
  kdlg_options_menu->insertItem(i18n("&KDevelop Setup..."),this,
			   SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);
  kdlg_menubar->insertItem(i18n("&Options"), kdlg_options_menu);

  kdlg_help_menu=new QPopupMenu;
  kdlg_help_menu->insertItem(Icon("back.xpm"),i18n("Back"),this, SLOT(slotHelpBack()),0,ID_HELP_BACK);
  kdlg_help_menu->insertItem(Icon("forward.xpm"),i18n("Forward"),this, SLOT(slotHelpForward()),0,ID_HELP_FORWARD);
  kdlg_help_menu->insertSeparator();
  kdlg_help_menu->insertItem(Icon("lookup.xpm"),i18n("&Search Marked Text"),this,
				 SLOT(slotHelpSearchText()),0,ID_HELP_SEARCH_TEXT);
  kdlg_help_menu->insertItem(Icon("contents.xpm"),i18n("Search for Help on..."),this,SLOT(slotHelpSearch()),0,ID_HELP_SEARCH);
  kdlg_help_menu->insertSeparator();
  kdlg_help_menu->insertItem(Icon("mini/kdehelp.xpm"),i18n("User Manual"),this,SLOT(slotHelpContents()),0 ,ID_HELP_CONTENTS);
  kdlg_help_menu->insertItem(Icon("mini/kdehelp.xpm"),i18n("Programming Handbook"),this,SLOT(slotHelpTutorial()),0 ,ID_HELP_TUTORIAL);
	kdlg_help_menu->insertItem(Icon("idea.xpm"),i18n("Tip of the Day"), this, SLOT(slotHelpTipOfDay()), 0, ID_HELP_TIP_OF_DAY);
  kdlg_help_menu->insertItem(i18n("KDevelop Homepage"),this, SLOT(slotHelpHomepage()),0,ID_HELP_HOMEPAGE);
  kdlg_help_menu->insertItem(Icon("filemail.xpm"),i18n("Bug Report..."),this, SLOT(slotHelpBugReport()),0,ID_HELP_BUG_REPORT);
  kdlg_help_menu->insertSeparator();
  kdlg_help_menu->insertItem(i18n("C/C++-Reference"),this,SLOT(slotHelpReference()),0,ID_HELP_REFERENCE);
  kdlg_help_menu->insertItem(Icon("mini/mini-book1.xpm"),i18n("&Qt-Library"),this, SLOT(slotHelpQtLib()),0,ID_HELP_QT_LIBRARY);
  kdlg_help_menu->insertItem(Icon("mini/mini-book1.xpm"),i18n("KDE-&Core-Library"),this,
				 SLOT(slotHelpKDECoreLib()),0,ID_HELP_KDE_CORE_LIBRARY);
  kdlg_help_menu->insertItem(Icon("mini/mini-book1.xpm"),i18n("KDE-&GUI-Library"),this,
				 SLOT(slotHelpKDEGUILib()),0,ID_HELP_KDE_GUI_LIBRARY);
  kdlg_help_menu->insertItem(Icon("mini/mini-book1.xpm"),i18n("KDE-&KFile-Library"),this,
				 SLOT(slotHelpKDEKFileLib()),0,ID_HELP_KDE_KFILE_LIBRARY);
  kdlg_help_menu->insertItem(Icon("mini/mini-book1.xpm"),i18n("KDE-&HTML-Library"),this,
				 SLOT(slotHelpKDEHTMLLib()),0,ID_HELP_KDE_HTML_LIBRARY);
  kdlg_help_menu->insertSeparator();
  kdlg_help_menu->insertItem(i18n("Project &API-Doc"),this,
				      SLOT(slotHelpAPI()),0,ID_HELP_PROJECT_API);

  kdlg_help_menu->insertItem(i18n("Project &User-Manual"),this,
				      SLOT(slotHelpManual()),0,ID_HELP_USER_MANUAL);
  //  kdlg_help_menu->insertItem(i18n("KDevelop Homepage"),this, SLOT(slotHelpHomepage()),0,ID_HELP_HOMEPAGE);
  kdlg_help_menu->insertSeparator();
  kdlg_help_menu->insertItem(i18n("Dialog editor notes..."),this, SLOT(slotHelpDlgNotes()),0,ID_HELP_DLGNOTES);
  kdlg_help_menu->insertItem(i18n("About KDevelop..."),this, SLOT(slotHelpAbout()),0,ID_HELP_ABOUT);
  kdlg_menubar->insertSeparator();
  kdlg_menubar->insertItem(i18n("&Help"),kdlg_help_menu);



///////////////////////////////////////////////////////////////////
// connects for the statusbar help
  connect(kdlg_file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_view_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_tools_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_options_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_help_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

  disableCommand(ID_FILE_NEW);
  disableCommand(ID_FILE_PRINT);
  disableCommand(ID_KDLG_FILE_CLOSE);
  disableCommand(ID_KDLG_FILE_SAVE);
  disableCommand(ID_KDLG_FILE_SAVE_AS);
 
  disableCommand(ID_VIEW_NEXT_ERROR);
  disableCommand(ID_VIEW_PREVIOUS_ERROR);

  disableCommand(ID_EDIT_UNDO);
  disableCommand(ID_EDIT_REDO);
  disableCommand(ID_KDLG_EDIT_UNDO);
  disableCommand(ID_KDLG_EDIT_REDO);
  disableCommand(ID_KDLG_EDIT_CUT);
  disableCommand(ID_KDLG_EDIT_COPY);
  disableCommand(ID_KDLG_EDIT_PASTE);
  disableCommand(ID_KDLG_EDIT_DELETE);

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
//  disableCommand(ID_BUILD_DEBUG);
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

}

void CKDevelop::initKDlgToolBar(){

  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("openprj.xpm"),ID_PROJECT_OPEN, true,i18n("Open Project"));
  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("newwidget.xpm"),ID_FILE_NEW,false,i18n("New..."));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("open.xpm"),ID_FILE_OPEN, true,i18n("Open..."));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("save.xpm"),ID_KDLG_FILE_SAVE,true,i18n("Save Dialog"));

  QFrame *separatorLine= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,20,separatorLine);

	toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("undo.xpm"),ID_KDLG_EDIT_UNDO,false,i18n("Undo"));
	toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("redo.xpm"),ID_KDLG_EDIT_REDO,false,i18n("Redo"));

  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("cut.xpm"),ID_KDLG_EDIT_CUT,true,i18n("Cut"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("copy.xpm"),ID_KDLG_EDIT_COPY, true,i18n("Copy"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("paste.xpm"),ID_KDLG_EDIT_PASTE, true,i18n("Paste"));

  QFrame *separatorLine1= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine1->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,20,separatorLine1);

  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("generate.xpm"),ID_KDLG_BUILD_GENERATE,false,i18n("Generate Sources"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("make.xpm"),ID_BUILD_MAKE, false,i18n("Make"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("rebuild.xpm"),ID_BUILD_REBUILD_ALL, false,i18n("Rebuild"));
  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();
	toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("debugger.xpm"),ID_DEBUG_START, false, i18n("Debug"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("run.xpm"),ID_BUILD_RUN, false,i18n("Run"));
  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("stop_proc.xpm"),ID_BUILD_STOP, false,i18n("Stop"));

  QFrame *sepDbgRun = new QFrame(toolBar(ID_KDLG_TOOLBAR));
  sepDbgRun->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,20,sepDbgRun);

  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("dbgrun.xpm"),ID_DEBUG_RUN, false, i18n("Continue with app execution. May start the app"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("dbgnext.xpm"),ID_DEBUG_NEXT, false,i18n("Execute one line of code, but run through functions"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("dbgstep.xpm"),ID_DEBUG_STEP, false,i18n("Execute one line of code, stepping into fn if appropriate"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("dbgstepout.xpm"),ID_DEBUG_FINISH, false,i18n("Execute to end of current stack frame"));

  QFrame *separatorLine2= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine2->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,20,separatorLine2);

	QPixmap pix;
  pix.load(KApplication::kde_icondir() + "/mini/kdevelop.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_TOOLS_KDEVELOP, true,i18n("Switch to edit-mode"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("tree_win.xpm"),ID_VIEW_TREEVIEW, true,i18n("Tree View"));
  toolBar(ID_KDLG_TOOLBAR)->insertButton(Icon("output_win.xpm"),ID_VIEW_OUTPUTVIEW, true,i18n("Output View"));
	toolBar(ID_KDLG_TOOLBAR)->setToggle(ID_VIEW_TREEVIEW);
	toolBar(ID_KDLG_TOOLBAR)->setToggle(ID_VIEW_OUTPUTVIEW);

  QFrame *separatorLine3= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine3->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,20,separatorLine3);

  QToolButton *btn_kdlg_what = whats_this->whatsThisButton(toolBar(ID_KDLG_TOOLBAR));
  QToolTip::add(btn_kdlg_what, i18n("What's this...?"));
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(ID_HELP_WHATS_THIS, btn_kdlg_what->sizeHint().width(), btn_kdlg_what);
  btn_kdlg_what->setFocusPolicy(QWidget::NoFocus);

  connect(toolBar(ID_KDLG_TOOLBAR), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  connect(toolBar(ID_KDLG_TOOLBAR), SIGNAL(pressed(int)), SLOT(statusCallback(int)));
}

void CKDevelop::initKDlgStatusBar(){
  kdlg_statusbar= new KStatusBar(this,"KDlgEdit_statusbar");

  kdlg_statusbar->insertItem(i18n("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"), ID_STATUS_EMPTY);
  kdlg_statusbar->changeItem("", ID_STATUS_EMPTY);
	
  kdlg_statusbar->insertItem(i18n("xxxxxxxxxxxxxx"), ID_KDLG_STATUS_WH);
  kdlg_statusbar->insertItem(i18n("xxxxxxxxxxxxxx"), ID_KDLG_STATUS_XY);
  kdlg_statusbar->insertItem(i18n("xxxxxxxxxxxxxxxxxxxxxxxxxxx"), ID_STATUS_EMPTY_2);
  kdlg_statusbar->changeItem("", ID_STATUS_EMPTY_2);
  kdlg_statusbar->changeItem("", ID_KDLG_STATUS_WH);
  kdlg_statusbar->changeItem("", ID_KDLG_STATUS_XY);

  kdlg_statusbar->insertItem(i18n("        "), ID_KDLG_STATUS_WIDGET);

  kdlg_statusbar->insertItem(i18n("Welcome to KDevelop!"), ID_STATUS_MSG);
  kdlg_statusbar->setInsertOrder(KStatusBar::RightToLeft);

  if(config->readBoolEntry("show_kdlg_statusbar", true)){
    kdlg_statusbar->show();
  }
  else{
    kdlg_statusbar->hide();
  }
}

void CKDevelop::setKDlgCaption()
{
  setCaption(kdlg_caption);
}

#include "./kdlgedit/kdlgreadmedlg.h"

void CKDevelop::slotHelpDlgNotes()
{
  KDlgReadmeDlg *readmedlg = new KDlgReadmeDlg(this);
  readmedlg->exec();

  config->setGroup("KDlgEdit");
  if (!readmedlg->isShowAgain())
    config->writeEntry("KDlgEdit_ShowReadme","false");
  else
    config->writeEntry("KDlgEdit_ShowReadme","true");

  delete readmedlg;
}











