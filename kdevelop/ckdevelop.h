/***************************************************************************
                     ckdevelop.h - the mainclass in kdevelop   
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

#ifndef CKDEVELOP_H
#define CKDEVELOP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class CKDevelop;
#include <iostream.h>

#include <qlist.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qwhatsthis.h>
#include <qtimer.h>

#include <keditcl.h>
#include <kapp.h>
#include <ktmainwindow.h>
#include <ktreelist.h>
#include <kprocess.h>
#include <htmlview.h>
#include <htmltoken.h>
#include <kfm.h>
#include <kiconloader.h>
#include <knewpanner.h>
#include <kfiledialog.h>
#include <kmsgbox.h>
#include <kaccel.h>
#include <kprogress.h>

class CDocBrowser;
class CClassView;
class KSwallowWidget;
#include "ceditwidget.h"
#include "coutputwidget.h"
#include "ctabctl.h"
#include "crealfileview.h"
#include "clogfileview.h"
#include "cproject.h"
#include "cdoctree.h"
#include "structdef.h"
#include "resource.h"
#include "cprintdlg.h"



/** the mainclass in kdevelop
  *@author Sandy Meier
  */
class CKDevelop : public KTMainWindow {
  Q_OBJECT
public:
  /**constructor*/
  CKDevelop();
  /**destructor*/
  ~CKDevelop(){};
  void init();
  void initConnections();
  void initKeyAccel();
  void initMenu();
  void initToolbar();
  void initStatusBar();
  void initWhatsThis();
  void initProject();
  void refreshTrees();

  void enableCommand(int id_);
  void disableCommand(int id_);
  void newFile(bool add_to_project);
  /** read the projectfile from the disk*/
  bool readProjectFile(QString file);
  /**@param type HEADER,SOURCE,SCRIPT,DATA
   * return true if a new subdir was added to the project
   */
  bool addFileToProject(QString complete_filename,QString type,bool refreshTrees=true);
  void delFileFromProject(QString rel_filename);
  /**@param filename the absolute filename*/
  void switchToFile(QString filename); // filename = abs
  void switchToWorkspace(int id);
  int getTabLocation(QString filename);
  
  /** set the correct toolbar and menubar,if a process is running
    * @param enable if true than enable,otherwise disable
    */
  void setToolMenuProcess(bool enable);

 public slots:
  /** generate a new file*/
  void slotFileNew();
  /**open a file*/
  void slotFileOpenFile();
  /** save the current file,if Untitled a dialog ask for a valid name*/
  void slotFileSave();
  /** save all files*/
  void slotFileSaveAll();
  /** save the current file under a different filename*/
  void slotFileSaveAs();
  /** close the cuurent file*/
  void slotFileClose();
  void slotFileCloseAll();
  void slotFilePrint();
  /** quit kdevelop*/
  void slotFileQuit();

  void slotEditUndo();
  void slotEditRedo();
  void slotEditCut();
  void slotEditCopy();
  void slotEditPaste();
  void slotEditSelectAll();
  void slotEditInvertSelection();
  void slotEditDeselectAll();
  void slotEditInsertFile();
  void slotEditSearch();
  void slotEditRepeatSearch();
  void slotEditReplace();


  void slotViewGotoLine();
  void slotViewTTreeView();
  void slotViewTOutputView();
  void showTreeView(bool show=true);
  void showOutputView(bool show=true);
  void slotViewTStdToolbar();
  void slotViewTBrowserToolbar();
  void slotViewTStatusbar();
  /** refresh all trees and other widgets*/
  void slotViewRefresh();

 /** generate a new project with KAppWizard*/
  void slotProjectNewAppl();
  /** generat a new project file */
  void slotProjectNew();
  /** open a projectfile and close the old one*/
  void slotProjectOpen();
  /** close the current project,return false if  canceled*/
  bool slotProjectClose();
  void  slotProjectWorkspaces(int);
  void slotProjectAddNewFile();
  
  void slotProjectAddExistingFiles();
  void slotProjectRemoveFile();
  void slotProjectNewClass();
  void slotProjectFileProperties();
  void slotProjectOptions();

  /** compile the actual sourcefile using setted options */
  void slotBuildCompileFile();
  void slotBuildRun();
  void slotBuildDebug();
  void slotBuildMake();
  void slotBuildRebuildAll();
  void slotBuildCleanRebuildAll();
  void slotBuildDistClean();
  void slotBuildAutoconf();
  void slotBuildConfigure();
  void slotBuildStop();
  void slotBuildAPI();
  void slotBuildManual();

  void slotToolsKIconEdit();
  void slotToolsKDbg();
  void slotToolsKTranslator();

  void slotOptionsEditor();
  void slotOptionsEditorColors();
  void slotOptionsSyntaxHighlightingDefaults();
  void slotOptionsSyntaxHighlighting();
  void slotOptionsKeys();
  void slotOptionsConfigureEnscript();
  void slotOptionsConfigureA2ps();
  /** show a configure-dialog for kdevelop*/
  void slotOptionsKDevelop();
  void slotOptionsDocBrowser();
  void slotOptionsAutosave();
  void slotOptionsMake(int id);

  void slotDocBack();
  void slotDocForward();
  void slotDocSText();
  void slotDocSText(QString text);
  void slotDocQtLib();
  void slotDocKDECoreLib();
  void slotDocKDEGUILib();
  void slotDocKDEKFileLib();
  void slotDocKDEHTMLLib();
  void slotDocAPI();
  void slotDocManual();
  void slotDocUpdateKDEDocumentation();

