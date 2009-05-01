// /*
//  * GDB Debugger Support
//  *
//  * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
//  * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
//  * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
//  * Copyright 2007 Hamish Rodda <rodda@kde.org>
//  *
//  * This program is free software; you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as
//  * published by the Free Software Foundation; either version 2 of the
//  * License, or (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public
//  * License along with this program; if not, write to the
//  * Free Software Foundation, Inc.,
//  * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//  */

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
#include <kaboutdata.h>
#include <KToolBar>
#include <KDialog>
#include <kwindowsystem.h>
#include <KXmlGuiWindow>
#include <KXMLGUIFactory>

#include <sublime/view.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/context.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <language/interfaces/editorcontext.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <execute/executepluginconstants.h>
#include <interfaces/launchconfigurationtype.h>

#include "variablewidget.h"
#include "disassemblewidget.h"
#include "gdbcontroller.h"
#include "processselection.h"
#include "memviewdlg.h"
#include "gdbparser.h"
#include "gdboutputwidget.h"
#include "gdbglobal.h"
#include "variablecollection.h"
#include "debugsession.h"

#include <iostream>
#include "gdblaunchconfig.h"


namespace GDBDebugger
{

K_PLUGIN_FACTORY(CppDebuggerFactory, registerPlugin<CppDebuggerPlugin>(); )
K_EXPORT_PLUGIN(CppDebuggerFactory(KAboutData("kdevgdb","kdevgdb", ki18n("GDB Support"), "0.1", ki18n("Support for running apps in GDB"), KAboutData::License_GPL)))

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
      if (view->widget()->metaObject()->indexOfSignal(SIGNAL(requestRaise())) != -1)
          QObject::connect(view->widget(), SIGNAL(requestRaise()), view, SLOT(requestRaise()));
  }

  /* At present, some debugger widgets (e.g. breakpoint) contain actions so that shortcuts
     work, but they don't need any toolbar.  So, suppress toolbar action.  */
  virtual QList<QAction*> toolBarActions( QWidget* viewWidget ) const
  {
      Q_UNUSED(viewWidget);
      return QList<QAction*>();
  }

private:
  CppDebuggerPlugin* m_plugin;
  GDBController* m_controller;
  QString m_id;
  Qt::DockWidgetArea m_defaultArea;
};

