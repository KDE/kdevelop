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
class CAddExistingFileDlg;
class QListViewItem;
class CErrorMessageParser;
#include "ceditwidget.h"
#include "coutputwidget.h"
#include "ctabctl.h"
#include "crealfileview.h"
#include "clogfileview.h"
#include "cproject.h"
#include "cdoctree.h"
#include "structdef.h"
#include "resource.h"
#include "./print/cprintdlg.h"
#include "./classparser/ParsedClass.h"


class KDlgEdit;
class KDlgEditWidget;
class KDlgPropWidget;
class KDlgWidgets;
class KDlgDialogs;
class KDlgItems;


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

  void setKeyAccel();
  
  void initKDlg();
  void initKDlgMenuBar();
  void initKDlgToolBar();
  void initKDlgStatusBar();
  void initKDlgKeyAccel();
  /** sets the Main window caption on startup if in KDlgedit mode, used by main() */
  void setKDlgCaption();
  		
  void enableCommand(int id_);
  void disableCommand(int id_);
	
  void newFile(bool add_to_project);
  /** read the projectfile from the disk*/
  bool readProjectFile(QString file);
  /**@param type HEADER,SOURCE,SCRIPT,DATA
   * return true if a new subdir was added to the project
   */
  bool addFileToProject(QString complete_filename,QString type,bool refreshTrees=true);
  /**@param filename the absolute filename*/
  void switchToFile(QString filename); // filename = abs
  /** Switch to a certain line in a certain file. */
  void switchToFile(QString filename, int lineNo);
  void switchToWorkspace(int id);
  int getTabLocation(QString filename);

  /** set the correct toolbar and menubar,if a process is running
    * @param enable if true than enable,otherwise disable
    */
  void setToolMenuProcess(bool enable);

  KDlgEditWidget* kdlg_get_edit_widget() { return kdlg_edit_widget; }
  KDlgPropWidget* kdlg_get_prop_widget() { return kdlg_prop_widget; }
  KDlgWidgets* kdlg_get_widgets_view() { return kdlg_widgets_view; }
  KDlgDialogs* kdlg_get_dialogs_view() { return kdlg_dialogs_view; }
  KDlgItems*   kdlg_get_items_view() { return kdlg_items_view; }


 public slots:

 ////////////////////////
 // FILE-Menu entries
 ///////////////////////
 
 /** generate a new file*/
 void slotFileNew();
  /**open a file*/
  void slotFileOpen();
  /** close the cuurent file*/
  void slotFileClose();
  void slotFileCloseAll();
  /** save the current file,if Untitled a dialog ask for a valid name*/
  void slotFileSave();
  /** save all files*/
  void slotFileSaveAll();
  /** save the current file under a different filename*/
  void slotFileSaveAs();
  /** opens the printing dialog */
  void slotFilePrint();
  /** quit kdevelop*/
  void slotFileQuit();
  
  ////////////////////////
  // EDIT-Menu entries
