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
#include "kdevelop.h"
#include "kdevcomponent.h"
#include "kdevelopfactory.h"
#include "kdevelopcore.h"
#include "kdevviewhandler.h"
#include <kparts/event.h>
#include <kedittoolbar.h>

KDevelop::KDevelop( QWidget* pParent, const char *name, WFlags f) :
  KParts::DockMainWindow( pParent, name, f)
   ,m_dockbaseAreaOfDocumentViews(0L)
   ,m_dockOnLeft(0L)
   ,m_dockOnBottom(0L)
   ,m_pCore(0L)
{
  initActions();
  //  initHelp();

  setXMLFile( "kdevelopui.rc" );
  m_pCore = new KDevelopCore(this);
  createGUI(0);
  m_pCore->loadInitialComponents();
}


KDevelop::~KDevelop()
{
}


void KDevelop::initActions(){
  KAction *pAction;
  pAction = KStdAction::configureToolbars (this,SLOT(slotOptionsEditToolbars()),actionCollection());
}
void KDevelop::slotOptionsEditToolbars(){
  KEditToolbar dlg(factory());
  if (dlg.exec()){
    createGUI(0);
  }
}
/** initializes the help messages (whats this and
statusbar help) on the KActions */
void KDevelop::initHelp(){
/*  m_paFileNew->setStatusText( i18n("Creates a new file") );
  m_paFileNew->setWhatsThis( i18n("New file\n\n"
                                  "Opens the New file dialog to let you create "
                                  "a new project file. You can choose between "
                                  "several templates for creating the new file.") );

#ifdef removed
  m_paFileOpen->setStatusText( i18n("Opens an existing file") );
  m_paFileOpen->setWhatsThis( i18n("Open file\n\n"
 																	"Shows the Open file dialog to "
 																	"select a file to be opened. Holding "
 																	"the button pressed will show a popup "
 																	"menu containing all filenames of your "
 																	"project's sources and header files. "
 																	"Selecting a filename on the menu will "
 																	"then open the file according to the "
 																	"file-type.") );

  m_paFileClose->setStatusText( i18n("Closes the actual file") );
  m_paFileClose->setWhatsThis( i18n("Close file\n\n"
                                  "Closes the file in the currently active window. "
                                  "If the file has unsaved contents you will be "
                                  "asked for saving the file before closing.") );

  m_paFileCloseAll->setStatusText( i18n("Closes all files") );
  m_paFileCloseAll->setWhatsThis( i18n("Close all\n\n"
                                  "Closes all files currently open. "
                                  "If a file has unsaved contents you will be "
                                  "asked for saving the file before closing.") );

  m_paFileSave->setStatusText( i18n("Saves the actual document") );
  m_paFileSave->setWhatsThis( i18n("Save file\n\n"
 																	"Saves the file opened in the actual "
 																	"editing view") );

  m_paFileSaveAs->setStatusText( i18n("Saves the document as...") );
  m_paFileSaveAs->setWhatsThis( i18n("Save As...\n\n"
                                  "Saves the file in the currently active window "
                                  "under a new filename. You will be asked for "
                                  "a new filename for the file to be saved.") );

  m_paFileSaveAll->setStatusText( i18n("Saves all changed files") );
  m_paFileSaveAll->setWhatsThis( i18n("Save all\n\n"
                                  "Saves all currently open files. You can automate "
                                  "file saving by enabling autosave in the KDevelop "
                                  "Setup together with the desired saving intervall.") );

  m_paFileQuit->setStatusText( i18n("Exits KDevelop") );
  m_paFileQuit->setWhatsThis( i18n("Quit\n\n"
                                  "Closes your current KDevelop session. You will be asked "
                                  "to save the contents of currently opened and changed files "
                                  "before KDevelop exits.") );

  m_paEditUndo->setStatusText( i18n("Reverts the last editing step") );
  m_paEditUndo->setWhatsThis( i18n("Undo\n\n"
	                                "Reverts the last editing step.") );

  m_paEditRedo->setStatusText( i18n("Re-execute the last undone step") );
  m_paEditRedo->setWhatsThis( i18n("Redo\n\n"
                                  "If an editing step was undone, redo "
	                                "lets you do this step again.") );

  m_paEditUndoHistory->setStatusText( i18n("Opens the Undo History dialog") );
//  m_paEditUndoHistory->setWhatsThis(  );

  m_paEditCut->setStatusText( i18n("Cuts the selected section and puts it to the clipboard") );
  m_paEditCut->setWhatsThis( i18n("Cut\n\n"
	                               "Cuts out the selected text and copies "
	                               "it to the system clipboard.") );

  m_paEditCopy->setStatusText( i18n("Copys the selected section to the clipboard") );
  m_paEditCopy->setWhatsThis( i18n("Copy\n\n"
                                  "Copies the selected text into the "
                                  "system clipboard.") );

  m_paEditPaste->setStatusText( i18n("Pastes the clipboard contents to actual position") );
  m_paEditPaste->setWhatsThis( i18n("Paste\n\n"
	                                  "Inserts the contents of the "
	                                  "system clipboard at the current "
	                                  "cursor position. ") );

  m_paEditIndent->setStatusText( i18n("Moves the selection to the right") );
  m_paEditIndent->setWhatsThis( i18n("Indent\n\n"
                                      "Moves a selection to the right.") );

  m_paEditUnindent->setStatusText( i18n("Moves the selection to the left") );
  m_paEditUnindent->setWhatsThis( i18n("Unindent\n\n"
                                      "Moves a selection to the left.") );

  m_paEditComment->setStatusText( i18n("Comments out the selected line(s)") );
  m_paEditComment->setWhatsThis( i18n("Comment\n\n"
                                      "This command comments out the selected lines.") );

  m_paEditUncomment->setStatusText( i18n("Uncomments the selected line(s)") );
  m_paEditUncomment->setWhatsThis( i18n("Uncomment\n\n"
                                        "Uncomment out commented lines.") );

  m_paEditInsertFile->setStatusText( i18n("Inserts a file at the current position") );
  m_paEditInsertFile->setWhatsThis( i18n("Insert File\n\n"
                                        "Inserts the contents of a selected "
                                        "file at the current cursor position.") );

  m_paEditSearch->setStatusText( i18n("Searchs the file for an expression") );
  m_paEditSearch->setWhatsThis( i18n("Search\n\n"
                                    "Opens the Search dialog to search "
                                    "for an expression in the current file.") );

  m_paEditRepeatSearch->setStatusText( i18n("Repeats the last search") );
  m_paEditRepeatSearch->setWhatsThis( i18n("Repeat Search\n\n"
                                    "Repeats the last search and marks "
                                    "the next result.") );

  m_paEditReplace->setStatusText( i18n("Searchs and replace expression") );
  m_paEditReplace->setWhatsThis( i18n("Search and Replace\n\n"
                                   "Opens the Search and Replace "
                                   "dialog to enter an expression to "
                                   "search for and an expression that "
                                   "will replace any matches.") );

  m_paEditSelectAll->setStatusText( i18n("Selects the whole document contents") );
//  m_paEditSelectAll->setWhatsThis(  );

  m_paEditDeselectAll->setStatusText( i18n("Deselects the whole document contents") );
//  m_paEditDeselectAll->setWhatsThis(  );

  m_paEditInvertSelection->setStatusText( i18n("Inverts the current selection") );
//  m_paEditInvertSelection->setWhatsThis(  );


  // View Actions
  m_paViewGotoLine->setStatusText( i18n("Goes to Line Number...") );
//  m_paViewGotoLine->setWhatsThis(  );
#endif

  m_paViewTreeView->setStatusText( i18n("Enables / disables the treeview") );
  m_paViewTreeView->setWhatsThis( i18n("Tree-View\n\n"
            											"Enables/ disables the tree window. The "																
            											"state of the window is displayed by the "
            											"toggle button: if the button is pressed, "
            											"the window is open, otherwise closed.") );

  m_paViewOutputView->setStatusText( i18n("Enables / disables the outputview") );
  m_paViewOutputView->setWhatsThis( i18n("Output-View\n\n"
             											"Enables/ disables the output window. The "																
             											"state of the window is displayed by the "
             											"toggle button: if the button is pressed, "
             											"the window is open, otherwise closed.") );

  m_paViewToolbar->setStatusText( i18n("Enables / disables the standard toolbar") );
//  m_paViewToolbar->setWhatsThis(  );

  m_paViewBrowserToolbar->setStatusText( i18n("Enables / disables the browser toolbar") );
//  m_paViewBrowserToolbar->setWhatsThis(  );

  m_paViewMDITaskbar->setStatusText( i18n("Enables / disables the MDI view taskbar") );
//  m_paViewMDITaskbar->setWhatsThis(  );

  m_paViewStatusbar->setStatusText( i18n("Enables / disables the statusbar") );
//  m_paViewStatusbar->setWhatsThis(  );
//


  m_paProjectFileProperties->setStatusText( i18n("Shows the file properties dialog") );
//  m_paProjectFileProperties->setWhatsThis(  );

  m_paProjectMessages->setStatusText( i18n("Invokes make to create the message file by extracting all i18n() macros") );
//  m_paProjectMessages->setWhatsThis(  );

  m_paProjectAPI->setStatusText( i18n("Creates the Project's API documentation with KDoc") );
//  m_paProjectAPI->setWhatsThis(  );

  m_paProjectManual->setStatusText( i18n("Creates the Project's User Manual") );
//  m_paProjectManual->setWhatsThis(  );

  m_paProjectMakeDistSourceTgz->setStatusText( i18n("Creates a tar.gz file from the current project sources") );
//  m_paProjectMakeDistSourceTgz->setWhatsThis(  );

  // Build Actions
  m_paBuildCompileFile->setStatusText( i18n("Compiles the current sourcefile") );
  m_paBuildCompileFile->setWhatsThis( i18n("Compile file\n\n"
                                      "Only compile the file opened in "
                                      "the C/C++ Files- window. The output "
                                      "is shown in the output window. If "
                                      "errors occur, clicking on the error line "
                                      "causes the file window to show you the "
                                      "line the error occured.") );

  m_paBuildMake->setStatusText( i18n("Invokes make-command") );
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

  m_paBuildRebuildAll->setStatusText( i18n("Rebuilds the project") );
  m_paBuildRebuildAll->setWhatsThis( i18n("Rebuild all\n\n"
                     								"After saving all files, rebuild all "
                     								"invokes the make-command set with the "
                     								"clean-option to remove all object files. "
                     								"Then, configure creates new Makefiles and "
                     								"the make-command will rebuild the project.") );

  m_paBuildCleanRebuildAll->setStatusText( i18n("Invokes make clean and rebuild all") );
//  m_paBuildCleanRebuildAll->setWhatsThis(  );

  m_paBuildStop->setStatusText( i18n("Stops the current process immediately") );
  m_paBuildStop->setWhatsThis( i18n("Stop\n\n"
                   								"If activated, the stop-command will interrupt "
                   	       				"the active process. This affects make-commands "
                   	           		"as well as documentation generation.") );

  m_paBuildExecute->setStatusText( i18n("Invokes make-command and runs the program") );
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

  m_paBuildExecuteWithArgs->setStatusText( i18n("Lets you set run-arguments to the binary and invokes the make-command") );
//  m_paBuildExecuteWithArgs->setWhatsThis(  );

  m_paBuildDistClean->setStatusText( i18n("Invokes make distclean and deletes all compiled files") );
//  m_paBuildDistClean->setWhatsThis(  );

  m_paBuildAutoconf->setStatusText( i18n("Invokes automake and co.") );
//  m_paBuildAutoconf->setWhatsThis(  );

  m_paBuildConfigure->setStatusText( i18n("Invokes ./configure") );
//  m_paBuildConfigure->setWhatsThis(  );


  // Options Actions
  m_paOptionsEditor->setStatusText( i18n("Sets the Editor's behavoir") );
//  m_paOptionsEditor->setWhatsThis(  );

  m_paOptionsEditorColors->setStatusText( i18n("Sets the Editor's colors") );
//  m_paOptionsEditorColors->setWhatsThis(  );

  m_paOptionsEditorDefaults->setStatusText( i18n("Sets the highlighting default colors") );
//  m_paOptionsEditorDefaults->setWhatsThis(  );

  m_paOptionsSyntaxHighlighting->setStatusText( i18n("Sets the highlighting colors") );
//  m_paOptionsSyntaxHighlighting->setWhatsThis(  );

  m_paOptionsDocumentationBrowser->setStatusText( i18n("Configures the Browser options") );
//  m_paOptionsDocumentationBrowser->setWhatsThis(  );

  m_paOptionsConfigureEnscript->setStatusText( i18n("Configures the printer to use enscript") );
//,  m_paOptionsConfigureEnscript->setWhatsThis(  );

  // Bookmark Actions
  m_paBookmarksToggle->setStatusText( i18n("Toggle a bookmark at the current cursor position") );
//  m_paBookmarksToggle->setWhatsThis(  );

  m_paBookmarksNext->setStatusText(  i18n("Switches to the next bookmark position") );
//  m_paBookmarksNext->setWhatsThis(  );

  m_paBookmarksPrevious->setStatusText(  i18n("Switches to the previous bookmark position") );
//  m_paBookmarksPrevious->setWhatsThis(  );

  m_paBookmarksClear->setStatusText( i18n("Clears the bookmark list") );
//  m_paBookmarksClear->setWhatsThis(  );

  // Help Actions
  m_paHelpBack->setStatusText( i18n("Switchs to last browser page") );
  m_paHelpBack->setWhatsThis( i18n("Documentation-Back\n\n"
                                   "This opens the previously visited page in the "
                                   "browser window. Holding the button pressed down "
                                   "will open a popup menu showing the browser history. "
                                   "Selecting an entry will open the according page in "
                                   "the browser window.") );

  m_paHelpForward->setStatusText( i18n("Switchs to next browser page") );
  m_paHelpForward->setWhatsThis( i18n("Documentation-Forward\n\n"
                                   "This opens the next page in the history list. "
                                   "Holding the button pressed will show you a popup "
                                   "menu to browse visited pages forward.") );

  m_paHelpSearchMarkedText->setStatusText( i18n("Searchs the selected text in the documentation") );
  m_paHelpSearchMarkedText->setWhatsThis( i18n("Search Marked Text\n\n"
                                   "Uses the searchdatabase to look for the selected "
                                   "text string and opens a summary page of found "
                                   "references for the selection in the browser window. "
                                   "There, you can select a documentation page of your "
                                   "choice and open it by following the link.") );

  m_paHelpSearchForHelpOn->setStatusText( i18n("Lets you search individually for an expression") );
  m_paHelpSearchForHelpOn->setWhatsThis( i18n("Search for Help on...\n\n"
                                  "Opens the Search for Help on... dialog. There, you can "
                                  "search for a keyword in the documentation directly "
                                  "using the search database.") );

  m_paHelpWhatsThis->setStatusText( i18n("Allows you to retrieve Whats This help over the KDevelop User Interface") );
//  m_paHelpWhatsThis->setWhatsThis(  );

  m_paHelpWelcome->setStatusText( i18n("Switches to the KDevelop Documentation's Welcome page") );
//  m_paHelpWelcome->setWhatsThis(  );

  m_paHelpUserManual->setStatusText( i18n("Switchs to KDevelop's User Manual") );
//  m_paHelpUserManual->setWhatsThis(  );

  m_paHelpProgramming->setStatusText( i18n("Switchs to the KDevelop Programming Handbook") );
//  m_paHelpProgramming->setWhatsThis(  );

  m_paHelpTutorial->setStatusText( i18n("Switches to the Tutorial Handbook") );
//  m_paHelpTutorial->setWhatsThis(  );

  m_paHelpKDELibRef->setStatusText( i18n("Switches to the KDE Library Reference Guide") );
//  m_paHelpKDELibRef->setWhatsThis(  );

  m_paHelpCReference->setStatusText( i18n("Switches to the C/C++ Reference") );
//  m_paHelpCReference->setWhatsThis(  );

  m_paHelpTipOfTheDay->setStatusText( i18n("Opens the Tip of the Day dialog with hints for using KDevelop") );
//  m_paHelpTipOfTheDay->setWhatsThis(  );

  m_paHelpHomepage->setStatusText( i18n("Enter the KDevelop Homepage") );
//  m_paHelpHomepage->setWhatsThis(  );

  m_paHelpReportBug->setStatusText( i18n("Sends a bug-report email to the KDevelop Team") );
//  m_paHelpReportBug->setWhatsThis(  );

  m_paHelpAboutApp->setStatusText( i18n("Information about the KDevelop IDE") );
//  m_paHelpAboutApp->setWhatsThis(  );

  m_paHelpAboutKDE->setStatusText( i18n("Information about the KDE Project") );
//  m_paHelpAboutKDE->setWhatsThis(  );
*/
}

