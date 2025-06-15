/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "plugin.h"

#include "config/globalconfigpage.h"
#include "debug.h"
#include "job.h"
#include "utils.h"
#include "visualizer.h"

#include <config-kdevelop.h>
#if HAVE_KSYSGUARD
#include "dialogs/processselection.h"
#include <QPointer>
#endif

#include <execute/iexecuteplugin.h>
#include <execute/iexecutepluginhelpers.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/launchconfigurationtype.h>
#include <shell/core.h>
#include <shell/launchconfiguration.h>
#include <shell/runcontroller.h>
#include <sublime/message.h>
// KF
#include <KActionCollection>
#include <KPluginFactory>
// Qt
#include <QAction>
#include <QApplication>
#include <QFile>

K_PLUGIN_FACTORY_WITH_JSON(HeaptrackFactory, "kdevheaptrack.json", registerPlugin<Heaptrack::Plugin>();)

namespace {
void postErrorMessage(const QString& messageText)
{
    auto* const message = new Sublime::Message(messageText, Sublime::Message::Error);
    KDevelop::ICore::self()->uiController()->postMessage(message);
}
}

namespace Heaptrack
{

Plugin::Plugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("kdevheaptrack"), parent, metaData)
{
    setXMLFile(QStringLiteral("kdevheaptrack.rc"));

    m_launchAction = new QAction(
        QIcon::fromTheme(QStringLiteral("office-chart-area")),
        i18nc("@action", "Run Heaptrack Analysis"),
        this);

    connect(m_launchAction, &QAction::triggered, this, &Plugin::launchHeaptrack);
    actionCollection()->addAction(QStringLiteral("heaptrack_launch"), m_launchAction);

#if HAVE_KSYSGUARD
    m_attachAction = new QAction(
        QIcon::fromTheme(QStringLiteral("office-chart-area")),
        i18nc("@action", "Attach to Process with Heaptrack"),
        this);

    connect(m_attachAction, &QAction::triggered, this, &Plugin::attachHeaptrack);
    actionCollection()->addAction(QStringLiteral("heaptrack_attach"), m_attachAction);
#endif
}

Plugin::~Plugin()
{
}

void Plugin::launchHeaptrack()
{
    IExecutePlugin* executePlugin = nullptr;

    // First we should check that our "kdevexecute" plugin is loaded. This is needed since
    // current plugin controller logic allows us to unload this plugin with keeping dependent
    // plugins like Heaptrack in "loaded" state. This seems to be wrong behaviour but now we have
    // to do additional checks.
    // TODO fix plugin controller to avoid such inconsistent states.
    auto pluginController = core()->pluginController();
    if (auto plugin = pluginController->pluginForExtension(
        QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))) {
        executePlugin = plugin->extension<IExecutePlugin>();
    } else {
        auto pluginInfo = pluginController->infoForPluginId(QStringLiteral("kdevexecute"));
        postErrorMessage(i18n("Unable to start Heaptrack analysis - \"%1\" plugin is not loaded.", pluginInfo.name()));
        return;
    }

    auto runController = KDevelop::Core::self()->runControllerInternal();
    auto defaultLaunch = runController->defaultLaunch();
    if (!defaultLaunch) {
        runController->showConfigurationDialog();
        defaultLaunch = runController->defaultLaunch();
        if (!defaultLaunch) {
            postErrorMessage(i18n("Configure a native application launch to perform Heaptrack analysis on."));
            return;
        }
    }

    if (!defaultLaunch->type()->launcherForId(QStringLiteral("nativeAppLauncher"))) {
        postErrorMessage(i18n("Heaptrack analysis can be started only for native applications."));
        return;
    }

    auto heaptrackJob = new Job(defaultLaunch, executePlugin);
    connect(heaptrackJob, &Job::finished, this, &Plugin::jobFinished);
    runController->registerJob(makeJobWithDependency(heaptrackJob, *executePlugin, defaultLaunch));

    m_launchAction->setEnabled(false);
}

void Plugin::attachHeaptrack()
{
#if HAVE_KSYSGUARD
    const auto pid = KDevMI::askUserForProcessId(activeMainWindow());
    if (pid == 0) {
        return;
    }

    auto heaptrackJob = new Job(pid);
    connect(heaptrackJob, &Job::finished, this, &Plugin::jobFinished);
    core()->runController()->registerJob(heaptrackJob);

    m_launchAction->setEnabled(false);
#endif
}

void Plugin::jobFinished(KJob* kjob)
{
    auto job = static_cast<Job*>(kjob);
    Q_ASSERT(job);
    const auto resultsFile = job->resultsFile();

    if (job->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded) {
        auto* const visualizer = new Visualizer(resultsFile, this);
        visualizer->start();
    } else if (!resultsFile.isEmpty()) {
        QFile::remove(resultsFile);
    }

    m_launchAction->setEnabled(true);
}

int Plugin::configPages() const
{
    return 1;
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (number) {
        return nullptr;
    }

    return new GlobalConfigPage(this, parent);
}

}

#include "plugin.moc"
#include "moc_plugin.cpp"
