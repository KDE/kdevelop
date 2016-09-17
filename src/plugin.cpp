/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include <unistd.h>

#include <QAction>
#include <QApplication>
#include <QDomElement>
#include <QFile>
#include <QMessageBox>
#include <QRegExp>
#include <QTreeView>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kprocess.h>

#include <execute/iexecuteplugin.h>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/launchconfigurationtype.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <util/executecompositejob.h>

#include <shell/problemmodel.h>
#include <shell/problemmodelset.h>

#include <language/interfaces/editorcontext.h>

#include "debug.h"
#include "job.h"
#include "plugin.h"

#include <KXMLGUIFactory>

#include "./config/clangtidypreferences.h"
#include "./config/genericconfigpage.h"
#include <project/projectconfigpage.h>

#include <QMessageBox>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ClangtidyFactory, "res/kdevclangtidy.json", registerPlugin<ClangTidy::Plugin>();)
namespace ClangTidy
{
Plugin::Plugin(QObject* parent, const QVariantList&)
    : IPlugin("kdevclangtidy", parent)
    , m_model(new KDevelop::ProblemModel(parent))
{
    qCDebug(KDEV_CLANGTIDY) << "setting clangtidy rc file";
    setXMLFile("kdevclangtidy.rc");

    QAction* act_checkfile;
    act_checkfile = actionCollection()->addAction("clangtidy_file", this, SLOT(runClangtidyFile()));
    act_checkfile->setStatusTip(i18n("Launches Clangtidy for current file"));
    act_checkfile->setText(i18n("clang-tidy"));

    //     TODO: Uncomment this only when discover a safe way to run clang-tidy on the whole project.
    //     QAction* act_check_all_files;
    //     act_check_all_files = actionCollection()->addAction ( "clangtidy_all", this, SLOT ( runClangtidyAll() ) );
    //     act_check_all_files->setStatusTip ( i18n ( "Launches clangtidy for all translation "
    //                                         "units of current project" ) );
    //     act_check_all_files->setText ( i18n ( "clang-tidy (all)" ) );

    IExecutePlugin* iface = KDevelop::ICore::self()
                                ->pluginController()
                                ->pluginForExtension("org.kdevelop.IExecutePlugin")
                                ->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->addModel(QStringLiteral("Clangtidy"), m_model.data());

    ConfigGroup projConf = KSharedConfig::openConfig()->group("Clangtidy");
    auto clangtidyPath = projConf.readEntry(ConfigGroup::ExecutablePath);

    if (clangtidyPath.isEmpty()) {
        clangtidyPath = QString("/usr/bin/clang-tidy");
    }

    collectAllAvailableChecks(clangtidyPath);
    projConf.writeEntry(ConfigGroup::AdditionalParameters, "");
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
    projConf.writeEntry(ConfigGroup::EnabledChecks, m_activeChecks.join(','));
}

void Plugin::unload()
{
    ProblemModelSet* pms = core()->languageController()->problemModelSet();
    pms->removeModel(QStringLiteral("Clangtidy"));
}

Plugin::~Plugin()
{
}

void Plugin::collectAllAvailableChecks(QString clangtidyPath)
{
    m_allChecks.clear();
    KProcess tidy;
    tidy << clangtidyPath << QLatin1String("-checks=*") << QLatin1String("--list-checks");
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

void Plugin::runClangtidy(bool allFiles)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    if (!doc) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"),
                              i18n("No suitable active file, unable to deduce project."));
        return;
    }

    KDevelop::IProject* project = core()->projectController()->findProjectForUrl(doc->url());
    if (!project) {
        QMessageBox::critical(nullptr, i18n("Error starting clang-tidy"), i18n("Active file isn't in a project"));
        return;
    }

    KSharedConfigPtr ptr = project->projectConfiguration();
    ConfigGroup groupConfig = ptr->group("Clangtidy");
    if (!groupConfig.isValid()) {
        QMessageBox::critical(nullptr, i18n("Error starting Clangtidy"),
                              i18n("Can't load parameters. They must be set in the project settings."));
        return;
    }

    auto clangtidyPath = groupConfig.readEntry(ConfigGroup::ExecutablePath);
    auto buildSystem = project->buildSystemManager();
    // todo: evaluate and change this to a dynamic data gotten from clang-tidy executable.
    Job::Parameters params;

    params.projectRootDir = project->path().toLocalFile();

    if (clangtidyPath.isEmpty()) {
        params.executablePath = QStringLiteral("/usr/bin/clang-tidy");
    } else {
        params.executablePath = clangtidyPath;
    }

    if (allFiles) {
        params.filePath = project->path().toUrl().toLocalFile();
    } else {
        params.filePath = doc->url().toLocalFile();
    }
    params.buildDir = buildSystem->buildDirectory(project->projectItem()).toLocalFile();
    params.additionalParameters = groupConfig.readEntry(ConfigGroup::AdditionalParameters);
    params.analiseTempDtors = groupConfig.readEntry(ConfigGroup::AnaliseTempDtors);
    params.enabledChecks = m_activeChecks.join(',');
    params.useConfigFile = groupConfig.readEntry(ConfigGroup::UseConfigFile);
    params.dumpConfig = groupConfig.readEntry(ConfigGroup::DumpConfig);
    params.enableChecksProfile = groupConfig.readEntry(ConfigGroup::EnableChecksProfile);
    params.exportFixes = groupConfig.readEntry(ConfigGroup::ExportFixes);
    params.extraArgs = groupConfig.readEntry(ConfigGroup::ExtraArgs);
    params.extraArgsBefore = groupConfig.readEntry(ConfigGroup::ExtraArgsBefore);
    params.autoFix = groupConfig.readEntry(ConfigGroup::AutoFix);
    params.headerFilter = groupConfig.readEntry(ConfigGroup::HeaderFilter);
    params.lineFilter = groupConfig.readEntry(ConfigGroup::LineFilter);
    params.listChecks = groupConfig.readEntry(ConfigGroup::ListChecks);
    params.checkSystemHeaders = groupConfig.readEntry(ConfigGroup::CheckSystemHeaders);

    if (!params.dumpConfig.isEmpty()) {
        Job* job = new ClangTidy::Job(params, this);
        //         connect ( job, SIGNAL ( finished ( KJob* ) ), this, SLOT ( result ( KJob* ) ) );
        core()->runController()->registerJob(job);
        params.dumpConfig = QString();
    }
    Job* job2 = new ClangTidy::Job(params, this);
    connect(job2, SIGNAL(finished(KJob*)), this, SLOT(result(KJob*)));
    core()->runController()->registerJob(job2);
}