void KDevelop::embedWidget(QWidget *w, KDevComponent::Role role, const QString &shortCaption, const QString &shortExplanation)
{
  // document view area has to be created first
  if ((role == KDevComponent::SelectView) || (role == KDevComponent::OutputView))
  {
    if (!m_dockbaseAreaOfDocumentViews)
    {
      kdDebug(9000) << "KDevelop::embedWidget failed. (No available KDockWidget to dock to)" << endl;
      return;
    }
  }

  KDockWidget *nextWidget = createDockWidget(QString(w->name()),
                                             w->icon()? *w->icon() : QPixmap(),
                                             0L,
                                             w->caption(),
                                             shortCaption);
  nextWidget->setWidget(w);
  nextWidget->setToolTipString(shortExplanation);

  if (role == KDevComponent::SelectView)
  {
    if (m_dockOnLeft)
      nextWidget->manualDock( m_dockOnLeft, KDockWidget::DockCenter, 35);
    else
      nextWidget->manualDock( m_dockbaseAreaOfDocumentViews, KDockWidget::DockLeft, 35);

    nextWidget->show();
    m_dockOnLeft = nextWidget; 
  }
  else
  {
    if (role == KDevComponent::OutputView)
    {
      if (m_dockOnBottom)
        nextWidget->manualDock( m_dockOnBottom, KDockWidget::DockCenter, 70);
      else
        nextWidget->manualDock( m_dockbaseAreaOfDocumentViews, KDockWidget::DockBottom, 70);
      nextWidget->show();
      m_dockOnBottom = nextWidget; 
    }
    else
    {
      if (role == KDevComponent::DocumentView)
      {
        // TODO: check the configuration!
        if( getMainDockWidget()->caption() != QString("default"))
        {
          // call the view handler service
          emit addView( w);
          KParts::GUIActivateEvent ev( true );
          QApplication::sendEvent( m_pCore->viewHandler(), &ev );
        }
        else
        {
          // default: stack dockwidgets
          m_dockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner | KDockWidget::DockCenter);
          nextWidget->manualDock( m_dockbaseAreaOfDocumentViews, KDockWidget::DockCenter);
        }
        m_dockOnLeft = nextWidget; 
      }
      else
      {
        if (role == KDevComponent::AreaOfDocumentViews)
        {
          // is the MDI mainframe when using QextMDI or QWorkspace
          nextWidget->setEnableDocking(KDockWidget::DockNone);
          nextWidget->setDockSite(KDockWidget::DockCorner);
          setView(nextWidget);
          setMainDockWidget( nextWidget );
          m_dockbaseAreaOfDocumentViews = nextWidget;
        }
      }
    }
  }
}


void KDevelop::stackView( QWidget* w)
{
  KDockWidget *nextWidget = createDockWidget(QString(w->name()),
                                             w->icon()? *w->icon() : QPixmap(),
                                             0L,
                                             w->caption());
  nextWidget->setWidget( w);
  nextWidget->manualDock( m_dockbaseAreaOfDocumentViews, KDockWidget::DockCenter);
}




#include "kdevelop.moc"
