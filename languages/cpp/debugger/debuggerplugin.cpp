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

#include "debuggerplugin.h"

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
#include <KToolBar>
#include <KDialog>

#include <sublime/view.h>

#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>
#include <iprojectcontroller.h>
#include <iproject.h>
#include <context.h>
#include <util/processlinemaker.h>

#include "variablewidget.h"
#include "gdbbreakpointwidget.h"
#include "framestackwidget.h"
#include "disassemblewidget.h"
#include "gdbcontroller.h"
#include "breakpoint.h"
#include "processselection.h"
#include "memviewdlg.h"
#include "gdbparser.h"
#include "gdboutputwidget.h"
#include "debuggerconfigwidget.h"

#include "gdbglobal.h"
#include "breakpointcontroller.h"

#include <iostream>

#include <kvbox.h>


namespace GDBDebugger
{

K_PLUGIN_FACTORY(CppDebuggerFactory, registerPlugin<CppDebuggerPlugin>(); )
K_EXPORT_PLUGIN(CppDebuggerFactory("kdevcppdebugger"))

class BreakpointListFactory : public KDevelop::IToolViewFactory
{
public:
  BreakpointListFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new GDBBreakpointWidget(m_plugin, m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::BottomDockWidgetArea;
  }

  virtual void viewCreated(Sublime::View* view)
  {
      QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

class VariableBreakpointListFactory : public KDevelop::IToolViewFactory
{
public:
  VariableBreakpointListFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new VariableWidget(m_plugin, m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::LeftDockWidgetArea;
  }

  virtual void viewCreated(Sublime::View* view)
  {
      QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

class FramestackViewFactory : public KDevelop::IToolViewFactory
{
public:
  FramestackViewFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new FramestackWidget(m_plugin, m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::BottomDockWidgetArea;
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

class DisassembleViewFactory : public KDevelop::IToolViewFactory
{
public:
  DisassembleViewFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new DisassembleWidget(m_plugin, m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::BottomDockWidgetArea;
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

class GDBOutputViewFactory : public KDevelop::IToolViewFactory
{
public:
  GDBOutputViewFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new GDBOutputWidget(m_plugin, m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::BottomDockWidgetArea;
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

class SpecialViewFactory : public KDevelop::IToolViewFactory
{
public:
  SpecialViewFactory(CppDebuggerPlugin* plugin, GDBController* controller): m_plugin(plugin), m_controller(controller) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new ViewerWidget(m_plugin, m_controller, parent);
  }

  virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
  {
    return Qt::BottomDockWidgetArea;
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
};

CppDebuggerPlugin::CppDebuggerPlugin( QObject *parent, const QVariantList & ) :
    KDevelop::IPlugin( CppDebuggerFactory::componentData(), parent ),
    controller(0), previousDebuggerState_(s_dbgNotStarted),
    justRestarted_(false), needRebuild_(true),
    running_(false), m_config(KGlobal::config(), "GDB Debugger"),
    m_breakpointController(new BreakpointController(this))
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IRunProvider )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IStatus )

    setXMLFile("kdevcppdebuggerui.rc");

    // Setup widgets and dbgcontroller

    controller = new GDBController(this);

    m_breakpointFactory = new BreakpointListFactory(this, controller);
    core()->uiController()->addToolView(i18n("Breakpoints"), m_breakpointFactory);

    m_variableFactory = new VariableBreakpointListFactory(this, controller);
    core()->uiController()->addToolView(i18n("Variables"), m_variableFactory);

    m_framestackFactory = new FramestackViewFactory(this, controller);
    core()->uiController()->addToolView(i18n("Frame Stack"), m_framestackFactory);

    m_disassembleFactory = new DisassembleViewFactory(this, controller);
    core()->uiController()->addToolView(i18n("Disassemble"), m_disassembleFactory);

    m_outputFactory = new GDBOutputViewFactory(this, controller);
    core()->uiController()->addToolView(i18n("GDB"), m_outputFactory);

    m_specialFactory = new SpecialViewFactory(this, controller);
    core()->uiController()->addToolView(i18n("Debug views"), m_specialFactory);

    setupActions();

//    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
//             this, SLOT(projectConfigWidget(KDialogBase*)) );

//     connect( core(), SIGNAL(contextMenu(Q3PopupMenu *, const KDevelop::Context *)),
//              this, SLOT(contextMenu(Q3PopupMenu *, const KDevelop::Context *)) );
//
//     connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
//              this, SLOT(slotStop(KDevPlugin*)) );
//     connect( core(), SIGNAL(projectClosed()),
//              this, SLOT(projectClosed()) );
//
//     connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
//              this, SLOT(slotActivePartChanged(KParts::Part*)) );

    procLineMaker = new KDevelop::ProcessLineMaker();

//     connect( procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
//              appFrontend(), SLOT(insertStdoutLine(const QString&)) );
//     connect( procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
//              appFrontend(), SLOT(insertStderrLine(const QString&)) );

    // The output from tracepoints goes to "application" window, because
    // we don't have any better alternative, and using yet another window
    // is undesirable. Besides, this makes tracepoint look even more similar
    // to printf debugging.
// PORTING TODO broken - need intermediate signal?
//     connect( gdbBreakpointWidget,   SIGNAL(tracingOutput(const QByteArray&)),
//              procLineMaker,         SLOT(slotReceivedStdout(const QByteArray&)));


    connect(core()->documentController(), SIGNAL(documentSaved(KDevelop::IDocument*)),
            this, SLOT(slotFileSaved()));

//     if (project())
//         connect(project(), SIGNAL(projectCompiled()),
//                 this, SLOT(slotProjectCompiled()));

    setupController();
}

void CppDebuggerPlugin::setupActions()
{
    KActionCollection* ac = actionCollection();

    KAction* action = m_startDebugger = new KAction(KIcon("dbgrun"), i18n("&Start"), this);
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

    m_restartDebugger = action = new KAction(KIcon("dbgrestart"), i18n("&Restart"), this);
    action->setToolTip( i18n("Restart program") );
    action->setWhatsThis( i18n("<b>Restarts application</b><p>"
                               "Restarts applications from the beginning."
                              ) );
    action->setEnabled(false);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRestart()));
    ac->addAction("debug_restart", action);


    m_stopDebugger = action = new KAction(KIcon("process-stop"), i18n("Sto&p"), this);
    action->setToolTip( i18n("Stop debugger") );
    action->setWhatsThis(i18n("<b>Stop debugger</b><p>Kills the executable and exits the debugger."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStopDebugger()));
    ac->addAction("debug_stop", action);

    m_interruptDebugger = action = new KAction(KIcon("media-playback-pause"), i18n("Interrupt"), this);
    action->setToolTip( i18n("Interrupt application") );
    action->setWhatsThis(i18n("<b>Interrupt application</b><p>Interrupts the debugged process or current GDB command."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotPause()));
    ac->addAction("debug_pause", action);

    m_runToCursor = action = new KAction(KIcon("dbgrunto"), i18n("Run to &Cursor"), this);
    action->setToolTip( i18n("Run to cursor") );
    action->setWhatsThis(i18n("<b>Run to cursor</b><p>Continues execution until the cursor position is reached."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRunToCursor()));
    ac->addAction("debug_runtocursor", action);


    m_setToCursor = action = new KAction(KIcon("dbgjumpto"), i18n("Set E&xecution Position to Cursor"), this);
    action->setToolTip( i18n("Jump to cursor") );
    action->setWhatsThis(i18n("<b>Set Execution Position </b><p>Set the execution pointer to the current cursor position."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotJumpToCursor()));
    ac->addAction("debug_jumptocursor", action);


    m_stepOver = action = new KAction(KIcon("dbgnext"), i18n("Step &Over"), this);
    action->setShortcut(Qt::Key_F10);
    action->setToolTip( i18n("Step over the next line") );
    action->setWhatsThis( i18n("<b>Step over</b><p>"
                               "Executes one line of source in the current source file. "
                               "If the source line is a call to a function the whole "
                               "function is executed and the app will stop at the line "
                               "following the function call.") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepOver()));
    ac->addAction("debug_stepover", action);


    m_stepOverInstruction = action = new KAction(KIcon("dbgnextinst"), i18n("Step over Ins&truction"), this);
    action->setToolTip( i18n("Step over instruction") );
    action->setWhatsThis(i18n("<b>Step over instruction</b><p>Steps over the next assembly instruction."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepOverInstruction()));
    ac->addAction("debug_stepoverinst", action);


    m_stepInto = action = new KAction(KIcon("dbgstep"), i18n("Step &Into"), this);
    action->setShortcut(Qt::Key_F11);
    action->setToolTip( i18n("Step into the next statement") );
    action->setWhatsThis( i18n("<b>Step into</b><p>"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepInto()));
    ac->addAction("debug_stepinto", action);


    m_stepIntoInstruction = action = new KAction(KIcon("dbgstepinst"), i18n("Step into I&nstruction"), this);
    action->setToolTip( i18n("Step into instruction") );
    action->setWhatsThis(i18n("<b>Step into instruction</b><p>Steps into the next assembly instruction."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStepIntoInstruction()));
    ac->addAction("debug_stepintoinst", action);


    m_stepOut = action = new KAction(KIcon("dbgstepout"), i18n("Step O&ut"), this);
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

    m_toggleBreakpoint = action = new KAction(i18n("Toggle Breakpoint"), this);
    action->setToolTip(i18n("Toggle breakpoint"));
    action->setWhatsThis(i18n("<b>Toggle breakpoint</b><p>Toggles the breakpoint at the current line in editor."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleBreakpoint()));
    ac->addAction("debug_toggle_breakpoint", action);
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
    /*kapp->dcopClient()->send(m_drkonqi, "MainApplication-Interface", "quit()", QByteArray());
    m_drkonqi = "";*/
}

CppDebuggerPlugin::~CppDebuggerPlugin()
{
    //kapp->dcopClient()->setNotifications(false);

    delete controller;
    delete floatingToolBar;
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

void CppDebuggerPlugin::contextMenu(QMenu *popup, const KDevelop::Context *context)
{
    if (!context->hasType( KDevelop::Context::EditorContext ))
        return;

    const KDevelop::EditorContext *econtext = static_cast<const KDevelop::EditorContext*>(context);
    m_contextIdent = econtext->currentWord();

    bool running = !(previousDebuggerState_ & s_dbgNotStarted);

    // If debugger is running, we insert items at the top.
    // The reason is user has explicitly run the debugger, so he's
    // surely debugging, not editing code or something. So, first
    // menu items should be about debugging, not some copy/paste/cut
    // things.
    if (!running)
        popup->insertSeparator();

    if (running)
    {
        popup->addAction(m_runToCursor);
    }

    if (econtext->url().isLocalFile())
    {
        popup->addAction(m_toggleBreakpoint);
    }
    if (!m_contextIdent.isEmpty())
    {
        // PORTING TODO
        //QString squeezed = KStringHandler::csqueeze(m_contextIdent, 30);
        QAction* action = popup->addAction( i18n("Evaluate: %1").arg(m_contextIdent),
                                    this, SLOT(contextEvaluate()));
        action->setWhatsThis(i18n("<b>Evaluate expression</b><p>Shows the value of the expression under the cursor."));
        action = popup->addAction( i18n("Watch: %1").arg(m_contextIdent), this, SLOT(contextWatch()));
        action->setWhatsThis(i18n("<b>Watch expression</b><p>Adds an expression under the cursor to the Variables/Watch list."));
    }

    popup->addSeparator();
}


void CppDebuggerPlugin::toggleBreakpoint()
{
    if (KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->activeDocument()) {
      KTextEditor::Cursor cursor = document->cursorPosition();

      if (!cursor.isValid())
        return;

      // PORTING TODO
      //emit toggleBreakpoint(document->url().path(), cursor.line());
    }
}


void CppDebuggerPlugin::contextWatch()
{
    emit addWatchVariable(m_contextIdent);
}

void CppDebuggerPlugin::contextEvaluate()
{
    emit evaluateExpression(m_contextIdent);
}

/*void CppDebuggerPlugin::projectConfigWidget(KDialogBase *dlg)
{
    KVBox *vbox = dlg->addVBoxPage(i18n("Debugger"), i18n("Debugger"), BarIcon( info()->icon(), KIconLoader::SizeMedium) );
    DebuggerConfigWidget *w = new DebuggerConfigWidget(this, vbox);
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
    connect( dlg, SIGNAL(finished()), controller, SLOT(configure()) );
}*/


void CppDebuggerPlugin::setupController()
{
    // variableTree -> gdbBreakpointWidget
//     connect( variableTree,          SIGNAL(toggleWatchpoint(const QString &)),
//              gdbBreakpointWidget,   SLOT(slotToggleWatchpoint(const QString &)));

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
    connect( controller,            SIGNAL(ttyStdout(const QByteArray&)),
             procLineMaker,         SLOT(slotReceivedStdout(const QByteArray&)));
    connect( controller,            SIGNAL(ttyStderr(const QByteArray&)),
             procLineMaker,         SLOT(slotReceivedStderr(const QByteArray&)));

//     connect(statusBarIndicator, SIGNAL(doubleClicked()),
//             controller, SLOT(explainDebuggerStatus()));

}


bool CppDebuggerPlugin::execute(const KDevelop::IRun & run, int serial)
{
    Q_ASSERT(instrumentorsProvided().contains(run.instrumentor()));

    QString shell = config().readEntry("Debugger Shell");
    if( !shell.isEmpty() )
    {
        shell = shell.simplified();
        QString shell_without_args = QStringList::split(QChar(' '), shell ).first();

        QFileInfo info( shell_without_args );
        /*if( info.isRelative() )
        {
            shell_without_args = build_dir + "/" + shell_without_args;
            info.setFile( shell_without_args );
        }*/
        if( !info.exists() )
        {
            KMessageBox::information(
                qApp->activeWindow(),
                i18n("Could not locate the debugging shell '%1'.").arg( shell_without_args ),
                i18n("Debugging Shell Not Found"), "gdb_error" );
            return false;
        }
    }

    if (controller->start(shell, run, serial))
    {
        //core()->running(this, true);

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

//         mainWindow()->setViewAvailable(framestackWidget, true);
//         mainWindow()->setViewAvailable(disassembleWidget, true);
//         mainWindow()->setViewAvailable(gdbOutputWidget, true);
//         mainWindow()->setViewAvailable(variableWidget, true);

        if (config().readEntry("Floating Toolbar", false))
        {
#ifndef QT_MAC
            floatingToolBar = new KToolBar(qApp->activeWindow());
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
    breakpoints()->clearExecutionPoint();

    delete floatingToolBar;
    floatingToolBar = 0;

    emit reset();

//     variableWidget->setEnabled(false);
//     framestackWidget->setEnabled(false);
//     disassembleWidget->setEnabled(false);
//     gdbOutputWidget->setEnabled(false);
//
//
//     mainWindow()->setViewAvailable(variableWidget, false);
//     mainWindow()->setViewAvailable(framestackWidget, false);
//     mainWindow()->setViewAvailable(disassembleWidget, false);
//     mainWindow()->setViewAvailable(gdbOutputWidget, false);

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

    //core()->running(this, false);
}

void CppDebuggerPlugin::slotDebuggerAbnormalExit()
{
    emit raiseOutputViews();

    KMessageBox::information(
        qApp->activeWindow(),
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

        slotRun_part2();
        return;
    }

    controller->slotRun();
}

void CppDebuggerPlugin::slotRun_part2()
{
    if (controller->stateIsOn( s_dbgNotStarted ))
    {
        emit showMessage(i18n("Debugging program"), 1000);
        if ( config().readEntry("Raise GDB On Start", false ) )
        {
            emit raiseOutputViews();
        }
        else
        {
            emit raiseFramestackViews();
        }

        emit clearViews();

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

        emit showMessage(i18n("Running program"), 1000);

        emit clearViews();
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
    emit showMessage(i18n("Choose a core file to examine..."), 1000);

    QString coreFile = KFileDialog::getOpenFileName(QDir::homePath());
    if (coreFile.isNull())
        return;

    emit showMessage(i18n("Examining core file %1").arg(coreFile), 1000);

    startDebugger();
    controller->slotCoreFile(coreFile);
}


void CppDebuggerPlugin::slotAttachProcess()
{
    emit showMessage(i18n("Choose a process to attach to..."), 1000);

    ProcessSelectionDialog dlg;
    if (!dlg.exec() || !dlg.pidSelected())
        return;

    int pid = dlg.pidSelected();
    attachProcess(pid);
}

void CppDebuggerPlugin::attachProcess(int pid)
{
    emit showMessage(i18n("Attaching to process %1").arg(pid), 1000);

    startDebugger();
    controller->slotAttachTo(pid);
}


void CppDebuggerPlugin::slotPause()
{
    controller->slotBreakInto();
}


void CppDebuggerPlugin::slotRunToCursor()
{
    if (KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            controller->slotRunUntil(doc->url().path(), cursor.line() + 1);
    }
}

void CppDebuggerPlugin::slotJumpToCursor()
{
    if (KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            controller->slotJumpTo(doc->url().path(), cursor.line() + 1);
    }
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
    emit addMemoryView();
}

void CppDebuggerPlugin::slotRefreshBPState( const Breakpoint& BP)
{
    if (BP.hasFileAndLine())
    {
        const FilePosBreakpoint& bp = dynamic_cast<const FilePosBreakpoint&>(BP);
        if (bp.isActionDie())
        {
            breakpoints()->setBreakpoint(bp.fileName(), bp.lineNum()-1, -1, true, false);
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
            breakpoints()->setBreakpoint(bp.fileName(), bp.lineNum()-1,
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
            //mainWindow()->setViewAvailable(variableWidget, true);
            emit raiseVariableViews();
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

    if (!msg.isEmpty())
        emit showMessage(msg, 3000);


    previousDebuggerState_ = state;
}

void CppDebuggerPlugin::slotEvent(GDBController::event_t e)
{
    if (e == GDBController::program_running ||
        e == GDBController::program_exited ||
        e == GDBController::debugger_exited)
    {
        breakpoints()->clearExecutionPoint();
    }
}


void CppDebuggerPlugin::slotShowStep(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
    {
        // Debugger counts lines from 1
        breakpoints()->gotoExecutionPoint(KUrl( fileName ), lineNum-1);
    }
    else
    {
        breakpoints()->clearExecutionPoint();
    }
}


void CppDebuggerPlugin::slotGotoSource(const QString &fileName, int lineNum)
{
    if ( ! fileName.isEmpty() )
        KDevelop::ICore::self()->documentController()->openDocument(KUrl( fileName ), KTextEditor::Cursor(lineNum, 0));
}

// Used to disable breakpoint actions when non-text document selected

// save/restore partial project session

QStringList CppDebuggerPlugin::instrumentorsProvided() const
{
    return QStringList() << "gdb";
}

KConfigGroup CppDebuggerPlugin::config() const
{
    return m_config;
}

void CppDebuggerPlugin::abort(int serial)
{
    kWarning() << "Implement";
}

QString CppDebuggerPlugin::statusName() const
{
    return i18n("Debugger");
}

void CppDebuggerPlugin::startDebugger()
{
    KDevelop::IRun run = KDevelop::ICore::self()->runController()->defaultRun();
    run.setInstrumentor("gdb");
    KDevelop::ICore::self()->runController()->execute(run);
}

BreakpointController * CppDebuggerPlugin::breakpoints() const
{
    return m_breakpointController;
}

}

#include "debuggerplugin.moc"