///////////////////////
  /** Undo last editing step */
  void slotEditUndo();
  /** Redo last editing step */
  void slotEditRedo();
  /** cuts a selection to the clipboard */
  void slotEditCut();
  /** copies a selection to the clipboard */
  void slotEditCopy();
  /** inserts the clipboard contents to the cursor position */
  void slotEditPaste();
  /** inserts a file at the cursor position */
  void slotEditInsertFile();
  /** opens the search dialog for the editing widget */
  void slotEditSearch();
  /** repeat last search */
  void slotEditRepeatSearch();
  /** opens the search and replace dialog */
  void slotEditReplace();
  void slotEditIndent();
  void slotEditUnindent();
  /** selects the whole editing widget text */
  void slotEditSelectAll();
  /** inverts the selection */
  void slotEditInvertSelection();
  /** remove all text selections */
  void slotEditDeselectAll();
  
  ////////////////////////
  // VIEW-Menu entries
  ///////////////////////
  /** opens the goto line dialog */
  void slotViewGotoLine();
  /** jump to the next error, based on the make output*/
  void slotViewNextError();
  /** jump to the previews error, based on the make output*/
  void slotViewPreviousError();
  /** dis-/enables the treeview */
  void slotViewTTreeView();
  void showTreeView(bool show=true);
  /** dis-/enables the outputview */
  void slotViewTOutputView();
  void showOutputView(bool show=true);
  /** en-/disable the toolbar */
  void slotViewTStdToolbar();
  /** en-/disable the browser toolbar */
  void slotViewTBrowserToolbar();
  /** en-/disable the statusbar */
  void slotViewTStatusbar();
  /** refresh all trees and other widgets*/
  void slotViewRefresh();
  
  ////////////////////////
  // PROJECT-Menu entries
  ///////////////////////
  /** generates a new project with KAppWizard*/
  void slotProjectNewAppl();
  /** generates a new project file */
  void slotProjectNew();
  /** opens a projectfile and close the old one*/
  void slotProjectOpen();
  /** opens a project committed by comandline or kfm */
  void slotProjectOpenCmdl(const char*);
  /** close the current project,return false if  canceled*/
  bool slotProjectClose();
  /** add a new file to the project-same as file new */
  void slotProjectAddNewFile();
  /** opens the add existing files dialog */
  void slotProjectAddExistingFiles();
  /** helper methods for slotProjectAddExistingFiles() */
  void slotAddExistingFiles();
  /** add a new po file to the project*/
  void slotProjectAddNewTranslationFile();
  /** remove a project file */
  void slotProjectRemoveFile();
  /** opens the New class dialog */
  void slotProjectNewClass();
  /** opens the properties dialog for the project files */
  void slotProjectFileProperties();
  /** opens the properties dialog for project files,rel_name is selected, used by RFV,LFV*/
  void slotShowFileProperties(QString rel_name);
  /** opens the project options dialog */
  void slotProjectOptions();
  /** selects the project workspace */
  void slotProjectWorkspaces(int);
  
  ////////////////////////
  // BUILD-Menu entries
  ///////////////////////
  /** compile the actual sourcefile using setted options */
  void slotBuildCompileFile();
  void slotBuildMake();
  //   void slotBuildMakeWith();
  void slotBuildRebuildAll();
  void slotBuildCleanRebuildAll();
  void slotBuildStop();
  void slotBuildRun();
  void slotBuildRunWithArgs();
  void slotBuildDebug();
  void slotBuildDistClean();
  void slotBuildAutoconf();
  void slotBuildConfigure();
	void slotBuildSetExecuteArgs();
  void slotBuildMessages();
  void slotBuildAPI();
  void slotBuildManual();
  
  ////////////////////////
  // TOOLS-Menu entries
  ///////////////////////
  void slotToolsKDbg();
  void slotToolsKIconEdit();
  void slotToolsKTranslator();
  
  ////////////////////////
  // OPTIONS-Menu entries
  ///////////////////////
  void slotOptionsEditor();
  void slotOptionsEditorColors();
  void slotOptionsSyntaxHighlightingDefaults();
  void slotOptionsSyntaxHighlighting();
  void slotOptionsDocBrowser();
  void slotOptionsConfigureEnscript();
  void slotOptionsConfigureA2ps();
  /** show a configure-dialog for kdevelop*/
  void slotOptionsKDevelop();
  /** sets the make command after it is changed in the Setup dialog */
  void slotOptionsMake();
  /** dis-/enables autosaving by setting in the Setup dialog */
  void slotOptionsAutosave(bool);
  /** sets the autosaving time intervall */
  void slotOptionsAutosaveTime(int);
  /** dis-/enalbes autoswitch by setting bAutoswitch */
  void slotOptionsAutoswitch(bool);
  /** toggles between autoswitching to CV or LFV */
  void slotOptionsDefaultCV(bool);
  /** shows the Update dialog and sends output to the messages widget */
  void slotOptionsUpdateKDEDocumentation();
  /** shows the create search database dialog called by the setup button */
  void  slotOptionsCreateSearchDatabase();
  
  ////////////////////////
  // BOOKMARKS-Menu entries
  ///////////////////////
  void slotBookmarksSet();
  void slotBookmarksAdd();
  void slotBookmarksClear();

  ////////////////////////
  // HELP-Menu entries
  ///////////////////////
  /** goes one page back in the documentation browser */
  void slotHelpBack();
  /** goes one page forward in the documentatio browser */
  void slotHelpForward();
  /** search marked text */
  void slotHelpSearchText();
  /** search marked text with a text string */
  void slotHelpSearchText(QString text);
  /** shows the Search for Help on.. dialog to insert a search expression */
  void slotHelpSearch();
  /** shows the C/C++-referenc */
  void slotHelpReference();
  /** shows the Qt-doc */
  void slotHelpQtLib();
  /** shows the kdecore-doc */
  void slotHelpKDECoreLib();
  /** shows the kdeui-doc */
  void slotHelpKDEGUILib();
  /** shows the kfile-doc */
  void slotHelpKDEKFileLib();
  /** shows the khtml / khtmlw -doc */
  void slotHelpKDEHTMLLib();
  /** shows the API of the current project */
  void slotHelpAPI();
  /** shows the manual of the current project */
  void slotHelpManual();
  /** shows the KDevelop manual */
  void slotHelpContents();
  void slotHelpHomepage();
  /** shows the aboutbox of KDevelop */
  void slotHelpAbout();

  
  ////////////////////////
  // KDlgEdit-View-Menu entries -- managed by kdevelop
  ///////////////////////
  void slotKDlgViewPropView();
  void slotKDlgViewToolbar();
  void slotKDlgViewStatusbar();
  
  ////////////////////////
  // All slots which are used if the user clicks or selects something in the view
  ///////////////////////
  /** swich construction for the toolbar icons, selecting the right slots */
  void slotToolbarClicked(int);
  /** click on the main window tabs: header, source,documentation or tools*/
  void slotSTabSelected(int item);
  /** set the window tab automatically without click */
  void slotSCurrentTab(int item);
  /** click on the treeview tabs: cv,lfv,wfv,doc*/
  void slotTTabSelected(int item);
  /** set the tree tab automatically without click */
  void slotTCurrentTab(int item);
	
  ///////////// -- the methods for the treeview selection
  /** click action on classviewer CV */
  void slotClassTreeSelected();
  /** click action on LFV */
  void slotLogFileTreeSelected(QString file);
  /** click action on RFV */
  void slotRealFileTreeSelected(QString file);
  /** click action on DOC */
  void slotDocTreeSelected(QString url_file);
  /** selection of classes in the browser toolbar */
  void slotClassChoiceCombo(int index);
  /** selection of methods in the browser toolbar */
  void slotMethodChoiceCombo(int index);
  /** add a file to the project */
  void slotAddFileToProject(QString abs_filename);
  void delFileFromProject(QString rel_filename);

  //////////////// -- the methods for the statusbar items
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

  void slotBufferMenu(const QPoint& pos);
  void slotShowC();
  void slotShowHeader();
  void slotShowHelp();
  void slotShowTools();
  void slotToggleLast();

  void slotMenuBuffersSelected(int id);
  void slotClickedOnMessagesWidget();
  void slotKeyPressedOnStdinStdoutWidget(int key);

  void slotURLSelected(KHTMLView* widget,const char* url,int,const char*);
  void slotDocumentDone( KHTMLView *_view );
  void slotURLonURL(KHTMLView*,const char* url);

  void slotReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotReceivedStderr(KProcess* proc,char* buffer,int buflen);

  void slotApplReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotApplReceivedStderr(KProcess* proc,char* buffer,int buflen);


  void slotSearchReceivedStdout(KProcess* proc,char* buffer,int buflen);
  void slotProcessExited(KProcess* proc);
  void slotSearchProcessExited(KProcess*);
  
  //////////////// -- the methods for signals generated from the CV
  /** Added method in the classview. */
  void slotCVAddMethod( CParsedMethod *aMethod );
  /** Added attribute in the classview. */
  void slotCVAddAttribute( CParsedAttribute *aAttr );
  /** The user wants to view the declaration of a method. */
  void slotCVViewDeclaration();
  /** The user wants to view the definition of a method/attr... */
  void slotCVViewDefinition();

  // return the position of the classdeclaration begin
  void CVGotoDefinition(QListViewItem *item);
  void CVGotoDeclaration(QListViewItem *item);


  /** a tool meth,used in the search engine*/
  int searchToolGetNumber(QString str);
  QString searchToolGetTitle(QString str);
  QString searchToolGetURL(QString str);
  void refreshClassCombo();
  void refreshMethodCombo( CParsedClass *aClass );
  void  saveCurrentWorkspaceIntoProject();

  void switchToKDevelop();
  void switchToKDlgEdit();

