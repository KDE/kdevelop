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

#include <QDir>


#include <q3popupmenu.h>
#include <QToolTip>
#include <QByteArray>
#include <QTimer>

#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kstatusbar.h>
#include <kparts/part.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kpluginfactory.h>

#include <icore.h>
#include <iuicontroller.h>
//#include <idocumentcontroller.h>

#include "variablewidget.h"
#include "gdbbreakpointwidget.h"
#include "framestackwidget.h"
#include "disassemblewidget.h"
#include "gdbcontroller.h"
#include "breakpoint.h"
#include "dbgpsdlg.h"
#include "dbgtoolbar.h"
#include "memviewdlg.h"
#include "gdbparser.h"
#include "gdboutputwidget.h"
#include "debuggerconfigwidget.h"

#include <iostream>

#include <kvbox.h>

namespace GDBDebugger
{

K_PLUGIN_FACTORY(CppDebuggerFactory, registerPlugin<CppDebuggerPlugin>(); )
K_EXPORT_PLUGIN(CppDebuggerFactory("kdevcppdebbugger"))

class BreakpointListFactory : public KDevelop::IToolViewFactory
{
public:
  BreakpointListFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    GDBBreakpointWidget* breakpoint = new GDBBreakpointWidget(m_controller, parent);

    // TODO move to the breakpoint widget
    QObject::connect( breakpoint, SIGNAL(refreshBPState(const Breakpoint&)),
             m_plugin,   SLOT(slotRefreshBPState(const Breakpoint&)));
    QObject::connect( breakpoint, SIGNAL(publishBPState(const Breakpoint&)),
             m_plugin,   SLOT(slotRefreshBPState(const Breakpoint&)));
    QObject::connect( breakpoint, SIGNAL(gotoSourcePosition(const QString&, int)),
             m_plugin,   SLOT(slotGotoSource(const QString&, int)) );

    return breakpoint;
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::BottomDockWidgetArea;
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

class VariableBreakpointListFactory : public KDevelop::IToolViewFactory
{
public:
  VariableBreakpointListFactory(GDBController* controller): m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new VariableWidget(m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::LeftDockWidgetArea;
  }

private:
  GDBController* m_controller;
};

class FramestackViewFactory : public KDevelop::IToolViewFactory
{
public:
  FramestackViewFactory(GDBController* controller): m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new FramestackWidget(m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::LeftDockWidgetArea;
  }

private:
  GDBController* m_controller;
};

class DisassembleViewFactory : public KDevelop::IToolViewFactory
{
public:
  DisassembleViewFactory(GDBController* controller): m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new DisassembleWidget(m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::LeftDockWidgetArea;
  }

private:
  GDBController* m_controller;
};

class GDBOutputViewFactory : public KDevelop::IToolViewFactory
{
public:
  GDBOutputViewFactory(GDBController* controller): m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new GDBOutputWidget(m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::LeftDockWidgetArea;
  }

private:
  GDBController* m_controller;
};

class SpecialViewFactory : public KDevelop::IToolViewFactory
{
public:
  SpecialViewFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    ViewerWidget* vw = new ViewerWidget(m_controller, parent);
    QObject::connect(vw, SIGNAL(setViewShown(bool)),
            m_plugin, SLOT(slotShowView(bool)));
    return vw;
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::LeftDockWidgetArea;
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

CppDebuggerPlugin::CppDebuggerPlugin( QObject *parent, const QVariantList & ) :
    KDevelop::IPlugin( CppDebuggerFactory::componentData(), parent ),
    controller(0), previousDebuggerState_(s_dbgNotStarted),
    justRestarted_(false), needRebuild_(true),
    running_(false)
{
    setXMLFile("kdevcppdebugger.rc");

    //m_debugger = new Debugger( partController() );

    /*statusBarIndicator = new LabelWithDoubleClick(
        " ", mainWindow()->statusBar());
    statusBarIndicator->setFixedWidth(15);
    statusBarIndicator->setAlignment(Qt::AlignCenter);
    mainWindow()->statusBar()->addWidget(statusBarIndicator, 0, true);
    statusBarIndicator->show();*/

    // Setup widgets and dbgcontroller

    controller = new GDBController(this);

    m_breakpointFactory = new BreakpointListFactory(this, controller);
    core()->uiController()->addToolView(i18n("Breakpoints"), m_breakpointFactory);

    m_variableFactory = new VariableBreakpointListFactory(controller);
    core()->uiController()->addToolView(i18n("Variables"), m_variableFactory);

    m_framestackFactory = new FramestackViewFactory(controller);
    core()->uiController()->addToolView(i18n("Frame Stack"), m_framestackFactory);

    m_disassembleFactory = new DisassembleViewFactory(controller);
    core()->uiController()->addToolView(i18n("Disassemble"), m_disassembleFactory);

    m_outputFactory = new GDBOutputViewFactory(controller);
    core()->uiController()->addToolView(i18n("GDB"), m_outputFactory);

    m_specialFactory = new SpecialViewFactory(this, controller);
    core()->uiController()->addToolView(i18n("Debug views"), m_outputFactory);

    // Now setup the actions
    KAction *action;
    KActionCollection* ac = actionCollection();

    action = new KAction(KIcon("dbgrun"), i18n("&Start"), this);
    action->setShortcut(Qt::Key_F9);
    action->setToolTip( i18n("Start in debugger") );
    action->setWhatsThis( i18n("<b>Start in debugger</b><p>"
                               "Starts the debugger with the project's main "
                               "executable. You may set some breakpoints "
                               "before this, or you can interrupt the program "
                               "while it is running, in order to get information "
                               "about variables, frame stack, and so on.") );
    ac->addAction("debug_run", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRun()));

