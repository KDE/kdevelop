/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
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

// plugin
#include <clangtidyconfig.h>
#include <clangtidyprojectconfig.h>
#include "config/clangtidypreferences.h"
#include "config/clangtidyprojectconfigpage.h"
#include "job.h"
#include "problemmodel.h"
#include "utils.h"
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
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>
#include <shell/problemmodelset.h>
#include <util/jobstatus.h>
// KF
#include <KActionCollection>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KPluginFactory>
// Qt
#include <QAction>
#include <QMessageBox>
#include <QMimeType>
#include <QMimeDatabase>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ClangTidyFactory, "kdevclangtidy.json", registerPlugin<ClangTidy::Plugin>();)

namespace ClangTidy
{

namespace Strings {
QString modelId() { return QStringLiteral("ClangTidy"); }
}

static
bool isSupportedMimeType(const QMimeType& mimeType)
{
    const QString mime = mimeType.name();
    return (mime == QLatin1String("text/x-c++src") || mime == QLatin1String("text/x-csrc"));
}

Plugin::Plugin(QObject* parent, const QVariantList& /*unused*/)
    : IPlugin(QStringLiteral("kdevclangtidy"), parent)
    , m_model(new ProblemModel(this, this))
{
    setXMLFile(QStringLiteral("kdevclangtidy.rc"));

    m_checkFileAction = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok")),
                                    i18n("Analyze Current File with Clang-Tidy"), this);
    connect(m_checkFileAction, &QAction::triggered, this, &Plugin::runClangTidyFile);
    actionCollection()->addAction(QStringLiteral("clangtidy_file"), m_checkFileAction);

    m_checkProjectAction = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok")),
                                       i18n("Analyze Current Project with Clang-Tidy"), this);
    connect(m_checkProjectAction, &QAction::triggered, this, &Plugin::runClangTidyAll);
    actionCollection()->addAction(QStringLiteral("clangtidy_project"), m_checkProjectAction);

    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(Strings::modelId(), i18n("Clang-Tidy"), m_model.data());

    auto clangTidyPath = KDevelop::Path(ClangTidySettings::clangtidyPath()).toLocalFile();

    // TODO: not only collect on plugin loading, but also on every change in the settings
    // TODO: should also check version on every job start to see if there was an update
    // behind our back while kdevelop running
    // TODO: collect in async job
    m_checkSet.setClangTidyPath(clangTidyPath);

    connect(core()->documentController(), &KDevelop::IDocumentController::documentClosed,
            this, &Plugin::updateActions);
    connect(core()->documentController(), &KDevelop::IDocumentController::documentActivated,
            this, &Plugin::updateActions);

    connect(core()->projectController(), &KDevelop::IProjectController::projectOpened,
            this, &Plugin::updateActions);

    updateActions();
}

Plugin::~Plugin() = default;

void Plugin::unload()
{
    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->removeModel(Strings::modelId());
}


void Plugin::updateActions()
{
    m_checkFileAction->setEnabled(false);
    m_checkProjectAction->setEnabled(false);

    if (isRunning()) {
        return;
    }

    KDevelop::IDocument* activeDocument = core()->documentController()->activeDocument();
    if (!activeDocument) {
        return;
    }

    auto currentProject = core()->projectController()->findProjectForUrl(activeDocument->url());
    if (!currentProject) {
        return;
    }

    if (isSupportedMimeType(activeDocument->mimeType())) {
        m_checkFileAction->setEnabled(true);
    }

    m_checkProjectAction->setEnabled(true);
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

    m_model->reset(project, url, allFiles);

    const auto buildDir = project->buildSystemManager()->buildDirectory(project->projectItem());

    QString error;
    const auto filePaths = Utils::filesFromCompilationDatabase(buildDir, url, allFiles, error);

    ClangTidyProjectSettings projectSettings;
    projectSettings.setSharedConfig(project->projectConfiguration());
    projectSettings.load();

    Job::Parameters params;

    params.projectRootDir = project->path().toLocalFile();

    auto clangTidyPath = KDevelop::Path(ClangTidySettings::clangtidyPath()).toLocalFile();
    params.executablePath = clangTidyPath;

    params.filePaths = filePaths;
    params.buildDir = buildDir.toLocalFile();

    params.additionalParameters = projectSettings.additionalParameters();

    const auto enabledChecks = projectSettings.enabledChecks();
    if (!enabledChecks.isEmpty()) {
        params.enabledChecks = enabledChecks;
    } else {
        // make sure the defaults are up-to-date TODO: make async
        m_checkSet.setClangTidyPath(clangTidyPath);
        params.enabledChecks = m_checkSet.defaults().join(QLatin1Char(','));
    }
    params.useConfigFile = projectSettings.useConfigFile();
    params.headerFilter = projectSettings.headerFilter();
    params.checkSystemHeaders = projectSettings.checkSystemHeaders();

    auto job = new ClangTidy::Job(params, this);
    connect(job, &Job::problemsDetected, m_model.data(), &ProblemModel::addProblems);
    connect(job, &KJob::finished, this, &Plugin::result);
    core()->uiController()->registerStatus(new KDevelop::JobStatus(job, QStringLiteral("clang-tidy")));
    core()->runController()->registerJob(job);

    m_runningJob = job;

    updateActions();
    showModel();
}

void Plugin::showModel()
{
    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->showModel(Strings::modelId());
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

void Plugin::result(KJob* job)
{
    auto* aj = qobject_cast<Job*>(job);
    if (aj == nullptr) {
        return;
    }

    m_model->finishAddProblems();

    if (aj->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded ||
        aj->status() == KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {
        showModel();
    } else {
        core()->uiController()->findToolView(i18ndc("kdevstandardoutputview", "@title:window", "Test"), nullptr,
                                             KDevelop::IUiController::FindFlags::Raise);
    }

    updateActions();
}

ContextMenuExtension Plugin::contextMenuExtension(Context* context, QWidget* parent)
{
    ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context, parent);

    if (context->hasType(KDevelop::Context::EditorContext) && !isRunning()) {
        IDocument* doc = core()->documentController()->activeDocument();
        if (isSupportedMimeType(doc->mimeType())) {
            auto action = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok")), i18n("Clang-Tidy"), parent);
            connect(action, &QAction::triggered, this, &Plugin::runClangTidyFile);
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, action);
        }
        auto action = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok")), i18n("Clang-Tidy"), parent);
        connect(action, &QAction::triggered, this, &Plugin::runClangTidyAll);
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, action);
    }

    if (context->hasType(KDevelop::Context::ProjectItemContext) && !isRunning()) {
        auto pContext = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        const auto items = pContext->items();
        if (items.size() != 1) {
            return extension;
        }

        const auto item = items.first();
        const auto itemType = item->type();
        if ((itemType != KDevelop::ProjectBaseItem::File) &&
            (itemType != KDevelop::ProjectBaseItem::Folder) &&
            (itemType != KDevelop::ProjectBaseItem::BuildFolder)) {
            return extension;
        }
        if (itemType == KDevelop::ProjectBaseItem::File) {
            const QMimeType mimetype = QMimeDatabase().mimeTypeForUrl(item->path().toUrl());
            if (!isSupportedMimeType(mimetype)) {
                return extension;
            }
        }

        auto action = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok")), i18n("Clang-Tidy"), parent);
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

    // ensure checkset is up-to-date TODO: async
    auto clangTidyPath = KDevelop::Path(ClangTidySettings::clangtidyPath()).toLocalFile();
    m_checkSet.setClangTidyPath(clangTidyPath);

    return new ProjectConfigPage(this, options.project, &m_checkSet, parent);
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
