/* This file is part of KDevelop
   Copyright 2017 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "plugin.h"

#include "config/globalconfigpage.h"
#include "debug.h"
#include "job.h"
#include "utils.h"
#include "visualizer.h"

#include <config-kdevelop.h>
#if KF5SysGuard_FOUND
#include "dialogs/processselection.h"
#include <QPointer>
#endif

#include <execute/iexecuteplugin.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/launchconfigurationtype.h>
#include <shell/core.h>
#include <shell/launchconfiguration.h>
#include <shell/runcontroller.h>
#include <sublime/message.h>
#include <util/executecompositejob.h>
// KF
#include <KActionCollection>
#include <KPluginFactory>
// Qt
#include <QAction>
#include <QApplication>
#include <QFile>

K_PLUGIN_FACTORY_WITH_JSON(HeaptrackFactory, "kdevheaptrack.json", registerPlugin<Heaptrack::Plugin>();)

namespace Heaptrack
{

Plugin::Plugin(QObject* parent, const QVariantList&)
    : IPlugin(QStringLiteral("kdevheaptrack"), parent)
{
    setXMLFile(QStringLiteral("kdevheaptrack.rc"));

    m_launchAction = new QAction(
        QIcon::fromTheme(QStringLiteral("office-chart-area")),
        i18n("Run Heaptrack Analysis"),
        this);

    connect(m_launchAction, &QAction::triggered, this, &Plugin::launchHeaptrack);
    actionCollection()->addAction(QStringLiteral("heaptrack_launch"), m_launchAction);

#if KF5SysGuard_FOUND
    m_attachAction = new QAction(
        QIcon::fromTheme(QStringLiteral("office-chart-area")),
        i18n("Attach to Process with Heaptrack"),
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
        const QString messageText = i18n("Unable to start Heaptrack analysis - \"%1\" plugin is not loaded.", pluginInfo.name());
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        KDevelop::ICore::self()->uiController()->postMessage(message);
        return;
    }

    auto runController = KDevelop::Core::self()->runControllerInternal();
    auto defaultLaunch = runController->defaultLaunch();
    if (!defaultLaunch) {
        runController->showConfigurationDialog();
    }

    // TODO: catch if still no defaultLaunch
    if (!defaultLaunch->type()->launcherForId(QStringLiteral("nativeAppLauncher"))) {
        const QString messageText = i18n("Heaptrack analysis can be started only for native applications.");
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        KDevelop::ICore::self()->uiController()->postMessage(message);
        return;
    }

    auto heaptrackJob = new Job(defaultLaunch, executePlugin);
    connect(heaptrackJob, &Job::finished, this, &Plugin::jobFinished);

    QList<KJob*> jobList;
    if (KJob* depJob = executePlugin->dependencyJob(defaultLaunch)) {
        jobList += depJob;
    }
    jobList += heaptrackJob;

    auto ecJob = new KDevelop::ExecuteCompositeJob(runController, jobList);
    ecJob->setObjectName(heaptrackJob->statusName());
    runController->registerJob(ecJob);

    m_launchAction->setEnabled(false);
}

void Plugin::attachHeaptrack()
{
#if KF5SysGuard_FOUND
    QPointer<KDevMI::ProcessSelectionDialog> dlg = new KDevMI::ProcessSelectionDialog(activeMainWindow());
    if (!dlg->exec() || !dlg->pidSelected()) {
        delete dlg;
        return;
    }

    auto heaptrackJob = new Job(dlg->pidSelected());
    delete dlg;
    connect(heaptrackJob, &Job::finished, this, &Plugin::jobFinished);

    heaptrackJob->setObjectName(heaptrackJob->statusName());
    core()->runController()->registerJob(heaptrackJob);

    m_launchAction->setEnabled(false);
#endif
}

void Plugin::jobFinished(KJob* kjob)
{
    auto job = static_cast<Job*>(kjob);
    Q_ASSERT(job);

    if (job->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded) {
        auto visualizer = new Visualizer(job->resultsFile(), this);
        visualizer->start();
    } else {
        QFile::remove(job->resultsFile());
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
