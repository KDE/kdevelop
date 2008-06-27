/*
 * GDB Debugger Support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "debuggerplugin.h"

#include <QDir>
#include <QToolTip>
#include <QByteArray>
#include <QTimer>
#include <QMenu>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QSignalMapper>

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
#include <kwindowsystem.h>
#include <KXmlGuiWindow>
#include <KXMLGUIFactory>

#include <sublime/view.h>

#include <icore.h>
#include <iuicontroller.h>
#include <idocumentcontroller.h>
#include <iprojectcontroller.h>
#include <iruncontroller.h>
#include <iproject.h>
#include <context.h>
#include <util/processlinemaker.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

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
#include "variablecollection.h"
#include "breakpointwidget.h"

#include <iostream>


namespace GDBDebugger
{

K_PLUGIN_FACTORY(CppDebuggerFactory, registerPlugin<CppDebuggerPlugin>(); )
K_EXPORT_PLUGIN(CppDebuggerFactory("kdevcppdebugger"))

template<class T>
class DebuggerToolFactory : public KDevelop::IToolViewFactory
{
public:
  DebuggerToolFactory(CppDebuggerPlugin* plugin, GDBController* controller,
                      const QString &id, Qt::DockWidgetArea defaultArea)
  : m_plugin(plugin), m_controller(controller),
    m_id(id), m_defaultArea(defaultArea) {}

  virtual QWidget* create(QWidget *parent = 0)
  {
    return new T(m_plugin, m_controller, parent);
  }

  virtual QString id() const
  {
    return m_id;
  }

  virtual Qt::DockWidgetArea defaultPosition()
  {
    return m_defaultArea;
  }

  virtual void viewCreated(Sublime::View* view)
  {
      QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
  QString m_id;
  Qt::DockWidgetArea m_defaultArea;
};

CppDebuggerPlugin::CppDebuggerPlugin( QObject *parent, const QVariantList & ) :
    KDevelop::IPlugin( CppDebuggerFactory::componentData(), parent ),
    controller(0), debuggerState_(s_dbgNotStarted|s_appNotStarted),
    justRestarted_(false),
    running_(false), m_config(KGlobal::config(), "GDB Debugger")
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IRunProvider )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IStatus )

    setXMLFile("kdevcppdebuggerui.rc");

    // Setup widgets and dbgcontroller

    controller = new GDBController(this);

    core()->uiController()->addToolView(
        i18n("Breakpoints"),
        new DebuggerToolFactory<BreakpointWidget>(
            this, controller, "org.kdevelop.debugger.BreakpointsView",
            Qt::BottomDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("Variables"),
        new DebuggerToolFactory<VariableWidget>(
            this, controller, "org.kdevelop.debugger.VariablesView",
            Qt::LeftDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("Frame Stack"),
        new DebuggerToolFactory<FramestackWidget>(
            this, controller, "org.kdevelop.debugger.StackView",
            Qt::BottomDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("Disassemble"),
        new DebuggerToolFactory<DisassembleWidget>(
            this, controller, "org.kdevelop.debugger.DisassemblerView",
            Qt::BottomDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("GDB"),
        new DebuggerToolFactory<GDBOutputWidget>(
            this, controller, "org.kdevelop.debugger.ConsoleView",
            Qt::BottomDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("Debug views"),
        new DebuggerToolFactory<ViewerWidget>(
            this, controller, "org.kdevelop.debugger.VariousViews",
            Qt::BottomDockWidgetArea));

    setupActions();

    setupDBus();

    procLineMaker = new KDevelop::ProcessLineMaker(this);

    connect( procLineMaker, SIGNAL(receivedStdoutLines(const QStringList&)),
             this, SLOT(applicationStandardOutputLines(const QStringList&)));
    connect( procLineMaker, SIGNAL(receivedStderrLines(const QStringList&)),
             this, SLOT(applicationStandardErrorLines(const QStringList&)) );

    // The output from tracepoints goes to "application" window, because
    // we don't have any better alternative, and using yet another window
    // is undesirable. Besides, this makes tracepoint look even more similar
    // to printf debugging.
// PORTING TODO broken - need intermediate signal?
//     connect( gdbBreakpointWidget,   SIGNAL(tracingOutput(const QByteArray&)),
//              procLineMaker,         SLOT(slotReceivedStdout(const QByteArray&)));

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
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStartDebugger()));

    m_restartDebugger = action = new KAction(KIcon("dbgrestart"), i18n("&Restart"), this);
    action->setToolTip( i18n("Restart program") );
    action->setWhatsThis( i18n("<b>Restarts application</b><p>"
                               "Restarts applications from the beginning."
                              ) );
    action->setEnabled(false);
    connect(action, SIGNAL(triggered(bool)), controller, SLOT(slotRestart()));
    ac->addAction("debug_restart", action);


    m_stopDebugger = action = new KAction(KIcon("process-stop"), i18n("Sto&p"), this);
    action->setToolTip( i18n("Stop debugger") );
    action->setWhatsThis(i18n("<b>Stop debugger</b><p>Kills the executable and exits the debugger."));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotStopDebugger()));
    ac->addAction("debug_stop", action);

    m_interruptDebugger = action = new KAction(KIcon("media-playback-pause"), i18n("Interrupt"), this);
    action->setToolTip( i18n("Interrupt application") );
    action->setWhatsThis(i18n("<b>Interrupt application</b><p>Interrupts the debugged process or current GDB command."));
    connect(action, SIGNAL(triggered(bool)), controller, SLOT(slotPauseApp()));
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
    connect(action, SIGNAL(triggered(bool)), controller, SLOT(slotStepOver()));
    ac->addAction("debug_stepover", action);


    m_stepOverInstruction = action = new KAction(KIcon("dbgnextinst"), i18n("Step over Ins&truction"), this);
    action->setToolTip( i18n("Step over instruction") );
    action->setWhatsThis(i18n("<b>Step over instruction</b><p>Steps over the next assembly instruction."));
    connect(action, SIGNAL(triggered(bool)), controller, SLOT(slotStepOverInstruction()));
    ac->addAction("debug_stepoverinst", action);


    m_stepInto = action = new KAction(KIcon("dbgstep"), i18n("Step &Into"), this);
    action->setShortcut(Qt::Key_F11);
    action->setToolTip( i18n("Step into the next statement") );
    action->setWhatsThis( i18n("<b>Step into</b><p>"
                               "Executes exactly one line of source. If the source line "
                               "is a call to a function then execution will stop after "
                               "the function has been entered.") );
    connect(action, SIGNAL(triggered(bool)), controller, SLOT(slotStepInto()));
    ac->addAction("debug_stepinto", action);


    m_stepIntoInstruction = action = new KAction(KIcon("dbgstepinst"), i18n("Step into I&nstruction"), this);
    action->setToolTip( i18n("Step into instruction") );
    action->setWhatsThis(i18n("<b>Step into instruction</b><p>Steps into the next assembly instruction."));
    connect(action, SIGNAL(triggered(bool)), controller, SLOT(slotStepIntoInstruction()));
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
    connect(action, SIGNAL(triggered(bool)), controller, SLOT(slotStepOut()));
    ac->addAction("debug_stepout", action);


    action = new KAction(KIcon("dbgmemview"), i18n("Viewers"), this);
    action->setToolTip( i18n("Debugger viewers") );
    action->setWhatsThis(i18n("<b>Debugger viewers</b><p>Various information about application being executed. There are 4 views available:<br>"
        "<b>Memory</b><br>"
        "<b>Disassemble</b><br>"
        "<b>Registers</b><br>"
        "<b>Libraries</b>"));
    connect(action, SIGNAL(triggered(bool)), this, SIGNAL(addMemoryView()));
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

void CppDebuggerPlugin::setupDBus()
{
    m_drkonqiMap = new QSignalMapper(this);
    connect(m_drkonqiMap, SIGNAL(mapped(QObject*)), this, SLOT(slotDebugExternalProcess(QObject*)));

    QDBusConnectionInterface* dbusInterface = QDBusConnection::sessionBus().interface();
    foreach (const QString& service, dbusInterface->registeredServiceNames().value())
        slotDBusServiceOwnerChanged(service, QString(), service);

    connect(dbusInterface, SIGNAL(serviceOwnerChanged(QString,QString,QString)), this, SLOT(slotDBusServiceOwnerChanged(QString,QString,QString)));
}

void CppDebuggerPlugin::slotDBusServiceOwnerChanged(const QString & name, const QString & oldOwner, const QString & newOwner)
{
    if (name.startsWith("org.kde.drkonqi")) {
        if (!oldOwner.isEmpty()) {
            // Deregistration
            if (m_drkonqis.contains(name))
                delete m_drkonqis.take(name);
        }

        if (!newOwner.isEmpty()) {
            // New registration
            QDBusInterface* drkonqiInterface = new QDBusInterface(name, "/krashinfo", QString(), QDBusConnection::sessionBus(), this);
            m_drkonqis.insert(name, drkonqiInterface);

            connect(drkonqiInterface, SIGNAL(acceptDebuggingApplication()), m_drkonqiMap, SLOT(map()));
            m_drkonqiMap->setMapping(drkonqiInterface, drkonqiInterface);

            drkonqiInterface->call("registerDebuggingApplication", i18n("Debug in &KDevelop"));
        }
    }
}

void CppDebuggerPlugin::slotDebugExternalProcess(QObject* interface)
{
    QDBusReply<int> reply = static_cast<QDBusInterface*>(interface)->call("pid");

    if (reply.isValid()) {
        attachProcess(reply.value());
        QTimer::singleShot(15000, this, SLOT(slotCloseDrKonqi()));
        //mainWindow()->raiseView(framestackWidget);
    }

    //mainWindow()->main()->raise();
}

void CppDebuggerPlugin::slotCloseDrKonqi()
{
    /*kapp->dcopClient()->send(m_drkonqi, "MainApplication-Interface", "quit()", QByteArray());
    m_drkonqi = "";*/
}