    action = new KAction(KIcon("dbgrestart"), i18n("&Restart"), this);
    action->setToolTip( i18n("Restart program") );
    action->setWhatsThis( i18n("<b>Restarts application</b><p>"
                               "Restarts applications from the beginning."
                              ) );
    action->setEnabled(false);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRestart()));
    ac->addAction("debug_restart", action);


    action = new KAction(KIcon("process-stop"), i18n("Sto&p"), this);
    action->setToolTip( i18n("Stop debugger") );
    action->setWhatsThis(i18n("<b>Stop debugger</b><p>Kills the executable and exits the debugger."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStop()));
    ac->addAction("debug_stop", action);

    action = new KAction(KIcon("media-playback-pause"), i18n("Interrupt"), this);
    action->setToolTip( i18n("Interrupt application") );
    action->setWhatsThis(i18n("<b>Interrupt application</b><p>Interrupts the debugged process or current GDB command."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotPause()));
    ac->addAction("debug_pause", action);

    action = new KAction(KIcon("dbgrunto"), i18n("Run to &Cursor"), this);
    action->setToolTip( i18n("Run to cursor") );
    action->setWhatsThis(i18n("<b>Run to cursor</b><p>Continues execution until the cursor position is reached."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRunToCursor()));
    ac->addAction("debug_runtocursor", action);


    action = new KAction(KIcon("dbgjumpto"), i18n("Set E&xecution Position to Cursor"), this);
    action->setToolTip( i18n("Jump to cursor") );
    action->setWhatsThis(i18n("<b>Set Execution Position </b><p>Set the execution pointer to the current cursor position."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotJumpToCursor()));
    ac->addAction("debug_jumptocursor", action);


    action = new KAction(KIcon("dbgnext"), i18n("Step &Over"), this);
    action->setShortcut(Qt::Key_F10);
    action->setToolTip( i18n("Step over the next line") );
    action->setWhatsThis( i18n("<b>Step over</b><p>"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepOver()));
    ac->addAction("debug_stepover", action);


    action = new KAction(KIcon("dbgnextinst"), i18n("Step over Ins&truction"), this);
    action->setToolTip( i18n("Step over instruction") );
    action->setWhatsThis(i18n("<b>Step over instruction</b><p>Steps over the next assembly instruction."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepOverInstruction()));
    ac->addAction("debug_stepoverinst", action);


    action = new KAction(KIcon("dbgstep"), i18n("Step &Into"), this);
    action->setShortcut(Qt::Key_F11);
    action->setToolTip( i18n("Step into the next statement") );
    action->setWhatsThis( i18n("<b>Step into</b><p>"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepInto()));
    ac->addAction("debug_stepinto", action);


    action = new KAction(KIcon("dbgstepinst"), i18n("Step into I&nstruction"), this);
    action->setToolTip( i18n("Step into instruction") );
    action->setWhatsThis(i18n("<b>Step into instruction</b><p>Steps into the next assembly instruction."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepIntoInstruction()));
    ac->addAction("debug_stepintoinst", action);