CppDebuggerPlugin::CppDebuggerPlugin( QObject *parent, const QVariantList & ) :
    KDevelop::IPlugin( CppDebuggerFactory::componentData(), parent ),
    m_config(KGlobal::config(), "GDB Debugger"), m_session(0)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IStatus )

    setXMLFile("kdevgdbui.rc");

    // Setup widgets and dbgcontroller
    m_controller = new GDBController(this);

    core()->uiController()->addToolView(
        i18n("Disassemble"),
        new DebuggerToolFactory<DisassembleWidget>(
            this, m_controller, "org.kdevelop.debugger.DisassemblerView",
            Qt::BottomDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("GDB"),
        new DebuggerToolFactory<GDBOutputWidget>(
            this, m_controller, "org.kdevelop.debugger.ConsoleView",
            Qt::BottomDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("Debug views"),
        new DebuggerToolFactory<ViewerWidget>(
            this, m_controller, "org.kdevelop.debugger.VariousViews",
            Qt::BottomDockWidgetArea));

    core()->uiController()->addToolView(
        i18n("Variables"),
        new DebuggerToolFactory<VariableWidget>(
            this, m_controller, "org.kdevelop.debugger.VariablesView",
            Qt::LeftDockWidgetArea));

    setupActions();

    setupDBus();

    KDevelop::LaunchConfigurationType* type = core()->runController()->launchConfigurationTypeForId( ExecutePlugin::nativeAppConfigTypeId );
    Q_ASSERT(type);
    type->addLauncher( new GdbLauncher( this ) );
    
    // The output from tracepoints goes to "application" window, because
    // we don't have any better alternative, and using yet another window
    // is undesirable. Besides, this makes tracepoint look even more similar
    // to printf debugging.
// PORTING TODO broken - need intermediate signal?
//     connect( gdbBreakpointWidget,   SIGNAL(tracingOutput(const QByteArray&)),
//              procLineMaker,         SLOT(slotReceivedStdout(const QByteArray&)));
}

void CppDebuggerPlugin::setupActions()
{
    KActionCollection* ac = actionCollection();

    KAction* action = new KAction(KIcon("dbgmemview"), i18n("Viewers"), this);
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

    //bool running = m_session && m_session->isRunning();

    // If debugger is running, we insert items at the top.
    // The reason is user has explicitly run the debugger, so he's
    // surely debugging, not editing code or something. So, first
    // menu items should be about debugging, not some copy/paste/cut
    // things.
    //if (!running)
        //popup->addSeparator();

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

void CppDebuggerPlugin::contextWatch()
{
    emit addWatchVariable(m_contextIdent);
}

void CppDebuggerPlugin::contextEvaluate()
{
    emit evaluateExpression(m_contextIdent);
}

DebugSession* CppDebuggerPlugin::createSession()
{
    if (m_session) {
        delete m_session;
    }
    m_session = new DebugSession(m_controller);
    KDevelop::ICore::self()->debugController()->addSession(m_session);
    connect(m_session, SIGNAL(showMessage(QString,int)), SLOT(controllerMessage(QString,int)));
    connect(m_session, SIGNAL(reset()), SIGNAL(reset()));
    connect(m_session, SIGNAL(finished()), SLOT(slotFinished()));
    connect(m_session, SIGNAL(raiseOutputViews()), SIGNAL(raiseOutputViews()));
    connect(m_session, SIGNAL(raiseFramestackViews()), SIGNAL(raiseFramestackViews()));
    connect(m_session, SIGNAL(raiseVariableViews()), SIGNAL(raiseVariableViews()));
    return m_session;
}


void CppDebuggerPlugin::projectClosed()
{
    if (m_session) {
        m_session->stopDebugger();
    }
}

void CppDebuggerPlugin::slotExamineCore()
{
    emit showMessage(this, i18n("Choose a core file to examine..."), 1000);

    KUrl coreFile = KFileDialog::getOpenUrl(QDir::homePath());
    if (!coreFile.isValid())
        return;

    emit showMessage(this, i18n("Examining core file %1", coreFile.url()), 1000);

    createSession()->examineCoreFile(coreFile);
}


void CppDebuggerPlugin::slotAttachProcess()
{
    emit showMessage(this, i18n("Choose a process to attach to..."), 1000);

    ProcessSelectionDialog dlg;
    if (!dlg.exec() || !dlg.pidSelected())
        return;

    int pid = dlg.pidSelected();
    attachProcess(pid);
}

void CppDebuggerPlugin::attachProcess(int pid)
{
    emit showMessage(this, i18n("Attaching to process %1", pid), 1000);

    createSession()->attachToProcess(pid);
}

// Used to disable breakpoint actions when non-text document selected

// save/restore partial project session

KConfigGroup CppDebuggerPlugin::config() const
{
    return m_config;
}

QString CppDebuggerPlugin::statusName() const
{
    return i18n("Debugger");
}

void CppDebuggerPlugin::demandAttention() const
{
    if ( QWidget * w = qApp->activeWindow() )
    {
        KWindowSystem::demandAttention( w->winId(), true );
    }
}

void CppDebuggerPlugin::slotFinished()
{
    Q_ASSERT(dynamic_cast<DebugSession*>(sender()));
    DebugSession* session = static_cast<DebugSession*>(sender());
    m_startDebugger->setDisabled(false);
}

void CppDebuggerPlugin::controllerMessage( const QString& msg, int timeout )
{
    emit showMessage(this, msg, timeout);
}

}

#include "debuggerplugin.moc"
