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
  kdlg_caption="KDevelop Dialogeditor: Widget_1.kdevdlg";

  kdlg_tabctl= new CTabCtl(top_panner);
  kdlg_tabctl->setFocusPolicy(QWidget::ClickFocus);

  kdlg_widgets_view= new KDlgWidgets(this,kdlg_tabctl,"widgets_view");
  kdlg_dialogs_view = new KDlgDialogs(kdlg_tabctl,"dialogs_view");
  kdlg_items_view = new KDlgItems(this,kdlg_tabctl,"items_view");
  kdlgedit=new KDlgEdit(this,"dialogeditor");

  kdlg_tabctl->addTab(kdlg_widgets_view,i18n("Widgets"));
  kdlg_tabctl->addTab(kdlg_dialogs_view,i18n("Dialogs"));
  kdlg_tabctl->addTab(kdlg_items_view,i18n("Items"));

  kdlg_top_panner = new KNewPanner(top_panner,"kdlg_top_panner",KNewPanner::Vertical,KNewPanner::Percent,
  			      config->readNumEntry("kdlg_top_panner_pos", 80));

  kdlg_prop_widget = new KDlgPropWidget(this,kdlg_top_panner,"KDlg_properties_widget"); // the properties window of kdlg
  kdlg_edit_widget = new KDlgEditWidget(this,kdlg_top_panner,"KDlg_edit_widget"); // the editing view of kdlg

  kdlg_top_panner->showLabels(true);
  kdlg_top_panner->setLabels(i18n("Widget Editor"),i18n("Widget Properties"));

  kdlg_top_panner->activate(kdlg_edit_widget,kdlg_prop_widget);// activate the top_panner

  initKDlgKeyAccel();
  initKDlgMenuBar();
  initKDlgToolBar();
  initKDlgStatusBar();
}

