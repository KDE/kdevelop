/***************************************************************************
                          kdevelop.h  -  description
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

#ifndef KDEVELOP_H
#define KDEVELOP_H

#include <kapp.h>
#include <kaction.h>
#include <kparts/dockmainwindow.h>
#include "kdevcomponent.h"


class KDevelopCore;


class KDevelop : public KParts::DockMainWindow
{
 Q_OBJECT
public:
  /** construtor */
  KDevelop(const char *name=0);
  /** destructor */
  ~KDevelop();

public slots:
  /** Embed the widgets of components in the GUI. */
  void embedWidget(QWidget *w, KDevComponent::Role role, const QString &shortCaption, const QString &shortExplanation);

protected slots: // Protected slots
  /** reimplemented from KParts::MainWindow */
  void slotSetStatusBarText( const QString &text);
private:
  /** sets up the KActions designed User Interface
			for the toolbars and menubar */
  void initActions();
  /** initializes the help messages (whats this and
      statusbar help) on the KActions */
  void initHelp();

  // File Actions

  KAction* m_paFileNew;
  KAction* m_paFileOpen;
  KAction* m_paFileClose;
  KAction* m_paFileCloseAll;
  KAction* m_paFileSave;
  KAction* m_paFileSaveAs;
  KAction* m_paFileSaveAll;
  KAction* m_paFilePrint;
  KAction* m_paFileQuit;
  // Edit Actions
  KAction* m_paEditUndo;
  KAction* m_paEditRedo;
  KAction* m_paEditUndoHistory;
  KAction* m_paEditCut;
  KAction* m_paEditCopy;
  KAction* m_paEditPaste;
  KAction* m_paEditIndent;
  KAction* m_paEditUnindent;
  KAction* m_paEditComment;
  KAction* m_paEditUncomment;
  KAction* m_paEditInsertFile;
  KAction* m_paEditSearch;
  KAction* m_paEditRepeatSearch;
  KAction* m_paEditReplace;
  KAction* m_paEditSelectAll;
  KAction* m_paEditDeselectAll;
  KAction* m_paEditInvertSelection;
  // View Actions
  KAction* m_paViewGotoLine;
  KToggleAction* m_paViewTreeView;
  KToggleAction* m_paViewOutputView;
  KToggleAction* m_paViewToolbar;
  KToggleAction* m_paViewBrowserToolbar;
  KToggleAction* m_paViewMDITaskbar;
  KToggleAction* m_paViewStatusbar;
  KAction* m_paViewDebugVar;
  KAction* m_paViewDebugBreakpoints;
  KAction* m_paViewDebugFrameStack;
  KAction* m_paViewDebugDisassemble;
  KAction* m_paViewDebugDebugger;
  KAction* m_paViewRefresh;
  // Project Actions
  KAction* m_paProjectNew;
  KAction* m_paProjectOpen;
  KAction* m_paProjectOpenRecent;
  KAction* m_paProjectClose;
  KAction* m_paProjectNewClass;
  KAction* m_paProjectAddExistingFiles;
  KAction* m_paProjectAddNewTranslationFile;
  KAction* m_paProjectFileProperties;
  KAction* m_paProjectMessages;
  KAction* m_paProjectAPI;
  KAction* m_paProjectManual;
  KAction* m_paProjectMakeDistSourceTgz;
  KAction* m_paProjectOptions;
  // Build Actions
  KAction* m_paBuildCompileFile;
  KAction* m_paBuildMake;
  KAction* m_paBuildRebuildAll;
  KAction* m_paBuildCleanRebuildAll;
  KAction* m_paBuildStop;
  KAction* m_paBuildExecute;
  KAction* m_paBuildExecuteWithArgs;
  KAction* m_paBuildDistClean;
  KAction* m_paBuildAutoconf;
  KAction* m_paBuildConfigure;
  // Debug Actions
  KAction* m_paDebugStart;
  KAction* m_paDebugExamineCore;
  KAction* m_paDebugNamedFile;
  KAction* m_paDebugAttatch;
  KAction* m_paDebugExecuteWithArgs;
  // Options Actions
  KAction* m_paOptionsEditor;
  KAction* m_paOptionsEditorColors;
  KAction* m_paOptionsEditorDefaults;
  KAction* m_paOptionsSyntaxHighlighting;
  KAction* m_paOptionsDocumentationBrowser;
  KAction* m_paOptionsConfigureEnscript;
  KAction* m_paOptionsKDevelopSetup;
  KAction* m_paOptionsEditToolbars;
  // Bookmark Actions
  KAction* m_paBookmarksToggle;
  KAction* m_paBookmarksNext;
  KAction* m_paBookmarksPrevious;
  KAction* m_paBookmarksClear;
  // Help Actions
  KAction* m_paHelpBack;
  KAction* m_paHelpForward;
  KAction* m_paHelpSearchMarkedText;
  KAction* m_paHelpSearchForHelpOn;
  KAction* m_paHelpWhatsThis;
  KAction* m_paHelpWelcome;
  KAction* m_paHelpUserManual;
  KAction* m_paHelpProgramming;
  KAction* m_paHelpTutorial;
  KAction* m_paHelpKDELibRef;
  KAction* m_paHelpCReference;
  KAction* m_paHelpTipOfTheDay;
  KAction* m_paHelpHomepage;
  KAction* m_paHelpReportBug;
  KAction* m_paHelpAboutApp;
  KAction* m_paHelpAboutKDE;

  //
  // the covering dockwidgets for all views in KDevelop (using Dockwidgets of kdeui)
  //

  /** The MDI-mainframe's dock cover */
  KDockWidget* m_dockbaseMDIMainFrm;

  /** The classview. */
  KDockWidget* m_dockbaseClassTree;
  /** The logical fileview. */
  KDockWidget* m_dockbaseLogFileTree;
  /** The real fileview. */
  KDockWidget* m_dockbaseRealFileTree;
  /** The debugger's tree of local variables */
  KDockWidget* m_dockbaseVarView;
  /** The documentation tree. */
  KDockWidget* m_dockbaseDocTree;
  /** splitview, contains a WidgetsView and a PropertyView */
  KDockWidget* m_dockbaseWidPropSplitView;

  /** Output from the compiler ... */
  KDockWidget* m_dockbaseMessagesView;
  /** Output from grep */
  KDockWidget* m_dockbaseGrepView;
  /** Output from the application */
  KDockWidget* m_dockbaseOutputView;
  /** Manages a list of breakpoints - Always active */
  KDockWidget* m_dockbaseBrkptManagerView;
  /** Manages a frame stack list */
  KDockWidget* m_dockbaseFrameStackView;
  /** show disassembled code being run */
  KDockWidget* m_dockbaseDisassembleView;
  /** debug aid. Switch on using compile switch GDB_MONITOR or DBG_MONITOR */
  KDockWidget* m_dockbaseDbgWidgetView;			

  KDevelopCore *m_kdevelopcore;
    
private slots:
   void slotFilePrint();
 void slotProjectNew();
 void slotOptionsKDevelopSetup();
 void slotOptionsEditToolbars();
};

#endif