CppDebuggerPlugin::~CppDebuggerPlugin()
{
    delete controller;
    delete floatingToolBar;

    GDBParser::destroy();
}


void CppDebuggerPlugin::initializeGuiState()
{
    stateChanged("stopped");
}

KDevelop::ContextMenuExtension CppDebuggerPlugin::contextMenuExtension( KDevelop::Context* context )
{
    KDevelop::ContextMenuExtension menuExt = KDevelop::IPlugin::contextMenuExtension( context );

    if( context->type() != KDevelop::Context::EditorContext )
        return menuExt;

    KDevelop::EditorContext *econtext = dynamic_cast<KDevelop::EditorContext*>(context);
    if (!econtext)
        return menuExt;

    m_contextIdent = econtext->currentWord();

    bool running = !(debuggerState_ & s_dbgNotStarted);

    // If debugger is running, we insert items at the top.
    // The reason is user has explicitly run the debugger, so he's
    // surely debugging, not editing code or something. So, first
    // menu items should be about debugging, not some copy/paste/cut
    // things.
    //if (!running)
        //popup->addSeparator();

    if (running)
    {
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, m_runToCursor);
    }

    if (econtext->url().isLocalFile())
    {
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, m_toggleBreakpoint);
    }
    if (!m_contextIdent.isEmpty())
    {
        // PORTING TODO
        //QString squeezed = KStringHandler::csqueeze(m_contextIdent, 30);
        QAction* action = new QAction( i18n("Evaluate: %1", m_contextIdent), this);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(contextEvaluate()));
        action->setWhatsThis(i18n("<b>Evaluate expression</b><p>Shows the value of the expression under the cursor."));
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, action);

        action = new QAction( i18n("Watch: %1", m_contextIdent), this);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(contextWatch()));
        action->setWhatsThis(i18n("<b>Watch expression</b><p>Adds an expression under the cursor to the Variables/Watch list."));
        menuExt.addAction( KDevelop::ContextMenuExtension::DebugGroup, action);
    }

    return menuExt;
}