protected:
  virtual void closeEvent(QCloseEvent* e);
  //  void mousePressEvent(QMouseEvent* event);
  /** called if a new subdirs was added to the project, shows a messagebox and start autoconf...*/
  void newSubDir();

private:
  //the menus for kdevelop main
  QPopupMenu* file_menu;				
  QPopupMenu* edit_menu;
  QPopupMenu* view_menu;
	QPopupMenu* bookmarks_menu;
  QPopupMenu* project_menu;
  QPopupMenu* workspaces_submenu;
  QPopupMenu* build_menu;
  QPopupMenu* tools_menu;
  QPopupMenu* options_menu;
  QPopupMenu* menu_buffers;
  QPopupMenu* help_menu;
  QWhatsThis* whats_this;

  // the menus for the dialogeditor- specific. other menus inserted as the standard above
  QPopupMenu* kdlg_file_menu;
  QPopupMenu* kdlg_edit_menu;
  QPopupMenu* kdlg_view_menu;
  QPopupMenu* kdlg_project_menu;
  QPopupMenu* kdlg_build_menu;
  QPopupMenu* kdlg_tools_menu;
  QPopupMenu* kdlg_options_menu;
  QPopupMenu* kdlg_help_menu;
	
  KMenuBar* kdev_menubar;
  KMenuBar* kdlg_menubar;

  KStatusBar* kdev_statusbar;
  KStatusBar* kdlg_statusbar;

  KNewPanner* view;
  KNewPanner* top_panner;
  KNewPanner* kdlg_top_panner;  // devides the top_panner for edit and properties widget of the dialogeditor
  
  KDlgEdit* kdlgedit;          // main class for the dialogeditor- handles menu/toolbar etc. events specified for the dialogeditor
  
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

  CTabCtl* kdlg_tabctl;  // the tabbar for the kdlg view
  KDlgEditWidget* kdlg_edit_widget; // the editing view of kdlg
  KDlgPropWidget* kdlg_prop_widget; // the properties window of kdlg
  KDlgWidgets* kdlg_widgets_view;  // the first tab of kdlg_tabctl
  KDlgDialogs* kdlg_dialogs_view;  // the second tab of kldg_tabctl
  KDlgItems*   kdlg_items_view;    // the third tab of kldg_tabctl

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
  int properties_view_pos;
  int workspace;

  CErrorMessageParser* error_parser;
  QString version;
  QString kdev_caption;
  QString kdlg_caption;
  bool project;
  bool bViewStatusbar;

  bool  prev_was_search_result;
  // Autosaving elements
  QTimer* saveTimer; // the timer
  bool bAutosave;    // enable=true
  int saveTimeout;   // timeout time

  bool bAutoswitch;
  bool bDefaultCV;
  bool bKDevelop;
  KProgress* statProg;
  //  QProgressBar* statProg;
  //some vars for the searchengine
  QString search_output;
  QString doc_search_text;
  // for more then one job in proc;checked in slotProcessExited(KProcess* proc);
  // values are "run","make" "refresh";
  QString next_job;
  QString make_cmd;
//   QString make_with_cmd;

  CConfigEnscriptDlg* enscriptconf;
  CConfigA2psDlg* a2psconf;

  CAddExistingFileDlg* add_dlg;

  enum {TOOLBAR_CLASS_CHOICE,TOOLBAR_METHOD_CHOICE};

  int lasttab;
  QString lastfile;

};

#endif












































