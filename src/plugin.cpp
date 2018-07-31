/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "plugin.h"
// Qt
#include <QAction>
#include <QMessageBox>
#include <QMimeType>
// KF
#include <KActionCollection>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KPluginFactory>
#include <KProcess>
// KDevPlatform
#include <interfaces/icore.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <execute/iexecuteplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>
#include <shell/problemmodel.h>
#include <shell/problemmodelset.h>
// plugin
#include "config/clangtidypreferences.h"
#include "config/perprojectconfigpage.h"
#include "job.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ClangTidyFactory, "kdevclangtidy.json", registerPlugin<ClangTidy::Plugin>();)

namespace ClangTidy
{
Plugin::Plugin(QObject* parent, const QVariantList& /*unused*/)
    : IPlugin("kdevclangtidy", parent)
    , m_model(new KDevelop::ProblemModel(parent))
{
    setXMLFile("kdevclangtidy.rc");

    QAction* act_checkfile;
    act_checkfile = actionCollection()->addAction("clangtidy_file", this, SLOT(runClangTidyFile()));
    act_checkfile->setStatusTip(i18n("Launches clang-tidy for current file"));
    act_checkfile->setText(i18n("Clang-Tidy (Current File)"));

    /*     TODO: Uncomment this only when discover a safe way to run clang-tidy on
    the whole project.
    //     QAction* act_check_all_files;
    //     act_check_all_files = actionCollection()->addAction ( "clangtidy_all",
    this, SLOT ( runClangTidyAll() ) );
    //     act_check_all_files->setStatusTip ( i18n ( "Launches clangtidy for all
    translation "
    //                                         "units of current project" ) );
    //     act_check_all_files->setText ( i18n ( "clang-tidy (all)" ) );
    */

    IExecutePlugin* iface = KDevelop::ICore::self()
                                ->pluginController()
                                ->pluginForExtension("org.kdevelop.IExecutePlugin")
                                ->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("ClangTidy"), i18n("Clang-Tidy"), m_model.data());

    m_config = KSharedConfig::openConfig()->group("ClangTidy");
    auto clangTidyPath = m_config.readEntry(ConfigGroup::ExecutablePath);
    if (clangTidyPath.isEmpty()) {
        clangTidyPath = QStandardPaths::findExecutable("clang-tidy");
    }

    collectAllAvailableChecks(clangTidyPath);

    m_config.writeEntry(ConfigGroup::AdditionalParameters, QString());
    for (auto check : m_allChecks) {
        bool enable = check.contains("cert") || check.contains("-core.") || check.contains("-cplusplus")
            || check.contains("-deadcode") || check.contains("-security") || check.contains("cppcoreguide");
        if (enable) {
            m_activeChecks << check;
        } else {
            m_activeChecks.removeAll(check);
        }
    }
    m_activeChecks.removeDuplicates();
    m_config.writeEntry(ConfigGroup::EnabledChecks, m_activeChecks.join(','));
}

Plugin::~Plugin() = default;

void Plugin::unload()
{
    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->removeModel(QStringLiteral("ClangTidy"));
}

void Plugin::collectAllAvailableChecks(const QString& clangTidyPath)
{
    m_allChecks.clear();
    KProcess tidy;
    tidy << clangTidyPath << QLatin1String("-checks=*") << QLatin1String("--list-checks");
    tidy.setOutputChannelMode(KProcess::OnlyStdoutChannel);
    tidy.start();

    if (!tidy.waitForStarted()) {
        qCDebug(KDEV_CLANGTIDY) << "Unable to execute clang-tidy.";
        return;
    }

    tidy.closeWriteChannel();
    if (!tidy.waitForFinished()) {
        qCDebug(KDEV_CLANGTIDY) << "Failed during clang-tidy execution.";
        return;
    }

    QTextStream ios(&tidy);
    QString each;
    while (ios.readLineInto(&each)) {
        m_allChecks.append(each.trimmed());
    }
    if (m_allChecks.size() > 3) {
        m_allChecks.removeAt(m_allChecks.length() - 1);
        m_allChecks.removeAt(0);
    }
    m_allChecks.removeDuplicates();
}

void Plugin::runClangTidy(bool allFiles)
{
    auto doc = core()->documentController()->activeDocument();
    if (doc == nullptr) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"),
                              i18n("No suitable active file, unable to deduce project."));
        return;
    }

    runClangTidy(doc->url(), allFiles);
}

