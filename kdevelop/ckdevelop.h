/***************************************************************************
                     ckdevelop.h - the mainclass in kdevelop   
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

#ifndef CKDEVELOP_H
#define CKDEVELOP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class CKDevelop;
#include <keditcl.h>
#include <kapp.h>
#include <ktmainwindow.h>
#include <ktreelist.h>

#include <qstring.h>
#include <kprocess.h>
#include <htmlview.h>
#include <kfm.h>
#include <kiconloader.h>
#include <qlist.h>
#include <qstrlist.h>
#include <knewpanner.h>
#include <kfiledialog.h>

#include "cdocbrowser.h"
#include "structdef.h"
#include "ceditwidget.h"
#include "cnewfiledlg.h"
#include "coutputwidget.h"
#include "ctabctl.h"
#include "crealfileview.h"
#include "clogfileview.h"
#include "cproject.h"
#include "cclassview.h"
#include "kswallow.h"
#include "cdoctree.h"
#include "resource.h"


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
  void initMenu();
  void initToolbar();
  void initStatusBar();
  void initProject();
  void refreshTrees();

  void enableCommand(int id_);
  void disableCommand(int id_);
  void newFile(bool add_to_project);
  /** read the projectfile from the disk*/
  bool readProjectFile(QString file);
  /**@param type HEADER,SOURCE,SCRIPT,DATA*/
  void addFileToProject(QString complete_filename,QString type);
  void delFileFromProject(QString rel_filename);
  /**@param filename the absolute filename*/
  void switchToFile(QString filename); // filename = abs
  int getTabLocation(QString filename);
  
  

  /** set the correct toolbar and menubar,if a process is running
    * @param enable if true than enable,otherwise disable
    */
  void setToolMenuProcess(bool enable);



 public slots:
 /** generate a new project*/
 void slotFileNewAppl();
  /** generate a new file*/
  void slotFileNewFile();
  /**open a file*/
  void slotFileOpenFile();
  /** open a projectfile and close the old one*/
  void slotFileOpenPrj();
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
  void slotEditGotoLine();

  void slotOptionsTStdToolbar();
  void slotOptionsTBrowserToolbar();
  void slotOptionsTStatusbar();
  void slotOptionsTTreeView();
  void slotOptionsTOutputView();
  /** refresh all trees and other widgets*/
  void slotOptionsRefresh();
  void slotOptionsEditor();
  void slotOptionsEditorColors();
  void slotOptionsSyntaxHighlighting();
  /** show a configure-dialog for kdevelop*/
  void slotOptionsKDevelop();

  void slotToolsKIconEdit();
  void slotToolsKDbg();

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


  void slotBuildRun();
  void slotBuildMake();
  void slotBuildRebuildAll();
  void slotBuildCleanRebuildAll();
  void slotBuildAutoconf();
  void slotBuildStop();
  void slotBuildAPI();
  void slotBuildManual();

  void slotProjectAddNewFile();
  void slotProjectAddExistingFile();
  void slotProjectRemoveFile();
  void slotProjectNewClass();
  void slotProjectFileProperties();
  void slotProjectOptions();
  void slotBookmarksAdd();
  void slotBookmarksEdit();

  void slotHelpContent();
  void slotHelpHomepage();
  void slotHelpAbout();
  void slotToolbarClicked(int);
  void slotURLSelected(KHTMLView* widget,const char* url,int,const char*);
  void slotReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotReceivedStderr(KProcess* proc,char* buffer,int buflen);
  void slotSearchReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotDocumentDone( KHTMLView *_view );
  void slotProcessExited(KProcess* proc);
  void slotSearchProcessExited(KProcess*);

  void slotLogFileTreeSelected(int index);
  void slotRealFileTreeSelected(int index);
  void slotDocTreeSelected(int index);

  void slotSTabSelected(int item);
  void slotMenuBuffersSelected(int id);
  void slotClickedOnOutputWidget();

  void slotClassChoiceCombo(int index);
  void slotMethodChoiceCombo(int index);
  void slotClassTreeSelected(int);

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
protected:
  virtual void closeEvent(QCloseEvent* e);
private:
  //the menus
  QPopupMenu* file_menu;
  QPopupMenu* edit_menu;
  QPopupMenu* view_menu;
  QPopupMenu* documentation_menu;
  QPopupMenu* build_menu;
  QPopupMenu* project_menu;
  QPopupMenu* options_menu;
  QPopupMenu* tools_menu;
  QPopupMenu* menu_buffers;
  QPopupMenu* menu_help;

  KNewPanner* view;
  KNewPanner* top_panner;
  
  
  KIconLoader icon_loader;
  KProcess process;
  KShellProcess shell_process;
  CProject prj; // at the moment only one project at the same time

  KConfig* config;
  int act_outbuffer_len;
  //  int s_tab_current; // actual Tab in the s_tab_view 

  // for the browser
  QStrList history_list;
  

  
  QList<TEditInfo> edit_infos;
  //some widgets for the mainview

  CTabCtl* s_tab_view; // the tabbar for the sourcescode und browser 
  CTabCtl* t_tab_view; // the tabbar for the trees
  CEditWidget* edit_widget; // a pointer to the actual editwidget
 
  CEditWidget* header_widget; // the editwidget for the headers/resources

  CEditWidget* cpp_widget;    //  the editwidget for cpp files
  CDocBrowser* browser_widget;
  KSwallowWidget* swallow_widget;
 
  CClassView* class_tree; // the classview
  CLogFileView* log_file_tree; // the logical filetree
  CRealFileView* real_file_tree; // the real filetree
  CDocTree* doc_tree; // the documentation tre
 
  QString version;
  bool project;
  bool bViewStatusbar;

  bool  prev_was_search_result;

  COutputWidget* output_widget; // output for the compiler ...


  //some vars for the searchengine
  KShellProcess search_process;
  QString search_output;
  QString doc_search_text;
  // for more then one job in proc;checked in slotProcessExited(KProcess* proc);
  // values are "run","make" "refresh";
  QString next_job;

  enum {TOOLBAR_CLASS_CHOICE,TOOLBAR_METHOD_CHOICE};
};

#endif