void Plugin::runClangtidyFile()
{
    bool allFiles = false;
    runClangtidy(allFiles);
}

void Plugin::runClangtidyAll()
{
    bool allFiles = true;
    runClangtidy(allFiles);
}

void Plugin::loadOutput()
{
}

void Plugin::result(KJob* job)
{
    Job* aj = dynamic_cast<Job*>(job);
    if (!aj) {
        return;
    }

    if (aj->status() == KDevelop::OutputExecuteJob::JobStatus::JobSucceeded) {
        m_model->setProblems(aj->problems());

        core()->uiController()->findToolView(i18nd("kdevproblemreporter", "Problems"), 0,
                                             KDevelop::IUiController::FindFlags::Raise);
    }
}

KDevelop::ContextMenuExtension Plugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::IDocument* doc = core()->documentController()->activeDocument();
    KDevelop::ContextMenuExtension extension = KDevelop::IPlugin::contextMenuExtension(context);

    if (context->type() == KDevelop::Context::EditorContext) {

        auto mime = doc->mimeType().name();
        if (mime == QLatin1String("text/x-c++src") || mime == QLatin1String("text/x-csrc")) {
            QAction* action
                = new QAction(QIcon::fromTheme("document-new"), i18n("Check current unit with clang-tidy"), this);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(runClangtidyFile()));
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, action);
        }
    }
    return extension;
}

KDevelop::ConfigPage* Plugin::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    } else {
        auto config = new GenericConfigPage(options.project, parent);
        config->setActiveChecksReceptorList(&m_activeChecks);
        config->setList(m_allChecks);
        return config;
    }
}

KDevelop::ConfigPage* Plugin::configPage(int number, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    } else {
        return new ClangtidyPreferences(this, parent);
    }
}
}

#include "plugin.moc"
