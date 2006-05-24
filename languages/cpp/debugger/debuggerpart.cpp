/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "debuggerpart.h"
#include "label_with_double_click.h"

#include <qdir.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qtooltip.h>

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
#include <kdockwidget.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevdebugger.h"
#include "domutil.h"
#include "variablewidget.h"
#include "gdbbreakpointwidget.h"
#include "framestackwidget.h"
#include "disassemblewidget.h"
#include "processwidget.h"
#include "gdbcontroller.h"
#include "breakpoint.h"
#include "dbgpsdlg.h"
#include "dbgtoolbar.h"
#include "memviewdlg.h"
#include "gdbparser.h"
#include "gdboutputwidget.h"
#include "debuggerconfigwidget.h"
#include "processlinemaker.h"

#include <iostream>

#include <kdevplugininfo.h>
#include <debugger.h>





namespace GDBDebugger
{

static const KDevPluginInfo data("kdevdebugger");

typedef KDevGenericFactory<DebuggerPart> DebuggerFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdebugger, DebuggerFactory( data ) )

DebuggerPart::DebuggerPart( QObject *parent, const char *name, const QStringList & ) :
    KDevPlugin( &data, parent, name ? name : "DebuggerPart" ),
    controller(0), previousDebuggerState_(s_dbgNotStarted), 
    justRestarted_(false), needRebuild_(true), justOpened_(true),
    running_(false)
{
    setObjId("DebuggerInterface");
    setInstance(DebuggerFactory::instance());

    setXMLFile("kdevdebugger.rc");

    m_debugger = new Debugger( partController() );
    
    statusBarIndicator = new LabelWithDoubleClick(
        " ", mainWindow()->statusBar());
    statusBarIndicator->setFixedWidth(15);
    statusBarIndicator->setAlignment(Qt::AlignCenter);
    mainWindow()->statusBar()->addWidget(statusBarIndicator, 0, true);
    statusBarIndicator->show();

    // Setup widgets and dbgcontroller

    controller = new GDBController(*projectDom());


    gdbBreakpointWidget = new GDBBreakpointWidget( controller,
                                                   0, "gdbBreakpointWidget" );
    gdbBreakpointWidget->setCaption(i18n("Breakpoint List"));
    QWhatsThis::add
        (gdbBreakpointWidget, i18n("<b>Breakpoint list</b><p>"
                                "Displays a list of breakpoints with "
                                "their current status. Clicking on a "
                                "breakpoint item allows you to change "
                                "the breakpoint and will take you "
                                "to the source in the editor window."));
    gdbBreakpointWidget->setIcon( SmallIcon("stop") );
    mainWindow()->embedOutputView(gdbBreakpointWidget, i18n("Breakpoints"), i18n("Debugger breakpoints"));

    variableWidget = new VariableWidget( controller, 
                                         gdbBreakpointWidget,
                                         0, "variablewidget");
    mainWindow()->embedSelectView(variableWidget, i18n("Variables"), 
                                  i18n("Debugger variable-view"));


    framestackWidget = new FramestackWidget( controller, 0, "framestackWidget" );
    framestackWidget->setEnabled(false);
    framestackWidget->setCaption(i18n("Frame Stack"));
    QWhatsThis::add
        (framestackWidget, i18n("<b>Frame stack</b><p>"
                                "Often referred to as the \"call stack\", "
                                "this is a list showing what function is "
                                "currently active and who called each "
                                "function to get to this point in your "
                                "program. By clicking on an item you "
                                "can see the values in any of the "
                                "previous calling functions."));
    framestackWidget->setIcon( SmallIcon("table") );
    mainWindow()->embedOutputView(framestackWidget, i18n("Frame Stack"), i18n("Debugger function call stack"));
    mainWindow()->setViewAvailable(framestackWidget, false);

    disassembleWidget = new DisassembleWidget( 0, "disassembleWidget" );
    disassembleWidget->setEnabled(false);
    disassembleWidget->setCaption(i18n("Machine Code Display"));
    QWhatsThis::add
        (disassembleWidget, i18n("<b>Machine code display</b><p>"
                                 "A machine code view into your running "
                                 "executable with the current instruction "
                                 "highlighted. You can step instruction by "
                                 "instruction using the debuggers toolbar "
                                 "buttons of \"step over\" instruction and "
                                 "\"step into\" instruction."));
    disassembleWidget->setIcon( SmallIcon("gear") );
    mainWindow()->embedOutputView(disassembleWidget, i18n("Disassemble"),
                                  i18n("Debugger disassemble view"));
    mainWindow()->setViewAvailable(disassembleWidget, false);

    gdbOutputWidget = new GDBOutputWidget( 0, "gdbOutputWidget" );
    gdbOutputWidget->setEnabled(false);
    gdbOutputWidget->setIcon( SmallIcon("inline_image") );
    gdbOutputWidget->setCaption(i18n("GDB Output"));
    QWhatsThis::add
        (gdbOutputWidget, i18n("<b>GDB output</b><p>"
                                 "Shows all gdb commands being executed. "
                                 "You can also issue any other gdb command while debugging."));
    mainWindow()->embedOutputView(gdbOutputWidget, i18n("GDB"),
                                  i18n("GDB output"));
    mainWindow()->setViewAvailable(gdbOutputWidget, false);

    // gdbBreakpointWidget -> this
    connect( gdbBreakpointWidget, SIGNAL(refreshBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( gdbBreakpointWidget, SIGNAL(publishBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( gdbBreakpointWidget, SIGNAL(gotoSourcePosition(const QString&, int)),
             this,             SLOT(slotGotoSource(const QString&, int)) );


    viewerWidget = new ViewerWidget( controller, 0, "view");
    mainWindow()->embedSelectView(viewerWidget, 
                                  i18n("Debug views"), 
                                  i18n("Special debugger views"));
    mainWindow()->setViewAvailable(viewerWidget, false);
    connect(viewerWidget, SIGNAL(setViewShown(bool)),
            this, SLOT(slotShowView(bool)));

    // Now setup the actions
    KAction *action;

//    action = new KAction(i18n("&Start"), "1rightarrow", CTRL+SHIFT+Key_F9,
    action = new KAction(i18n("&Start"), "dbgrun", Key_F9,
                         this, SLOT(slotRun()),
                         actionCollection(), "debug_run");
    action->setToolTip( i18n("Start in debugger (F9)") );
    action->setWhatsThis( i18n("<b>Start in debugger</b><p>"
                               "Starts the debugger with the project's main "
                               "executable. You may set some breakpoints "
                               "before this, or you can interrupt the program "
                               "while it is running, in order to get information "
                               "about variables, frame stack, and so on.") );

    action = new KAction(i18n("&Restart"), "dbgrestart", 0,
                         this, SLOT(slotRestart()),
                         actionCollection(), "debug_restart");
    action->setToolTip( i18n("Restart program") );
    action->setWhatsThis( i18n("<b>Restarts application</b><p>"
                               "Restarts applications from the beginning."
                              ) );
    action->setEnabled(false);


    action = new KAction(i18n("Sto&p"), "stop", 0,
                         this, SLOT(slotStop()),
                         actionCollection(), "debug_stop");
    action->setToolTip( i18n("Stop debugger") );
    action->setWhatsThis(i18n("<b>Stop debugger</b><p>Kills the executable and exits the debugger."));

    action = new KAction(i18n("Interrupt"), "player_pause", 0,
                         this, SLOT(slotPause()),
                         actionCollection(), "debug_pause");
    action->setToolTip( i18n("Interrupt application") );
    action->setWhatsThis(i18n("<b>Interrupt application</b><p>Interrupts the debugged process or current GDB command."));

    action = new KAction(i18n("Run to &Cursor"), "dbgrunto", 0,
                         this, SLOT(slotRunToCursor()),
                         actionCollection(), "debug_runtocursor");
    action->setToolTip( i18n("Run to cursor") );
    action->setWhatsThis(i18n("<b>Run to cursor</b><p>Continues execution until the cursor position is reached."));


    action = new KAction(i18n("Set E&xecution Position to Cursor"), "dbgjumpto", 0,
                         this, SLOT(slotJumpToCursor()),
                         actionCollection(), "debug_jumptocursor");
    action->setToolTip( i18n("Jump to cursor") );
    action->setWhatsThis(i18n("<b>Set Execution Position </b><p>Set the execution pointer to the current cursor position."));


    action = new KAction(i18n("Step &Over"), "dbgnext", Key_F10,
                         this, SLOT(slotStepOver()),
                         actionCollection(), "debug_stepover");
    action->setToolTip( i18n("Step over the next line (F10)") );
    action->setWhatsThis( i18n("<b>Step over</b><p>"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );


    action = new KAction(i18n("Step over Ins&truction"), "dbgnextinst", 0,
                         this, SLOT(slotStepOverInstruction()),
                         actionCollection(), "debug_stepoverinst");
    action->setToolTip( i18n("Step over instruction") );
    action->setWhatsThis(i18n("<b>Step over instruction</b><p>Steps over the next assembly instruction."));


    action = new KAction(i18n("Step &Into"), "dbgstep", Key_F11,
                         this, SLOT(slotStepInto()),
                         actionCollection(), "debug_stepinto");
    action->setToolTip( i18n("Step into the next statement (F11)") );
    action->setWhatsThis( i18n("<b>Step into</b><p>"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );


    action = new KAction(i18n("Step into I&nstruction"), "dbgstepinst", 0,
                         this, SLOT(slotStepIntoInstruction()),
                         actionCollection(), "debug_stepintoinst");
    action->setToolTip( i18n("Step into instruction") );
    action->setWhatsThis(i18n("<b>Step into instruction</b><p>Steps into the next assembly instruction."));


    action = new KAction(i18n("Step O&ut"), "dbgstepout", Key_F12,
                         this, SLOT(slotStepOut()),
                         actionCollection(), "debug_stepout");
    action->setToolTip( i18n("Steps out of the current function (F12)") );
    action->setWhatsThis( i18n("<b>Step out</b><p>"
                               "Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "program execution is in the outermost frame (i.e. in "
                               "main()) then this operation has no effect.") );


    action = new KAction(i18n("Viewers"), "dbgmemview", 0,
                         this, SLOT(slotMemoryView()),
                         actionCollection(), "debug_memview");
    action->setToolTip( i18n("Debugger viewers") );
    action->setWhatsThis(i18n("<b>Debugger viewers</b><p>Various information about application being executed. There are 4 views available:<br>"
        "<b>Memory</b><br>"
        "<b>Disassemble</b><br>"
        "<b>Registers</b><br>"
        "<b>Libraries</b>"));


    action = new KAction(i18n("Examine Core File..."), "core", 0,
                         this, SLOT(slotExamineCore()),
                         actionCollection(), "debug_core");
    action->setToolTip( i18n("Examine core file") );
    action->setWhatsThis( i18n("<b>Examine core file</b><p>"
                               "This loads a core file, which is typically created "
                               "after the application has crashed, e.g. with a "
                               "segmentation fault. The core file contains an "
                               "image of the program memory at the time it crashed, "
                               "allowing you to do a post-mortem analysis.") );


    action = new KAction(i18n("Attach to Process"), "connect_creating", 0,
                         this, SLOT(slotAttachProcess()),
                         actionCollection(), "debug_attach");
    action->setToolTip( i18n("Attach to process") );
    action->setWhatsThis(i18n("<b>Attach to process</b><p>Attaches the debugger to a running process."));

    action = new KAction(i18n("Toggle Breakpoint"), 0, 0,
                         this, SLOT(toggleBreakpoint()),
                         actionCollection(), "debug_toggle_breakpoint");
    action->setToolTip(i18n("Toggle breakpoint"));
    action->setWhatsThis(i18n("<b>Toggle breakpoint</b><p>Toggles the breakpoint at the current line in editor."));

    connect( mainWindow()->main()->guiFactory(), SIGNAL(clientAdded(KXMLGUIClient*)),
             this, SLOT(guiClientAdded(KXMLGUIClient*)) );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );

    connect( partController(), SIGNAL(loadedFile(const KURL &)),
             gdbBreakpointWidget, SLOT(slotRefreshBP(const KURL &)) );
    connect( debugger(), SIGNAL(toggledBreakpoint(const QString &, int)),
             gdbBreakpointWidget, SLOT(slotToggleBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(editedBreakpoint(const QString &, int)),
             gdbBreakpointWidget, SLOT(slotEditBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             gdbBreakpointWidget, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );

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

    // The output from tracepoints goes to "application" window, because
    // we don't have any better alternative, and using yet another window
    // is undesirable. Besides, this makes tracepoint look even more similar
    // to printf debugging.
    connect( gdbBreakpointWidget,   SIGNAL(tracingOutput(const char*)),
             procLineMaker,         SLOT(slotReceivedStdout(const char*)));


    connect(partController(), SIGNAL(savedFile(const KURL &)),
            this, SLOT(slotFileSaved()));

    if (project())
        connect(project(), SIGNAL(projectCompiled()),
                this, SLOT(slotProjectCompiled()));

    setupController();
    QTimer::singleShot(0, this, SLOT(setupDcop()));
}

void DebuggerPart::setupDcop()
{
    QCStringList objects = kapp->dcopClient()->registeredApplications();
    for (QCStringList::Iterator it = objects.begin(); it != objects.end(); ++it)
        if ((*it).find("drkonqi-") == 0)
            slotDCOPApplicationRegistered(*it);

    connect(kapp->dcopClient(), SIGNAL(applicationRegistered(const QCString&)), SLOT(slotDCOPApplicationRegistered(const QCString&)));
    kapp->dcopClient()->setNotifications(true);
}

void DebuggerPart::slotDCOPApplicationRegistered(const QCString& appId)
{
    if (appId.find("drkonqi-") == 0) {
        QByteArray answer;
        QCString replyType;

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,90)
        kapp->dcopClient()->call(appId, "krashinfo", "appName()", QByteArray(), replyType, answer, true, 5000);
# else
        kapp->dcopClient()->call(appId, "krashinfo", "appName()", QByteArray(), replyType, answer, true);
# endif
#else
        kapp->dcopClient()->call(appId, "krashinfo", "appName()", QByteArray(), replyType, answer, true);
#endif

        QDataStream d(answer, IO_ReadOnly);
        QCString appName;
        d >> appName;

        if (appName.length() && project() && project()->mainProgram().endsWith(appName)) {
            kapp->dcopClient()->send(appId, "krashinfo", "registerDebuggingApplication(QString)", i18n("Debug in &KDevelop"));
            connectDCOPSignal(appId, "krashinfo", "acceptDebuggingApplication()", "slotDebugExternalProcess()", true);
        }
    }
}

ASYNC DebuggerPart::slotDebugExternalProcess()
{
    QByteArray answer;
    QCString replyType;

#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,90)
    kapp->dcopClient()->call(kapp->dcopClient()->senderId(), "krashinfo", "pid()", QByteArray(), replyType, answer, true, 5000);
# else
    kapp->dcopClient()->call(kapp->dcopClient()->senderId(), "krashinfo", "pid()", QByteArray(), replyType, answer, true);
# endif
#else
    kapp->dcopClient()->call(kapp->dcopClient()->senderId(), "krashinfo", "pid()", QByteArray(), replyType, answer, true);
#endif

    QDataStream d(answer, IO_ReadOnly);
    int pid;
    d >> pid;

    if (attachProcess(pid) && m_drkonqi.isEmpty()) {
        m_drkonqi = kapp->dcopClient()->senderId();
        QTimer::singleShot(15000, this, SLOT(slotCloseDrKonqi()));
        mainWindow()->raiseView(framestackWidget);
    }

    mainWindow()->main()->raise();
}

ASYNC DebuggerPart::slotDebugCommandLine(const QString& /*command*/)
{
    KMessageBox::information(0, "Asked to debug command line");
}

void DebuggerPart::slotCloseDrKonqi()
{
    kapp->dcopClient()->send(m_drkonqi, "MainApplication-Interface", "quit()", QByteArray());
    m_drkonqi = "";
}

DebuggerPart::~DebuggerPart()
{
    kapp->dcopClient()->setNotifications(false);

    if (variableWidget)
        mainWindow()->removeView(variableWidget);
    if (gdbBreakpointWidget)
        mainWindow()->removeView(gdbBreakpointWidget);
    if (framestackWidget)
        mainWindow()->removeView(framestackWidget);
    if (disassembleWidget)
        mainWindow()->removeView(disassembleWidget);
    if(gdbOutputWidget)
        mainWindow()->removeView(gdbOutputWidget);

    delete variableWidget;
    delete gdbBreakpointWidget;
    delete framestackWidget;
    delete disassembleWidget;
    delete gdbOutputWidget;
    delete controller;
    delete floatingToolBar;
    delete statusBarIndicator;
    delete procLineMaker;

    GDBParser::destroy();
}


void DebuggerPart::guiClientAdded( KXMLGUIClient* client )
{
    // Can't change state until after XMLGUI has been loaded...
    // Anyone know of a better way of doing this?
    if( client == this )
        stateChanged( QString("stopped") );
}

void DebuggerPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    if (!context->hasType( Context::EditorContext ))
        return;

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    m_contextIdent = econtext->currentWord();

    bool running = !(previousDebuggerState_ & s_dbgNotStarted);

    // If debugger is running, we insert items at the top. 
    // The reason is user has explicitly run the debugger, so he's
    // surely debugging, not editing code or something. So, first
    // menu items should be about debugging, not some copy/paste/cut
    // things.
    if (!running)
        popup->insertSeparator();

    int index = running ? 0 : -1;        
    if (running)
    {
        // Too bad we can't add QAction to popup menu in Qt3.
        KAction* act = actionCollection()->action("debug_runtocursor");
        Q_ASSERT(act);
        if (act)
        {
            int id = popup->insertItem( act->iconSet(), i18n("Run to &Cursor"),
                                        this, SLOT(slotRunToCursor()),
                                        0, -1, index);

            popup->setWhatsThis(id, act->whatsThis());
            index += running;
        }
    }
    if (econtext->url().isLocalFile())
    {
        int id = popup->insertItem( i18n("Toggle Breakpoint"), 
                                    this, SLOT(toggleBreakpoint()),
                                    0, -1, index);
        index += running;
        popup->setWhatsThis(id, i18n("<b>Toggle breakpoint</b><p>Toggles breakpoint at the current line."));
    }
    if (!m_contextIdent.isEmpty())
    {
        QString squeezed = KStringHandler::csqueeze(m_contextIdent, 30);
        int id = popup->insertItem( i18n("Evaluate: %1").arg(squeezed), 
                                    this, SLOT(contextEvaluate()),
                                    0, -1, index);
        index += running;
        popup->setWhatsThis(id, i18n("<b>Evaluate expression</b><p>Shows the value of the expression under the cursor."));
        int id2 = popup->insertItem( i18n("Watch: %1").arg(squeezed), 
                                     this, SLOT(contextWatch()),
                                    0, -1, index);
        index += running;
        popup->setWhatsThis(id2, i18n("<b>Watch expression</b><p>Adds an expression under the cursor to the Variables/Watch list."));
    }
    if (running)
        popup->insertSeparator(index);
}


void DebuggerPart::toggleBreakpoint()
{
    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(partController()->activeWidget());

    if (!rwpart || !cursorIface)
        return;

    uint line, col;
    cursorIface->cursorPositionReal(&line, &col);

    gdbBreakpointWidget->slotToggleBreakpoint(rwpart->url().path(), line);
}


void DebuggerPart::contextWatch()
{
    variableWidget->slotAddWatchVariable(m_contextIdent);
}

void DebuggerPart::contextEvaluate()
{
    variableWidget->slotEvaluateExpression(m_contextIdent);
}

void DebuggerPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("Debugger"), i18n("Debugger"), BarIcon( info()->icon(), KIcon::SizeMedium) );
    DebuggerConfigWidget *w = new DebuggerConfigWidget(this, vbox, "debugger config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(finished()), controller, SLOT(configure()) );
}


void DebuggerPart::setupController()
{
    VariableTree *variableTree = variableWidget->varTree();

    // variableTree -> gdbBreakpointWidget
    connect( variableTree,          SIGNAL(toggleWatchpoint(const QString &)),
             gdbBreakpointWidget,   SLOT(slotToggleWatchpoint(const QString &)));

    // disassembleWidget -> controller
    connect( disassembleWidget,     SIGNAL(disassemble(const QString&, const QString&)),
             controller,            SLOT(slotDisassemble(const QString&, const QString&)));

    // gdbOutputWidget -> controller
    connect( gdbOutputWidget,       SIGNAL(userGDBCmd(const QString &)),
             controller,            SLOT(slotUserGDBCmd(const QString&)));
    connect( gdbOutputWidget,       SIGNAL(breakInto()),
             controller,            SLOT(slotBreakInto()));
  
    connect( controller,            SIGNAL(breakpointHit(int)),
             gdbBreakpointWidget,   SLOT(slotBreakpointHit(int)));

    // controller -> disassembleWidget
    connect( controller,            SIGNAL(showStepInSource(const QString&, int, const QString&)),
             disassembleWidget,     SLOT(slotShowStepInSource(const QString&, int, const QString&)));
    connect( controller,            SIGNAL(rawGDBDisassemble(char*)),
             disassembleWidget,     SLOT(slotDisassemble(char*)));

    // controller -> this
    connect( controller,            SIGNAL(dbgStatus(const QString&, int)),
             this,                  SLOT(slotStatus(const QString&, int)));
    connect( controller,            SIGNAL(showStepInSource(const QString&, int, const QString&)),
             this,                  SLOT(slotShowStep(const QString&, int)));
    connect( controller,            SIGNAL(debuggerAbnormalExit()),
         this,                  SLOT(slotDebuggerAbnormalExit()));

    connect(controller, SIGNAL(event(GDBController::event_t)),
            this,       SLOT(slotEvent(GDBController::event_t)));

    // controller -> procLineMaker
    connect( controller,            SIGNAL(ttyStdout(const char*)),
             procLineMaker,         SLOT(slotReceivedStdout(const char*)));
    connect( controller,            SIGNAL(ttyStderr(const char*)),
             procLineMaker,         SLOT(slotReceivedStderr(const char*)));

    // controller -> gdbOutputWidget
    connect( controller,            SIGNAL(gdbInternalCommandStdout(const char*)),
             gdbOutputWidget,       SLOT(slotInternalCommandStdout(const char*)) );
    connect( controller,            SIGNAL(gdbUserCommandStdout(const char*)),
             gdbOutputWidget,       SLOT(slotUserCommandStdout(const char*)) );

    connect( controller,            SIGNAL(gdbStderr(const char*)),
             gdbOutputWidget,       SLOT(slotReceivedStderr(const char*)) );
    connect( controller,            SIGNAL(dbgStatus(const QString&, int)),
             gdbOutputWidget,       SLOT(slotDbgStatus(const QString&, int)));

    // controller -> viewerWidget
    connect( controller, SIGNAL(dbgStatus(const QString&, int)),
             viewerWidget, SLOT(slotDebuggerState(const QString&, int)));


    connect(statusBarIndicator, SIGNAL(doubleClicked()),
            controller, SLOT(explainDebuggerStatus()));

}


bool DebuggerPart::startDebugger()
{
    QString build_dir;              // Currently selected build directory
    DomUtil::PairList run_envvars;  // List with the environment variables
    QString run_directory;          // Directory from where the program should be run
    QString program;                // Absolute path to application
    QString run_arguments;          // Command line arguments to be passed to the application

    if (project()) {
        build_dir     = project()->buildDirectory();
        run_envvars   = project()->runEnvironmentVars();
        run_directory = project()->runDirectory();
        program       = project()->mainProgram();
        run_arguments = DomUtil::readEntry(*projectDom(), "/kdevdebugger/general/programargs");
    }

    QString shell = DomUtil::readEntry(*projectDom(), "/kdevdebugger/general/dbgshell");
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

    if (controller->start(shell, run_envvars, run_directory, 
                          program, run_arguments))
    {
        core()->running(this, true);

        stateChanged( QString("active") );
        
        KActionCollection *ac = actionCollection();
        ac->action("debug_run")->setText( i18n("&Continue") );
        
        ac->action("debug_run")->setToolTip( 
            i18n("Continues the application execution") );
        ac->action("debug_run")->setWhatsThis( 
            i18n("Continue application execution\n\n"
                 "Continues the execution of your application in the "
                 "debugger. This only takes effect when the application "
                 "has been halted by the debugger (i.e. a breakpoint has "
                 "been activated or the interrupt was pressed).") );
        
        mainWindow()->setViewAvailable(framestackWidget, true);
        mainWindow()->setViewAvailable(disassembleWidget, true);
        mainWindow()->setViewAvailable(gdbOutputWidget, true);
        
        framestackWidget->setEnabled(true);
        disassembleWidget->setEnabled(true);

        gdbOutputWidget->setEnabled(true);
     

        if (DomUtil::readBoolEntry(*projectDom(), "/kdevdebugger/general/floatingtoolbar", false))
        {
            floatingToolBar = new DbgToolBar(this, mainWindow()->main());
            floatingToolBar->show();
        }

        running_ = true;
        return true;
    }
    else
    {
        return false;
    }
}

void DebuggerPart::slotStopDebugger()
{
    running_ = false;
    controller->slotStopDebugger();
    debugger()->clearExecutionPoint();

    delete floatingToolBar;
    floatingToolBar = 0;

    gdbBreakpointWidget->reset();
    disassembleWidget->clear();
    gdbOutputWidget->clear();
    disassembleWidget->slotActivate(false);

//     variableWidget->setEnabled(false);
    framestackWidget->setEnabled(false);
    disassembleWidget->setEnabled(false);
    gdbOutputWidget->setEnabled(false);
    

//    mainWindow()->setViewAvailable(variableWidget, false);
    mainWindow()->setViewAvailable(framestackWidget, false);
    mainWindow()->setViewAvailable(disassembleWidget, false);
    mainWindow()->setViewAvailable(gdbOutputWidget, false);

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

void DebuggerPart::slotShowView(bool show)
{
    const QWidget* s = static_cast<const QWidget*>(sender());
    QWidget* ncs = const_cast<QWidget*>(s);    
    mainWindow()->setViewAvailable(ncs, show);
    if (show)
        mainWindow()->raiseView(ncs);
}

void DebuggerPart::slotDebuggerAbnormalExit()
{
    mainWindow()->raiseView(gdbOutputWidget);

    KMessageBox::error(
        mainWindow()->main(), 
        i18n("<b>GDB exited abnormally</b>"
             "<p>This is likely a bug in GDB. "
             "Examine the gdb output window and then stop the debugger"),
        i18n("GDB exited abnormally"));

    // Note: we don't stop the debugger here, becuse that will hide gdb
    // window and prevent the user from finding the exact reason of the
    // problem.
}

void DebuggerPart::slotFileSaved()
{
    needRebuild_ = true;
}

void DebuggerPart::slotProjectCompiled()
{
    needRebuild_ = false;
}

void DebuggerPart::projectClosed()
{
    slotStopDebugger();
}

void DebuggerPart::slotRun()
{
    if( controller->stateIsOn( s_dbgNotStarted ) ||
        controller->stateIsOn( s_appNotStarted ) )
    {
        if (running_ && controller->stateIsOn(s_dbgNotStarted))
        {
            // User has already run the debugger, but it's not running.
            // Most likely, the debugger has crashed, and the debuggerpart
            // was left in 'running' state so that the user can examine
            // gdb output or something. But now, need to fully shut down
            // previous debug session.
            slotStopDebugger();
        }

        // We're either starting gdb for the first time,
        // or starting the application under gdb. In both
        // cases, might need to rebuild the application.

        // Note that this logic somewhat duplicates the
        // isDirty method present in a number of project plugins.
        // But there, it's a private method we can't conveniently
        // access. Besides, the custom makefiles project manager won't 
        // care about a file unless it's explicitly added, so it can
        // miss dependencies.

        needRebuild_ |= haveModifiedFiles();

        bool rebuild = false;
        if (needRebuild_ && project())
        {
            if (justOpened_)
            {
                // Always rebuild the project after opening. User likely
                // don't remember if he modified anything, so can't say
                // anything definitive.
                rebuild = true;
                justOpened_ = false;
            }
            else
            {
                // We don't add "Don't ask again" checkbox to the
                // message because it's not clear if one cooked
                // decision will be right for all cases when we're starting
                // debugging with modified code, and because it's not clear
                // how user can reset this "don't ask again" setting.
                int r = KMessageBox::questionYesNoCancel(
                    0, 
                    "<b>" + i18n("Rebuild the project?") + "</b>" +
                    i18n("<p>The project is out of date. Rebuild it?"),
                    i18n("Rebuild the project?"));
                if (r == KMessageBox::Cancel)
                {
                    return;
                }
                if (r == KMessageBox::Yes)
                {
                    rebuild = true;
                }                
                else
                {
                    // If the user said don't rebuild, try to avoid
                    // asking the same question again. 
                    // Note that this only affects 'were any files changed'
                    // check, if a file is changed but not saved we'll
                    // still ask the user again. That's bad, but I don't know
                    // a better solution -- it's hard to check that 
                    // the file has the same content as it had when the user
                    // last answered 'no, don't rebuild'.
                    needRebuild_ = false;
                }
            }

            if (rebuild)
            {                 
                disconnect(SIGNAL(buildProject()));
                // The KDevProject has no method to build the project,
                // so try connecting to a slot has is present to all
                // existing project managers.
                // Note: this assumes that 'slotBuild' will save
                // modified files.
                
                if (connect(this, SIGNAL(buildProject()),
                            project(), SLOT(slotBuild())))
                {
                    connect(project(), SIGNAL(projectCompiled()), 
                            this, SLOT(slotRun_part2()));
                    
                    emit buildProject();
                    rebuild = true;
                }
            }
        }
        if (!rebuild)
        {
            slotRun_part2();
        }
        return;
    }
    else
    {
        // When continuing the program, don't try to rebuild -- user
        // has explicitly asked to "continue".
        mainWindow()->statusBar()->message(i18n("Continuing program"), 1000);
    }
    controller->slotRun();
}

void DebuggerPart::slotRun_part2()
{        
    needRebuild_ = false;

    disconnect(project(), SIGNAL(projectCompiled()), 
               this, SLOT(slotRun_part2()));

    if (controller->stateIsOn( s_dbgNotStarted ))
    {
        mainWindow()->statusBar()->message(i18n("Debugging program"), 1000);
        mainWindow()->raiseView(gdbOutputWidget);
        appFrontend()->clearView();
        startDebugger();
    }
    else if (controller->stateIsOn( s_appNotStarted ) )
    {
        KActionCollection *ac = actionCollection();
        ac->action("debug_run")->setText( i18n("&Continue") );
        ac->action("debug_run")->setToolTip( i18n("Continues the application execution") );
        ac->action("debug_run")->setWhatsThis( i18n("Continue application execution\n\n"
            "Continues the execution of your application in the "
            "debugger. This only takes effect when the application "
            "has been halted by the debugger (i.e. a breakpoint has "
            "been activated or the interrupt was pressed).") );

        mainWindow()->statusBar()->message(i18n("Running program"), 1000);

        appFrontend()->clearView();
    }

    controller->slotRun();
}


void DebuggerPart::slotRestart()
{
    // We implement restart as kill + slotRun, as opposed as plain "run"
    // command because kill + slotRun allows any special logic in slotRun
    // to apply for restart.
    //
    // That includes:
    // - checking for out-of-date project
    // - special setup for remote debugging.
    //
    // Had we used plain 'run' command, restart for remote debugging simply
    // would not work.
    controller->slotKill();
    slotRun();
}

void DebuggerPart::slotExamineCore()
{
    mainWindow()->statusBar()->message(i18n("Choose a core file to examine..."), 1000);

    QString dirName = project()? project()->projectDirectory() : QDir::homeDirPath();
    QString coreFile = KFileDialog::getOpenFileName(dirName);
    if (coreFile.isNull())
        return;

    mainWindow()->statusBar()->message(i18n("Examining core file %1").arg(coreFile), 1000);

    startDebugger();
    controller->slotCoreFile(coreFile);
}


void DebuggerPart::slotAttachProcess()
{
    mainWindow()->statusBar()->message(i18n("Choose a process to attach to..."), 1000);

    Dbg_PS_Dialog dlg;
    if (!dlg.exec() || !dlg.pidSelected())
        return;

    int pid = dlg.pidSelected();
    attachProcess(pid);
}

bool DebuggerPart::attachProcess(int pid)
{
    mainWindow()->statusBar()->message(i18n("Attaching to process %1").arg(pid), 1000);

    bool ret = startDebugger();
    controller->slotAttachTo(pid);
    return ret;
}


void DebuggerPart::slotStop(KDevPlugin* which)
{
    if( which != 0 && which != this )
        return;

//    if( !controller->stateIsOn( s_dbgNotStarted ) && !controller->stateIsOn( s_shuttingDown ) )
        slotStopDebugger();
}


void DebuggerPart::slotPause()
{
    controller->slotBreakInto();
}


void DebuggerPart::slotRunToCursor()
{
    KParts::ReadWritePart *rwpart
        = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    KTextEditor::ViewCursorInterface *cursorIface
        = dynamic_cast<KTextEditor::ViewCursorInterface*>(partController()->activeWidget());

    if (!rwpart || !rwpart->url().isLocalFile() || !cursorIface)
        return;

    uint line, col;
    cursorIface->cursorPosition(&line, &col);

    controller->slotRunUntil(rwpart->url().path(), ++line);
}

void DebuggerPart::slotJumpToCursor()
{
    KParts::ReadWritePart *rwpart
            = dynamic_cast<KParts::ReadWritePart*>(partController()->activePart());
    KTextEditor::ViewCursorInterface *cursorIface
            = dynamic_cast<KTextEditor::ViewCursorInterface*>(partController()->activeWidget());

    if (!rwpart || !rwpart->url().isLocalFile() || !cursorIface)
        return;

    uint line, col;
    cursorIface->cursorPositionReal(&line, &col);

    controller->slotJumpTo(rwpart->url().path(), ++line);
}

void DebuggerPart::slotStepOver()
{
    controller->slotStepOver();
}


void DebuggerPart::slotStepOverInstruction()
{
    controller->slotStepOver();
}


void DebuggerPart::slotStepIntoInstruction()
{
    controller->slotStepIntoIns();
}


void DebuggerPart::slotStepInto()
{
    controller->slotStepInto();
}


void DebuggerPart::slotStepOut()
{
    controller->slotStepOutOff();
}


void DebuggerPart::slotMemoryView()
{
    viewerWidget->slotAddMemoryView();
}

void DebuggerPart::slotRefreshBPState( const Breakpoint& BP)
{
    if (BP.hasFileAndLine())
    {
        const FilePosBreakpoint& bp = dynamic_cast<const FilePosBreakpoint&>(BP);
        if (bp.isActionDie())
        {
            debugger()->setBreakpoint(bp.fileName(), bp.lineNum()-1, -1, true, false);
        }
        else if (bp.isActionClear())
        {
            // Do nothing. This is always a result of breakpoint deletion,
            // either via click on gutter, or via breakpoints window.
            // We should not add marker for a breakpoint that's being deleted,
            // because if user removes marker, and we re-add it here until
            // we see 'actionDie' this can confuse the code.
            // And no need to clear the marker, since we'll soon see 'actionDie'
            // and clear it for good.
        }
        else
            debugger()->setBreakpoint(bp.fileName(), bp.lineNum()-1,
                                  1/*bp->id()*/, bp.isEnabled(), bp.isPending() );
    }
}

void DebuggerPart::slotStatus(const QString &msg, int state)
{
    QString stateIndicator, stateIndicatorFull;

    if (state & s_dbgNotStarted)
    {
        stateIndicator = " ";
        stateIndicatorFull = "Debugger not started";
        stateChanged( QString("stopped") );

        // If the view is undocked, don't hide it. User has explicitly
        // undocked it and moved into a convenient position. Don't
        // force him to undock the widget and move it again next 
        // time the debugger starts.
        bool undocked = false;

        if (KDockWidget* dockWidget = 
            static_cast<KDockWidget*>(
                variableWidget->parentWidget()->qt_cast("KDockWidget")))
        {
            if (dockWidget->currentDockPosition() == KDockWidget::DockDesktop)
                undocked = true;
        }

        if (!undocked)
        {
            mainWindow()->lowerView(variableWidget);
        }
    }
    else if (state & s_dbgBusy)
    {
        stateIndicator = "R";
        stateIndicatorFull = "Debugger is busy";
        stateChanged( QString("active") );
    }
    else if (state & s_programExited)
    {
        stateIndicator = "E";
        stateIndicatorFull = "Application has exited";
        stateChanged( QString("stopped") );
    }
    else
    {
        stateIndicator = "P";
        stateIndicatorFull = "Application is paused";
        stateChanged( QString("paused") );
        // On the first stop, show the variables view.
        // We do it on first stop, and not at debugger start, because
        // a program might just successfully run till completion. If we show
        // the var views on start and hide on stop, this will look like flicker.
        // On the other hand, if application is paused, it's very
        // likely that the user wants to see variables.
        if (justRestarted_)
        {
            justRestarted_ = false;
            mainWindow()->raiseView(variableWidget);
        }
    }

    if (state & s_appNotStarted)
    {
        KActionCollection *ac = actionCollection();
        ac->action("debug_run")->setText( i18n("To start something","Start") );
        ac->action("debug_run")->setToolTip( i18n("Restart the program in the debugger") );
        ac->action("debug_run")->setWhatsThis( i18n("Restart in debugger\n\n"
                                           "Restarts the program in the debugger") );
    }


    bool program_running = !(state & s_appNotStarted);
    bool attached_or_core = (state & s_attached) || (state & s_core);

    // If program is started, enable the 'restart' comand.
    actionCollection()->action("debug_restart")->setEnabled(
        program_running && !attached_or_core);


    // As soon as debugger clears 's_appNotStarted' flag, we
    // set 'justRestarted' variable. 
    // The other approach would be to set justRestarted in slotRun, slotCore
    // and slotAttach.
    // Note that setting this var in startDebugger is not OK, because the 
    // initial state of debugger is exactly the same as state after pause,
    // so we'll always show varaibles view.
    if ((previousDebuggerState_ & s_appNotStarted) &&
        !(state & s_appNotStarted))
    {
        justRestarted_ = true;
    }
    if (state & s_appNotStarted)
    {
        justRestarted_ = false;
    }

    // And now? :-)
    kdDebug(9012) << "Debugger state: " << stateIndicator << ": " << endl;
    kdDebug(9012) << "   " << msg << endl;

    statusBarIndicator->setText(stateIndicator);
    QToolTip::add(statusBarIndicator, stateIndicatorFull);
    if (!msg.isEmpty())
        mainWindow()->statusBar()->message(msg, 3000);
    

    previousDebuggerState_ = state;
}

void DebuggerPart::slotEvent(GDBController::event_t e)
{
    if (e == GDBController::program_running ||
        e == GDBController::program_exited ||
        e == GDBController::debugger_exited)
    {
        debugger()->clearExecutionPoint();        
    }
}


void DebuggerPart::slotShowStep(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
    {
        // Debugger counts lines from 1
        debugger()->gotoExecutionPoint(KURL( fileName ), lineNum-1);
    }
    else
    {
        debugger()->clearExecutionPoint();
    }
}


void DebuggerPart::slotGotoSource(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
        partController()->editDocument(KURL( fileName ), lineNum);
}


void DebuggerPart::slotActivePartChanged( KParts::Part* part )
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

void DebuggerPart::restorePartialProjectSession(const QDomElement* el)
{
    gdbBreakpointWidget->restorePartialProjectSession(el);
    gdbOutputWidget->restorePartialProjectSession(el);
}

void DebuggerPart::savePartialProjectSession(QDomElement* el)
{
    gdbBreakpointWidget->savePartialProjectSession(el);
    gdbOutputWidget->savePartialProjectSession(el);
}

bool DebuggerPart::haveModifiedFiles()
{
    bool have_modified = false;
    KURL::List const& filelist = partController()->openURLs();
    KURL::List::ConstIterator it = filelist.begin();
    while ( it != filelist.end() )
    {
        if (partController()->documentState(*it) != Clean)
            have_modified = true;

        ++it;
    }

    return have_modified;
}

}

KDevAppFrontend * GDBDebugger::DebuggerPart::appFrontend( )
{
    return extension<KDevAppFrontend>("KDevelop/AppFrontend");
}

KDevDebugger * GDBDebugger::DebuggerPart::debugger()
{
    return m_debugger;
}

#include "debuggerpart.moc"