    action = new KAction(KIcon("dbgstepout"), i18n("Step O&ut"), this);
    action->setShortcut(Qt::Key_F12);
    action->setToolTip( i18n("Steps out of the current function") );
    action->setWhatsThis( i18n("<b>Step out</b><p>"
                               "Executes the application until the currently executing "
                               "function is completed. The debugger will then display "
                               "the line after the original call to that function. If "
                               "program execution is in the outermost frame (i.e. in "
                               "main()) then this operation has no effect.") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepOut()));
    ac->addAction("debug_stepout", action);


    action = new KAction(KIcon("dbgmemview"), i18n("Viewers"), this);
    action->setToolTip( i18n("Debugger viewers") );
    action->setWhatsThis(i18n("<b>Debugger viewers</b><p>Various information about application being executed. There are 4 views available:<br>"
        "<b>Memory</b><br>"
        "<b>Disassemble</b><br>"
        "<b>Registers</b><br>"
        "<b>Libraries</b>"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotMemoryView()));
    ac->addAction("debug_memview", action);


    action = new KAction(KIcon("core"), i18n("Examine Core File..."), this);
    action->setToolTip( i18n("Examine core file") );
    action->setWhatsThis( i18n("<b>Examine core file</b><p>"
                               "This loads a core file, which is typically created "
                               "after the application has crashed, e.g. with a "
                               "segmentation fault. The core file contains an "
                               "image of the program memory at the time it crashed, "
                               "allowing you to do a post-mortem analysis.") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotExamineCore()));
    ac->addAction("debug_core", action);


    action = new KAction(KIcon("connect_creating"), i18n("Attach to Process"), this);
    action->setToolTip( i18n("Attach to process") );
    action->setWhatsThis(i18n("<b>Attach to process</b><p>Attaches the debugger to a running process."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAttachProcess()));
    ac->addAction("debug_attach", action);

    action = new KAction(i18n("Toggle Breakpoint"), this);
    action->setToolTip(i18n("Toggle breakpoint"));
    action->setWhatsThis(i18n("<b>Toggle breakpoint</b><p>Toggles the breakpoint at the current line in editor."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleBreakpoint()));
    ac->addAction("debug_toggle_breakpoint", action);

//    connect( mainWindow()->main()->guiFactory(), SIGNAL(clientAdded(KXMLGUIClient*)),
//             this, SLOT(guiClientAdded(KXMLGUIClient*)) );

//    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
//             this, SLOT(projectConfigWidget(KDialogBase*)) );

//     connect( debugger(), SIGNAL(toggledBreakpoint(const QString &, int)),
//              gdbBreakpointWidget, SLOT(slotToggleBreakpoint(const QString &, int)) );
//     connect( debugger(), SIGNAL(editedBreakpoint(const QString &, int)),
//              gdbBreakpointWidget, SLOT(slotEditBreakpoint(const QString &, int)) );
//     connect( debugger(), SIGNAL(toggledBreakpointEnabled(const QString &, int)),
//              gdbBreakpointWidget, SLOT(slotToggleBreakpointEnabled(const QString &, int)) );

//     connect( core(), SIGNAL(contextMenu(Q3PopupMenu *, const Context *)),
//              this, SLOT(contextMenu(Q3PopupMenu *, const Context *)) );
// 
//     connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
//              this, SLOT(slotStop(KDevPlugin*)) );
//     connect( core(), SIGNAL(projectClosed()),
//              this, SLOT(projectClosed()) );
// 
//     connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
//              this, SLOT(slotActivePartChanged(KParts::Part*)) );

    procLineMaker = new ProcessLineMaker();

    connect( procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
             appFrontend(), SLOT(insertStdoutLine(const QString&)) );
    connect( procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
             appFrontend(), SLOT(insertStderrLine(const QString&)) );

    // The output from tracepoints goes to "application" window, because
    // we don't have any better alternative, and using yet another window
    // is undesirable. Besides, this makes tracepoint look even more similar
    // to printf debugging.
    connect( gdbBreakpointWidget,   SIGNAL(tracingOutput(const QByteArray&)),
             procLineMaker,         SLOT(slotReceivedStdout(const QByteArray&)));


    connect(core()->documentController(), SIGNAL(documentSaved(KDevelop::IDocument*)),
            this, SLOT(slotFileSaved()));