void CKDevelop::initKDlgMenuBar(){
  kdlg_menubar=new KMenuBar(this,"KDlg_menubar");
  QPixmap pix;
  QString  path;

  ///////////////////////////////////////////////////////////////////
  // File-menu entries
  kdlg_file_menu= new QPopupMenu;
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/newwidget.xpm");
  kdlg_file_menu->insertItem(pix,i18n("&New Dialog"), kdlgedit, SLOT(slotFileNew()), 0,ID_KDLG_FILE_NEW);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  kdlg_file_menu->insertItem(pix, i18n("&Open..."), kdlgedit, SLOT(slotFileOpen()), 0, ID_KDLG_FILE_OPEN);
  kdlg_file_menu->insertItem(i18n("&Close"), kdlgedit, SLOT(slotFileClose()),0, ID_KDLG_FILE_CLOSE);
  kdlg_file_menu->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save.xpm");
  kdlg_file_menu->insertItem(pix, i18n("&Save"), kdlgedit, SLOT(slotFileSave()),0, ID_KDLG_FILE_SAVE);
  kdlg_file_menu->insertItem(i18n("Save &As..."), this, SLOT(slotFileSaveAs()),0 ,ID_FILE_SAVE_AS);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/saveall.xpm");
  kdlg_file_menu->insertItem(pix,i18n("Save All"), this, SLOT(slotFileSaveAll()),0,ID_FILE_SAVE_ALL);
  kdlg_file_menu->insertSeparator();
  kdlg_file_menu->insertItem(i18n("&Quit"), this, SLOT(slotFileQuit()), 0, ID_FILE_QUIT);

  kdlg_menubar->insertItem(i18n("&File"), kdlg_file_menu);

  ///////////////////////////////////////////////////////////////////
  // Edit-menu entries

  kdlg_edit_menu= new QPopupMenu;
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/undo.xpm");
  kdlg_edit_menu->insertItem(pix, i18n("U&ndo"), kdlgedit, SLOT(slotEditUndo()), 0, ID_KDLG_EDIT_UNDO);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/redo.xpm");
  kdlg_edit_menu->insertItem(pix, i18n("R&edo"), kdlgedit, SLOT(slotEditRedo()), 0, ID_KDLG_EDIT_REDO);
  kdlg_edit_menu->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/cut.xpm");
  kdlg_edit_menu->insertItem(pix, i18n("C&ut"), kdlgedit, SLOT(slotEditCut()), 0, ID_KDLG_EDIT_CUT);
  pix.load(KApplication::kde_toolbardir() + "/delete.xpm");
  kdlg_edit_menu->insertItem(pix, i18n("&Delete"), kdlgedit, SLOT(slotEditDelete()), 0, ID_KDLG_EDIT_DELETE);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/copy.xpm");
  kdlg_edit_menu->insertItem(pix, i18n("&Copy"), kdlgedit, SLOT(slotEditCopy()), 0, ID_KDLG_EDIT_COPY);
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/paste.xpm");
  kdlg_edit_menu->insertItem(pix, i18n("&Paste"), kdlgedit, SLOT(slotEditPaste()), 0, ID_KDLG_EDIT_PASTE);
  kdlg_edit_menu->insertSeparator();
  kdlg_edit_menu->insertItem(i18n("&Properties"), kdlgedit, SLOT(slotEditProperties()), 0, ID_KDLG_EDIT_PROPERTIES);
  kdlg_menubar->insertItem(i18n("&Edit"), kdlg_edit_menu);

  ///////////////////////////////////////////////////////////////////
  // View-menu entries
  kdlg_view_menu= new QPopupMenu;
  kdlg_view_menu->insertItem(i18n("&Widgets-View"),this, SLOT(slotViewTTreeView()),0,ID_VIEW_TREEVIEW);
  if(view_menu->isItemChecked(ID_VIEW_TREEVIEW))
    kdlg_view_menu->setItemChecked(ID_VIEW_TREEVIEW,true);

  kdlg_view_menu->insertItem(i18n("&Output-View"),this, SLOT(slotViewTOutputView()),0,ID_VIEW_OUTPUTVIEW);
  if(view_menu->isItemChecked(ID_VIEW_OUTPUTVIEW))
    kdlg_view_menu->setItemChecked(ID_VIEW_OUTPUTVIEW,true);

  config->setGroup("General Options");
  kdlg_view_menu->insertItem(i18n("&Properties-View"), this, SLOT(slotKDlgViewPropView()), 0, ID_KDLG_VIEW_PROPVIEW);
  kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_PROPVIEW,config->readBoolEntry("show_properties_view",true));

  kdlg_view_menu->insertSeparator();
  kdlg_view_menu->insertItem(i18n("&Toolbar"), this, SLOT(slotKDlgViewToolbar()),0,ID_KDLG_VIEW_TOOLBAR);
  kdlg_view_menu->setItemChecked(ID_KDLG_VIEW_TOOLBAR,config->readBoolEntry("show_kdlg_toolbar", true));

  kdlg_view_menu->insertItem(i18n("Status&bar"),this, SLOT(slotViewTStatusbar()),0,ID_VIEW_STATUSBAR);
  if(view_menu->isItemChecked(ID_VIEW_STATUSBAR))
    kdlg_view_menu->setItemChecked(ID_VIEW_STATUSBAR,true);

  kdlg_view_menu->insertSeparator();
  kdlg_view_menu->insertItem(Icon("reload.xpm"),i18n("&Refresh"),kdlgedit,
			   SLOT(slotViewRefresh()),0,ID_KDLG_VIEW_REFRESH);

  kdlg_view_menu->insertSeparator();
  kdlg_view_menu->insertItem(i18n("&Grid..."),kdlgedit,
			   SLOT(slotViewGrid()),0,ID_KDLG_VIEW_GRID);

  kdlg_menubar->insertItem(i18n("&View"), kdlg_view_menu);

  ///////////////////////////////////////////////////////////////////
  // Project-menu entries

  // submenu for adding projectfiles
  QPopupMenu* kdlg_p2 = new QPopupMenu;
  kdlg_p2->insertItem(i18n("&New File..."), this, SLOT(slotProjectAddNewFile()),0,ID_PROJECT_ADD_FILE_NEW);
  kdlg_p2->insertItem(i18n("&Existing File(s)..."), this,
		 SLOT(slotProjectAddExistingFiles()),0,ID_PROJECT_ADD_FILE_EXIST);

  // project-menu
  kdlg_project_menu = new QPopupMenu;
  kdlg_project_menu->insertItem(i18n("Application Wizard..."), this, SLOT(slotProjectNewAppl()),0,ID_PROJECT_KAPPWIZARD);
  kdlg_project_menu->insertItem(i18n("New"), this, SLOT(slotProjectNew()),0, ID_PROJECT_NEW);
  kdlg_project_menu->insertItem(i18n("&Open..."), this, SLOT(slotProjectOpen()),0,ID_PROJECT_OPEN);
  kdlg_project_menu->insertItem(i18n("C&lose"),this, SLOT(slotProjectClose()),0,ID_PROJECT_CLOSE);

  kdlg_project_menu->insertSeparator();
  kdlg_project_menu->insertItem(i18n("&New Class..."), this,
			   SLOT(slotProjectNewClass()),0,ID_PROJECT_NEW_CLASS);
  kdlg_project_menu->insertItem(i18n("&Add File(s) to Project"),kdlg_p2,ID_PROJECT_ADD_FILE);
  //  kdlg_project_menu->insertItem(i18n("&Remove File from Project"), this,
  //			   SLOT(slotProjectRemoveFile()),0,ID_PROJECT_REMOVE_FILE);

  kdlg_project_menu->insertItem(i18n("Add new &Translation File..."), this,
			   SLOT(slotProjectAddNewTranslationFile()),0,ID_PROJECT_ADD_NEW_TRANSLATION_FILE);

  kdlg_project_menu->insertSeparator();		
  kdlg_project_menu->insertItem(i18n("&File Properties..."), this, SLOT(slotProjectFileProperties())
			   ,0,ID_PROJECT_FILE_PROPERTIES);
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
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/generate.xpm");
  kdlg_build_menu->insertItem(pix,i18n("&Generate Sources..."),kdlgedit,SLOT(slotBuildGenerate()),0,ID_KDLG_BUILD_GENERATE);
  kdlg_build_menu->insertSeparator();
  kdlg_build_menu->insertItem(Icon("make.xpm"),i18n("&Make"),this,
			 SLOT(slotBuildMake()),0,ID_BUILD_MAKE);

  kdlg_build_menu->insertItem(Icon("rebuild.xpm"),i18n("&Rebuild all"), this,
			 SLOT(slotBuildRebuildAll()),0,ID_BUILD_REBUILD_ALL);

  kdlg_build_menu->insertItem(i18n("&Clean/Rebuild all"), this,
			 SLOT(slotBuildCleanRebuildAll()),0,ID_BUILD_CLEAN_REBUILD_ALL);
  kdlg_build_menu->insertSeparator();
  kdlg_build_menu->insertItem(Icon("stop.xpm"),i18n("&Stop Build"), this, SLOT(slotBuildStop()),0,ID_BUILD_STOP);
  kdlg_build_menu->insertSeparator();

  kdlg_build_menu->insertItem(Icon("run.xpm"),i18n("&Execute  "),this,SLOT(slotBuildRun()),0,ID_BUILD_RUN);
	kdlg_build_menu->insertItem(Icon("run.xpm"),i18n("Execute &with Arguments"),this,SLOT(slotBuildRunWithArgs()),0,ID_BUILD_RUN_WITH_ARGS);

  kdlg_build_menu->insertItem(Icon("debugger.xpm"),i18n("&Debug..."),this,SLOT(slotBuildDebug()),0,ID_BUILD_DEBUG);
  kdlg_build_menu->insertSeparator();
  kdlg_build_menu->insertItem(i18n("&DistClean"),this,SLOT(slotBuildDistClean()),0,ID_BUILD_DISTCLEAN);
  kdlg_build_menu->insertItem(i18n("&Autoconf"),this,SLOT(slotBuildAutoconf()),0,ID_BUILD_AUTOCONF);
  kdlg_build_menu->insertItem(i18n("C&onfigure"), this, SLOT(slotBuildConfigure()),0,ID_BUILD_CONFIGURE);
  kdlg_build_menu->insertSeparator();
	kdlg_build_menu->insertItem(i18n("Execution &arguments"),this,SLOT(slotBuildSetExecuteArgs()),0,ID_BUILD_SET_ARGS);
	kdlg_build_menu->insertItem(i18n("Make &messages"), this, SLOT(slotBuildMessages()),0, ID_BUILD_MESSAGES);
  kdlg_build_menu->insertItem(i18n("Make &API-Doc"), this,
			 SLOT(slotBuildAPI()),0,ID_BUILD_MAKE_PROJECT_API);
  kdlg_build_menu->insertItem(i18n("Make &User-Manual"), this,
			 SLOT(slotBuildManual()),0,ID_BUILD_MAKE_USER_MANUAL);

  kdlg_menubar->insertItem(i18n("&Build"), kdlg_build_menu);

  ///////////////////////////////////////////////////////////////////
  // Tools-menu entries
  kdlg_tools_menu= new QPopupMenu;
  kdlg_tools_menu->insertItem(i18n("&KDevelop"),this,SLOT(switchToKDevelop()),0,ID_KDLG_TOOLS_KDEVELOP);
  kdlg_tools_menu->insertItem(i18n("K&Dbg"),this, SLOT(slotToolsKDbg()),0,ID_TOOLS_KDBG);
  kdlg_tools_menu->insertItem(i18n("K&Iconedit"),this, SLOT(slotToolsKIconEdit()),0,ID_TOOLS_KICONEDIT);
  kdlg_tools_menu->insertItem(i18n("K&Translator"),this, SLOT(slotToolsKTranslator()),0,ID_TOOLS_KTRANSLATOR);
  kdlg_menubar->insertItem(i18n("&Tools"), kdlg_tools_menu);


  kdlg_options_menu = new QPopupMenu;
  kdlg_options_menu->insertItem(i18n("Documentation &Browser..."),this,
			   SLOT(slotOptionsDocBrowser()),0,ID_OPTIONS_DOCBROWSER);
  kdlg_options_menu->insertSeparator();
  kdlg_options_menu->insertItem(i18n("&KDevelop Setup..."),this,
			   SLOT(slotOptionsKDevelop()),0,ID_OPTIONS_KDEVELOP);
  kdlg_menubar->insertItem(i18n("&Options"), kdlg_options_menu);

  kdlg_help_menu=new QPopupMenu;
  kdlg_help_menu->insertItem(i18n("Back"),this, SLOT(slotHelpBack()),0,ID_HELP_BACK);
  kdlg_help_menu->insertItem(i18n("Forward"),this, SLOT(slotHelpForward()),0,ID_HELP_FORWARD);
  kdlg_help_menu->insertSeparator();
  kdlg_help_menu->insertItem(Icon("lookup.xpm"),i18n("&Search Marked Text"),this,
				 SLOT(slotHelpSearchText()),0,ID_HELP_SEARCH_TEXT);
  kdlg_help_menu->insertItem(Icon("contents.xpm"),i18n("Search for Help on..."),this,SLOT(slotHelpSearch()),0,ID_HELP_SEARCH);
  kdlg_help_menu->insertSeparator();
  kdlg_help_menu->insertItem(Icon("mini/kdehelp.xpm"),i18n("Contents"),this,SLOT(slotHelpContents()),0 ,ID_HELP_CONTENTS);

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
  kdlg_help_menu->insertItem(i18n("About KDevelop..."),this, SLOT(slotHelpAbout()),0,ID_HELP_ABOUT);
  kdlg_menubar->insertItem(i18n("&Help"),kdlg_help_menu);

