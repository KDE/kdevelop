/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *	                                                                       *
 *                         Adapted for ruby debugging                      *
 *                         --------------------------                      *
 *  begin                : Mon Nov 1 2004                                  *
 *  copyright            : (C) 2004 by Richard Dale                        *
 *  email                : Richard_Dale@tipitina.demon.co.uk               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>

#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kdevgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kparts/part.h>
#include <ktexteditor/viewcursorinterface.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <qtimer.h>
#include <kstringhandler.h>
#include <kstandarddirs.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevdebugger.h"
#include "domutil.h"
#include "variablewidget.h"
#include "rdbbreakpointwidget.h"
#include "framestackwidget.h"
#include "processwidget.h"
#include "rdbcontroller.h"
#include "breakpoint.h"
#include "dbgpsdlg.h"
#include "dbgtoolbar.h"
#include "rdbparser.h"
#include "rdboutputwidget.h"
#include "processlinemaker.h"

#include <iostream>

#include <kdevplugininfo.h>
#include <debugger.h>

#include "debuggerpart.h"

namespace RDBDebugger
{
static const KDevPluginInfo data("kdevrbdebugger");

typedef KDevGenericFactory<RubyDebuggerPart> RubyDebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevrbdebugger, RubyDebuggerFactory( data ) )

