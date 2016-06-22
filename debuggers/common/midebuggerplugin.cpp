/*
 * Common code for MI debugger support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "midebuggerplugin.h"

#include "midebugjobs.h"
#include "dialogs/processselection.h"

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <language/interfaces/editorcontext.h>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>
#include <KStringHandler>

#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QSignalMapper>
#include <QTimer>

using namespace KDevelop;
using namespace KDevMI;

MIDebuggerPlugin::MIDebuggerPlugin(const QString &componentName, QObject *parent)
    : KDevelop::IPlugin(componentName, parent)
{
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IStatus)

    core()->debugController()->initializeUi();

    setupToolviews();
    setupActions();
    setupDBus();
}

void MIDebuggerPlugin::setupToolviews()
{
    // TODO: port tool views
    /*
    disassemblefactory = new DebuggerToolFactory<DisassembleWidget>(
    this, "org.kdevelop.debugger.DisassemblerView", Qt::BottomDockWidgetArea);

    lldbfactory = new DebuggerToolFactory<LLDBOutputWidget>(
    this, "org.kdevelop.debugger.ConsoleView",Qt::BottomDockWidgetArea);

    core()->uiController()->addToolView(
        i18n("Disassemble/Registers"),
        disassemblefactory);

    core()->uiController()->addToolView(
        i18n("LLDB"),
        lldbfactory);

#ifndef WITH_OKTETA
    memoryviewerfactory = nullptr;
#else
    memoryviewerfactory = new DebuggerToolFactory<MemoryViewerWidget>(
    this, "org.kdevelop.debugger.MemoryView", Qt::BottomDockWidgetArea);
    core()->uiController()->addToolView(
        i18n("Memory"),
        memoryviewerfactory);
#endif
*/
}

void MIDebuggerPlugin::setupActions()
{
    KActionCollection* ac = actionCollection();

    QAction * action = new QAction(this);
    action->setIcon(QIcon::fromTheme("core"));
    action->setText(i18n("Examine Core File..."));
    action->setToolTip(i18n("Examine core file"));
    action->setWhatsThis(i18n("<b>Examine core file</b>"
                              "<p>This loads a core file, which is typically created "
                              "after the application has crashed, e.g. with a "
                              "segmentation fault. The core file contains an "
                              "image of the program memory at the time it crashed, "
                              "allowing you to do a post-mortem analysis.</p>"));
    connect(action, &QAction::triggered, this, &MIDebuggerPlugin::slotExamineCore);
    ac->addAction("debug_core", action);

    #ifdef KDEV_ENABLE_DBG_ATTACH_DIALOG
    action = new QAction(this);
    action->setIcon(QIcon::fromTheme("connect_creating"));
    action->setText(i18n("Attach to Process..."));
    action->setToolTip(i18n("Attach to process"));
    action->setWhatsThis(i18n("<b>Attach to process</b>"
                              "<p>Attaches the debugger to a running process.</p>"));
    connect(action, &QAction::triggered, this, &MIDebuggerPlugin::slotAttachProcess);
    ac->addAction("debug_attach", action);
    #endif
}

void MIDebuggerPlugin::setupDBus()
{
    m_drkonqiMap = new QSignalMapper(this);
    connect(m_drkonqiMap, static_cast<void(QSignalMapper::*)(QObject*)>(&QSignalMapper::mapped),
            this, &MIDebuggerPlugin::slotDebugExternalProcess);

    QDBusConnectionInterface* dbusInterface = QDBusConnection::sessionBus().interface();
    for (const auto &service : dbusInterface->registeredServiceNames().value()) {
        slotDBusServiceRegistered(service);
    }

    QDBusServiceWatcher* watcher = new QDBusServiceWatcher(this);
    connect(watcher, &QDBusServiceWatcher::serviceRegistered,
            this, &MIDebuggerPlugin::slotDBusServiceRegistered);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &MIDebuggerPlugin::slotDBusServiceUnregistered);
}

void MIDebuggerPlugin::unload()
{
    // TODO: port tool views
    /*
    core()->uiController()->removeToolView(disassemblefactory);
    core()->uiController()->removeToolView(lldbfactory);
    core()->uiController()->removeToolView(memoryviewerfactory);
    */
}

MIDebuggerPlugin::~MIDebuggerPlugin()
{
}

void MIDebuggerPlugin::slotDBusServiceRegistered(const QString& service)
{
    if (service.startsWith("org.kde.drkonqi")) {
        // New registration
        QDBusInterface* drkonqiInterface = new QDBusInterface(service, "/krashinfo",
                                                              QString(), QDBusConnection::sessionBus(),
                                                              this);
        m_drkonqis.insert(service, drkonqiInterface);

        connect(drkonqiInterface, SIGNAL(acceptDebuggingApplication()), m_drkonqiMap, SLOT(map()));
        m_drkonqiMap->setMapping(drkonqiInterface, drkonqiInterface);

        drkonqiInterface->call("registerDebuggingApplication", i18n("KDevelop"));
    }
}