///////////////////////////////////////////////////////////////////
// connects for the statusbar help
  connect(kdlg_file_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_edit_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_view_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_p2,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_project_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_build_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_tools_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_options_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));
  connect(kdlg_help_menu,SIGNAL(highlighted(int)), SLOT(statusCallback(int)));

  disableCommand(ID_FILE_NEW);
  disableCommand(ID_FILE_PRINT);

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

  disableCommand(ID_PROJECT_ADD_FILE_NEW);
  disableCommand(ID_PROJECT_CLOSE);
  disableCommand(ID_PROJECT_ADD_FILE_EXIST);
  disableCommand(ID_PROJECT_ADD_FILE);
  //  disableCommand(ID_PROJECT_REMOVE_FILE);
  disableCommand(ID_PROJECT_NEW_CLASS);
  disableCommand(ID_PROJECT_ADD_NEW_TRANSLATION_FILE);
  disableCommand(ID_PROJECT_FILE_PROPERTIES);
  disableCommand(ID_PROJECT_OPTIONS);
  disableCommand(ID_PROJECT_WORKSPACES);

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
  disableCommand(ID_BUILD_MESSAGES);  	
  disableCommand(ID_BUILD_MAKE_PROJECT_API);
  disableCommand(ID_BUILD_MAKE_USER_MANUAL);
  disableCommand(ID_BUILD_COMPILE_FILE);
  disableCommand(ID_BUILD_SET_ARGS);

  disableCommand(ID_HELP_BACK);
  disableCommand(ID_HELP_FORWARD);
  disableCommand(ID_HELP_PROJECT_API);
  disableCommand(ID_HELP_USER_MANUAL);

}