RubyDebuggerPart::RubyDebuggerPart( QObject *parent, const char *name, const QStringList & ) :
    KDevPlugin( &data, parent, name ? name : "RubyDebuggerPart" ),
    controller(0)
{
//    setObjId("RubyDebuggerInterface");
    setInstance(RubyDebuggerFactory::instance());

    setXMLFile("kdevrbdebugger.rc");

    m_debugger = new Debugger( partController() );
    
	statusBarIndicator = new QLabel(" ", mainWindow()->statusBar());
    statusBarIndicator->setFixedWidth(15);
    mainWindow()->statusBar()->addWidget(statusBarIndicator, 0, true);
    statusBarIndicator->show();

    // Setup widgets and dbgcontroller
    variableWidget = new VariableWidget( 0, "rdbVariablewidget");
//     /*variableWidget*/->setEnabled(false);
    variableWidget->setIcon(SmallIcon("math_brace"));
    variableWidget->setCaption(i18n("Variable Tree"));
    QWhatsThis::add
        (variableWidget, i18n("<b>Variable tree</b><p>"
                              "The variable tree allows you to see "
                              "the variable values as you step "
                              "through your program using the internal "
                              "debugger. Click the right mouse button on items in "
                              "this view to get a popup menu.\n"
                              "To speed up stepping through your code "
                              "leave the tree items closed.\n"));
    mainWindow()->embedSelectView(variableWidget, i18n("Variables / Watch"), i18n("Debugger variable-view"));
//    mainWindow()->setViewAvailable(variableWidget, false);

    rdbBreakpointWidget = new RDBBreakpointWidget( 0, "rdbBreakpointWidget" );
    rdbBreakpointWidget->setCaption(i18n("Breakpoint List"));
    QWhatsThis::add
        (rdbBreakpointWidget, i18n("<b>Breakpoint list</b><p>"
                                "Displays a list of breakpoints with "
                                "their current status. Clicking on a "
                                "breakpoint item allows you to change "
                                "the breakpoint and will take you "
                                "to the source in the editor window."));
    rdbBreakpointWidget->setIcon( SmallIcon("stop") );
    mainWindow()->embedOutputView(rdbBreakpointWidget, i18n("Breakpoints"), i18n("Debugger breakpoints"));

    framestackWidget = new FramestackWidget( 0, "rdbFramestackWidget" );
    framestackWidget->setEnabled(false);
    framestackWidget->setCaption(i18n("Frame Stack"));
    QWhatsThis::add
        (framestackWidget, i18n("<b>Frame stack</b><p>"
                                "Often referred to as the \"call stack\", "
                                "this is a list showing what method is "
                                "currently active and who called each "
                                "method to get to this point in your "
                                "program. By clicking on an item you "
                                "can see the values in any of the "
                                "previous calling methods."));
    framestackWidget->setIcon( SmallIcon("table") );
    mainWindow()->embedOutputView(framestackWidget, i18n("Frame Stack"), i18n("Debugger method call stack"));
    mainWindow()->setViewAvailable(framestackWidget, false);


    rdbOutputWidget = new RDBOutputWidget( 0, "rdbOutputWidget" );
    rdbOutputWidget->setEnabled(false);
    rdbOutputWidget->setIcon( SmallIcon("inline_image") );
    rdbOutputWidget->setCaption(i18n("RDB Output"));
    QWhatsThis::add
        (rdbOutputWidget, i18n("<b>RDB output</b><p>"
                                 "Shows all rdb commands being executed. "
                                 "You can also issue any other rdb command while debugging."));
    mainWindow()->embedOutputView(rdbOutputWidget, i18n("RDB"),
                                  i18n("RDB output"));
    mainWindow()->setViewAvailable(rdbOutputWidget, false);

    // rdbBreakpointWidget -> this
    connect( rdbBreakpointWidget, SIGNAL(refreshBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( rdbBreakpointWidget, SIGNAL(publishBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( rdbBreakpointWidget, SIGNAL(gotoSourcePosition(const QString&, int)),
             this,             SLOT(slotGotoSource(const QString&, int)) );

    // Now setup the actions
    KAction *action;

//    action = new KAction(i18n("&Start"), "1rightarrow", CTRL+SHIFT+Key_F9,
    action = new KAction(i18n("&Start"), "dbgrun", CTRL+SHIFT+Key_F9,
                         this, SLOT(slotRun()),
                         actionCollection(), "debug_run");
    action->setToolTip( i18n("Start in debugger") );
    action->setWhatsThis( i18n("<b>Start in debugger</b><p>"
                               "Starts the debugger with the project's main "
                               "executable. You may set some breakpoints "
                               "before this, or you can interrupt the program "
                               "while it is running, in order to get information "
                               "about variables, frame stack, and so on.") );

    action = new KAction(i18n("Sto&p"), "stop", 0,
                         this, SLOT(slotStop()),
                         actionCollection(), "debug_stop");
    action->setToolTip( i18n("Stop debugger") );
    action->setWhatsThis(i18n("<b>Stop debugger</b><p>Kills the executable and exits the debugger."));

    action = new KAction(i18n("Interrupt"), "player_pause", 0,
                         this, SLOT(slotPause()),
                         actionCollection(), "debug_pause");
    action->setToolTip( i18n("Interrupt application") );
    action->setWhatsThis(i18n("<b>Interrupt application</b><p>Interrupts the debugged process or current RDB command."));

    action = new KAction(i18n("Run to &Cursor"), "dbgrunto", 0,
                         this, SLOT(slotRunToCursor()),
                         actionCollection(), "debug_runtocursor");
    action->setToolTip( i18n("Run to cursor") );
    action->setWhatsThis(i18n("<b>Run to cursor</b><p>Continues execution until the cursor position is reached."));


    action = new KAction(i18n("Step &Over"), "dbgnext", 0,
                         this, SLOT(slotStepOver()),
                         actionCollection(), "debug_stepover");
    action->setToolTip( i18n("Step over the next line") );
    action->setWhatsThis( i18n("<b>Step over</b><p>"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a method the whole "
                               "method is executed and the app will stop at the line "
                               "following the method call.") );


    action = new KAction(i18n("Step &Into"), "dbgstep", 0,
                         this, SLOT(slotStepInto()),
                         actionCollection(), "debug_stepinto");
    action->setToolTip( i18n("Step into the next statement") );
    action->setWhatsThis( i18n("<b>Step into</b><p>"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a method then execution will stop after "
                               "the method has been entered.") );



    action = new KAction(i18n("Step O&ut"), "dbgstepout", 0,
                         this, SLOT(slotStepOut()),
                         actionCollection(), "debug_stepout");
    action->setToolTip( i18n("Steps out of the current method") );
    action->setWhatsThis( i18n("<b>Step out</b><p>"
                               "Executes the application until the currently executing "
                               "method is completed. The debugger will then display "
                               "the line after the original call to that method. If "
                               "program execution is in the outermost frame (i.e. in "
                               "the topleveltoggleWatchpoint) then this operation has no effect.") );


    action = new KAction(i18n("Toggle Breakpoint"), 0, 0,
                         this, SLOT(toggleBreakpoint()),
                         actionCollection(), "debug_toggle_breakpoint");
    action->setToolTip(i18n("Toggle breakpoint"));
    action->setWhatsThis(i18n("<b>Toggle breakpoint</b><p>Toggles the breakpoint at the current line in editor."));

    connect( mainWindow()->main()->guiFactory(), SIGNAL(clientAdded(KXMLGUIClient*)),
             this, SLOT(guiClientAdded(KXMLGUIClient*)) );


    connect( partController(), SIGNAL(loadedFile(const KURL &)),
             rdbBreakpointWidget, SLOT(slotRefreshBP(const KURL &)) );
    connect( debugger(), SIGNAL(toggledBreakpoint(const QString &, int)),
             rdbBreakpointWidget, SLOT(slotToggleBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(editedBreakpoint(const QString &, int)),
             rdbBreakpointWidget, SLOT(slotEditBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             rdbBreakpointWidget, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );

    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

    connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
             this, SLOT(slotStop(KDevPlugin*)) );
    connect( core(), SIGNAL(projectClosed()),
             this, SLOT(projectClosed()) );

    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );

    procLineMaker = new ProcessLineMaker();

    connect( procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
             appFrontend(), SLOT(insertStdoutLine(const QString&)) );
    connect( procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
             appFrontend(), SLOT(insertStderrLine(const QString&)) );

    setupController();
    QTimer::singleShot(0, this, SLOT(setupDcop()));
}

RubyDebuggerPart::~RubyDebuggerPart()
{
    kapp->dcopClient()->setNotifications(false);

    if (variableWidget)
        mainWindow()->removeView(variableWidget);
    if (rdbBreakpointWidget)
        mainWindow()->removeView(rdbBreakpointWidget);
    if (framestackWidget)
        mainWindow()->removeView(framestackWidget);
    if(rdbOutputWidget)
        mainWindow()->removeView(rdbOutputWidget);

    delete variableWidget;
    delete rdbBreakpointWidget;
    delete framestackWidget;
    delete rdbOutputWidget;
    delete controller;
    delete floatingToolBar;
    delete statusBarIndicator;
    delete procLineMaker;
}


void RubyDebuggerPart::guiClientAdded( KXMLGUIClient* client )
{
    // Can't change state until after XMLGUI has been loaded...
    // Anyone know of a better way of doing this?
    if( client == this )
        stateChanged( QString("stopped") );
}

void RubyDebuggerPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType( Context::EditorContext ))
        return;

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    m_contextIdent = econtext->currentWord();

    popup->insertSeparator();
    if (econtext->url().isLocalFile())
    {
        int id = popup->insertItem( i18n("Toggle Breakpoint"), this, SLOT(toggleBreakpoint()) );
        popup->setWhatsThis(id, i18n("<b>Toggle breakpoint</b><p>Toggles breakpoint at the current line."));
    }
    if (!m_contextIdent.isEmpty())
    {
        QString squeezed = KStringHandler::csqueeze(m_contextIdent, 30);
        int id = popup->insertItem( i18n("Watch: %1").arg(squeezed), this, SLOT(contextWatch()) );
        popup->setWhatsThis(id, i18n("<b>Watch</b><p>Adds an expression under the cursor to the Variables/Watch list."));
        
		id = popup->insertItem( i18n("Inspect: %1").arg(squeezed), this, SLOT(contextRubyInspect()) );
        popup->setWhatsThis(id, i18n("<b>Inspect</b><p>Evaluates an expression under the cursor."));
    }
}


void RubyDebuggerPart::toggleBreakpoint()
{
    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(partController()->activeWidget());

    if (!rwpart || !cursorIface)
        return;

    uint line, col;
    cursorIface->cursorPositionReal(&line, &col);

    rdbBreakpointWidget->slotToggleBreakpoint(rwpart->url().path(), line);
}


void RubyDebuggerPart::contextWatch()
{
    variableWidget->slotAddWatchExpression(m_contextIdent);
}

// Evaluates the selected text
void RubyDebuggerPart::contextRubyInspect()
{
	emit rubyInspect(m_contextIdent);
}


void RubyDebuggerPart::setupController()
{
    VariableTree *variableTree = variableWidget->varTree();

    controller = new RDBController(variableTree, framestackWidget, *projectDom());

    // this -> controller
    connect( this,                  SIGNAL(rubyInspect(const QString&)),
             controller,            SLOT(slotRubyInspect(const QString&)));

    // variableTree -> framestackWidget
    connect( variableTree,          SIGNAL(selectFrame(int, int)),
             framestackWidget,      SLOT(slotSelectFrame(int, int)));
    
	// framestackWidget -> variableTree
    connect( framestackWidget,      SIGNAL(frameActive(int, int, const QString&)),
             variableTree,          SLOT(slotFrameActive(int, int, const QString&)));
	
    // variableTree -> controller
    connect( variableTree,          SIGNAL(expandItem(VarItem*, const QCString&)),
             controller,            SLOT(slotExpandItem(VarItem*, const QCString&)));
    connect( variableTree,          SIGNAL(fetchGlobals(bool)),
             controller,            SLOT(slotFetchGlobals(bool)));
    connect( variableTree,          SIGNAL(addWatchExpression(const QString&, bool)),
             controller,            SLOT(slotAddWatchExpression(const QString&, bool)));  
    connect( variableTree,          SIGNAL(removeWatchExpression(int)),
             controller,            SLOT(slotRemoveWatchExpression(int)));  
    
    // framestackWidget -> controller
    connect( framestackWidget,      SIGNAL(selectFrame(int,int,const QString&)),
             controller,            SLOT(slotSelectFrame(int,int,const QString&)));

    // rdbBreakpointWidget -> controller
    connect( rdbBreakpointWidget,   SIGNAL(clearAllBreakpoints()),
             controller,            SLOT(slotClearAllBreakpoints()));
    connect( rdbBreakpointWidget,   SIGNAL(publishBPState(const Breakpoint&)),
             controller,            SLOT(slotBPState(const Breakpoint &)));


    // rdbOutputWidget -> controller
    connect( rdbOutputWidget,       SIGNAL(userRDBCmd(const QString &)),
             controller,            SLOT(slotUserRDBCmd(const QString&)));
    connect( rdbOutputWidget,       SIGNAL(breakInto()),
             controller,            SLOT(slotBreakInto()));

    // controller -> rdbBreakpointWidget
    connect( controller,            SIGNAL(acceptPendingBPs()),
             rdbBreakpointWidget,   SLOT(slotSetPendingBPs()));
    connect( controller,            SIGNAL(unableToSetBPNow(int)),
             rdbBreakpointWidget,   SLOT(slotUnableToSetBPNow(int)));
    connect( controller,            SIGNAL(rawRDBBreakpointList (char*)),
             rdbBreakpointWidget,   SLOT(slotParseRDBBrkptList(char*)));
    connect( controller,            SIGNAL(rawRDBBreakpointSet(char*, int)),
             rdbBreakpointWidget,   SLOT(slotParseRDBBreakpointSet(char*, int)));


    // controller -> this
    connect( controller,            SIGNAL(dbgStatus(const QString&, int)),
             this,                  SLOT(slotStatus(const QString&, int)));
    connect( controller,            SIGNAL(showStepInSource(const QString&, int, const QString&)),
             this,                  SLOT(slotShowStep(const QString&, int)));

    // controller -> procLineMaker
    connect( controller,            SIGNAL(ttyStdout(const char*)),
             procLineMaker,         SLOT(slotReceivedStdout(const char*)));
    connect( controller,            SIGNAL(ttyStderr(const char*)),
             procLineMaker,         SLOT(slotReceivedStderr(const char*)));

    // controller -> rdbOutputWidget
    connect( controller,            SIGNAL(rdbStdout(const char*)),
             rdbOutputWidget,       SLOT(slotReceivedStdout(const char*)) );
    connect( controller,            SIGNAL(rdbStderr(const char*)),
             rdbOutputWidget,       SLOT(slotReceivedStderr(const char*)) );
    connect( controller,            SIGNAL(dbgStatus(const QString&, int)),
             rdbOutputWidget,       SLOT(slotDbgStatus(const QString&, int)));

}


bool RubyDebuggerPart::startDebugger()
{
    QString build_dir;              // Currently selected build directory
    QString run_directory;          // Directory from where the program should be run
    QString program;                // Absolute path to application
    QString run_arguments;          // Command line arguments to be passed to the application
    QString ruby_interpreter;       // Absolute path to the ruby interpreter
    QString debuggee_path;          // Absolute path to debuggee.rb debugger script

    if (project()) {
        build_dir     = project()->buildDirectory();
        run_directory = project()->runDirectory();
    }
	
	int runMainProgram = DomUtil::readIntEntry(*projectDom(), "/kdevrubysupport/run/runmainprogram");
	
	if (runMainProgram == 0) {
    	program = project()->projectDirectory() + "/" + DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/mainprogram");
	} else {
		KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
		if (ro_part != 0) {
			program = ro_part->url().path();
		}
	}
	
    run_arguments = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/programargs");

    QString shell = DomUtil::readEntry(*projectDom(), "/kdevrbdebugger/general/dbgshell");
    if( !shell.isEmpty() )
    {
        QFileInfo info( shell );
        if( info.isRelative() )
        {
            shell = build_dir + "/" + shell;
            info.setFile( shell );
        }
        if( !info.exists() )
        {
            KMessageBox::error(
                mainWindow()->main(),
                i18n("Could not locate the debugging shell '%1'.").arg( shell ),
                i18n("Debugging Shell Not Found") );
            return false;
        }
    }

    core()->running(this, true);

    stateChanged( QString("active") );

    KActionCollection *ac = actionCollection();
    ac->action("debug_run")->setText( i18n("&Continue") );
//    ac->action("debug_run")->setIcon( "dbgrun" );
    ac->action("debug_run")->setToolTip( i18n("Continues the application execution") );
    ac->action("debug_run")->setWhatsThis( i18n("Continue application execution\n\n"
                                           "Continues the execution of your application in the "
                                           "debugger. This only takes effect when the application "
                                           "has been halted by the debugger (i.e. a breakpoint has "
                                           "been activated or the interrupt was pressed).") );


//    mainWindow()->setViewAvailable(variableWidget, true);
    mainWindow()->setViewAvailable(framestackWidget, true);
    mainWindow()->setViewAvailable(rdbOutputWidget, true);

//     variableWidget->setEnabled(true);
    framestackWidget->setEnabled(true);

    rdbOutputWidget->clear();
    rdbOutputWidget->setEnabled(true);

    if (DomUtil::readBoolEntry(*projectDom(), "/kdevrbdebugger/general/floatingtoolbar", true))
    {
        floatingToolBar = new DbgToolBar(this, mainWindow()->main());
        floatingToolBar->show();
    }
    
	ruby_interpreter = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/interpreter");
    
	int coding = DomUtil::readIntEntry(*projectDom(), "/kdevrubysupport/run/charactercoding");
	QString character_coding("-K");
	
	switch (coding) {
	case 0:
		character_coding.append("A");
		break;
	case 1:
		character_coding.append("E");
		break;
	case 2:
		character_coding.append("S");
		break;
	case 3:
		character_coding.append("U");
		break;
	}
	
//	ruby_interpreter.append(QString(" -K") + code);
	
	debuggee_path = ::locate("data", "kdevrbdebugger/debuggee.rb", instance());
	
    controller->slotStart(ruby_interpreter, character_coding, run_directory, debuggee_path, program, run_arguments);
    return true;
}

void RubyDebuggerPart::slotStopDebugger()
{
    controller->slotStopDebugger();
    debugger()->clearExecutionPoint();

    delete floatingToolBar;
    floatingToolBar = 0;

    rdbBreakpointWidget->reset();
    framestackWidget->clear();
    variableWidget->varTree()->clear();

//     variableWidget->setEnabled(false);
    framestackWidget->setEnabled(false);
    rdbOutputWidget->setEnabled(false);

//    mainWindow()->setViewAvailable(variableWidget, false);
    mainWindow()->setViewAvailable(framestackWidget, false);
    mainWindow()->setViewAvailable(rdbOutputWidget, false);

    KActionCollection *ac = actionCollection();
    ac->action("debug_run")->setText( i18n("&Start") );
//    ac->action("debug_run")->setIcon( "1rightarrow" );
    ac->action("debug_run")->setToolTip( i18n("Runs the program in the debugger") );
    ac->action("debug_run")->setWhatsThis( i18n("Start in debugger\n\n"
                                           "Starts the debugger with the project's main "
                                           "executable. You may set some breakpoints "
                                           "before this, or you can interrupt the program "
                                           "while it is running, in order to get information "
                                           "about variables, frame stack, and so on.") );

    stateChanged( QString("stopped") );

    core()->running(this, false);
}

void RubyDebuggerPart::projectClosed()
{
    slotStopDebugger();
}

void RubyDebuggerPart::slotRun()
{
	if (controller->stateIsOn(s_programExited)) {
		rdbBreakpointWidget->reset();
	}
    
	if ( controller->stateIsOn( s_dbgNotStarted ) ) {
        mainWindow()->statusBar()->message(i18n("Debugging program"), 1000);
        mainWindow()->raiseView(rdbOutputWidget);
        appFrontend()->clearView();
        startDebugger();
    } else {
        KActionCollection *ac = actionCollection();
        ac->action("debug_run")->setText( i18n("&Continue") );
        ac->action("debug_run")->setToolTip( i18n("Continues the application execution") );
        ac->action("debug_run")->setWhatsThis( i18n("Continue application execution\n\n"
            "Continues the execution of your application in the "
            "debugger. This only takes effect when the application "
            "has been halted by the debugger (i.e. a breakpoint has "
            "been activated or the interrupt was pressed).") );

        mainWindow()->statusBar()->message(i18n("Continuing program"), 1000);
    }
	
    controller->slotRun();
}



void RubyDebuggerPart::slotStop(KDevPlugin* which)
{
    if( which != 0 && which != this )
        return;

//    if( !controller->stateIsOn( s_dbgNotStarted ) && !controller->stateIsOn( s_shuttingDown ) )
        slotStopDebugger();
}


void RubyDebuggerPart::slotPause()
{
    controller->slotBreakInto();
}


void RubyDebuggerPart::slotRunToCursor()
{
    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(partController()->activeWidget());

    if (!rwpart || !rwpart->url().isLocalFile() || !cursorIface)
        return;

    uint line, col;
    cursorIface->cursorPosition(&line, &col);

    controller->slotRunUntil(rwpart->url().path(), line);
}

void RubyDebuggerPart::slotStepOver()
{
    controller->slotStepOver();
}



void RubyDebuggerPart::slotStepInto()
{
    controller->slotStepInto();
}


void RubyDebuggerPart::slotStepOut()
{
    controller->slotStepOutOff();
}




void RubyDebuggerPart::slotRefreshBPState( const Breakpoint& BP)
{
    if (BP.type() == BP_TYPE_FilePos)
    {
        const FilePosBreakpoint& bp = dynamic_cast<const FilePosBreakpoint&>(BP);
        if (bp.isActionDie())
            debugger()->setBreakpoint(bp.fileName(), bp.lineNum()-1, -1, true, false);
        else
            debugger()->setBreakpoint(bp.fileName(), bp.lineNum()-1,
                                  1/*bp->id()*/, bp.isEnabled(), bp.isPending() );
    }
}


void RubyDebuggerPart::slotStatus(const QString &msg, int state)
{
    QString stateIndicator;

    if (state & s_dbgNotStarted)
    {
        stateIndicator = " ";
    }
    else if (state & s_appBusy)
    {
        stateIndicator = "A";
        debugger()->clearExecutionPoint();
        stateChanged( QString("active") );
    }
    else if (state & s_programExited)
    {
        stateIndicator = "E";
        stateChanged( QString("stopped") );
        KActionCollection *ac = actionCollection();
        ac->action("debug_run")->setText( i18n("Restart") );
//        ac->action("debug_run")->setIcon( "1rightarrow" );
        ac->action("debug_run")->setToolTip( i18n("Restart the program in the debugger") );
        ac->action("debug_run")->setWhatsThis( i18n("Restart in debugger\n\n"
                                           "Restarts the program in the debugger") );
//        slotStop();
    }
    else
    {
        stateIndicator = "P";
        stateChanged( QString("paused") );
    }

    // And now? :-)
    kdDebug(9012) << "Debugger state: " << stateIndicator << ": " << endl;
    kdDebug(9012) << "   " << msg << endl;

    statusBarIndicator->setText(stateIndicator);
    if (!msg.isEmpty())
        mainWindow()->statusBar()->message(msg, 3000);
}


void RubyDebuggerPart::slotShowStep(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
    {
        // Debugger counts lines from 1
        debugger()->gotoExecutionPoint(KURL( fileName ), lineNum-1);
    }
}


void RubyDebuggerPart::slotGotoSource(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
        partController()->editDocument(KURL( fileName ), lineNum);
}


void RubyDebuggerPart::slotActivePartChanged( KParts::Part* part )
{
    KAction* action = actionCollection()->action("debug_toggle_breakpoint");
    if(!action)
        return;

    if(!part)
    {
        action->setEnabled(false);
        return;
    }
    KTextEditor::ViewCursorInterface *iface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
    action->setEnabled( iface != 0 );
}

void RubyDebuggerPart::restorePartialProjectSession(const QDomElement* el)
{
    rdbBreakpointWidget->restorePartialProjectSession(el);
}

void RubyDebuggerPart::savePartialProjectSession(QDomElement* el)
{
    rdbBreakpointWidget->savePartialProjectSession(el);
}

}

KDevAppFrontend * RDBDebugger::RubyDebuggerPart::appFrontend( )
{
   return extension<KDevAppFrontend>("KDevelop/AppFrontend");
}

KDevDebugger * RDBDebugger::RubyDebuggerPart::debugger()
{
    return m_debugger;
}

#include "debuggerpart.moc"