void MIDebuggerPlugin::slotDBusServiceUnregistered(const QString& service)
{
    if (service.startsWith("org.kde.drkonqi")) {
        // Deregistration
        if (m_drkonqis.contains(service))
            delete m_drkonqis.take(service);
    }
}

void MIDebuggerPlugin::slotDebugExternalProcess(QObject* interface)
{
    auto dbusInterface = static_cast<QDBusInterface*>(interface);

    QDBusReply<int> reply = dbusInterface->call("pid");
    if (reply.isValid()) {
        attachProcess(reply.value());
        QTimer::singleShot(500, this, &MIDebuggerPlugin::slotCloseDrKonqi);

        m_drkonqi = m_drkonqis.key(dbusInterface);
    }

    core()->uiController()->activeMainWindow()->raise();
}

void MIDebuggerPlugin::slotCloseDrKonqi()
{
    if (!m_drkonqi.isEmpty()) {
        QDBusInterface drkonqiInterface(m_drkonqi, "/MainApplication", "org.kde.KApplication");
        drkonqiInterface.call("quit");
        m_drkonqi.clear();
    }
}

ContextMenuExtension MIDebuggerPlugin::contextMenuExtension(Context* context)
{
    ContextMenuExtension menuExt = IPlugin::contextMenuExtension(context);

    if (context->type() != KDevelop::Context::EditorContext)
        return menuExt;

    EditorContext *econtext = dynamic_cast<EditorContext*>(context);
    if (!econtext)
        return menuExt;

    QString contextIdent = econtext->currentWord();

    if (!contextIdent.isEmpty())
    {
        QString squeezed = KStringHandler::csqueeze(contextIdent, 30);

        QAction* action = new QAction(this);
        action->setText(i18n("Evaluate: %1", squeezed));
        action->setWhatsThis(i18n("<b>Evaluate expression</b>"
                                  "<p>Shows the value of the expression under the cursor.</p>"));
        connect(action, &QAction::triggered, this, [this, contextIdent](){
            emit addWatchVariable(contextIdent);
        });
        menuExt.addAction(ContextMenuExtension::DebugGroup, action);

        action = new QAction(this);
        action->setText(i18n("Watch: %1", squeezed));
        action->setWhatsThis(i18n("<b>Watch expression</b>"
                                  "<p>Adds the expression under the cursor to the Variables/Watch list.</p>"));
        connect(action, &QAction::triggered, this, [this, contextIdent](){
            emit evaluateExpression(contextIdent);
        });
        menuExt.addAction(ContextMenuExtension::DebugGroup, action);
    }

    return menuExt;
}

void MIDebuggerPlugin::slotExamineCore()
{
    showStatusMessage(i18n("Choose a core file to examine..."), 1000);

    if (core()->debugController()->currentSession() != nullptr) {
        KMessageBox::ButtonCode answer = KMessageBox::warningYesNo(
            core()->uiController()->activeMainWindow(),
            i18n("A program is already being debugged. Do you want to abort the "
                 "currently running debug session and continue?"));
        if (answer == KMessageBox::No)
            return;
    }
    MIExamineCoreJob *job = new MIExamineCoreJob(this, core()->runController());
    core()->runController()->registerJob(job);
    // job->start() is called in registerJob
}

#ifdef KDEV_ENABLE_DBG_ATTACH_DIALOG
void MIDebuggerPlugin::slotAttachProcess()
{
    showStatusMessage(i18n("Choose a process to attach to..."), 1000);

    if (core()->debugController()->currentSession() != nullptr) {
        KMessageBox::ButtonCode answer = KMessageBox::warningYesNo(
            core()->uiController()->activeMainWindow(),
            i18n("A program is already being debugged. Do you want to abort the "
                 "currently running debug session and continue?"));
        if (answer == KMessageBox::No)
            return;
    }

    ProcessSelectionDialog dlg(core()->uiController()->activeMainWindow());
    if (!dlg.exec() || !dlg.pidSelected())
        return;

    // TODO: move check into process selection dialog
    int pid = dlg.pidSelected();
    if (QApplication::applicationPid() == pid)
        KMessageBox::error(core()->uiController()->activeMainWindow(),
                           i18n("Not attaching to process %1: cannot attach the debugger to itself.", pid));
    else
        attachProcess(pid);
}
#endif

void MIDebuggerPlugin::attachProcess(int pid)
{
    MIAttachProcessJob *job = new MIAttachProcessJob(this, pid, core()->runController());
    core()->runController()->registerJob(job);
    // job->start() is called in registerJob
}

QString MIDebuggerPlugin::statusName() const
{
    return i18n("Debugger");
}

void MIDebuggerPlugin::showStatusMessage(const QString& msg, int timeout)
{
    emit showMessage(this, msg, timeout);
}
