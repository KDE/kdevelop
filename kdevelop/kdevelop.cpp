/***************************************************************************
                          kdevelop.cpp  -  description
                             -------------------
    begin                : Mit Jun 14 14:43:39 CEST 2000
    copyright            : (C) 2000 by The KDevelop Team
    email                : kdevelop-team@fara.cs.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qdialog.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstdaction.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kdockwidget.h>
#include <kmenubar.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kedittoolbar.h>
#include "lib/kdevcomponent.h"
#include "lib/projectmanagement/newprojectdlg.h"
#include "classparser/ClassParser.h"
#include "kdevelop.h"
#include "kdevcomponent.h"
#include "kdevelopfactory.h"
#include "kdevelopcore.h"


KDevelop::KDevelop(const char *name) : KParts::DockMainWindow( name )
{
  initActions();
  initHelp();
  //  setXMLFile( "/mnt/rnolden/Development/kdevelop/kdevelop/kdevelopui.rc" );

  setXMLFile( "kdevelopui.rc" );

  createGUI( 0L );

  (void) new KDevelopCore(this);
}


KDevelop::~KDevelop()
{
}


/** sets up the KActions designed User Interface
for the toolbars and menubar */
void KDevelop::initActions(){
  /////////////////////////////////////
  // File Menu
  ////////////////////////////////////
  m_paFileNew = KStdAction::openNew( this, SLOT( slotFileNew() ), actionCollection(), "file_new");
  m_paFileOpen = KStdAction::open( this, SLOT( slotFileOpen() ), actionCollection(), "file_open" );
  m_paFileClose = KStdAction::close( this, SLOT( slotFileClose() ), actionCollection(),  "file_close");
  m_paFileCloseAll = new KAction( i18n("Close All"), 0, this, SLOT( slotFileCloseAll() ), actionCollection(), "file_close_all");
  // Separator
  m_paFileSave = KStdAction::save( this, SLOT( slotFileSave() ), actionCollection(), "file_save" );
  m_paFileSaveAs = KStdAction::saveAs( this, SLOT( slotFileSaveAs() ), actionCollection(), "file_save_as" );
  m_paFileSaveAll = new KAction( i18n("Save A&ll"), "save_all", 0, this, SLOT( slotFileSaveAll() ), actionCollection(), "file_save_all");
  // Separator
  m_paFilePrint = KStdAction::print( this, SLOT( slotFilePrint() ), actionCollection(),  "file_print");
  m_paFileQuit = KStdAction::quit( this, SLOT( slotFileQuit() ), actionCollection(),  "file_quit");

  /////////////////////////////////////
  // Edit Menu
  ////////////////////////////////////
  m_paEditUndo = KStdAction::undo( this, SLOT( slotEditUndo() ), actionCollection(), "edit_undo" );
  m_paEditRedo = KStdAction::redo( this, SLOT( slotEditRedo() ), actionCollection(),"edit_redo" );
  m_paEditUndoHistory = new KAction( i18n("Undo/Redo &History..."), 0, this, SLOT( slotEditUndoHistory() ),
      actionCollection(), "edit_undo_history");
  // Separator
  m_paEditCut = KStdAction::cut( this, SLOT( slotEditCut() ), actionCollection(), "edit_cut" );
  m_paEditCopy = KStdAction::copy( this, SLOT( slotEditCopy() ), actionCollection(),"edit_copy" );
  m_paEditPaste = KStdAction::paste( this, SLOT( slotEditPaste() ), actionCollection(),"edit_paste" );
  // Separator

  m_paEditIndent = new KAction( i18n("In&dent"), "indent", CTRL+Key_I, this, SLOT( slotEditIndent() ),
      actionCollection(), "edit_indent");
  m_paEditUnindent = new KAction( i18n("Uninden&t"), "unindent", CTRL+Key_U, this, SLOT( slotEditUnindent() ),
      actionCollection(), "edit_unindent");
  // Separator
  m_paEditComment = new KAction( i18n("Comment"), CTRL+Key_M, this, SLOT( slotEditComment() ),
      actionCollection(), "edit_comment");
  m_paEditUncomment = new KAction( i18n("Uncomment"), CTRL+ALT+Key_M, this, SLOT( slotEditUncomment() ),
      actionCollection(), "edit_uncomment");
  // Separator
  m_paEditInsertFile = new KAction(i18n("&Insert File"), 0, this, SLOT( slotEditInsertFile() ),
      actionCollection(), "edit_insert_file");
  // Separator
  m_paEditSearch = KStdAction::find( this, SLOT( slotEditSearch() ), actionCollection(),"edit_search" );
  m_paEditRepeatSearch = KStdAction::findNext( this, SLOT(slotEditRepeatSearch()),
      actionCollection(), "edit_repeat_search" );
  m_paEditReplace = KStdAction::replace( this, SLOT( slotEditReplace() ), actionCollection(), "edit_replace" );
  // Separator
  m_paEditSelectAll = KStdAction::selectAll( this, SLOT( slotEditSelectAll() ), actionCollection(),"edit_select_all" );
  m_paEditDeselectAll = new KAction( i18n("&Deselect All"), 0, this, SLOT( slotEditDeselectAll() ),
      actionCollection(), "edit_deselect_all");
  m_paEditInvertSelection = new KAction( i18n("In&vert Selection"), 0, this, SLOT( slotEditInvertSelection() ),
      actionCollection(), "edit_invert_selection");

  /////////////////////////////////////
  // View Menu
  ////////////////////////////////////
  m_paViewGotoLine = KStdAction::gotoLine( this, SLOT(  slotViewGotoLine() ), actionCollection(), "view_goto_line");
  // Separator
  m_paViewTreeView = new KToggleAction( i18n("&Tree-View"), "tree_win", CTRL+Key_T, actionCollection(), "view_treeview");
  m_paViewOutputView = new KToggleAction( i18n("&Output-View"), "output_win",
        CTRL+Key_B, actionCollection(), "view_output_view");
  // Separator
  m_paViewToolbar = KStdAction::showToolbar( this, SLOT( slotViewToolbar() ), actionCollection() );
  m_paViewBrowserToolbar = new KToggleAction( i18n("&Browser-Toolbar"), 0, actionCollection(), "view_browser_toolbar");
  m_paViewMDITaskbar = new KToggleAction( i18n("Tas&kbar"), 0, actionCollection(), "view_taskbar");
  m_paViewStatusbar = KStdAction::showStatusbar( this, SLOT( slotViewStatusbar() ), actionCollection(), "view_statusbar" );
  // Separator
  m_paViewTreeView->setChecked( true );
  m_paViewOutputView->setChecked( true );
  m_paViewBrowserToolbar->setChecked( true );
  m_paViewMDITaskbar->setChecked( true );
  connect( m_paViewTreeView, SIGNAL( activated() ), this, SLOT( slotViewTreeView() ) );
  connect( m_paViewOutputView, SIGNAL( activated() ), this, SLOT( slotViewOutputView() ) );
  connect( m_paViewBrowserToolbar, SIGNAL( activated() ), this, SLOT( slotViewBrowserToolbar() ) );
  connect( m_paViewMDITaskbar, SIGNAL( activated() ), this, SLOT( slotViewMDITaskbar() ) );
  ////////////////
  // Debug Viewers submenu
  m_paViewDebugVar = new KAction( i18n("&Variables"), 0, this, SLOT( slotViewDebugVar() ),
          actionCollection(), "view_debug_variables");
  m_paViewDebugBreakpoints = new KAction( i18n("&Breakpoints"), 0, this, SLOT( slotViewDebugBreakpoints() ),
          actionCollection(), "view_debug_breakpoints");
  m_paViewDebugFrameStack = new KAction( i18n("&Frame Stack"), 0, this, SLOT( slotViewDebugFrameStack() ),
          actionCollection(), "view_debug_framestack");
  m_paViewDebugDisassemble = new KAction( i18n("&Disassemble"), 0, this, SLOT( slotViewDebugDisassemble() ),
          actionCollection(), "view_debug_disassemble");
  m_paViewDebugDebugger = new KAction( i18n("Debu&gger"), 0, this, SLOT( slotViewDebugDebugger() ),
          actionCollection(), "view_debug_debugger");
  ////////////////
  // Separator
  m_paViewRefresh = new KAction( i18n("&Refresh"), "reload", 0, this, SLOT( slotViewRefresh() ), actionCollection(), "view_refresh");


  /////////////////////////////////////
  // Project Menu
  ////////////////////////////////////
  m_paProjectNew = new KAction( i18n("&New..."), 0, this, SLOT( slotProjectNew() ),
          actionCollection(), "project_new");
  m_paProjectOpen = new KAction( i18n("&Open..."), "openprj", 0, this, SLOT( slotProjectOpen() ),
          actionCollection(), "project_open");
  m_paProjectOpenRecent = new KAction( i18n("Open &recent project..."), 0, this, SLOT( slotProjectOpenRecent() ),
          actionCollection(), "project_open_recent");
  m_paProjectClose = new KAction( i18n("C&lose"), 0, this, SLOT( slotProjectClose() ), actionCollection(), "project_close");
  // Separator
  m_paProjectNewClass = new KAction( i18n("New &Class..."), 0, this, SLOT( slotProjectNewClass() ),
          actionCollection(), "project_new_class");
  m_paProjectAddExistingFiles = new KAction( i18n("&Add existing File(s)..."), 0, this, SLOT( slotProjectAddExistingFiles() ),
          actionCollection(), "project_add_existing_files");
  m_paProjectAddNewTranslationFile = new KAction( i18n("Add new &Translation File..."), "locale", 0,
          this, SLOT( slotProjectAddNewTranslationFile() ), actionCollection(), "project_add_translation");
  m_paProjectFileProperties = new KAction( i18n("&File Properties..."), "file_properties", 0,
          this, SLOT( slotProjectFileProperties() ), actionCollection(), "project_file_properties");
  // Separator
  m_paProjectMessages = new KAction( i18n("Make &messages and merge"), 0, this, SLOT( slotProjectMessages() ),
          actionCollection(), "project_make_messages");
  m_paProjectAPI = new KAction( i18n("Make AP&I-Doc"), 0, this, SLOT( slotProjectAPI() ),
          actionCollection(), "project_make_api_doc");
  m_paProjectManual = new KAction( i18n("Make &User-Manual..."), "mini-book1",0, this, SLOT( slotProjectManual() ),
          actionCollection(), "project_make_user_doc");
  m_paProjectMakeDistSourceTgz = new KAction( i18n("&Source-tgz"), 0, this, SLOT( slotProjectMakeDistSourceTgz() ),
          actionCollection(), "project_dist_targz");
  // Separator
  m_paProjectOptions = new KAction( i18n("&Options..."), 0, this, SLOT( slotProjectOptions() ),
          actionCollection(), "project_options");

  /////////////////////////////////////
  // Build Menu
  ////////////////////////////////////
  m_paBuildCompileFile = new KAction( i18n("Compile &File"), "compfile",0, this, SLOT( slotBuildCompileFile() ),
          actionCollection(), "build_compile_file");
  m_paBuildMake = new KAction( i18n("&Make"), "make_kdevelop", 0, this, SLOT( slotBuildMake() ), actionCollection(), "build_build");
  m_paBuildRebuildAll = new KAction( i18n("&Rebuild all"), "rebuild", 0, this, SLOT( slotBuildRebuildAll() ),
          actionCollection(), "build_rebuild");
  m_paBuildCleanRebuildAll = new KAction( i18n("&Clean/Rebuild all"), 0, this, SLOT( slotBuildCleanRebuildAll() ),
          actionCollection(), "build_clean_rebuild_all");
  // Separator
  m_paBuildStop = new KAction( i18n("&Stop Build"), "stop_proc", 0, this, SLOT( slotBuildStop() ), actionCollection(), "build_stop");
  // Separator
  m_paBuildExecute = new KAction( i18n("&Execute"), "run", 0, this, SLOT( slotBuildExecute() ), actionCollection(), "build_execute");
  m_paBuildExecuteWithArgs = new KAction( i18n("Execute &with Arguments..."), "run", 0, this, SLOT( slotBuildExecuteWithArgs() ),
          actionCollection(), "build_execute_with_arguments");
  // Separator
  m_paBuildDistClean = new KAction( i18n("DistC&lean"), 0, this, SLOT( slotBuildDistClean() ),
          actionCollection(), "build_distclean");
  m_paBuildAutoconf = new KAction( i18n("&Autoconf and automake"), 0, this, SLOT( slotBuildAutoconf() ),
          actionCollection(), "build_autoconf_automake");
  m_paBuildConfigure = new KAction( i18n("C&onfigure..."), 0, this, SLOT( slotBuildConfigure() ),
          actionCollection(), "build_configure");

  /////////////////////////////////////
  // Debug Menu
  ////////////////////////////////////
  // Debug menu
  // slotBuildDebug -> slotDebugStart
  m_paDebugStart = new KAction( i18n("&Start"), 0, this, SLOT( slotDebugStart() ), actionCollection(),"debug_start" );
  //////////////
  // Debug "Start (other)..." submenu
  m_paDebugExamineCore = new KAction( i18n("Examine core file"), "debugger", 0, this, SLOT( slotDebugExamineCore() ),
          actionCollection(),"debug_examine_core" );
  m_paDebugNamedFile = new KAction( i18n("Debug another executable"), "debugger", 0, this, SLOT( slotDebugNamedFile() ),
          actionCollection(), "debug_debug_other_exe");
  m_paDebugAttatch = new KAction( i18n("Attach to process"), "debugger", 0, this, SLOT( slotDebugAttach() ),
          actionCollection(), "debug_attatch_to_process");
  m_paDebugExecuteWithArgs = new KAction( i18n("Debug with arguments"), "debugger", 0, this, SLOT( slotDebugExecuteWithArgs() ),
          actionCollection(), "debug_debug_with_args");
  /////////////
  // Separator
//  ***********************
//   METHODS NEEDED
//  ***********************
//  m_pa = new KAction( i18n("Run"), 0, this, SLOT( () ), actionCollection(), "debug_run");
//  m_pa = new KAction( i18n("Run to cursor"), 0, this, SLOT( () ), actionCollection(), "debug_run_to_cursor");
//  m_pa = new KAction( i18n("Step over"), 0, this, SLOT( () ), actionCollection(), "debug_step_over");
//  m_pa = new KAction( i18n("Step over instr."), 0, this, SLOT( () ), actionCollection(), "debug_step_over_instr");
//  m_pa = new KAction( i18n("Step into"), 0, this, SLOT( () ), actionCollection(), "debug_step_into");
//  m_pa = new KAction( i18n("Step into instr."), 0, this, SLOT( () ), actionCollection(), "debug_step_into_instr");
//  m_pa = new KAction( i18n("Step out"), 0, this, SLOT( () ), actionCollection(), "debug_step_out");
//  // Separator
//  m_pa = new KAction( i18n("Interrupt"), 0, this, SLOT( () ), actionCollection(), "debug_interrupt");
//  m_pa = new KAction( i18n("Stop"), 0, this, SLOT( () ), actionCollection(), "debug_stop");


  /////////////////////////////////////
  // Options Menu
  ////////////////////////////////////
  m_paOptionsEditor = new KAction( i18n("&Editor..."), 0, this, SLOT( slotOptionsEditor() ),
				   actionCollection(), "options_editor");
  m_paOptionsEditorColors = new KAction( i18n("Editor &Colors..."), 0, this, SLOT( slotOptionsEditorColors() ),
					 actionCollection(), "options_editor_colors");
  m_paOptionsEditorDefaults = new KAction( i18n("Editor &Defaults..."), 0, this, SLOT( slotOptionsEditorDefaults() ),
					   actionCollection(), "options_editor_defaults");
  m_paOptionsSyntaxHighlighting = new KAction( i18n("&Syntax Highlighting..."), 0, this, SLOT( slotOptionsSyntaxHighlighting() ),
					       actionCollection(), "options_syntax_highlighting");
  // Separator
  m_paOptionsDocumentationBrowser = new KAction( i18n("Documentation &Browser..."), 0, this, SLOT( slotOptionsDocumentationBrowser() ),
						 actionCollection(), "options_documentation_browser");
  /////////////
  // Configure Printer submenu
  m_paOptionsConfigureEnscript = new KAction( i18n("&Enscript..."), 0, this, SLOT( slotOptionsConfigureEnscript() ),
					      actionCollection(), "options_configure_printer_enscript");
  // Separator
  m_paOptionsKDevelopSetup = new KAction( i18n("&KDevelop Setup..."), 0, this, SLOT( slotOptionsKDevelopSetup() ),
					  actionCollection(), "options_kdevelop_setup");

  m_paOptionsEditToolbars = KStdAction::configureToolbars(this, SLOT(slotOptionsEditToolbars()), actionCollection(),"options_configure_toolbars");

  /////////////////////////////////////
  // Window Menu
  ////////////////////////////////////
  /////////////////////////////////////
  // Bookmarks Menu
  ////////////////////////////////////
  m_paBookmarksToggle = new KAction( i18n("&Toggle Bookmark"), "bookmark_add", 0, this, SLOT( slotBookmarksToggle() ),
         actionCollection(), "bookmarks_toggle");
  m_paBookmarksNext = new KAction( i18n("&Next Bookmark"), "bookmark", 0, this, SLOT( slotBookmarksNext() ),
         actionCollection(), "bookmarks_next");
  m_paBookmarksPrevious = new KAction( i18n("&Previous Bookmark"), "bookmark", 0, this, SLOT( slotBookmarksPrevious() ),
         actionCollection(), "bookmarks_previous");
  m_paBookmarksClear = new KAction( i18n("&Clear Bookmarks"), 0, this, SLOT( slotBookmarksClear() ),
         actionCollection(), "bookmarks_clear");

  /////////////////////////////////////
  // Help Menu
  ////////////////////////////////////
  KHelpMenu * m_helpMenu = new KHelpMenu( this, KDevelopFactory::aboutData() );

  m_paHelpBack = KStdAction::back( this, SLOT( slotHelpBack() ), actionCollection(), "help_back");
  m_paHelpForward = KStdAction::forward( this, SLOT( slotHelpForward() ), actionCollection(), "help_forward");
  // Separator
  m_paHelpSearchMarkedText = new KAction( i18n("&Search Marked Text"), "lookup", 0, this, SLOT( slotHelpSearchMarkedText() ),
          actionCollection(), "help_search_marked_text");
  m_paHelpSearchForHelpOn = new KAction( i18n("Search for Help on..."), "contents", 0, this, SLOT( slotHelpSearchForHelpOn() ),
          actionCollection(), "help_search_for_help_on");
  m_paHelpWhatsThis = KStdAction::whatsThis( m_helpMenu, SLOT( contextHelpActivated() ), actionCollection(), "help_whats_this");
  // Separator
  m_paHelpWelcome = new KAction( i18n("&Welcome to KDevelop"), "mini-book1", 0,
          this, SLOT( slotHelpWelcome() ), actionCollection(), "help_welcome");
  m_paHelpUserManual = KStdAction::helpContents( this, SLOT( slotHelpContents() ), actionCollection(), "help_user_manual");
  m_paHelpProgramming = new KAction( i18n("Programming Handbook"), "mini-book1", 0, this, SLOT( slotHelpProgramming() ),
          actionCollection(), "help_programming");
  m_paHelpTutorial = new KAction( i18n("Tutorial Handbook"), "mini-book1", 0, this, SLOT( slotHelpTutorial() ),
          actionCollection(), "help_tutorial");
  m_paHelpKDELibRef = new KAction( i18n("KDE Library Reference"), "mini-book1", 0, this, SLOT( slotHelpKDELibRef() ),
          actionCollection(), "help_library_reference");
  m_paHelpCReference = new KAction( i18n("C/C++-Reference"), "mini-book1", 0, this, SLOT( slotHelpCReference() ),
          actionCollection(), "help_c_reference");
  // Separator
  m_paHelpTipOfTheDay = new KAction( i18n("Tip of the Day"), "idea", 0, this, SLOT( slotHelpTipOfTheDay() ),
          actionCollection(), "help_tip_of_the_day");
  m_paHelpHomepage = KStdAction::home( this, SLOT( slotHelpHomepage() ), actionCollection(), "help_kdevelop_homepage");
  // Separator
  m_paHelpReportBug = KStdAction::reportBug( m_helpMenu, SLOT( reportBug() ), actionCollection(), "help_report_bug");
  m_paHelpAboutApp = KStdAction::aboutApp( m_helpMenu, SLOT( aboutApplication() ), actionCollection(), "help_about_app");

  m_paHelpAboutKDE = KStdAction::aboutKDE( m_helpMenu, SLOT( aboutKDE() ), actionCollection(), "help_about_kde");

}


