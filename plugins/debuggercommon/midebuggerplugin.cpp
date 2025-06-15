/*
    SPDX-FileCopyrightText: 1999-2001 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "midebuggerplugin.h"

#include "itoolviewfactoryholder.h"
#include "midebugjobs.h"
#include "midebugsession.h"
#include "dialogs/processselection.h"
#include "dialogs/selectcoredialog.h"

#include <debuglog.h>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
#include <isession.h>
#include <util/scopeddialog.h>

#include <KActionCollection>
#include <KLocalizedString>
#include <KParts/MainWindow>

#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

using namespace KDevelop;
using namespace KDevMI;

namespace {
[[nodiscard]] const QString& replaceSessionQuestionText()
{
    static const QString text = i18n(
        "A program is already being debugged. Do you want to abort the currently running debug session and continue?");
    return text;
}
}

class KDevMI::DBusProxy : public QObject
{
    Q_OBJECT

public:
    DBusProxy(const QString& service, const QString& name, QObject* parent)
        : QObject(parent),
          m_dbusInterface(service, QStringLiteral("/debugger")),
          m_name(name), m_valid(true)
    {}

    ~DBusProxy() override
    {
        if (m_valid) {
            m_dbusInterface.call(QStringLiteral("debuggerClosed"), m_name);
        }
    }

    QDBusInterface* interface()
    {
        return &m_dbusInterface;
    }

    void Invalidate()
    {
        m_valid = false;
    }

public Q_SLOTS:
    void debuggerAccepted(const QString& name)
    {
        if (name == m_name) {
            emit debugProcess(this);
        }
    }

    void debuggingFinished()
    {
        m_dbusInterface.call(QStringLiteral("debuggingFinished"), m_name);
    }

Q_SIGNALS:
    void debugProcess(DBusProxy*);

private:
    QDBusInterface m_dbusInterface;
    QString m_name;
    bool m_valid;
};

MIDebuggerPlugin::MIDebuggerPlugin(const QString& componentName, const QString& displayName, QObject* parent,
                                   const KPluginMetaData& metaData)
    : KDevelop::IPlugin(componentName, parent, metaData)
    , m_displayName(displayName)
{
    core()->debugController()->initializeUi();

    if (qobject_cast<QGuiApplication*>(QCoreApplication::instance())) { // For our guiless test
        setupActions();
    }
    setupDBus();
}

void MIDebuggerPlugin::setupActions()
{
    KActionCollection* ac = actionCollection();

    auto * action = new QAction(this);
    action->setIcon(QIcon::fromTheme(QStringLiteral("core")));
    action->setText(i18nc("@action", "Examine Core File with %1", m_displayName));
    action->setWhatsThis(i18nc("@info:whatsthis",
                              "<b>Examine core file</b>"
                              "<p>This loads a core file, which is typically created "
                              "after the application has crashed, e.g. with a "
                              "segmentation fault. The core file contains an "
                              "image of the program memory at the time it crashed, "
                              "allowing you to do a post-mortem analysis.</p>"));
    connect(action, &QAction::triggered, this, &MIDebuggerPlugin::slotExamineCore);
    ac->addAction(QStringLiteral("debug_core"), action);

#if HAVE_KSYSGUARD
    action = new QAction(this);
    action->setIcon(QIcon::fromTheme(QStringLiteral("connect_creating")));
    action->setText(i18nc("@action", "Attach to Process with %1", m_displayName));
    action->setWhatsThis(i18nc("@info:whatsthis",
                              "<b>Attach to process</b>"
                              "<p>Attaches the debugger to a running process.</p>"));
    connect(action, &QAction::triggered, this, &MIDebuggerPlugin::slotAttachProcess);
    ac->addAction(QStringLiteral("debug_attach"), action);
#endif
}

void MIDebuggerPlugin::setupDBus()
{
    auto serviceRegistered = [this](const QString& service) {
        if (m_drkonqis.contains(service))
            return;
        // New registration
        const QString name = i18n("KDevelop (%1) - %2", m_displayName, core()->activeSession()->name());
        auto drkonqiProxy = new DBusProxy(service, name, this);
        m_drkonqis.insert(service, drkonqiProxy);
        connect(drkonqiProxy->interface(), SIGNAL(acceptDebuggingApplication(QString)),
                drkonqiProxy, SLOT(debuggerAccepted(QString)));
        connect(drkonqiProxy, &DBusProxy::debugProcess,
                this, &MIDebuggerPlugin::slotDebugExternalProcess);

        drkonqiProxy->interface()->call(QStringLiteral("registerDebuggingApplication"), name, QCoreApplication::applicationPid());
    };
    auto serviceUnregistered = [this](const QString& service) {
        // Deregistration
        if (auto* proxy = m_drkonqis.take(service)) {
            proxy->Invalidate();
            delete proxy;
        }
    };

    m_watcher = new QDBusServiceWatcher(QStringLiteral("org.kde.drkonqi*"), QDBusConnection::sessionBus(),
                                        QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, serviceRegistered);
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, serviceUnregistered);

    auto registeredServiceNames = QDBusConnection::sessionBus().interface()->registeredServiceNames();
    if (!registeredServiceNames.isValid()) {
        return;
    }
    for (const auto &serviceName : registeredServiceNames.value()) {
        if (serviceName.startsWith(QStringLiteral("org.kde.drkonqi."))) {
            serviceRegistered(serviceName);
        }
    }
}

void MIDebuggerPlugin::unload()
{
    // FIXME: this function is not invoked when the GDB plugin or the LLDB plugin
    //        is unloaded, because the overriding functions do not call it.
    qDeleteAll(m_drkonqis.values());
    m_drkonqis.clear();
}

MIDebuggerPlugin::~MIDebuggerPlugin() { }

void MIDebuggerPlugin::slotDebugExternalProcess(DBusProxy* proxy)
{
    QDBusReply<int> reply = proxy->interface()->call(QStringLiteral("pid"));
    if (reply.isValid()) {
        connect(attachProcess(reply.value()), &KJob::result,
                proxy, &DBusProxy::debuggingFinished);
    }

    if (auto* mainWindow = core()->uiController()->activeMainWindow()) {
        mainWindow->raise();
    }
}

MIDebugSession* MIDebuggerPlugin::createSession()
{
    auto* const debugController = core()->debugController();
    auto* const session = createSessionObject();

    // Our tool views are registered only while a debug session created by this plugin is the current session of
    // DebugController. This way, for example, only LLDB (and not GDB) tool views are present during an LLDB session.

    auto toolViewFactoryHolder = reuseOrCreateToolViewFactoryHolder(debugController->currentSession());

    // Register the tool views *before* adding the new session to the DebugController. Otherwise,
    // the tool views are registered after the Debug sublime area becomes active, which usually
    // resets custom tool view positions to the default ones and never restores tool view visibility.
    // Once these tool view issues are fixed in Sublime, consider reordering the next
    // two statements to optimize (the tool view widgets would not have to set up the UI
    // for a null current session before adapting it for the just created session).
    session->initializeToolViewFactoryHolder(std::move(toolViewFactoryHolder));
    debugController->addSession(session);

    connect(session, &MIDebugSession::showMessage, this, &MIDebuggerPlugin::showStatusMessage);
    connect(session, &MIDebugSession::raiseDebuggerConsoleViews, this, &MIDebuggerPlugin::raiseDebuggerConsoleViews);
    connect(session, &MIDebugSession::reset, this, &MIDebuggerPlugin::reset);
    return session;
}

void MIDebuggerPlugin::slotExamineCore()
{
    showStatusMessage(i18n("Choose a core file to examine..."), 1000);

    if (!core()->debugController()->canAddSession(replaceSessionQuestionText())) {
        return;
    }

    const ScopedDialog<SelectCoreDialog> dialog(core()->uiController()->activeMainWindow());
    if (dialog->exec() == QDialog::Rejected) {
        return;
    }

    auto* const runController = core()->runController();
    MIExamineCoreJob::CoreInfo coreInfo{dialog->executableFile(), dialog->core()};
    auto* const job = new MIExamineCoreJob(this, std::move(coreInfo), runController);
    runController->registerJob(job);
    // job->start() is called in registerJob
}

#if HAVE_KSYSGUARD
void MIDebuggerPlugin::slotAttachProcess()
{
    showStatusMessage(i18n("Choose a process to attach to..."), 1000);

    if (!core()->debugController()->canAddSession(replaceSessionQuestionText())) {
        return;
    }

    const auto pid = askUserForProcessId(core()->uiController()->activeMainWindow());
    if (pid == 0) {
        return;
    }

    // TODO: move check into process selection dialog
    if (QApplication::applicationPid() == pid) {
        const QString messageText =
            i18n("Not attaching to process %1: cannot attach the debugger to itself.", pid);
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        ICore::self()->uiController()->postMessage(message);
    }
    else
        attachProcess(pid);
}
#endif

MIAttachProcessJob* MIDebuggerPlugin::attachProcess(int pid)
{
    auto *job = new MIAttachProcessJob(this, pid, core()->runController());
    core()->runController()->registerJob(job);
    // job->start() is called in registerJob

    return job;
}

QString MIDebuggerPlugin::statusName() const
{
    return i18n("Debugger");
}

void MIDebuggerPlugin::showStatusMessage(const QString& msg, int timeout)
{
    emit showMessage(this, msg, timeout);
}

auto MIDebuggerPlugin::reuseOrCreateToolViewFactoryHolder(IDebugSession* previousSession) -> ToolViewFactoryHolderPtr
{
    if (!previousSession) {
        qCDebug(DEBUGGERCOMMON) << "no current session to reuse tool views of";
        return createToolViewFactoryHolder();
    }

    // WARNING: kdevdebuggercommon is a static library, so when the GDB plugin casts an LLDB
    //          session to MIDebugSession* or vice versa, nullptr is the result. This does
    //          not cause issues only as long as both of the following statements are true:
    //          1) the holders of different plugins are incompatible, i.e.
    //             the GDB plugin cannot reuse an LLDB holder and vice versa;
    //          2) the tool view button IDs (tool document specifiers) used by these plugins are nonoverlapping.
    auto* const previousMiSession = qobject_cast<MIDebugSession*>(previousSession);
    if (!previousMiSession) {
        qCDebug(DEBUGGERCOMMON) << "not reusing the tool views of the current non-MI session" << previousSession
                                << "for" << this;
        return createToolViewFactoryHolder();
    }

    if (auto previousHolder = previousMiSession->takeToolViewFactoryHolder()) {
        const auto* const previousPlugin = previousHolder->plugin();
        if (previousPlugin == this) {
            qCDebug(DEBUGGERCOMMON) << "reusing the tool views of the current session of the same plugin" << this;
            return previousHolder;
        }
        // The previous holder is associated with another or already destroyed (null) plugin. Make sure to
        // destroy it at the end of this scope and consequently unregister the old tool views before a new
        // holder is created and registers new tool views. Otherwise, the old tool view buttons with matching
        // IDs (tool document specifiers) would prevent the new buttons from showing up on the button bar.
        qCDebug(DEBUGGERCOMMON) << "not reusing the tool views of the current session, because its plugin"
                                << previousPlugin << "does not match" << this;
    } else {
        qCDebug(DEBUGGERCOMMON) << "the current session does not have tool views to reuse for" << this;
    }

    return createToolViewFactoryHolder();
}

#include "midebuggerplugin.moc"
#include "moc_midebuggerplugin.cpp"
