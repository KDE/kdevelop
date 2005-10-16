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

#include <qdir.h>
#include <q3vbox.h>
#include <q3whatsthis.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <Q3CString>
#include <QLabel>

#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kdevgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kparts/part.h>
#include <ktexteditor/view.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <qtimer.h>
#include <kstringhandler.h>
#include <kxmlguifactory.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevappfrontend.h"
#include "kdevdocumentcontroller.h"
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

DebuggerPart::DebuggerPart( QObject *parent, const char *, const QStringList & ) :
    KDevPlugin( &data, parent ),
    controller(0), previousDebuggerState_(s_dbgNotStarted), 
    justRestarted_(false)
{
    setObjId("DebuggerInterface");
    setInstance(DebuggerFactory::instance());

    setXMLFile("kdevdebugger.rc");

    m_debugger = new Debugger( documentController() );
    
    statusBarIndicator = new QLabel(" ", mainWindow()->statusBar());
    statusBarIndicator->setFixedWidth(15);
    mainWindow()->statusBar()->addWidget(statusBarIndicator, 0, true);
    statusBarIndicator->show();

    // Setup widgets and dbgcontroller
    variableWidget = new VariableWidget( 0, "variablewidget");
    mainWindow()->embedSelectView(variableWidget, i18n("Variables"), i18n("Debugger variable-view"));


    gdbBreakpointWidget = new GDBBreakpointWidget( 0, "gdbBreakpointWidget" );
    gdbBreakpointWidget->setCaption(i18n("Breakpoint List"));
    Q3WhatsThis::add
        (gdbBreakpointWidget, i18n("<b>Breakpoint list</b><p>"
                                "Displays a list of breakpoints with "
                                "their current status. Clicking on a "
                                "breakpoint item allows you to change "
                                "the breakpoint and will take you "
                                "to the source in the editor window."));
    gdbBreakpointWidget->setIcon( SmallIcon("stop") );
    mainWindow()->embedOutputView(gdbBreakpointWidget, i18n("Breakpoints"), i18n("Debugger breakpoints"));

    framestackWidget = new FramestackWidget( 0, "framestackWidget" );
    framestackWidget->setEnabled(false);
    framestackWidget->setCaption(i18n("Frame Stack"));
    Q3WhatsThis::add
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
    Q3WhatsThis::add
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
    Q3WhatsThis::add
        (gdbOutputWidget, i18n("<b>GDB output</b><p>"
                                 "Shows all gdb commands being executed. "
                                 "You can also issue any other gdb command while debugging."));
    mainWindow()->embedOutputView(gdbOutputWidget, i18n("GDB"),
                                  i18n("GDB output"));
    mainWindow()->setViewAvailable(gdbOutputWidget, false);

    VariableTree *variableTree = variableWidget->varTree();

    // variableTree -> framestackWidget
    connect( variableTree,     SIGNAL(selectFrame(int, int)),
             framestackWidget, SLOT(slotSelectFrame(int, int)));

    // gdbBreakpointWidget -> this
    connect( gdbBreakpointWidget, SIGNAL(refreshBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( gdbBreakpointWidget, SIGNAL(publishBPState(const Breakpoint&)),
             this,             SLOT(slotRefreshBPState(const Breakpoint&)));
    connect( gdbBreakpointWidget, SIGNAL(gotoSourcePosition(const QString&, int)),
             this,             SLOT(slotGotoSource(const QString&, int)) );

    // Now setup the actions
    KAction *action;

//    action = new KAction(i18n("&Start"), "1rightarrow", CTRL+SHIFT+Key_F9,
    action = new KAction(i18n("&Start"), "dbgrun", Qt::Key_F9,
                         this, SLOT(slotRun()),
                         actionCollection(), "debug_run");
    action->setToolTip( i18n("Start in debugger") );
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


    action = new KAction(i18n("Step &Over"), "dbgnext", Qt::Key_F10,
                         this, SLOT(slotStepOver()),
                         actionCollection(), "debug_stepover");
    action->setToolTip( i18n("Step over the next line") );
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


    action = new KAction(i18n("Step &Into"), "dbgstep", Qt::Key_F11,
                         this, SLOT(slotStepInto()),
                         actionCollection(), "debug_stepinto");
    action->setToolTip( i18n("Step into the next statement") );
    action->setWhatsThis( i18n("<b>Step into</b><p>"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );


    action = new KAction(i18n("Step into I&nstruction"), "dbgstepinst", 0,
                         this, SLOT(slotStepIntoInstruction()),
                         actionCollection(), "debug_stepintoinst");
    action->setToolTip( i18n("Step into instruction") );
    action->setWhatsThis(i18n("<b>Step into instruction</b><p>Steps into the next assembly instruction."));


    action = new KAction(i18n("Step O&ut"), "dbgstepout", Qt::Key_F12,
                         this, SLOT(slotStepOut()),
                         actionCollection(), "debug_stepout");
    action->setToolTip( i18n("Steps out of the current function") );
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

    connect( documentController(), SIGNAL(loadedFile(const KURL &)),
             gdbBreakpointWidget, SLOT(slotRefreshBP(const KURL &)) );
    connect( debugger(), SIGNAL(toggledBreakpoint(const QString &, int)),
             gdbBreakpointWidget, SLOT(slotToggleBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(editedBreakpoint(const QString &, int)),
             gdbBreakpointWidget, SLOT(slotEditBreakpoint(const QString &, int)) );
    connect( debugger(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
             gdbBreakpointWidget, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );

    connect( core(), SIGNAL(contextMenu(Q3PopupMenu *, const Context *)),
             this, SLOT(contextMenu(Q3PopupMenu *, const Context *)) );

    connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
             this, SLOT(slotStop(KDevPlugin*)) );
    connect( core(), SIGNAL(projectClosed()),
             this, SLOT(projectClosed()) );

    connect( documentController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );

    procLineMaker = new ProcessLineMaker();

    connect( procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
             appFrontend(), SLOT(insertStdoutLine(const QString&)) );
    connect( procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
             appFrontend(), SLOT(insertStderrLine(const QString&)) );

    setupController();
    QTimer::singleShot(0, this, SLOT(setupDcop()));
}

void DebuggerPart::setupDcop()
{
    foreach (DCOPCString string, kapp->dcopClient()->registeredApplications())
        if (string.find("drkonqi-") == 0)
            slotDCOPApplicationRegistered(string);

    connect(kapp->dcopClient(), SIGNAL(applicationRegistered(const DCOPCString&)), SLOT(slotDCOPApplicationRegistered(const DCOPCString&)));
    kapp->dcopClient()->setNotifications(true);
}

void DebuggerPart::slotDCOPApplicationRegistered(const DCOPCString& appId)
{
    if (appId.find("drkonqi-") == 0) {
        QByteArray answer;
        DCOPCString replyType;

        kapp->dcopClient()->call(appId, "krashinfo", "appName()", QByteArray(), replyType, answer, true, 5000);

        QDataStream d(&answer, QIODevice::ReadOnly);
        DCOPCString appName;
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
    DCOPCString replyType;

    kapp->dcopClient()->call(kapp->dcopClient()->senderId(), "krashinfo", "pid()", QByteArray(), replyType, answer, true, 5000);

    QDataStream d(&answer, QIODevice::ReadOnly);
    int pid;
    d >> pid;

    if (attachProcess(pid) && m_drkonqi.isEmpty()) {
        m_drkonqi = kapp->dcopClient()->senderId();
        QTimer::singleShot(15000, this, SLOT(slotCloseDrKonqi()));
        mainWindow()->raiseView(framestackWidget);
    }

    mainWindow()->main()->raise();
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

void DebuggerPart::contextMenu(Q3PopupMenu *popup, const Context *context)
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
        = dynamic_cast<KParts::ReadWritePart*>(documentController()->activePart());
    KTextEditor::View *view
        = dynamic_cast<KTextEditor::View*>(documentController()->activeWidget());

    if (!rwpart || !view)
        return;

    gdbBreakpointWidget->slotToggleBreakpoint(rwpart->url().path(), view->cursorPosition().line());
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
    KVBox *vbox = dlg->addVBoxPage(i18n("Debugger"), i18n("Debugger"), BarIcon( info()->icon(), KIcon::SizeMedium) );
    DebuggerConfigWidget *w = new DebuggerConfigWidget(this, vbox, "debugger config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(finished()), controller, SLOT(configure()) );
}


void DebuggerPart::setupController()
{
    VariableTree *variableTree = variableWidget->varTree();

    controller = new GDBController(variableTree, framestackWidget, *projectDom());

    // variableTree -> controller
    connect( variableTree,          SIGNAL(expandItem(TrimmableItem*)),
             controller,            SLOT(slotExpandItem(TrimmableItem*)));
    connect( variableTree,          SIGNAL(expandUserItem(VarItem*, const Q3CString&)),
             controller,            SLOT(slotExpandUserItem(VarItem*, const Q3CString&)));
    connect( variableTree,          SIGNAL(setLocalViewState(bool)),
             controller,            SLOT(slotSetLocalViewState(bool)));
    connect( variableTree,          SIGNAL(varItemConstructed(VarItem*)),
             controller,            SLOT(slotVarItemConstructed(VarItem*)));     // jw
    connect( variableTree,          SIGNAL(produceVariablesInfo()),
             controller,            SLOT(slotProduceVariablesInfo()));
    connect( variableTree,          SIGNAL(setValue(const QString&, 
                                                    const QString&)),
             controller,            SLOT(slotSetValue(const QString&, 
                                                      const QString&)));

    // variableTree -> gdbBreakpointWidget
    connect( variableTree,          SIGNAL(toggleWatchpoint(const QString &)),
             gdbBreakpointWidget,   SLOT(slotToggleWatchpoint(const QString &)));

    // framestackWidget -> controller
    connect( framestackWidget,      SIGNAL(selectFrame(int,int,bool)),
             controller,            SLOT(slotSelectFrame(int,int,bool)));
    connect( framestackWidget,      SIGNAL(produceBacktrace(int)),
             controller,            SLOT(slotProduceBacktrace(int)));


    // gdbBreakpointWidget -> controller
    connect( gdbBreakpointWidget,   SIGNAL(clearAllBreakpoints()),
             controller,            SLOT(slotClearAllBreakpoints()));
    connect( gdbBreakpointWidget,   SIGNAL(publishBPState(const Breakpoint&)),
             controller,            SLOT(slotBPState(const Breakpoint &)));

    // disassembleWidget -> controller
    connect( disassembleWidget,     SIGNAL(disassemble(const QString&, const QString&)),
             controller,            SLOT(slotDisassemble(const QString&, const QString&)));

    // gdbOutputWidget -> controller
    connect( gdbOutputWidget,       SIGNAL(userGDBCmd(const QString &)),
             controller,            SLOT(slotUserGDBCmd(const QString&)));
    connect( gdbOutputWidget,       SIGNAL(breakInto()),
             controller,            SLOT(slotBreakInto()));

    // controller -> gdbBreakpointWidget
    connect( controller,            SIGNAL(acceptPendingBPs()),
             gdbBreakpointWidget,   SLOT(slotSetPendingBPs()));
    connect( controller,            SIGNAL(unableToSetBPNow(int)),
             gdbBreakpointWidget,   SLOT(slotUnableToSetBPNow(int)));
    connect( controller,            SIGNAL(rawGDBBreakpointList (char*)),
             gdbBreakpointWidget,   SLOT(slotParseGDBBrkptList(char*)));
    connect( controller,            SIGNAL(rawGDBBreakpointSet(char*, int)),
             gdbBreakpointWidget,   SLOT(slotParseGDBBreakpointSet(char*, int)));

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
    connect( controller,            SIGNAL(debuggerRunError(int)),
	     this,                  SLOT(errRunningDebugger(int)));

    // controller -> procLineMaker
    connect( controller,            SIGNAL(ttyStdout(const char*)),
             procLineMaker,         SLOT(slotReceivedStdout(const char*)));
    // The output from tracepoints goes to "application" window, because
    // we don't have any better alternative, and using yet another window
    // is undesirable. Besides, this makes tracepoint look even more similar
    // to printf debugging.
    connect( controller,            SIGNAL(tracingOutput(const char*)),
             procLineMaker,         SLOT(slotReceivedStdout(const char*)));
    connect( controller,            SIGNAL(ttyStderr(const char*)),
             procLineMaker,         SLOT(slotReceivedStderr(const char*)));

    // controller -> gdbOutputWidget
    connect( controller,            SIGNAL(gdbStdout(const char*)),
             gdbOutputWidget,       SLOT(slotReceivedStdout(const char*)) );
    connect( controller,            SIGNAL(gdbStderr(const char*)),
             gdbOutputWidget,       SLOT(slotReceivedStderr(const char*)) );
    connect( controller,            SIGNAL(dbgStatus(const QString&, int)),
             gdbOutputWidget,       SLOT(slotDbgStatus(const QString&, int)));

    // controller -> variableTree
    connect( controller, SIGNAL(dbgStatus(const QString&, int)),
             variableTree, SLOT(slotDbgStatus(const QString&, int)));
    connect( controller, SIGNAL(parametersReady(const char*)),
             variableTree, SLOT(slotParametersReady(const char*)));
    connect( controller, SIGNAL(localsReady(const char*)),
             variableTree, SLOT(slotLocalsReady(const char*)));
    connect( controller, SIGNAL(currentFrame(int, int)),
             variableTree, SLOT(slotCurrentFrame(int, int)));

    // gdbBreakpointWidget -> disassembleWidget
    connect( gdbBreakpointWidget,   SIGNAL(publishBPState(const Breakpoint&)),
             disassembleWidget,     SLOT(slotBPState(const Breakpoint &)));
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
    mainWindow()->setViewAvailable(disassembleWidget, true);
    mainWindow()->setViewAvailable(gdbOutputWidget, true);

//     variableWidget->setEnabled(true);
    framestackWidget->setEnabled(true);
    disassembleWidget->setEnabled(true);

    gdbOutputWidget->clear();
    gdbOutputWidget->setEnabled(true);

    if (DomUtil::readBoolEntry(*projectDom(), "/kdevdebugger/general/floatingtoolbar", false))
    {
        floatingToolBar = new DbgToolBar(this, mainWindow()->main());
        floatingToolBar->show();
    }

    controller->slotStart(shell, run_envvars, run_directory, program, run_arguments);
    return true;
}

void DebuggerPart::slotStopDebugger()
{
    controller->slotStopDebugger();
    debugger()->clearExecutionPoint();

    delete floatingToolBar;
    floatingToolBar = 0;

    gdbBreakpointWidget->reset();
    framestackWidget->clear();
    variableWidget->clear();
    disassembleWidget->clear();
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

void DebuggerPart::errRunningDebugger(int errorCode)
{
  if (errorCode == 127)
  {
    KMessageBox::error(mainWindow()->main(), i18n("GDB could not be found. Please make sure it is installed"
                                     " and in the path and try again"), i18n("Debugger Not Found"));
  }
  slotStopDebugger();
}

void DebuggerPart::projectClosed()
{
    slotStopDebugger();
}

void DebuggerPart::slotRun()
{
    if( controller->stateIsOn( s_dbgNotStarted ) )
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
    else
    {
        mainWindow()->statusBar()->message(i18n("Continuing program"), 1000);
    }
    controller->slotRun();
}

void DebuggerPart::slotRestart()
{
    // We could have directly connect KAction to controller->slotRestart()
    // but controller is created after actions, and I did not want to 
    // create unconnected action and connect it later, as it would make
    // it harder to understand the code.
    controller->slotRestart();
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
        = qobject_cast<KParts::ReadWritePart*>(documentController()->activePart());
    KTextEditor::View *view
        = qobject_cast<KTextEditor::View*>(documentController()->activeWidget());

    if (!rwpart || !rwpart->url().isLocalFile() || !view)
        return;

    controller->slotRunUntil(rwpart->url().path(), view->cursorPosition().line() + 1);
}

void DebuggerPart::slotJumpToCursor()
{
    KParts::ReadWritePart *rwpart
            = dynamic_cast<KParts::ReadWritePart*>(documentController()->activePart());
    KTextEditor::View *view
            = dynamic_cast<KTextEditor::View*>(documentController()->activeWidget());

    if (!rwpart || !rwpart->url().isLocalFile() || !view)
        return;

    controller->slotJumpTo(rwpart->url().path(), view->cursorPosition().line() + 1);
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
    // Hmm, couldn't this be made non-modal?

    MemoryViewDialog *dlg = new MemoryViewDialog();
    connect( dlg,        SIGNAL(disassemble(const QString&, const QString&)),
             controller, SLOT(slotDisassemble(const QString&, const QString&)));
    connect( dlg,        SIGNAL(memoryDump(const QString&, const QString&)),
             controller, SLOT(slotMemoryDump(const QString&, const QString&)));
    connect( dlg,        SIGNAL(registers()),
             controller, SLOT(slotRegisters()));
    connect( dlg,        SIGNAL(libraries()),
             controller, SLOT(slotLibraries()));

    connect( controller, SIGNAL(rawGDBMemoryDump(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));
    connect( controller, SIGNAL(rawGDBDisassemble(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));
    connect( controller, SIGNAL(rawGDBRegisters(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));
    connect( controller, SIGNAL(rawGDBLibraries(char*)),
             dlg,        SLOT(slotRawGDBMemoryView(char*)));

    dlg->exec();
    delete dlg;
}


void DebuggerPart::slotRefreshBPState( const Breakpoint& BP)
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


void DebuggerPart::slotStatus(const QString &msg, int state)
{
    QString stateIndicator;

    if (state & s_dbgNotStarted)
    {
        stateIndicator = " ";
        mainWindow()->lowerView(variableWidget);
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
        ac->action("debug_run")->setText( i18n("To start something","Start") );
        ac->action("debug_run")->setToolTip( i18n("Restart the program in the debugger") );
        ac->action("debug_run")->setWhatsThis( i18n("Restart in debugger\n\n"
                                           "Restarts the program in the debugger") );
    }
    else
    {
        stateIndicator = "P";
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

    // If program is started, enable the 'restart' comand.
    actionCollection()->action("debug_restart")->setEnabled(
        !(state & s_programExited));


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
    if (!msg.isEmpty())
        mainWindow()->statusBar()->message(msg, 3000);

    previousDebuggerState_ = state;
}


void DebuggerPart::slotShowStep(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
    {
        // Debugger counts lines from 1
        debugger()->gotoExecutionPoint(KURL( fileName ), lineNum-1);
    }
}


void DebuggerPart::slotGotoSource(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
        documentController()->editDocument(KURL( fileName ), lineNum);
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
    KTextEditor::View *iface
        = qobject_cast<KTextEditor::View*>(part->widget());
    action->setEnabled( iface != 0 );
}

void DebuggerPart::restorePartialProjectSession(const QDomElement* el)
{
    gdbBreakpointWidget->restorePartialProjectSession(el);
}

void DebuggerPart::savePartialProjectSession(QDomElement* el)
{
    gdbBreakpointWidget->savePartialProjectSession(el);
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