/** reimplemented from KParts::MainWindow
 */
void KDevelop::slotSetStatusBarText( const QString &text){
}


/** initializes the help messages (whats this and
statusbar help) on the KActions */
void KDevelop::initHelp(){
  m_paFileNew->setShortText( i18n("Creates a new file") );
  m_paFileNew->setWhatsThis( i18n("New file\n\n"
                                  "Opens the New file dialog to let you create "
                                  "a new project file. You can choose between "
                                  "several templates for creating the new file.") );

  m_paFileOpen->setShortText( i18n("Opens an existing file") );
  m_paFileOpen->setWhatsThis( i18n("Open file\n\n"
 																	"Shows the Open file dialog to "
 																	"select a file to be opened. Holding "
 																	"the button pressed will show a popup "
 																	"menu containing all filenames of your "
 																	"project's sources and header files. "
 																	"Selecting a filename on the menu will "
 																	"then open the file according to the "
 																	"file-type.") );

  m_paFileClose->setShortText( i18n("Closes the actual file") );
  m_paFileClose->setWhatsThis( i18n("Close file\n\n"
                                  "Closes the file in the currently active window. "
                                  "If the file has unsaved contents you will be "
                                  "asked for saving the file before closing.") );

  m_paFileCloseAll->setShortText( i18n("Closes all files") );
  m_paFileCloseAll->setWhatsThis( i18n("Close all\n\n"
                                  "Closes all files currently open. "
                                  "If a file has unsaved contents you will be "
                                  "asked for saving the file before closing.") );

  m_paFileSave->setShortText( i18n("Saves the actual document") );
  m_paFileSave->setWhatsThis( i18n("Save file\n\n"
 																	"Saves the file opened in the actual "
 																	"editing view") );

  m_paFileSaveAs->setShortText( i18n("Saves the document as...") );
  m_paFileSaveAs->setWhatsThis( i18n("Save As...\n\n"
                                  "Saves the file in the currently active window "
                                  "under a new filename. You will be asked for "
                                  "a new filename for the file to be saved.") );

  m_paFileSaveAll->setShortText( i18n("Saves all changed files") );
  m_paFileSaveAll->setWhatsThis( i18n("Save all\n\n"
                                  "Saves all currently open files. You can automate "
                                  "file saving by enabling autosave in the KDevelop "
                                  "Setup together with the desired saving intervall.") );

  m_paFilePrint->setShortText( i18n("Prints the current document") );
  m_paFilePrint->setWhatsThis( i18n("Print\n\n"
                                  "Opens the printing dialog. There, you can "
                                  "configure which printing program you wish "
                                  "to use, either a2ps or ensrcipt, and print "
                                  "your project files.") );

  m_paFileQuit->setShortText( i18n("Exits KDevelop") );
  m_paFileQuit->setWhatsThis( i18n("Quit\n\n"
                                  "Closes your current KDevelop session. You will be asked "
                                  "to save the contents of currently opened and changed files "
                                  "before KDevelop exits.") );


  m_paEditUndo->setShortText( i18n("Reverts the last editing step") );
  m_paEditUndo->setWhatsThis( i18n("Undo\n\n"
	                                "Reverts the last editing step.") );

  m_paEditRedo->setShortText( i18n("Re-execute the last undone step") );
  m_paEditRedo->setWhatsThis( i18n("Redo\n\n"
                                  "If an editing step was undone, redo "
	                                "lets you do this step again.") );

  m_paEditUndoHistory->setShortText( i18n("Opens the Undo History dialog") );
//  m_paEditUndoHistory->setWhatsThis(  );

  m_paEditCut->setShortText( i18n("Cuts the selected section and puts it to the clipboard") );
  m_paEditCut->setWhatsThis( i18n("Cut\n\n"
	                               "Cuts out the selected text and copies "
	                               "it to the system clipboard.") );

  m_paEditCopy->setShortText( i18n("Copys the selected section to the clipboard") );
  m_paEditCopy->setWhatsThis( i18n("Copy\n\n"
                                  "Copies the selected text into the "
                                  "system clipboard.") );

  m_paEditPaste->setShortText( i18n("Pastes the clipboard contents to actual position") );
  m_paEditPaste->setWhatsThis( i18n("Paste\n\n"
	                                  "Inserts the contents of the "
	                                  "system clipboard at the current "
	                                  "cursor position. ") );

  m_paEditIndent->setShortText( i18n("Moves the selection to the right") );
  m_paEditIndent->setWhatsThis( i18n("Indent\n\n"
                                      "Moves a selection to the right.") );

  m_paEditUnindent->setShortText( i18n("Moves the selection to the left") );
  m_paEditUnindent->setWhatsThis( i18n("Unindent\n\n"
                                      "Moves a selection to the left.") );

  m_paEditComment->setShortText( i18n("Comments out the selected line(s)") );
  m_paEditComment->setWhatsThis( i18n("Comment\n\n"
                                      "This command comments out the selected lines.") );

  m_paEditUncomment->setShortText( i18n("Uncomments the selected line(s)") );
  m_paEditUncomment->setWhatsThis( i18n("Uncomment\n\n"
                                        "Uncomment out commented lines.") );

  m_paEditInsertFile->setShortText( i18n("Inserts a file at the current position") );
  m_paEditInsertFile->setWhatsThis( i18n("Insert File\n\n"
                                        "Inserts the contents of a selected "
                                        "file at the current cursor position.") );

  m_paEditSearch->setShortText( i18n("Searchs the file for an expression") );
  m_paEditSearch->setWhatsThis( i18n("Search\n\n"
                                    "Opens the Search dialog to search "
                                    "for an expression in the current file.") );

  m_paEditRepeatSearch->setShortText( i18n("Repeats the last search") );
  m_paEditRepeatSearch->setWhatsThis( i18n("Repeat Search\n\n"
                                    "Repeats the last search and marks "
                                    "the next result.") );

  m_paEditReplace->setShortText( i18n("Searchs and replace expression") );
  m_paEditReplace->setWhatsThis( i18n("Search and Replace\n\n"
                                   "Opens the Search and Replace "
                                   "dialog to enter an expression to "
                                   "search for and an expression that "
                                   "will replace any matches.") );

  m_paEditSelectAll->setShortText( i18n("Selects the whole document contents") );
//  m_paEditSelectAll->setWhatsThis(  );

  m_paEditDeselectAll->setShortText( i18n("Deselects the whole document contents") );
//  m_paEditDeselectAll->setWhatsThis(  );

  m_paEditInvertSelection->setShortText( i18n("Inverts the current selection") );
//  m_paEditInvertSelection->setWhatsThis(  );


  // View Actions
  m_paViewGotoLine->setShortText( i18n("Goes to Line Number...") );
//  m_paViewGotoLine->setWhatsThis(  );

  m_paViewTreeView->setShortText( i18n("Enables / disables the treeview") );
  m_paViewTreeView->setWhatsThis( i18n("Tree-View\n\n"
            											"Enables/ disables the tree window. The "																
            											"state of the window is displayed by the "
            											"toggle button: if the button is pressed, "
            											"the window is open, otherwise closed.") );

  m_paViewOutputView->setShortText( i18n("Enables / disables the outputview") );
  m_paViewOutputView->setWhatsThis( i18n("Output-View\n\n"
             											"Enables/ disables the output window. The "																
             											"state of the window is displayed by the "
             											"toggle button: if the button is pressed, "
             											"the window is open, otherwise closed.") );

  m_paViewToolbar->setShortText( i18n("Enables / disables the standard toolbar") );
//  m_paViewToolbar->setWhatsThis(  );

  m_paViewBrowserToolbar->setShortText( i18n("Enables / disables the browser toolbar") );
//  m_paViewBrowserToolbar->setWhatsThis(  );

  m_paViewMDITaskbar->setShortText( i18n("Enables / disables the MDI view taskbar") );
//  m_paViewMDITaskbar->setWhatsThis(  );

  m_paViewStatusbar->setShortText( i18n("Enables / disables the statusbar") );
//  m_paViewStatusbar->setWhatsThis(  );
//
//  m_paViewDebugVar->setShortText(  );
//  m_paViewDebugVar->setWhatsThis(  );
//
//  m_paViewDebugBreakpoints->setShortText(  );
//  m_paViewDebugBreakpoints->setWhatsThis(  );
//
//  m_paViewDebugFrameStack->setShortText(  );
//  m_paViewDebugFrameStack->setWhatsThis(  );
//
//  m_paViewDebugDisassemble->setShortText(  );
//  m_paViewDebugDisassemble->setWhatsThis(  );
//
//  m_paViewDebugDebugger->setShortText(  );
//  m_paViewDebugDebugger->setWhatsThis(  );

//  m_paViewRefresh->setShortText(  );
//  m_paViewRefresh->setWhatsThis(  );

  // Project Actions
  m_paProjectNew->setShortText( i18n("Generates a new project with Application Wizard") );
//  m_paProjectNew->setWhatsThis(  );

  m_paProjectOpen->setShortText( i18n("Opens an existing project") );
  m_paProjectOpen->setWhatsThis(  i18n("Open project\n\n"
		                      				"Shows the open project dialog "
																	"to select a project to be opened") );

//  m_paProjectOpenRecent->setShortText(  );
//,  m_paProjectOpenRecent->setWhatsThis(  );

  m_paProjectClose->setShortText( i18n("Closes the current project") );
//  m_paProjectClose->setWhatsThis(  );

  m_paProjectNewClass->setShortText( i18n("Creates a new Class frame structure and files") );
//  m_paProjectNewClass->setWhatsThis(  );

  m_paProjectAddExistingFiles->setShortText( i18n("Adds existing file(s) to the project") );
//  m_paProjectAddExistingFiles->setWhatsThis(  );

  m_paProjectAddNewTranslationFile->setShortText( i18n("Adds a new language for internationalization to the project") );
//  m_paProjectAddNewTranslationFile->setWhatsThis(  );

  m_paProjectFileProperties->setShortText( i18n("Shows the file properties dialog") );
//  m_paProjectFileProperties->setWhatsThis(  );

  m_paProjectMessages->setShortText( i18n("Invokes make to create the message file by extracting all i18n() macros") );
//  m_paProjectMessages->setWhatsThis(  );

  m_paProjectAPI->setShortText( i18n("Creates the Project's API documentation with KDoc") );
//  m_paProjectAPI->setWhatsThis(  );

  m_paProjectManual->setShortText( i18n("Creates the Project's User Manual") );
//  m_paProjectManual->setWhatsThis(  );

  m_paProjectMakeDistSourceTgz->setShortText( i18n("Creates a tar.gz file from the current project sources") );
//  m_paProjectMakeDistSourceTgz->setWhatsThis(  );

  m_paProjectOptions->setShortText( i18n("Sets project and compiler options") );
//  m_paProjectOptions->setWhatsThis(  );

  // Build Actions
  m_paBuildCompileFile->setShortText( i18n("Compiles the current sourcefile") );
  m_paBuildCompileFile->setWhatsThis( i18n("Compile file\n\n"
                                      "Only compile the file opened in "
                                      "the C/C++ Files- window. The output "
                                      "is shown in the output window. If "
                                      "errors occur, clicking on the error line "
                                      "causes the file window to show you the "
                                      "line the error occured.") );

  m_paBuildMake->setShortText( i18n("Invokes make-command") );
  m_paBuildMake->setWhatsThis( i18n("Make\n\n"
                      								"Invokes the make-command set in the "
                      	              "options-menu for the current project "
                      	              "after saving all files. "
                      	              "This will compile all changed sources "
                      	              "since the last compilation was invoked.\n"
                      	              "The output window opens to show compiler "
                      	              "messages. If errors occur, clicking on the "
                      	              "error line will open the file where the "
                      	              "error was found and sets the cursor to the "
                      	              "error line.") );

  m_paBuildRebuildAll->setShortText( i18n("Rebuilds the project") );
  m_paBuildRebuildAll->setWhatsThis( i18n("Rebuild all\n\n"
                     								"After saving all files, rebuild all "
                     								"invokes the make-command set with the "
                     								"clean-option to remove all object files. "
                     								"Then, configure creates new Makefiles and "
                     								"the make-command will rebuild the project.") );

  m_paBuildCleanRebuildAll->setShortText( i18n("Invokes make clean and rebuild all") );
//  m_paBuildCleanRebuildAll->setWhatsThis(  );

  m_paBuildStop->setShortText( i18n("Stops the current process immediately") );
  m_paBuildStop->setWhatsThis( i18n("Stop\n\n"
                   								"If activated, the stop-command will interrupt "
                   	       				"the active process. This affects make-commands "
                   	           		"as well as documentation generation.") );

  m_paBuildExecute->setShortText( i18n("Invokes make-command and runs the program") );
  m_paBuildExecute->setWhatsThis( i18n("Execute\n\n"
                    								"After saving all files,the make-command is "
                    								"called to build the project. Then the binary "
                    								"is executed out of the project directory.\n"
                    	           		"Be aware that this function is only valid for "
                    	              "programs and that references to e.g. pixmaps "
                    	              "or html help files that are supposed to be "
                    	              "installed will cause some strange behavoir "
                    	              "like testing the helpmenu will open an error "
                    	              "message that the index.html file is not found.") );

  m_paBuildExecuteWithArgs->setShortText( i18n("Lets you set run-arguments to the binary and invokes the make-command") );
//  m_paBuildExecuteWithArgs->setWhatsThis(  );

  m_paBuildDistClean->setShortText( i18n("Invokes make distclean and deletes all compiled files") );
//  m_paBuildDistClean->setWhatsThis(  );

  m_paBuildAutoconf->setShortText( i18n("Invokes automake and co.") );
//  m_paBuildAutoconf->setWhatsThis(  );

  m_paBuildConfigure->setShortText( i18n("Invokes ./configure") );
//  m_paBuildConfigure->setWhatsThis(  );


  // Debug Actions
//  m_paDebugStart->setShortText(  );
//  m_paDebugStart->setWhatsThis(  );
//
//  m_paDebugExamineCore->setShortText(  );
//  m_paDebugExamineCore->setWhatsThis(  );
//
//  m_paDebugNamedFile->setShortText(  );
//  m_paDebugNamedFile->setWhatsThis(  );
//
//  m_paDebugAttatch->setShortText(  );
//  m_paDebugAttatch->setWhatsThis(  );
//
//  m_paDebugExecuteWithArgs->setShortText(  );
//  m_paDebugExecuteWithArgs->setWhatsThis(  );

  // Options Actions
  m_paOptionsEditor->setShortText( i18n("Sets the Editor's behavoir") );
//  m_paOptionsEditor->setWhatsThis(  );

  m_paOptionsEditorColors->setShortText( i18n("Sets the Editor's colors") );
//  m_paOptionsEditorColors->setWhatsThis(  );

  m_paOptionsEditorDefaults->setShortText( i18n("Sets the highlighting default colors") );
//  m_paOptionsEditorDefaults->setWhatsThis(  );

  m_paOptionsSyntaxHighlighting->setShortText( i18n("Sets the highlighting colors") );
//  m_paOptionsSyntaxHighlighting->setWhatsThis(  );

  m_paOptionsDocumentationBrowser->setShortText( i18n("Configures the Browser options") );
//  m_paOptionsDocumentationBrowser->setWhatsThis(  );

  m_paOptionsConfigureEnscript->setShortText( i18n("Configures the printer to use enscript") );
//,  m_paOptionsConfigureEnscript->setWhatsThis(  );

  m_paOptionsKDevelopSetup->setShortText( i18n("Configures KDevelop") );
//  m_paOptionsKDevelopSetup->setWhatsThis(  );

  // Bookmark Actions
  m_paBookmarksToggle->setShortText( i18n("Toggle a bookmark at the current cursor position") );
//  m_paBookmarksToggle->setWhatsThis(  );

  m_paBookmarksNext->setShortText(  i18n("Switches to the next bookmark position") );
//  m_paBookmarksNext->setWhatsThis(  );

  m_paBookmarksPrevious->setShortText(  i18n("Switches to the previous bookmark position") );
//  m_paBookmarksPrevious->setWhatsThis(  );

  m_paBookmarksClear->setShortText( i18n("Clears the bookmark list") );
//  m_paBookmarksClear->setWhatsThis(  );

  // Help Actions
  m_paHelpBack->setShortText( i18n("Switchs to last browser page") );
  m_paHelpBack->setWhatsThis( i18n("Documentation-Back\n\n"
                                   "This opens the previously visited page in the "
                                   "browser window. Holding the button pressed down "
                                   "will open a popup menu showing the browser history. "
                                   "Selecting an entry will open the according page in "
                                   "the browser window.") );

  m_paHelpForward->setShortText( i18n("Switchs to next browser page") );
  m_paHelpForward->setWhatsThis( i18n("Documentation-Forward\n\n"
                                   "This opens the next page in the history list. "
                                   "Holding the button pressed will show you a popup "
                                   "menu to browse visited pages forward.") );

  m_paHelpSearchMarkedText->setShortText( i18n("Searchs the selected text in the documentation") );
  m_paHelpSearchMarkedText->setWhatsThis( i18n("Search Marked Text\n\n"
                                   "Uses the searchdatabase to look for the selected "
                                   "text string and opens a summary page of found "
                                   "references for the selection in the browser window. "
                                   "There, you can select a documentation page of your "
                                   "choice and open it by following the link.") );

  m_paHelpSearchForHelpOn->setShortText( i18n("Lets you search individually for an expression") );
  m_paHelpSearchForHelpOn->setWhatsThis( i18n("Search for Help on...\n\n"
                                  "Opens the Search for Help on... dialog. There, you can "
                                  "search for a keyword in the documentation directly "
                                  "using the search database.") );

  m_paHelpWhatsThis->setShortText( i18n("Allows you to retrieve Whats This help over the KDevelop User Interface") );
//  m_paHelpWhatsThis->setWhatsThis(  );

  m_paHelpWelcome->setShortText( i18n("Switches to the KDevelop Documentation's Welcome page") );
//  m_paHelpWelcome->setWhatsThis(  );

  m_paHelpUserManual->setShortText( i18n("Switchs to KDevelop's User Manual") );
//  m_paHelpUserManual->setWhatsThis(  );

  m_paHelpProgramming->setShortText( i18n("Switchs to the KDevelop Programming Handbook") );
//  m_paHelpProgramming->setWhatsThis(  );

  m_paHelpTutorial->setShortText( i18n("Switches to the Tutorial Handbook") );
//  m_paHelpTutorial->setWhatsThis(  );

  m_paHelpKDELibRef->setShortText( i18n("Switches to the KDE Library Reference Guide") );
//  m_paHelpKDELibRef->setWhatsThis(  );

  m_paHelpCReference->setShortText( i18n("Switches to the C/C++ Reference") );
//  m_paHelpCReference->setWhatsThis(  );

  m_paHelpTipOfTheDay->setShortText( i18n("Opens the Tip of the Day dialog with hints for using KDevelop") );
//  m_paHelpTipOfTheDay->setWhatsThis(  );

  m_paHelpHomepage->setShortText( i18n("Enter the KDevelop Homepage") );
//  m_paHelpHomepage->setWhatsThis(  );

  m_paHelpReportBug->setShortText( i18n("Sends a bug-report email to the KDevelop Team") );
//  m_paHelpReportBug->setWhatsThis(  );

  m_paHelpAboutApp->setShortText( i18n("Information about the KDevelop IDE") );
//  m_paHelpAboutApp->setWhatsThis(  );

  m_paHelpAboutKDE->setShortText( i18n("Information about the KDE Project") );
//  m_paHelpAboutKDE->setWhatsThis(  );

}