void Plugin::runClangTidy(const QUrl& url, bool allFiles)
{
    KDevelop::IProject* project = core()->projectController()->findProjectForUrl(url);
    if (project == nullptr) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"), i18n("Active file isn't in a project"));
        return;
    }

    m_config = project->projectConfiguration()->group("ClangTidy");
    if (!m_config.isValid()) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"),
                              i18n("Can't load parameters. They must be set in the "
                                   "project settings."));
        return;
    }

    ConfigGroup configGroup = KSharedConfig::openConfig()->group("ClangTidy");
    auto clangTidyPath = configGroup.readEntry(ConfigGroup::ExecutablePath);

    Job::Parameters params;

    params.projectRootDir = project->path().toLocalFile();

    if (clangTidyPath.isEmpty()) {
        params.executablePath = QStandardPaths::findExecutable("clang-tidy");
    } else {
        params.executablePath = clangTidyPath;
    }

    if (allFiles) {
        params.filePath = project->path().toUrl().toLocalFile();
    } else {
        params.filePath = url.toLocalFile();
    }
    if (const auto buildSystem = project->buildSystemManager()) {
        params.buildDir = buildSystem->buildDirectory(project->projectItem()).toLocalFile();
    }
    params.additionalParameters = m_config.readEntry(ConfigGroup::AdditionalParameters);
    params.analiseTempDtors = m_config.readEntry(ConfigGroup::AnaliseTempDtors);
    params.enabledChecks = m_activeChecks.join(',');
    params.useConfigFile = m_config.readEntry(ConfigGroup::UseConfigFile);
    params.dumpConfig = m_config.readEntry(ConfigGroup::DumpConfig);
    params.enableChecksProfile = m_config.readEntry(ConfigGroup::EnableChecksProfile);
    params.exportFixes = m_config.readEntry(ConfigGroup::ExportFixes);
    params.extraArgs = m_config.readEntry(ConfigGroup::ExtraArgs);
    params.extraArgsBefore = m_config.readEntry(ConfigGroup::ExtraArgsBefore);
    params.autoFix = m_config.readEntry(ConfigGroup::AutoFix);
    params.headerFilter = m_config.readEntry(ConfigGroup::HeaderFilter);
    params.lineFilter = m_config.readEntry(ConfigGroup::LineFilter);
    params.listChecks = m_config.readEntry(ConfigGroup::ListChecks);
    params.checkSystemHeaders = m_config.readEntry(ConfigGroup::CheckSystemHeaders);

    if (!params.dumpConfig.isEmpty()) {
        auto job = new ClangTidy::Job(params, this);
        core()->runController()->registerJob(job);
        params.dumpConfig = QString();
    }
    auto job2 = new ClangTidy::Job(params, this);
    connect(job2, SIGNAL(finished(KJob*)), this, SLOT(result(KJob*)));
    core()->runController()->registerJob(job2);

    m_runningJob = job2;
}

bool Plugin::isRunning() const
{
    return !m_runningJob.isNull();
}

void Plugin::runClangTidyFile()
{
    bool allFiles = false;
    runClangTidy(allFiles);
}

void Plugin::runClangTidyAll()
{
    bool allFiles = true;
    runClangTidy(allFiles);
}

void Plugin::loadOutput()
{
}

void Plugin::result(KJob* job)
{
    Job* aj = dynamic_cast<Job*>(job);
    if (aj == nullptr) {
        return;
    }

    if (aj->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded) {
        m_model->setProblems(aj->problems());

        core()->uiController()->findToolView(i18nd("kdevproblemreporter", "Problems"), nullptr,
                                             KDevelop::IUiController::FindFlags::Raise);
    }
}

ContextMenuExtension Plugin::contextMenuExtension(Context* context, QWidget* parent)
{
    ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);

    if (context->hasType(KDevelop::Context::EditorContext) && !isRunning()) {
        IDocument* doc = core()->documentController()->activeDocument();
        const auto mime = doc->mimeType().name();
        if (mime == QLatin1String("text/x-c++src") || mime == QLatin1String("text/x-csrc")) {
            auto action = new QAction(QIcon::fromTheme("dialog-ok"), i18n("Clang-Tidy"), parent);
            connect(action, &QAction::triggered, this, &Plugin::runClangTidyFile);
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, action);
        }
    }

    if (context->hasType(KDevelop::Context::ProjectItemContext) && !isRunning()) {
        auto pContext = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if (pContext->items().size() != 1)
            return extension;

        auto item = pContext->items().first();

        switch (item->type()) {
            case KDevelop::ProjectBaseItem::File:
                break;

            default:
                return extension;
        }

        auto action = new QAction(QIcon::fromTheme("dialog-ok"), i18n("Clang-Tidy"), parent);
        connect(action, &QAction::triggered, this, [this, item]() {
            runClangTidy(item->path().toUrl());
        });
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, action);
    }

    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }
    auto config = new PerProjectConfigPage(options.project, parent);
    config->setActiveChecksReceptorList(&m_activeChecks);
    config->setList(m_allChecks);
    return config;
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }
    return new ClangTidyPreferences(this, parent);
}
} // namespace ClangTidy

#include "plugin.moc"
