/* This file is part of KDevelop
 *  Copyright 2013 Christoph Thielecke <crissi99@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#include <unistd.h>

#include <QAction>
#include <QRegExp>
#include <QFile>
#include <QTreeView>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QDomElement>
#include <QApplication>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <execute/iexecuteplugin.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/launchconfigurationtype.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/ilanguagecontroller.h>
#include <util/executecompositejob.h>

#include <shell/problemmodelset.h>
#include <shell/problemmodel.h>

#include <language/interfaces/editorcontext.h>

#include "debug.h"
#include "plugin.h"
#include "job.h"

#include <KXMLGUIFactory>

#include "./config/genericconfigpage.h"
#include "./config/cppcheckpreferences.h"
#include <project/projectconfigpage.h>

#include <QMessageBox>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(CppcheckFactory, "kdevcppcheck.json", registerPlugin<cppcheck::Plugin>();)

namespace cppcheck
{

Plugin::Plugin(QObject *parent, const QVariantList&)
    : IPlugin("kdevcppcheck", parent)
    , m_model(new KDevelop::ProblemModel(parent))
{
    qCDebug(KDEV_CPPCHECK) << "setting cppcheck rc file";
    setXMLFile("kdevcppcheck.rc");

    QAction* act_checkfile;
    act_checkfile = actionCollection()->addAction("cppcheck_file", this, SLOT(runCppcheckFile()));
    act_checkfile->setStatusTip(i18n("Launches Cppcheck for current file"));
    act_checkfile->setText(i18n("Cppcheck"));

    QAction* act_check_all_files;
    act_check_all_files = actionCollection()->addAction("cppcheck_all", this, SLOT(runCppcheckAll()));
    act_check_all_files->setStatusTip(i18n("Launches Cppcheck for all files"));
    act_check_all_files->setText(i18n("Cppcheck (all Files)"));

    IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IExecutePlugin")->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    ProblemModelSet *pms = core()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("Cppcheck"), m_model.data());
}

void Plugin::unload()
{
    ProblemModelSet *pms = core()->languageController()->problemModelSet();
    pms->removeModel(QStringLiteral("Cppcheck"));
}

Plugin::~Plugin()
{
}

void Plugin::runCppcheck(bool allFiles)
{
    KDevelop::IDocument *doc = core()->documentController()->activeDocument();
    if (!doc) {
        QMessageBox::critical(nullptr,
                              i18n("Error starting Cppcheck"),
                              i18n("No active file, unable to deduce project."));
        return;
    }

    KDevelop::IProject *project = core()->projectController()->findProjectForUrl(doc->url());
    if (!project) {
        QMessageBox::critical(nullptr,
                              i18n("Error starting Cppcheck"),
                              i18n("Active file isn't in a project"));
        return;
    }

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
    params.parameters           = group.readEntry("cppcheckParameters", QString(""));
    params.checkStyle           = group.readEntry("AdditionalCheckStyle", false);
    params.checkPerformance     = group.readEntry("AdditionalCheckPerformance", false);
    params.checkPortability     = group.readEntry("AdditionalCheckPortability", false);
    params.checkInformation     = group.readEntry("AdditionalCheckInformation", false);
    params.checkUnusedFunction  = group.readEntry("AdditionalCheckUnusedFunction", false);
    params.checkMissingInclude  = group.readEntry("AdditionalCheckMissingInclude", false);

    if (cppcheckPath.toLocalFile().isEmpty())
        params.executable = QStringLiteral("/usr/bin/cppcheck");
    else
        params.executable = cppcheckPath.toLocalFile();

    if (allFiles)
        params.path = project->path().toUrl().toLocalFile();
    else
        params.path = doc->url().toLocalFile();

    Job* job = new cppcheck::Job(params, this);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(result(KJob*)));
    core()->runController()->registerJob( job );
}

void Plugin::runCppcheckFile()
{
    bool allFiles = false;
    runCppcheck(allFiles);
}

void Plugin::runCppcheckAll()
{
   bool allFiles = true;
   runCppcheck(allFiles);
}


void Plugin::loadOutput()
{
}

void Plugin::result(KJob *job)
{
    Job *aj = dynamic_cast<Job*>(job);
    if (!aj)
        return;

    if (aj->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded) {
        m_model->setProblems(aj->problems());

        core()->uiController()->findToolView(
            i18nd("kdevproblemreporter", "Problems"),
            0,
            KDevelop::IUiController::FindFlags::Raise);
    }
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);

    if ( context->type() == KDevelop::Context::EditorContext ) {
        QAction* action = new QAction(i18n("Cppcheck (Current File)"), this);
        connect(action, SIGNAL(triggered(bool)), this, SLOT(runCppcheckFile()));
        extension.addAction(KDevelop::ContextMenuExtension::AnalyzeGroup, action);
    }
    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const ProjectConfigOptions &options, QWidget *parent)
{
    if (number != 0)
        return nullptr;
    else
        return new GenericConfigPage(options.project, parent);
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget *parent)
{
    if (number != 0)
        return nullptr;
    else
        return new CppCheckPreferences(this, parent);
}

}
#include "plugin.moc"