//     if (project())
//         connect(project(), SIGNAL(projectCompiled()),
//                 this, SLOT(slotProjectCompiled()));

    setupController();
}

void CppDebuggerPlugin::setupDcop()
{
    /*QCStringList objects = kapp->dcopClient()->registeredApplications();
    for (QCStringList::Iterator it = objects.begin(); it != objects.end(); ++it)
        if ((*it).indexOf("drkonqi-") == 0)
            slotDCOPApplicationRegistered(*it);

    connect(kapp->dcopClient(), SIGNAL(applicationRegistered(const QByteArray&)), SLOT(slotDCOPApplicationRegistered(const QByteArray&)));
    kapp->dcopClient()->setNotifications(true);*/
}

/*void CppDebuggerPlugin::slotDCOPApplicationRegistered(const QByteArray& appId)
{
    if (appId.indexOf("drkonqi-") == 0) {
        QByteArray answer;
        QByteArray replyType;

        kapp->dcopClient()->call(appId, "krashinfo", "appName()", QByteArray(), replyType, answer, true, 5000);

        QDataStream d(answer, QIODevice::ReadOnly);
        QByteArray appName;
        d >> appName;

        if (appName.length() && project() && project()->mainProgram().endsWith(appName)) {
            kapp->dcopClient()->send(appId, "krashinfo", "registerDebuggingApplication(QString)", i18n("Debug in &KDevelop"));
            connectDCOPSignal(appId, "krashinfo", "acceptDebuggingApplication()", "slotDebugExternalProcess()", true);
        }
    }
}

ASYNC CppDebuggerPlugin::slotDebugExternalProcess()
{
    QByteArray answer;
    QByteArray replyType;

    kapp->dcopClient()->call(kapp->dcopClient()->senderId(), "krashinfo", "pid()", QByteArray(), replyType, answer, true, 5000);

    QDataStream d(answer, QIODevice::ReadOnly);
    int pid;
    d >> pid;

    if (attachProcess(pid) && m_drkonqi.isEmpty()) {
        m_drkonqi = kapp->dcopClient()->senderId();
        QTimer::singleShot(15000, this, SLOT(slotCloseDrKonqi()));
        mainWindow()->raiseView(framestackWidget);
    }

    mainWindow()->main()->raise();
}

ASYNC CppDebuggerPlugin::slotDebugCommandLine(const QString& /command/)
{
    KMessageBox::information(0, "Asked to debug command line");
}*/

void CppDebuggerPlugin::slotCloseDrKonqi()
{
    kapp->dcopClient()->send(m_drkonqi, "MainApplication-Interface", "quit()", QByteArray());
    m_drkonqi = "";
}

CppDebuggerPlugin::~CppDebuggerPlugin()
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


void CppDebuggerPlugin::guiClientAdded( KXMLGUIClient* client )
{
    // Can't change state until after XMLGUI has been loaded...
    // Anyone know of a better way of doing this?
    if( client == this )
        stateChanged( QString("stopped") );
}

void CppDebuggerPlugin::contextMenu(Q3PopupMenu *popup, const Context *context)
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


void CppDebuggerPlugin::toggleBreakpoint()
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


void CppDebuggerPlugin::contextWatch()
{
    variableWidget->slotAddWatchVariable(m_contextIdent);
}

void CppDebuggerPlugin::contextEvaluate()
{
    variableWidget->slotEvaluateExpression(m_contextIdent);
}