void KDevelop::embedWidget(QWidget *w, KDevComponent::Role role, const QString &shortCaption, const QString &shortExplanation)
{
    // This is a hack to get the ball rolling...
    static KDockWidget *leftWidget = 0L;
    static KDockWidget::DockPosition leftpos = KDockWidget::DockLeft;
    static KDockWidget *bottomWidget = 0L;
    static KDockWidget::DockPosition bottompos = KDockWidget::DockBottom;
    static KDockWidget *mfWidget = 0L;

    KDockWidget *nextWidget = createDockWidget(QString(w->name()),
                                               w->icon()? *w->icon() : QPixmap(),
                                               0L,
                                               w->caption(),
                                               "");//shortCaption);
    nextWidget->setWidget(w);
    nextWidget->setToolTipString(shortExplanation);
    if (role == KDevComponent::SelectView) {
        if(leftWidget)
          nextWidget->manualDock(leftWidget, leftpos, 35);
        leftWidget = nextWidget;
        leftpos = KDockWidget::DockCenter;
        nextWidget->show();
    } else if (role == KDevComponent::OutputView) {
        if(bottomWidget)
          nextWidget->manualDock(bottomWidget, bottompos, 70);
        bottomWidget = nextWidget;
        bottompos = KDockWidget::DockCenter;
        nextWidget->show();
    } else if(role == KDevComponent::DocumentView) {
        // MDI view mainframe widget
        // another ugly hack to get it run, will be fixed soon...
        nextWidget->setEnableDocking(KDockWidget::DockNone);
        nextWidget->setDockSite(KDockWidget::DockCorner);
        setView(nextWidget);
        setMainDockWidget( nextWidget );
        mfWidget = nextWidget;
        ((KDockWidget*)(bottomWidget->parentWidget()->parentWidget()->parentWidget()))->manualDock(mfWidget,KDockWidget::DockBottom,70);
        ((KDockWidget*)(leftWidget->parentWidget()->parentWidget()->parentWidget()))->manualDock(mfWidget,KDockWidget::DockLeft,35);
    }
}

void KDevelop::slotFilePrint()
{
    KLibFactory *factory = KLibLoader::self()->factory("libkdevprintplugin");
    if (!factory)
        return;

    QStringList args;
    args << "/vmlinuz"; // temporary ;-)
    QObject *obj = factory->create(this, "print dialog", "KDevPrintDialog", args);
    if (!obj->inherits("QDialog")) {
        kdDebug(9000) << "Print plugin doesn't provide a dialog" << endl;
        return;
    }

    QDialog *dlg = (QDialog *)obj;
    dlg->exec();
    delete dlg;
}

void KDevelop::slotProjectNew(){
  NewProjectDlg* dlg = new NewProjectDlg();
  dlg->show();
  delete dlg;
}

void KDevelop::slotOptionsEditToolbars(){
  KEditToolbar dlg(actionCollection());
  
  if (dlg.exec())
    createGUI(0);
}

#include "kdevelop.moc"