void CppDebuggerPlugin::toggleBreakpoint()
{
    if (KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->activeDocument()) {
      KTextEditor::Cursor cursor = document->cursorPosition();

      if (!cursor.isValid())
        return;

      emit toggleBreakpoint(document->url(), cursor);
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

void CppDebuggerPlugin::setupController()
{
    // variableTree -> gdbBreakpointWidget
//     connect( variableTree,          SIGNAL(toggleWatchpoint(const QString &)),
//              gdbBreakpointWidget,   SLOT(slotToggleWatchpoint(const QString &)));

    // controller -> this
    connect( controller,            SIGNAL(debuggerAbnormalExit()),
         this,                  SLOT(slotDebuggerAbnormalExit()));

    connect( controller, SIGNAL(stateChanged(DBGStateFlags, DBGStateFlags)),
             this,       SLOT(slotStateChanged(DBGStateFlags, DBGStateFlags)));

    connect(controller, SIGNAL(showMessage(const QString&, int)), this, SIGNAL(showMessage(const QString&, int)));

    // controller -> procLineMaker
    connect( controller,            SIGNAL(ttyStdout(const QByteArray&)),
             procLineMaker,         SLOT(slotReceivedStdout(const QByteArray&)));
    connect( controller,            SIGNAL(ttyStderr(const QByteArray&)),
             procLineMaker,         SLOT(slotReceivedStderr(const QByteArray&)));

//     connect(statusBarIndicator, SIGNAL(doubleClicked()),
//             controller, SLOT(explainDebuggerStatus()));

    connect(this, SIGNAL(addWatchVariable(const QString&)), controller->variables(), SLOT(slotAddWatchVariable(const QString&)));
    connect(this, SIGNAL(evaluateExpression(const QString&)), controller->variables(), SLOT(slotEvaluateExpression(const QString&)));
}

bool CppDebuggerPlugin::execute(const KDevelop::IRun & run, KJob* job)
{
    Q_ASSERT(instrumentorsProvided().contains(run.instrumentor()));

    return controller->startProgram(run, job);
}

void CppDebuggerPlugin::slotStopDebugger()
{
    controller->stopDebugger();

    emit reset();
}

void CppDebuggerPlugin::slotDebuggerAbnormalExit()
{
    emit raiseOutputViews();

    KMessageBox::information(
        qApp->activeWindow(),
        i18n("<b>GDB exited abnormally</b>"
             "<p>This is likely a bug in GDB. "
             "Examine the gdb output window and then stop the debugger"),
        i18n("GDB exited abnormally"));

    // Note: we don't stop the debugger here, becuse that will hide gdb
    // window and prevent the user from finding the exact reason of the
    // problem.
}

void CppDebuggerPlugin::projectClosed()
{
    slotStopDebugger();
}

void CppDebuggerPlugin::slotExamineCore()
{
    emit showMessage(i18n("Choose a core file to examine..."), 1000);

    KUrl coreFile = KFileDialog::getOpenUrl(QDir::homePath());
    if (!coreFile.isValid())
        return;

    emit showMessage(i18n("Examining core file %1", coreFile.url()), 1000);

    controller->examineCoreFile(coreFile);
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
    emit showMessage(i18n("Attaching to process %1", pid), 1000);

    controller->attachToProcess(pid);
}


void CppDebuggerPlugin::slotRunToCursor()
{
    if (KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            controller->runUntil(doc->url().path(), cursor.line() + 1);
    }
}

void CppDebuggerPlugin::slotJumpToCursor()
{
    if (KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            controller->jumpTo(doc->url().path(), cursor.line() + 1);
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

QString CppDebuggerPlugin::translatedInstrumentor(const QString&) const
{
    return i18n("GDB");
}

KConfigGroup CppDebuggerPlugin::config() const
{
    return m_config;
}

void CppDebuggerPlugin::abort(KJob* job)
{
    Q_UNUSED(job);
    slotStopDebugger();
}

QString CppDebuggerPlugin::statusName() const
{
    return i18n("Debugger");
}

void CppDebuggerPlugin::slotStartDebugger()
{
    if (debuggerState_ & s_dbgNotStarted) {
        // Prevent multiple runs while setting up
        // TODO: remove restriction if supporting multiple debug sessions
        m_startDebugger->setEnabled(false);

        KDevelop::IRun run = KDevelop::ICore::self()->runController()->defaultRun();
        run.setInstrumentor("gdb");
        KDevelop::ICore::self()->runController()->execute(run);
    }
}

void CppDebuggerPlugin::slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState)
{
    QString message;

    DBGStateFlags changedState = oldState ^ newState;
    if (changedState & s_dbgNotStarted) {
        if (newState & s_dbgNotStarted) {
            message = i18n("Debugger stopped");
            if (floatingToolBar)
                floatingToolBar->hide();

        } else {
            if (config().readEntry("Floating Toolbar", false))
            {
            #ifndef QT_MAC
                if (!floatingToolBar) {
                    floatingToolBar = new KToolBar(qApp->activeWindow());
                    floatingToolBar->show();
                }
            #endif
            }
        }

        //core()->running(this, false);
        // TODO enable/disable tool views as applicable
    }

    // As soon as debugger clears 's_appNotStarted' flag, we
    // set 'justRestarted' variable.
    // The other approach would be to set justRestarted in slotRun, slotCore
    // and slotAttach.
    // Note that setting this var in startDebugger is not OK, because the
    // initial state of debugger is exactly the same as state after pause,
    // so we'll always show varaibles view.
    if (changedState & s_appNotStarted) {
        if (newState & s_appNotStarted) {
            m_startDebugger->setText( i18n("&Start") );
            m_startDebugger->setToolTip( i18n("Runs the program in the debugger") );
            m_startDebugger->setWhatsThis( i18n("Start in debugger\n\n"
                                                  "Starts the debugger with the project's main "
                                                  "executable. You may set some breakpoints "
                                                  "before this, or you can interrupt the program "
                                                  "while it is running, in order to get information "
                                                  "about variables, frame stack, and so on.") );
            m_startDebugger->disconnect(controller);
            connect(m_startDebugger, SIGNAL(triggered(bool)), this, SLOT(slotStartDebugger()));

            stateChanged("stopped");

            justRestarted_ = false;

        } else {
            m_startDebugger->setText( i18n("&Continue") );
            m_startDebugger->setToolTip( i18n("Continues the application execution") );
            m_startDebugger->setWhatsThis( i18n("Continue application execution\n\n"
                "Continues the execution of your application in the "
                "debugger. This only takes effect when the application "
                "has been halted by the debugger (i.e. a breakpoint has "
                "been activated or the interrupt was pressed).") );
            m_startDebugger->disconnect(this);
            connect(m_startDebugger, SIGNAL(triggered(bool)), controller, SLOT(slotRun()));
            m_startDebugger->setEnabled(true);

            if ( config().readEntry("Raise GDB On Start", false ) )
            {
                emit raiseOutputViews();
            }
            else
            {
                emit raiseFramestackViews();
            }

            emit clearViews();

            stateChanged("active");
            justRestarted_ = true;
        }
    }

    if (changedState & s_explicitBreakInto)
        if (!(newState & s_explicitBreakInto))
            message = "Application interrupted";

    if (changedState & s_programExited) {
        if (newState & s_programExited) {
            message = i18n("Process exited");
            stateChanged("stopped");
        }
    }

    if (changedState & s_appRunning) {
        if (newState & s_appRunning) {
            message = "Application is running";
            stateChanged("active");
        }
        else
        {
            if (!(newState & s_appNotStarted)) {
                message = "Application is paused";
                stateChanged("paused");

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
        }
    }


    bool program_running = !(newState & s_appNotStarted);
    bool attached_or_core = (newState & s_attached) || (newState & s_core);

    // If program is started, enable the 'restart' comand.
    m_restartDebugger->setEnabled(program_running && !attached_or_core);

    // And now? :-)
    kDebug(9012) << "Debugger state: " << newState << ": ";
    kDebug(9012) << "   " << message;

    if (!message.isEmpty())
        emit showMessage(message, 3000);
}

void CppDebuggerPlugin::demandAttention() const
{
    if ( QWidget * w = qApp->activeWindow() )
    {
        KWindowSystem::demandAttention( w->winId(), true );
    }
}

void CppDebuggerPlugin::applicationStandardOutputLines(const QStringList& lines)
{
    foreach (const QString& line, lines)
        emit output(controller->job(), line, KDevelop::IRunProvider::StandardOutput);
}

void CppDebuggerPlugin::applicationStandardErrorLines(const QStringList& lines)
{
    foreach (const QString& line, lines)
        emit output(controller->job(), line, KDevelop::IRunProvider::StandardError);
}

}

#include "debuggerplugin.moc"