void CppDebuggerPlugin::projectConfigWidget(KDialogBase *dlg)
{
    KVBox *vbox = dlg->addVBoxPage(i18n("Debugger"), i18n("Debugger"), BarIcon( info()->icon(), KIconLoader::SizeMedium) );
    DebuggerConfigWidget *w = new DebuggerConfigWidget(this, vbox, "debugger config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(finished()), controller, SLOT(configure()) );
}


void CppDebuggerPlugin::setupController()
{
    VariableTree *variableTree = variableWidget->varTree();

    // variableTree -> gdbBreakpointWidget
    connect( variableTree,          SIGNAL(toggleWatchpoint(const QString &)),
             gdbBreakpointWidget,   SLOT(slotToggleWatchpoint(const QString &)));

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


bool CppDebuggerPlugin::startDebugger()
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
        run_arguments = project()->debugArguments();
    }

    QString shell = DomUtil::readEntry(*projectDom(), "/kdevdebugger/general/dbgshell");
    if( !shell.isEmpty() )
    {
        shell = shell.simplified();
        QString shell_without_args = QStringList::split(QChar(' '), shell ).first();

        QFileInfo info( shell_without_args );
        if( info.isRelative() )
        {
            shell_without_args = build_dir + "/" + shell_without_args;
            info.setFile( shell_without_args );
        }
        if( !info.exists() )
        {
            KMessageBox::information(
                mainWindow()->main(),
                i18n("Could not locate the debugging shell '%1'.").arg( shell_without_args ),
                i18n("Debugging Shell Not Found"), "gdb_error" );
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
        mainWindow()->setViewAvailable(variableWidget, true);

        framestackWidget->setEnabled(true);
        disassembleWidget->setEnabled(true);

        gdbOutputWidget->setEnabled(true);


        if (DomUtil::readBoolEntry(*projectDom(), "/kdevdebugger/general/floatingtoolbar", false))
        {
#ifndef QT_MAC
            floatingToolBar = new DbgToolBar(this, mainWindow()->main());
            floatingToolBar->show();
#endif
        }

        running_ = true;
        return true;
    }
    else
    {
        return false;
    }
}

void CppDebuggerPlugin::slotStopDebugger()
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


    mainWindow()->setViewAvailable(variableWidget, false);
    mainWindow()->setViewAvailable(framestackWidget, false);
    mainWindow()->setViewAvailable(disassembleWidget, false);
    mainWindow()->setViewAvailable(gdbOutputWidget, false);

    KActionCollection *ac = actionCollection();
    ac->action("debug_run")->setText( i18n("&Start") );
//    ac->action("debug_run")->setIcon( "arrow-right" );
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

void CppDebuggerPlugin::slotShowView(bool show)
{
    const QWidget* s = static_cast<const QWidget*>(sender());
    QWidget* ncs = const_cast<QWidget*>(s);
    mainWindow()->setViewAvailable(ncs, show);
    if (show)
        mainWindow()->raiseView(ncs);
}

void CppDebuggerPlugin::slotDebuggerAbnormalExit()
{
    mainWindow()->raiseView(gdbOutputWidget);

    KMessageBox::information(
        mainWindow()->main(),
        i18n("<b>GDB exited abnormally</b>"
             "<p>This is likely a bug in GDB. "
             "Examine the gdb output window and then stop the debugger"),
        i18n("GDB exited abnormally"), "gdb_error");

    // Note: we don't stop the debugger here, becuse that will hide gdb
    // window and prevent the user from finding the exact reason of the
    // problem.
}

void CppDebuggerPlugin::slotFileSaved()
{
    needRebuild_ = true;
}

void CppDebuggerPlugin::slotProjectCompiled()
{
    needRebuild_ = false;
}

void CppDebuggerPlugin::projectClosed()
{
    slotStopDebugger();
}

void CppDebuggerPlugin::slotRun()
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

void CppDebuggerPlugin::slotRun_part2()
{
    needRebuild_ = false;

    disconnect(project(), SIGNAL(projectCompiled()),
               this, SLOT(slotRun_part2()));

    if (controller->stateIsOn( s_dbgNotStarted ))
    {
        mainWindow()->statusBar()->message(i18n("Debugging program"), 1000);
        if ( DomUtil::readBoolEntry( *projectDom(), "/kdevdebugger/general/raiseGDBOnStart", false ) )
        {
            mainWindow()->raiseView( gdbOutputWidget );
        }else
        {
            mainWindow()->raiseView( framestackWidget );
        }
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


void CppDebuggerPlugin::slotRestart()
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

void CppDebuggerPlugin::slotExamineCore()
{
    mainWindow()->statusBar()->message(i18n("Choose a core file to examine..."), 1000);

    QString dirName = project()? project()->projectDirectory() : QDir::homePath();
    QString coreFile = KFileDialog::getOpenFileName(dirName);
    if (coreFile.isNull())
        return;

    mainWindow()->statusBar()->message(i18n("Examining core file %1").arg(coreFile), 1000);

    startDebugger();
    controller->slotCoreFile(coreFile);
}


void CppDebuggerPlugin::slotAttachProcess()
{
    mainWindow()->statusBar()->message(i18n("Choose a process to attach to..."), 1000);

    Dbg_PS_Dialog dlg;
    if (!dlg.exec() || !dlg.pidSelected())
        return;

    int pid = dlg.pidSelected();
    attachProcess(pid);
}

bool CppDebuggerPlugin::attachProcess(int pid)
{
    mainWindow()->statusBar()->message(i18n("Attaching to process %1").arg(pid), 1000);

    bool ret = startDebugger();
    controller->slotAttachTo(pid);
    return ret;
}


void CppDebuggerPlugin::slotStop(KDevPlugin* which)
{
    if( which != 0 && which != this )
        return;

//    if( !controller->stateIsOn( s_dbgNotStarted ) && !controller->stateIsOn( s_shuttingDown ) )
        slotStopDebugger();
}


void CppDebuggerPlugin::slotPause()
{
    controller->slotBreakInto();
}


void CppDebuggerPlugin::slotRunToCursor()
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

void CppDebuggerPlugin::slotJumpToCursor()
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

void CppDebuggerPlugin::slotStepOver()
{
    controller->slotStepOver();
}


void CppDebuggerPlugin::slotStepOverInstruction()
{
    controller->slotStepOverIns();
}


void CppDebuggerPlugin::slotStepIntoInstruction()
{
    controller->slotStepIntoIns();
}


void CppDebuggerPlugin::slotStepInto()
{
    controller->slotStepInto();
}


void CppDebuggerPlugin::slotStepOut()
{
    controller->slotStepOutOff();
}


void CppDebuggerPlugin::slotMemoryView()
{
    viewerWidget->slotAddMemoryView();
}

void CppDebuggerPlugin::slotRefreshBPState( const Breakpoint& BP)
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

void CppDebuggerPlugin::slotStatus(const QString &msg, int state)
{
    QString stateIndicator, stateIndicatorFull;

    if (state & s_dbgNotStarted)
    {
        stateIndicator = " ";
        stateIndicatorFull = "Debugger not started";
        stateChanged( QString("stopped") );
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
            mainWindow()->setViewAvailable(variableWidget, true);
            mainWindow()->raiseView(variableWidget);
        }
    }

    if (state & s_appNotStarted)
    {
        KActionCollection *ac = actionCollection();
        ac->action("debug_run")->setText( i18nc("To start something","Start") );
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
    kDebug(9012) << "Debugger state: " << stateIndicator << ": ";
    kDebug(9012) << "   " << msg;

    /*statusBarIndicator->setText(stateIndicator);
    statusBarIndicator->setToolTip( stateIndicatorFull);
    if (!msg.isEmpty())
        mainWindow()->statusBar()->message(msg, 3000);*/


    previousDebuggerState_ = state;
}

void CppDebuggerPlugin::slotEvent(GDBController::event_t e)
{
    if (e == GDBController::program_running ||
        e == GDBController::program_exited ||
        e == GDBController::debugger_exited)
    {
        debugger()->clearExecutionPoint();
    }
}


void CppDebuggerPlugin::slotShowStep(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
    {
        // Debugger counts lines from 1
        debugger()->gotoExecutionPoint(KUrl( fileName ), lineNum-1);
    }
    else
    {
        debugger()->clearExecutionPoint();
    }
}


void CppDebuggerPlugin::slotGotoSource(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
        partController()->editDocument(KUrl( fileName ), lineNum);
}


void CppDebuggerPlugin::slotActivePartChanged( KParts::Part* part )
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

void CppDebuggerPlugin::restorePartialProjectSession(const QDomElement* el)
{
    gdbBreakpointWidget->restorePartialProjectSession(el);
    gdbOutputWidget->restorePartialProjectSession(el);
}

void CppDebuggerPlugin::savePartialProjectSession(QDomElement* el)
{
    gdbBreakpointWidget->savePartialProjectSession(el);
    gdbOutputWidget->savePartialProjectSession(el);
}

bool CppDebuggerPlugin::haveModifiedFiles()
{
    bool have_modified = false;
    KUrl::List const& filelist = partController()->openURLs();
    KUrl::List::ConstIterator it = filelist.begin();
    while ( it != filelist.end() )
    {
        if (partController()->documentState(*it) != Clean)
            have_modified = true;

        ++it;
    }

    return have_modified;
}

}

KDevAppFrontend * GDBDebugger::CppDebuggerPlugin::appFrontend( )
{
    return extension<KDevAppFrontend>("KDevelop/AppFrontend");
}

KDevDebugger * GDBDebugger::CppDebuggerPlugin::debugger()
{
    return m_debugger;
}

#include "debuggerpart.moc"
