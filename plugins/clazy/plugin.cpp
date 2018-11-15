/* This file is part of KDevelop

   Copyright 2018 Anton Anikin <anton@anikin.xyz>

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

#include "checksdb.h"
#include "config/globalconfigpage.h"
#include "config/projectconfigpage.h"
#include "debug.h"
#include "globalsettings.h"
#include "problemmodel.h"

#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <language/interfaces/editorcontext.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>
#include <util/jobstatus.h>

#include <KActionCollection>
#include <KMessageBox>
#include <KPluginFactory>

#include <QAction>
#include <QApplication>

K_PLUGIN_FACTORY_WITH_JSON(ClazyFactory, "kdevclazy.json", registerPlugin<Clazy::Plugin>();)

namespace Clazy
{

Plugin::Plugin(QObject* parent, const QVariantList&)
    : IPlugin(QStringLiteral("kdevclazy"), parent)
    , m_job(nullptr)
    , m_project(nullptr)
    , m_model(new ProblemModel(this))
    , m_db(nullptr)
{
    setXMLFile(QStringLiteral("kdevclazy.rc"));

    const QIcon clazyIcon = QIcon::fromTheme(QStringLiteral("clazy"));

    auto runFileAnalysis = [this]() { runClazy(false); };
    auto runProjectAnalysis = [this]() { runClazy(true); };

    m_menuActionFile = new QAction(clazyIcon, i18n("Analyze Current File with Clazy"), this);
    connect(m_menuActionFile, &QAction::triggered, this, runFileAnalysis);
    actionCollection()->addAction(QStringLiteral("clazy_file"), m_menuActionFile);

    m_contextActionFile = new QAction(clazyIcon, i18n("Clazy"), this);
    connect(m_contextActionFile, &QAction::triggered, this, runFileAnalysis);

    m_menuActionProject = new QAction(clazyIcon, i18n("Analyze Current Project with Clazy"), this);
    connect(m_menuActionProject, &QAction::triggered, this, runProjectAnalysis);
    actionCollection()->addAction(QStringLiteral("clazy_project"), m_menuActionProject);

    m_contextActionProject = new QAction(clazyIcon, i18n("Clazy"), this);
    connect(m_contextActionProject, &QAction::triggered, this, runProjectAnalysis);

    m_contextActionProjectItem = new QAction(clazyIcon, i18n("Clazy"), this);

    connect(core()->documentController(), &KDevelop::IDocumentController::documentClosed,
            this, &Plugin::updateActions);

    connect(core()->documentController(), &KDevelop::IDocumentController::documentActivated,
            this, &Plugin::updateActions);

    connect(core()->projectController(), &KDevelop::IProjectController::projectOpened,
            this, &Plugin::updateActions);

    connect(core()->projectController(), &KDevelop::IProjectController::projectClosed,
            this, &Plugin::projectClosed);

    updateActions();
}

Plugin::~Plugin()
{
    killClazy();
}

bool Plugin::isRunning() const
{
    return (m_job != nullptr);
}

void Plugin::killClazy()
{
    if (m_job) {
        m_job->kill(KJob::EmitResult);
    }
}

void Plugin::raiseProblemsView()
{
    m_model->show();
}

void Plugin::raiseOutputView()
{
    core()->uiController()->findToolView(
        i18ndc("kdevstandardoutputview", "@title:window", "Test"),
        nullptr,
        KDevelop::IUiController::FindFlags::Raise);
}

void Plugin::updateActions()
{
    m_project = nullptr;

    m_menuActionFile->setEnabled(false);
    m_menuActionProject->setEnabled(false);

    if (isRunning()) {
        return;
    }

    auto activeDocument = core()->documentController()->activeDocument();
    if (!activeDocument) {
        return;
    }

    m_project = core()->projectController()->findProjectForUrl(activeDocument->url());
    if (!m_project) {
        return;
    }

    if (!m_project->buildSystemManager()) {
        return;
    }

    m_menuActionFile->setEnabled(true);
    m_menuActionProject->setEnabled(true);
}

void Plugin::projectClosed(KDevelop::IProject* project)
{
    if (project != m_model->project()) {
        return;
    }

    killClazy();
    m_model->reset();
}

void Plugin::runClazy(bool checkProject)
{
    auto doc = core()->documentController()->activeDocument();
    Q_ASSERT(doc);

    if (checkProject) {
        runClazy(m_project, m_project->path().toUrl().toLocalFile());
    } else {
        runClazy(m_project, doc->url().toLocalFile());
    }
}

void Plugin::runClazy(KDevelop::IProject* project, const QString& path)
{
    JobParameters params(project, path);
    if (!params.isValid()) {
        const QString errorMessage = i18n("Unable to start Clazy check for '%1':\n\n%2", path, params.error());
        KMessageBox::error(qApp->activeWindow(), errorMessage, i18n("Clazy Error"));
        return;
    }

    m_model->reset(project, path);

    if (m_db.isNull()) {
        reloadDB();
    }

    m_job = new Job(params, m_db);
    connect(m_job, &Job::problemsDetected, m_model, &ProblemModel::addProblems);
    connect(m_job, &Job::finished, this, &Plugin::result);

    core()->uiController()->registerStatus(new KDevelop::JobStatus(m_job, QStringLiteral("clazy")));
    core()->runController()->registerJob(m_job);

    if (GlobalSettings::hideOutputView()) {
        raiseProblemsView();
    } else {
        raiseOutputView();
    }

    updateActions();
}

void Plugin::result(KJob*)
{
    if (!core()->projectController()->projects().contains(m_model->project())) {
        m_model->reset();
    } else {
        m_model->setProblems();

        if (m_job->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded ||
            m_job->status() == KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {

            raiseProblemsView();
        } else {
            raiseOutputView();
        }
    }

    m_job = nullptr; // job automatically deletes itself later

    updateActions();
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    Q_UNUSED(parent);
    KDevelop::ContextMenuExtension extension;

    if (context->hasType(KDevelop::Context::EditorContext) && m_project && m_project->buildSystemManager() && !isRunning()) {
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeFileGroup, m_contextActionFile);
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, m_contextActionProject);
    }

    if (context->hasType(KDevelop::Context::ProjectItemContext) && !isRunning()) {
        auto pContext = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if (pContext->items().size() != 1) {
            return extension;
        }

        const auto items = pContext->items();
        const auto item = items.first();
        switch (item->type()) {
            case KDevelop::ProjectBaseItem::File:
            case KDevelop::ProjectBaseItem::Folder:
            case KDevelop::ProjectBaseItem::BuildFolder:
                break;

            default:
                return extension;
        }
        if (!item->project()->buildSystemManager()) {
            return extension;
        }


        m_contextActionProjectItem->disconnect();
        connect(m_contextActionProjectItem, &QAction::triggered, this, [this, item](){
            runClazy(item->project(), item->path().toLocalFile());
        });

        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeProjectGroup, m_contextActionProjectItem);
    }

    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    if (m_db.isNull()) {
        reloadDB();
    }

    return number ? nullptr : new ProjectConfigPage(this, options.project, parent);
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    return number ? nullptr : new GlobalConfigPage(this, parent);
}

QSharedPointer<const ChecksDB> Plugin::checksDB() const
{
    return m_db;
}

void Plugin::reloadDB()
{
    m_db.reset(new ChecksDB(GlobalSettings::docsPath()));
    connect(GlobalSettings::self(), &GlobalSettings::docsPathChanged, this, &Plugin::reloadDB);
}

}

#include "plugin.moc"
