/* This file is part of KDevelop
   Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
   Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

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

#include "config/genericconfigpage.h"
#include "config/cppcheckpreferences.h"
#include "debug.h"
#include "job.h"

#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <kactioncollection.h>
#include <kpluginfactory.h>
#include <language/interfaces/editorcontext.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>
#include <shell/problemmodel.h>
#include <shell/problemmodelset.h>
#include <util/jobstatus.h>

#include <QMessageBox>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(CppcheckFactory, "kdevcppcheck.json", registerPlugin<cppcheck::Plugin>();)

namespace cppcheck
{

static const QString modelName = QStringLiteral("Cppcheck");

Plugin::Plugin(QObject* parent, const QVariantList&)
    : IPlugin("kdevcppcheck", parent)
    , m_project(nullptr)
    , m_model(new KDevelop::ProblemModel(this))
{
    qCDebug(KDEV_CPPCHECK) << "setting cppcheck rc file";
    setXMLFile("kdevcppcheck.rc");

    m_model->setFeatures(
        KDevelop::ProblemModel::SeverityFilter |
        KDevelop::ProblemModel::Grouping |
        KDevelop::ProblemModel::CanByPassScopeFilter);

    m_actionFile = new QAction(i18n("Cppcheck (Current File)"), this);
    connect(m_actionFile, &QAction::triggered, [this](){
        runCppcheck(false);
    });
    actionCollection()->addAction("cppcheck_file", m_actionFile);

    m_actionProject = new QAction(i18n("Cppcheck (Current Project)"), this);
    connect(m_actionProject, &QAction::triggered, [this](){
        runCppcheck(true);
    });
    actionCollection()->addAction("cppcheck_project", m_actionProject);

    m_actionProjectItem = new QAction("Cppcheck", this);

    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(modelName, m_model.data());
}

void Plugin::unload()
{
    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->removeModel(modelName);
}

Plugin::~Plugin()
{
}

void Plugin::raiseProblemsView()
{
    core()->languageController()->problemModelSet()->showModel(modelName);
}

void Plugin::raiseOutputView()
{
    core()->uiController()->findToolView(
        i18ndc("kdevstandardoutputview", "@title:window", "Test"),
        nullptr,
        KDevelop::IUiController::FindFlags::Raise);
}

void Plugin::runCppcheck(bool checkProject)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    if (!doc) {
        QMessageBox::critical(nullptr,
                              i18n("Error starting Cppcheck"),
                              i18n("No active file, unable to deduce project."));
        return;
    }

    KDevelop::IProject* project = core()->projectController()->findProjectForUrl(doc->url());
    if (!project) {
        QMessageBox::critical(nullptr,
                              i18n("Error starting Cppcheck"),
                              i18n("Active file isn't in a project"));
        return;
    }

    if (checkProject)
        runCppcheck(project, project->path().toUrl().toLocalFile());
    else
        runCppcheck(project, doc->url().toLocalFile());
}

void Plugin::runCppcheck(KDevelop::IProject* project, const QString& path)
{
    KSharedConfigPtr ptr = project->projectConfiguration();
    KConfigGroup group = ptr->group("Cppcheck");
    if (!group.isValid()) {
        QMessageBox::critical(nullptr,
                              i18n("Error starting Cppcheck"),
                              i18n("Can't load parameters. They must be set in the project settings."));
        return;
    }

    KConfigGroup group2 = KSharedConfig::openConfig()->group("Cppcheck");
    QUrl cppcheckPath = group2.readEntry("Cppcheck Path");

    Job::Parameters params;

    if (cppcheckPath.toLocalFile().isEmpty())
        params.executable = QStringLiteral("/usr/bin/cppcheck");
    else
        params.executable = cppcheckPath.toLocalFile();

    params.path                 = path;
    params.parameters           = group.readEntry("cppcheckParameters", QString(""));
    params.checkStyle           = group.readEntry("AdditionalCheckStyle", false);
    params.checkPerformance     = group.readEntry("AdditionalCheckPerformance", false);
    params.checkPortability     = group.readEntry("AdditionalCheckPortability", false);
    params.checkInformation     = group.readEntry("AdditionalCheckInformation", false);
    params.checkUnusedFunction  = group.readEntry("AdditionalCheckUnusedFunction", false);
    params.checkMissingInclude  = group.readEntry("AdditionalCheckMissingInclude", false);

    m_problems.clear();
    m_project = project;

    Job* job = new cppcheck::Job(params, this);
    connect(job, &Job::problemsDetected, this, &Plugin::problemsDetected);
    connect(job, &Job::finished,         this, &Plugin::result);

    core()->uiController()->registerStatus(new KDevelop::JobStatus(job, "Cppcheck"));
    core()->runController()->registerJob( job );
}

void Plugin::problemsDetected(const QVector<KDevelop::IProblem::Ptr>& problems)
{
    static int maxLength = 0;

    if (m_problems.isEmpty())
        maxLength = 0;

    // Fix problems with incorrect range, which produced by cppcheck's errors
    // without <location> element. In this case location automaticlly gets "/"
    // which entails showing file dialog after selecting such problem in
    // ProblemsView. To avoid this we set project's root path as problem location.
    foreach (auto problem, problems) {
        auto range = problem->finalLocation();
        if (range.document.isEmpty()) {
            range.document = KDevelop::IndexedString(m_project->path().toLocalFile());
            problem->setFinalLocation(range);
        }
    }

    m_problems.append(problems);

    foreach (auto p, problems) {
        m_model->addProblem(p);

        // This performs adjusing of columns width in the ProblemsView.
        // Should be fixed in ProblemsView ?
        if (maxLength < p->description().length()) {
            maxLength = p->description().length();
            m_model->setProblems(m_problems);
        }
    }
}

void Plugin::result(KJob* kjob)
{
    Job* job = dynamic_cast<Job*>(kjob);
    if (!job)
        return;

    m_model->setProblems(m_problems);

    if (job->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded)
        raiseProblemsView();
    else
        raiseOutputView();

    m_project = nullptr;
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);

    if ( context->type() == KDevelop::Context::EditorContext ) {
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeGroup, m_actionFile);
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeGroup, m_actionProject);
    }

    if (context->hasType(KDevelop::Context::ProjectItemContext)) {
        auto pContext = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if (pContext->items().size() != 1)
            return extension;

        auto item = pContext->items().first();

        switch (item->type()) {
            case KDevelop::ProjectBaseItem::File:
            case KDevelop::ProjectBaseItem::Folder:
            case KDevelop::ProjectBaseItem::BuildFolder:
                break;

            default:
                return extension;
        }

        m_actionProjectItem->disconnect();
        connect(m_actionProjectItem, &QAction::triggered, [this, item](){
            runCppcheck(item->project(), item->path().toLocalFile());
        });

        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeGroup, m_actionProjectItem);
    }

    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number)
        return nullptr;
    else
        return new GenericConfigPage(options.project, parent);
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (number)
        return nullptr;
    else
        return new CppCheckPreferences(this, parent);
}

}

#include "plugin.moc"