  void slotHelpContent();
  void slotHelpHomepage();
  void slotHelpAbout();

  void slotToolbarClicked(int);
  void slotURLSelected(KHTMLView* widget,const char* url,int,const char*);

  void slotReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotReceivedStderr(KProcess* proc,char* buffer,int buflen);

  void slotApplReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotApplReceivedStderr(KProcess* proc,char* buffer,int buflen);
  

  void slotSearchReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotDocumentDone( KHTMLView *_view );
  void slotProcessExited(KProcess* proc);
  void slotSearchProcessExited(KProcess*);

  void slotLogFileTreeSelected(int index);
  void slotRealFileTreeSelected(int index);
  void slotDocTreeSelected(int index);
  void slotClassTreeSelected(int);

  void slotCVViewDeclaration(int index);
  void slotCVViewDefinition(int index);
  
  /**header,cpp,browser,tools*/
  void slotSTabSelected(int item);
  /**cv,lfv,wfv,doc*/
  void slotTTabSelected(int item);
  void slotSCurrentTab(int item);
  void slotMenuBuffersSelected(int id);
  void slotClickedOnMessagesWidget();
  void slotKeyPressedOnStdinStdoutWidget(int key);

  void slotClassChoiceCombo(int index);
  void slotMethodChoiceCombo(int index);

  /** change the status message to text */
  void slotStatusMsg(const char *text);
  /** change the status message of the whole statusbar temporary */
  void slotStatusHelpMsg(const char *text);
  /** switch argument for Statusbar help entries on slot selection */
  void statusCallback(int id_);
  /** change Statusbar status of INS and OVR */
  void slotNewStatus();
  /** change Statusbar status of Line and Column */
  void slotNewLineColumn();
  void slotNewUndo();
  
  
  // return the position of the classdeclaration begin
  int CVGotoClassDecl(QString classname);
  void CVGotoClassVarDecl(QString classname,QString var_name);
  void CVGotoMethodeImpl(QString classname,QString name);

  /** a tool meth,used in the search engine*/
  int searchToolGetNumber(QString str);
  QString searchToolGetTitle(QString str);
  QString searchToolGetURL(QString str);
  void  slotCreateSearchDatabase();
  void refreshClassCombos();
  void  saveCurrentWorkspaceIntoProject();
  
protected:
  virtual void closeEvent(QCloseEvent* e);
  //  void mousePressEvent(QMouseEvent* event);
 
private:
  //the menus
  QPopupMenu* file_menu;
  QPopupMenu* edit_menu;
  QPopupMenu* view_menu;
  QPopupMenu* project_menu;
  QPopupMenu* workspaces_submenu;
  QPopupMenu* build_menu;
  QPopupMenu* tools_menu;
  QPopupMenu* options_menu;
  QPopupMenu* make_submenu;
  QPopupMenu* menu_buffers;
  QPopupMenu* help_menu;
  QWhatsThis* whats_this;
  
  KNewPanner* view;
  KNewPanner* top_panner;
  
  bool beep; // set this to true, if you want a beep after a process,slotProcessExited()
  

  KIconLoader icon_loader;
  KProcess process; // for tools,compiler,make,kodc
  KProcess appl_process; //only for selfmade appl
  KShellProcess shell_process; // for kdoc,sgmltools ...
  KShellProcess search_process; // search with glimpse
  CProject* prj; // at the moment only one project at the same time

  KAccel *accel;
  KConfig* config;
  int act_outbuffer_len;

  // for the browser
  QStrList history_list;
  
  QList<TEditInfo> edit_infos;

  //some widgets for the mainview
  CTabCtl* s_tab_view; // the tabbar for the sourcescode und browser 
  CTabCtl* t_tab_view; // the tabbar for the trees
  CTabCtl* o_tab_view; // the tabbar for the output_widgets 

  CEditWidget* edit_widget; // a pointer to the actual editwidget
  CEditWidget* header_widget; // the editwidget for the headers/resources
  CEditWidget* cpp_widget;    //  the editwidget for cpp files
  CDocBrowser* browser_widget;
  KSwallowWidget* swallow_widget;
 
  CClassView* class_tree; // the classview
  CLogFileView* log_file_tree; // the logical filetree
  CRealFileView* real_file_tree; // the real filetree
  CDocTree* doc_tree; // the documentation tre
  
  COutputWidget* messages_widget; // output for the compiler ...
  COutputWidget* stdin_stdout_widget;
  COutputWidget* stderr_widget;

  int tree_view_pos;
  int output_view_pos;
  int workspace;

  QString version;
  bool project;
  bool bViewStatusbar;

  bool  prev_was_search_result;
  // Autosaving elements
  QTimer* saveTimer; // the timer
  bool bAutosave;    // enable=true
  int saveTimeout;   // timeout time

//  KProgress* statProg;
  //some vars for the searchengine
  QString search_output;
  QString doc_search_text;
  // for more then one job in proc;checked in slotProcessExited(KProcess* proc);
  // values are "run","make" "refresh";
  QString next_job;
  QString make_cmd;

  CConfigEnscriptDlg* enscriptconf;
  CConfigA2psDlg* a2psconf;

  enum {TOOLBAR_CLASS_CHOICE,TOOLBAR_METHOD_CHOICE};
};

#endif



