void CKDevelop::initKDlgToolBar(){
  QPixmap pix;
  QString  path;

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/openprj.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_PROJECT_OPEN, true,i18n("Open Project"));
  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/newwidget.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_FILE_NEW,false,i18n("Create New Dialog"));

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/open.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_FILE_OPEN, true,i18n("Open Dialog"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/save.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_FILE_SAVE,true,i18n("Save Dialog"));

  QFrame *separatorLine= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,10,separatorLine);

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/undo.xpm");
	toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_EDIT_UNDO,false,i18n("Undo"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/redo.xpm");
	toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_EDIT_REDO,false,i18n("Redo"));

  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/cut.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_EDIT_CUT,true,i18n("Cut"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/copy.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_EDIT_COPY, true,i18n("Copy"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/paste.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_EDIT_PASTE, true,i18n("Paste"));

  QFrame *separatorLine1= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine1->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,20,separatorLine1);

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/generate.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_BUILD_GENERATE,false,i18n("Generate widget sources"));

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/make.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_BUILD_MAKE, false,i18n("Make"));
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/rebuild.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_BUILD_REBUILD_ALL, false,i18n("Rebuild"));
  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();
	
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/debugger.xpm");
	toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_BUILD_DEBUG, false, i18n("Debug program"));
  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();
	
  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/run.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_BUILD_RUN, false,i18n("Run"));

  toolBar(ID_KDLG_TOOLBAR)->insertSeparator();

  pix.load(KApplication::kde_datadir() + "/kdevelop/toolbar/stop.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_BUILD_STOP, false,i18n("Stop"));

  QFrame *separatorLine2= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine2->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,30,separatorLine2);

  pix.load(KApplication::kde_icondir() + "/mini/kdevelop.xpm");
  toolBar(ID_KDLG_TOOLBAR)->insertButton(pix,ID_KDLG_TOOLS_KDEVELOP, true,i18n("Switch to edit-mode"));

  QFrame *separatorLine3= new QFrame(toolBar(ID_KDLG_TOOLBAR));
  separatorLine3->setFrameStyle(QFrame::VLine|QFrame::Sunken);
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(0,30,separatorLine3);

  QToolButton *btn_kdlg_what = whats_this->whatsThisButton(toolBar(ID_KDLG_TOOLBAR));
  QToolTip::add(btn_kdlg_what, i18n("What's this...?"));
  toolBar(ID_KDLG_TOOLBAR)->insertWidget(ID_HELP_WHATS_THIS, btn_kdlg_what->sizeHint().width(), btn_kdlg_what);
  btn_kdlg_what->setFocusPolicy(QWidget::NoFocus);

  connect(toolBar(ID_KDLG_TOOLBAR), SIGNAL(clicked(int)), SLOT(slotToolbarClicked(int)));
  config->setGroup("General Options");
  if(config->readBoolEntry("show_kdlg_toolbar", true)){
    enableToolBar(KToolBar::Show,ID_KDLG_TOOLBAR);
  }
  else{
    enableToolBar(KToolBar::Hide,ID_KDLG_TOOLBAR);
  }


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

  kdlg_statusbar->insertItem(i18n("Widget_1"), ID_KDLG_STATUS_WIDGET);

  kdlg_statusbar->insertItem(i18n("Welcome to KDevelop!"), ID_STATUS_MSG);
  kdlg_statusbar->setInsertOrder(KStatusBar::RightToLeft);

  if(config->readBoolEntry("show_kdlg_statusbar", true)){
    kdlg_statusbar->show();
  }
  else{
    kdlg_statusbar->hide();
  }
}


void CKDevelop::initKDlgKeyAccel(){
  accel->connectItem( KAccel::Open , kdlgedit, SLOT(slotFileOpen()) );
  accel->connectItem( KAccel::Close , kdlgedit, SLOT(slotFileClose()) );
  accel->connectItem( KAccel::Save , kdlgedit, SLOT(slotFileSave()) );

  //edit menu
  accel->connectItem( KAccel::Undo , kdlgedit, SLOT(slotEditUndo()) );
  accel->connectItem( KAccel::Cut , kdlgedit, SLOT(slotEditCut()) );
  accel->connectItem( KAccel::Copy , kdlgedit, SLOT(slotEditCopy()) );
  accel->connectItem( KAccel::Paste , kdlgedit, SLOT(slotEditPaste()) );

  // tools-menu
  accel->connectItem("KDevKDlg",this,SLOT(switchToKDevelop()) );

  accel->readSettings();
}

void CKDevelop::setKDlgCaption(){
  setCaption(kdlg_caption);
}











































































































































