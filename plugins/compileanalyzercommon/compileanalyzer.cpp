/*
    SPDX-FileCopyrightText: 2018 Anton Anikin <anton@anikin.xyz>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "compileanalyzer.h"

// lib
#include "compileanalyzeutils.h"
#include "compileanalyzejob.h"
#include "compileanalyzeproblemmodel.h"
// KDevPlatform
#include <interfaces/iplugin.h>
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
// Qt
#include <QAction>
#include <QMessageBox>
#include <QMimeType>
#include <QMimeDatabase>
#include <QThread>

namespace KDevelop
{

bool isSupportedMimeType(const QMimeType& mimeType)
{
    const QString mime = mimeType.name();
    return (mime == QLatin1String("text/x-c++src") || mime == QLatin1String("text/x-csrc"));
}

CompileAnalyzer::CompileAnalyzer(IPlugin* plugin,
                                 const QString& toolName, const QString& toolIconName,
                                 const QString& fileActionId, const QString& allActionId,
                                 const QString& modelId,
                                 ProblemModel::Features modelFeatures,
                                 QObject* parent)
    : QObject(parent)
    , m_core(plugin->core())
    , m_toolName(toolName)
    , m_toolIcon(QIcon::fromTheme(toolIconName))
    , m_modelId(modelId)
    , m_model(new CompileAnalyzeProblemModel(toolName, this))
{
    m_model->setFeatures(modelFeatures);

    ProblemModelSet* problemModelSet = core()->languageController()->problemModelSet();
    problemModelSet->addModel(m_modelId, m_toolName, m_model);

    auto actionCollection = plugin->actionCollection();

    m_checkFileAction = new QAction(m_toolIcon,
                                    i18nc("@action", "Analyze Current File with %1", m_toolName), this);
    connect(m_checkFileAction, &QAction::triggered, this, &CompileAnalyzer::runToolOnFile);
    actionCollection->addAction(fileActionId, m_checkFileAction);

    m_checkProjectAction = new QAction(m_toolIcon,
                                       i18nc("@action", "Analyze Current Project with %1", m_toolName), this);
    connect(m_checkProjectAction, &QAction::triggered, this, &CompileAnalyzer::runToolOnAll);
    actionCollection->addAction(allActionId, m_checkProjectAction);

    connect(core()->documentController(), &KDevelop::IDocumentController::documentClosed,
            this, &CompileAnalyzer::updateActions);
    connect(core()->documentController(), &KDevelop::IDocumentController::documentActivated,
            this, &CompileAnalyzer::updateActions);
    // TODO: updateActions() should be connected to IDocumentController::documentUrlChanged as well. However, this
    // works incorrectly, because IProjectController::findProjectForUrl() returns nullptr for a just-renamed document.
    // The same applies to cppcheck::Plugin::updateActions(). The delayed inclusion of a renamed file into its project
    // negatively affects correctness of other slots connected to the documentUrlChanged signal, such as
    // CurrentProjectSet::setCurrentDocument() and (in case of custom project formatting configuration)
    // SourceFormatterController::updateFormatTextAction(). See also a similar TODO in
    // ProjectManagerView::ProjectManagerView().

    connect(core()->projectController(), &KDevelop::IProjectController::projectOpened,
            this, &CompileAnalyzer::updateActions);
    connect(core()->projectController(), &KDevelop::IProjectController::projectClosed,
            this, &CompileAnalyzer::handleProjectClosed);

    connect(m_model, &CompileAnalyzeProblemModel::rerunRequested,
            this, &CompileAnalyzer::handleRerunRequest);

    updateActions();
}

CompileAnalyzer::~CompileAnalyzer()
{
    killJob();

    ProblemModelSet* problemModelSet = core()->languageController()->problemModelSet();
    problemModelSet->removeModel(m_modelId);
}

bool CompileAnalyzer::isOutputToolViewPreferred() const
{
    return false;
}

ICore* CompileAnalyzer::core() const
{
    return m_core;
}

void CompileAnalyzer::updateActions()
{
    m_checkFileAction->setEnabled(false);
    m_checkProjectAction->setEnabled(false);

    if (isRunning()) {
        return;
    }

    IDocument* activeDocument = core()->documentController()->activeDocument();
    if (!activeDocument) {
        return;
    }

    auto currentProject = core()->projectController()->findProjectForUrl(activeDocument->url());
    if (!currentProject) {
        return;
    }

    if (!currentProject->buildSystemManager()) {
        return;
    }

    if (isSupportedMimeType(activeDocument->mimeType())) {
        m_checkFileAction->setEnabled(true);
    }
    m_checkProjectAction->setEnabled(true);
}

void CompileAnalyzer::handleProjectClosed(IProject* project)
{
    if (project != m_model->project()) {
        return;
    }

    killJob();
    m_model->reset();
}

void CompileAnalyzer::runTool(bool allFiles)
{
    auto doc = core()->documentController()->activeDocument();
    if (doc == nullptr) {
        QMessageBox::critical(nullptr, m_toolName,
                              i18n("No suitable active file, unable to deduce project."));
        return;
    }

    runTool(doc->url(), allFiles);
}

void CompileAnalyzer::runTool(const QUrl& url, bool allFiles)
{
    KDevelop::IProject* project = core()->projectController()->findProjectForUrl(url);
    if (!project) {
        QMessageBox::critical(nullptr, m_toolName,
                              i18n("Active file isn't in a project."));
        return;
    }

    m_model->reset(project, url, allFiles);

    const auto buildDir = project->buildSystemManager()->buildDirectory(project->projectItem());

    QString error;
    const auto filePaths = Utils::filesFromCompilationDatabase(buildDir, url, allFiles, error);

    if (!error.isEmpty()) {
        QMessageBox::critical(nullptr, m_toolName,
                              i18n("Unable to start check for '%1':\n\n%2", url.toLocalFile(), error));
        return;
    }

    m_job = createJob(project, buildDir, url, filePaths);

    connect(m_job, &CompileAnalyzeJob::problemsDetected, m_model, &CompileAnalyzeProblemModel::addProblems);
    connect(m_job, &KJob::finished, this, &CompileAnalyzer::result);

    core()->uiController()->registerStatus(new KDevelop::JobStatus(m_job, m_toolName));
    core()->runController()->registerJob(m_job);

    updateActions();

    if (isOutputToolViewPreferred()) {
        raiseOutputToolView();
    } else {
        raiseProblemsToolView();
    }
}

void CompileAnalyzer::raiseProblemsToolView()
{
    ProblemModelSet* problemModelSet = core()->languageController()->problemModelSet();
    problemModelSet->showModel(m_modelId);
}

void CompileAnalyzer::raiseOutputToolView()
{
    core()->uiController()->raiseToolView(QStringLiteral("org.kdevelop.OutputView.Analyze"));
}

bool CompileAnalyzer::isRunning() const
{
    return (m_job != nullptr);
}

void CompileAnalyzer::killJob()
{
    if (m_job) {
        m_job->kill();
    }
}

void CompileAnalyzer::runToolOnFile()
{
    bool allFiles = false;
    runTool(allFiles);
}

void CompileAnalyzer::runToolOnAll()
{
    bool allFiles = true;
    runTool(allFiles);
}

void CompileAnalyzer::handleRerunRequest(const QUrl& url, bool allFiles)
{
    if (!isRunning()) {
        runTool(url, allFiles);
    }
}

void CompileAnalyzer::result(KJob* job)
{
    Q_UNUSED(job);

    if (!core()->projectController()->projects().contains(m_model->project())) {
        m_model->reset();
    } else {
        const auto status = m_job->status();
        m_model->finishAddProblems(status == OutputExecuteJob::JobStatus::JobSucceeded);

        if (status == OutputExecuteJob::JobStatus::JobSucceeded || status == OutputExecuteJob::JobStatus::JobCanceled) {
            raiseProblemsToolView();
        } else {
            raiseOutputToolView();
        }
    }

    m_job = nullptr; // job automatically deletes itself later

    updateActions();
}

void CompileAnalyzer::fillContextMenuExtension(ContextMenuExtension &extension,
                                               Context* context, QWidget* parent)
{
    if (context->hasType(KDevelop::Context::EditorContext) && !isRunning()) {
        IDocument* doc = core()->documentController()->activeDocument();

        auto project = core()->projectController()->findProjectForUrl(doc->url());
        if (!project || !project->buildSystemManager()) {
            return;
        }
        if (isSupportedMimeType(doc->mimeType())) {
            auto action = new QAction(m_toolIcon, m_toolName, parent);
            connect(action, &QAction::triggered, this, &CompileAnalyzer::runToolOnFile);
            extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, action);
        }
        auto action = new QAction(m_toolIcon, m_toolName, parent);
        connect(action, &QAction::triggered, this, &CompileAnalyzer::runToolOnAll);
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, action);
    }

    if (context->hasType(KDevelop::Context::ProjectItemContext) && !isRunning()) {
        auto projectItemContext = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        const auto items = projectItemContext->items();
        if (items.size() != 1) {
            return;
        }

        const auto item = items.first();
        const auto itemType = item->type();
        if ((itemType != KDevelop::ProjectBaseItem::File) &&
            (itemType != KDevelop::ProjectBaseItem::Folder) &&
            (itemType != KDevelop::ProjectBaseItem::BuildFolder)) {
            return;
        }
        if (itemType == KDevelop::ProjectBaseItem::File) {
            const QMimeType mimetype = QMimeDatabase().mimeTypeForUrl(item->path().toUrl());
            if (!isSupportedMimeType(mimetype)) {
                return;
            }
        }
        if (!item->project()->buildSystemManager()) {
            return;
        }

        auto action = new QAction(m_toolIcon, m_toolName, parent);
        connect(action, &QAction::triggered, this, [this, item]() {
            runTool(item->path().toUrl());
        });
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, action);
    }
}

}

#include "moc_compileanalyzer.cpp"
