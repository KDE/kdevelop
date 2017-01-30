
/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Andreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "projectcontroller.h"

#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QTemporaryFile>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KConfigGroup>
#include <KIO/FileCopyJob>
#include <KIO/ListJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPassivePopup>
#include <KRecentFilesAction>
#include <KSharedConfig>
#include <KStandardAction>

#include <sublime/area.h>
#include <interfaces/iplugin.h>
#include <interfaces/isession.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iselectioncontroller.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/projectchangesmodel.h>
#include <project/projectmodel.h>
#include <project/projectbuildsetmodel.h>
#include <projectconfigpage.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parseprojectjob.h>
#include <interfaces/iruncontroller.h>
#include <vcs/widgets/vcsdiffpatchsources.h>
#include <vcs/widgets/vcscommitdialog.h>

#include "core.h"
// TODO: Should get rid off this include (should depend on IProject only)
#include "project.h"
#include "mainwindow.h"
#include "shellextension.h"
#include "plugincontroller.h"
#include "configdialog.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "openprojectdialog.h"
#include "sessioncontroller.h"
#include "session.h"
#include "debug.h"

namespace KDevelop
{

class ProjectControllerPrivate
{
public:
    QList<IProject*> m_projects;
    QMap< IProject*, QList<IPlugin*> > m_projectPlugins;
    QPointer<KRecentFilesAction> m_recentAction;
    Core* m_core;
//     IProject* m_currentProject;
    ProjectModel* model;
    QPointer<QAction> m_openProject;
    QPointer<QAction> m_fetchProject;
    QPointer<QAction> m_closeProject;
    QPointer<QAction> m_openConfig;
    IProjectDialogProvider* dialog;
    QList<QUrl> m_currentlyOpening; // project-file urls that are being opened
    ProjectController* q;
    ProjectBuildSetModel* buildset;
    bool m_foundProjectFile; //Temporary flag used while searching the hierarchy for a project file
    bool m_cleaningUp; //Temporary flag enabled while destroying the project-controller
    QPointer<ProjectChangesModel> m_changesModel;
    QHash< IProject*, QPointer<KJob> > m_parseJobs; // parse jobs that add files from the project to the background parser.

    explicit ProjectControllerPrivate( ProjectController* p )
        : m_core(nullptr), model(nullptr), dialog(nullptr), q(p), buildset(nullptr), m_foundProjectFile(false), m_cleaningUp(false)
    {
    }

    void unloadAllProjectPlugins()
    {
        if( m_projects.isEmpty() )
            m_core->pluginControllerInternal()->unloadProjectPlugins();
    }

    void projectConfig( QObject * obj )
    {
        if( !obj )
            return;
        Project* proj = qobject_cast<Project*>(obj);
        if( !proj )
            return;

        QVector<KDevelop::ConfigPage*> configPages;
        auto mainWindow = m_core->uiController()->activeMainWindow();

        ProjectConfigOptions options;
        options.developerFile = proj->developerFile();
        options.developerTempFile = proj->developerTempFile();
        options.projectTempFile = proj->projectTempFile();
        options.project = proj;

        foreach (IPlugin* plugin, findPluginsForProject(proj)) {
            for (int i = 0; i < plugin->perProjectConfigPages(); ++i) {
                configPages.append(plugin->perProjectConfigPage(i, options, mainWindow));
            }
        }

        std::sort(configPages.begin(), configPages.end(),
                  [](const ConfigPage* a, const ConfigPage* b) {
            return a->name() < b->name();
        });

        auto cfgDlg = new KDevelop::ConfigDialog(configPages, mainWindow);
        cfgDlg->setAttribute(Qt::WA_DeleteOnClose);
        cfgDlg->setModal(true);
        QObject::connect(cfgDlg, &ConfigDialog::configSaved, cfgDlg, [this, proj](ConfigPage* page) {
            Q_UNUSED(page)
            Q_ASSERT_X(proj, Q_FUNC_INFO,
                    "ConfigDialog signalled project config change, but no project set for configuring!");
            emit q->projectConfigurationChanged(proj);
        });
        cfgDlg->setWindowTitle(i18n("Configure Project %1", proj->name()));
        QObject::connect(cfgDlg, &KDevelop::ConfigDialog::finished, [this, proj]() {
            proj->projectConfiguration()->sync();
        });
        cfgDlg->show();
    }

    void saveListOfOpenedProjects()
    {
        auto activeSession = Core::self()->activeSession();
        if (!activeSession) {
            return;
        }

        QList<QUrl> openProjects;
        openProjects.reserve( m_projects.size() );

        foreach( IProject* project, m_projects ) {
            openProjects.append(project->projectFile().toUrl());
        }

        activeSession->setContainedProjects( openProjects );
    }

    // Recursively collects builder dependencies for a project.
    static void collectBuilders( QList< IProjectBuilder* >& destination, IProjectBuilder* topBuilder, IProject* project )
    {
        QList< IProjectBuilder* > auxBuilders = topBuilder->additionalBuilderPlugins( project );
        destination.append( auxBuilders );
        foreach( IProjectBuilder* auxBuilder, auxBuilders ) {
            collectBuilders( destination, auxBuilder, project );
        }
    }

    QVector<IPlugin*> findPluginsForProject( IProject* project )
    {
        QList<IPlugin*> plugins = m_core->pluginController()->loadedPlugins();
        QVector<IPlugin*> projectPlugins;
        QList<IProjectBuilder*> buildersForKcm;
        // Important to also include the "top" builder for the project, so
        // projects with only one such builder are kept working. Otherwise the project config
        // dialog is empty for such cases.
        if( IBuildSystemManager* buildSystemManager = project->buildSystemManager() ) {
            buildersForKcm << buildSystemManager->builder();
            collectBuilders( buildersForKcm, buildSystemManager->builder(), project );
        }

        foreach(auto plugin, plugins) {
            auto info = m_core->pluginController()->pluginInfo(plugin);
            IProjectFileManager* manager = plugin->extension<KDevelop::IProjectFileManager>();
            if( manager && manager != project->projectFileManager() )
            {
                // current plugin is a manager but does not apply to given project, skip
                continue;
            }
            IProjectBuilder* builder = plugin->extension<KDevelop::IProjectBuilder>();
            if ( builder && !buildersForKcm.contains( builder ) )
            {
                continue;
            }
            qCDebug(SHELL) << "Using plugin" << info.pluginId() << "for project" << project->name();
            projectPlugins << plugin;
        }

        return projectPlugins;
    }

    void updateActionStates()
    {
        // if only one project loaded, this is always our target
        int itemCount = (m_projects.size() == 1) ? 1 : 0;

        if (itemCount == 0) {
            // otherwise base on selection
            ProjectItemContext* itemContext = dynamic_cast<ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
            if (itemContext) {
                itemCount = itemContext->items().count();
            }
        }

        m_openConfig->setEnabled(itemCount == 1);
        m_closeProject->setEnabled(itemCount > 0);
    }

    void openProjectConfig()
    {
        // if only one project loaded, this is our target
        IProject *project = (m_projects.count() == 1) ? m_projects.at(0) : nullptr;

        // otherwise base on selection
        if (!project) {
            ProjectItemContext* ctx = dynamic_cast<ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
            if (ctx && ctx->items().count() == 1) {
                project = ctx->items().at(0)->project();
            }
        }

        if (project) {
            q->configureProject(project);
        }
    }

    void closeSelectedProjects()
    {
        QSet<IProject*> projects;

        // if only one project loaded, this is our target
        if (m_projects.count() == 1) {
            projects.insert(m_projects.at(0));
        } else {
            // otherwise base on selection
            ProjectItemContext* ctx =  dynamic_cast<ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
            if (ctx) {
                foreach (ProjectBaseItem* item, ctx->items()) {
                    projects.insert(item->project());
                }
            }
        }

        foreach (IProject* project, projects) {
            q->closeProject(project);
        }
    }

    void importProject(const QUrl& url_)
    {
        QUrl url(url_);
        if (url.isLocalFile()) {
            const QString path = QFileInfo(url.toLocalFile()).canonicalFilePath();
            if (!path.isEmpty()) {
                url = QUrl::fromLocalFile(path);
            }
        }

        if ( !url.isValid() )
        {
            KMessageBox::error(Core::self()->uiControllerInternal()->activeMainWindow(),
                            i18n("Invalid Location: %1", url.toDisplayString(QUrl::PreferLocalFile)));
            return;
        }

        if ( m_currentlyOpening.contains(url))
        {
            qCDebug(SHELL) << "Already opening " << url << ". Aborting.";
            KPassivePopup::message( i18n( "Project already being opened"),
                                    i18n( "Already opening %1, not opening again",
                                        url.toDisplayString(QUrl::PreferLocalFile) ),
                                    m_core->uiController()->activeMainWindow() );
            return;
        }

        foreach( IProject* project, m_projects )
        {
            if ( url == project->projectFile().toUrl() )
            {
                if ( dialog->userWantsReopen() )
                { // close first, then open again by falling through
                    q->closeProject(project);
                } else { // abort
                    return;
                }
            }
        }

        m_currentlyOpening += url;

        m_core->pluginControllerInternal()->loadProjectPlugins();

        Project* project = new Project();
        QObject::connect(project, &Project::aboutToOpen,
                         q, &ProjectController::projectAboutToBeOpened);
        if ( !project->open( Path(url) ) )
        {
            m_currentlyOpening.removeAll(url);
            q->abortOpeningProject(project);
            project->deleteLater();
        }
    }

    void areaChanged(Sublime::Area* area) {
        KActionCollection* ac = m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();
        ac->action(QStringLiteral("commit_current_project"))->setEnabled(area->objectName() == QLatin1String("code"));
        ac->action(QStringLiteral("commit_current_project"))->setVisible(area->objectName() == QLatin1String("code"));
    }
};

IProjectDialogProvider::IProjectDialogProvider()
{}

IProjectDialogProvider::~IProjectDialogProvider()
{}

ProjectDialogProvider::ProjectDialogProvider(ProjectControllerPrivate* const p) : d(p)
{}

ProjectDialogProvider::~ProjectDialogProvider()
{}

bool writeNewProjectFile( const QString& localConfigFile, const QString& name, const QString& createdFrom, const QString& manager )
{
    KSharedConfigPtr cfg = KSharedConfig::openConfig( localConfigFile, KConfig::SimpleConfig );
    if (!cfg->isConfigWritable(true)) {
        qCDebug(SHELL) << "can't write to configfile";
        return false;
    }
    KConfigGroup grp = cfg->group( "Project" );
    grp.writeEntry( "Name", name );
    grp.writeEntry( "CreatedFrom", createdFrom );
    grp.writeEntry( "Manager", manager );
    cfg->sync();
    return true;
}

bool writeProjectSettingsToConfigFile(const QUrl& projectFileUrl, OpenProjectDialog* dlg)
{
    if ( !projectFileUrl.isLocalFile() ) {
        QTemporaryFile tmp;
        if ( !tmp.open() ) {
            return false;
        }
        if ( !writeNewProjectFile( tmp.fileName(), dlg->projectName(), dlg->selectedUrl().fileName(), dlg->projectManager() ) ) {
            return false;
        }
        // explicitly close file before uploading it, see also: https://bugs.kde.org/show_bug.cgi?id=254519
        tmp.close();

        auto uploadJob = KIO::file_copy(QUrl::fromLocalFile(tmp.fileName()), projectFileUrl);
        KJobWidgets::setWindow(uploadJob, Core::self()->uiControllerInternal()->defaultMainWindow());
        return uploadJob->exec();
    }
    // Here and above we take .filename() part of the selectedUrl() to make it relative to the project root,
    // thus keeping .kdev file relocatable
    return writeNewProjectFile( projectFileUrl.toLocalFile(), dlg->projectName(), dlg->selectedUrl().fileName(), dlg->projectManager() );
}


bool projectFileExists( const QUrl& u )
{
    if( u.isLocalFile() )
    {
        return QFileInfo::exists( u.toLocalFile() );
    } else
    {
        auto statJob = KIO::stat(u, KIO::StatJob::DestinationSide, 0, KIO::HideProgressInfo);
        KJobWidgets::setWindow(statJob, Core::self()->uiControllerInternal()->activeMainWindow());
        return statJob->exec();
    }
}

bool equalProjectFile( const QString& configPath, OpenProjectDialog* dlg )
{
    KSharedConfigPtr cfg = KSharedConfig::openConfig( configPath, KConfig::SimpleConfig );
    KConfigGroup grp = cfg->group( "Project" );
    QString defaultName = dlg->projectFileUrl().adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).fileName();
    return (grp.readEntry( "Name", QString() ) == dlg->projectName() || dlg->projectName() == defaultName) &&
           grp.readEntry( "Manager", QString() ) == dlg->projectManager();
}

QUrl ProjectDialogProvider::askProjectConfigLocation(bool fetch, const QUrl& startUrl,
                                                     const QUrl& repoUrl, IPlugin* vcsOrProviderPlugin)
{
    Q_ASSERT(d);
    OpenProjectDialog dlg(fetch, startUrl, repoUrl, vcsOrProviderPlugin, Core::self()->uiController()->activeMainWindow());
    if(dlg.exec() == QDialog::Rejected)
        return QUrl();

    QUrl projectFileUrl = dlg.projectFileUrl();
    qCDebug(SHELL) << "selected project:" << projectFileUrl << dlg.projectName() << dlg.projectManager();
    if ( dlg.projectManager() == "<built-in>" ) {
        return projectFileUrl;
    }

    // controls if existing project file should be saved
    bool writeProjectConfigToFile = true;
    if( projectFileExists( projectFileUrl ) )
    {
        // check whether config is equal
        bool shouldAsk = true;
        if( projectFileUrl == dlg.selectedUrl() )
        {
            if( projectFileUrl.isLocalFile() )
            {
                shouldAsk = !equalProjectFile( projectFileUrl.toLocalFile(), &dlg );
            } else {
                shouldAsk = false;

                QTemporaryFile tmpFile;
                if (tmpFile.open()) {
                    auto downloadJob = KIO::file_copy(projectFileUrl, QUrl::fromLocalFile(tmpFile.fileName()));
                    KJobWidgets::setWindow(downloadJob, qApp->activeWindow());
                    if (downloadJob->exec()) {
                        shouldAsk = !equalProjectFile(tmpFile.fileName(), &dlg);
                    }
                }
            }
        }

        if ( shouldAsk )
        {
            KGuiItem yes = KStandardGuiItem::yes();
            yes.setText(i18n("Override"));
            yes.setToolTip(i18nc("@info:tooltip", "Continue to open the project and use the just provided project configuration."));
            yes.setIcon(QIcon());
            KGuiItem no = KStandardGuiItem::no();
            no.setText(i18n("Open Existing File"));
            no.setToolTip(i18nc("@info:tooltip", "Continue to open the project but use the existing project configuration."));
            no.setIcon(QIcon());
            KGuiItem cancel = KStandardGuiItem::cancel();
            cancel.setToolTip(i18nc("@info:tooltip", "Cancel and do not open the project."));
            int ret = KMessageBox::questionYesNoCancel(qApp->activeWindow(),
                i18n("There already exists a project configuration file at %1.\n"
                     "Do you want to override it or open the existing file?", projectFileUrl.toDisplayString(QUrl::PreferLocalFile)),
                i18n("Override existing project configuration"), yes, no, cancel );
            if ( ret == KMessageBox::No )
            {
                writeProjectConfigToFile = false;
            } else if ( ret == KMessageBox::Cancel )
            {
                return QUrl();
            } // else fall through and write new file
        } else {
            writeProjectConfigToFile = false;
        }
    }

    if (writeProjectConfigToFile) {
        if (!writeProjectSettingsToConfigFile(projectFileUrl, &dlg)) {
            KMessageBox::error(d->m_core->uiControllerInternal()->defaultMainWindow(),
                i18n("Unable to create configuration file %1", projectFileUrl.url()));
            return QUrl();
        }
    }
    return projectFileUrl;
}

bool ProjectDialogProvider::userWantsReopen()
{
    Q_ASSERT(d);
    return (KMessageBox::questionYesNo( d->m_core->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Reopen the current project?" ) )
                == KMessageBox::No) ? false : true;
}

void ProjectController::setDialogProvider(IProjectDialogProvider* dialog)
{
    Q_ASSERT(d->dialog);
    delete d->dialog;
    d->dialog = dialog;
}

ProjectController::ProjectController( Core* core )
        : IProjectController( core ), d( new ProjectControllerPrivate( this ) )
{
    qRegisterMetaType<QList<QUrl>>();

    setObjectName(QStringLiteral("ProjectController"));

    d->m_core = core;
    d->model = new ProjectModel();

    //NOTE: this is required to be called here, such that the
    //      actions are available when the UI controller gets
    //      initialized *before* the project controller
    if (Core::self()->setupFlags() != Core::NoUi) {
        setupActions();
    }
}

void ProjectController::setupActions()
{
    KActionCollection * ac =
        d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    QAction*action;

    d->m_openProject = action = ac->addAction( QStringLiteral("project_open") );
    action->setText(i18nc( "@action", "Open / Import Project..." ) );
    action->setToolTip( i18nc( "@info:tooltip", "Open or import project" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Open an existing KDevelop 4 project or import "
                                                    "an existing Project into KDevelop 4. This entry "
                                                    "allows one to select a KDevelop4 project file "
                                                    "or an existing directory to open it in KDevelop. "
                                                    "When opening an existing directory that does "
                                                    "not yet have a KDevelop4 project file, the file "
                                                    "will be created." ) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("project-open")));
    connect(action, &QAction::triggered, this, [&] { openProject(); });

    d->m_fetchProject = action = ac->addAction( QStringLiteral("project_fetch") );
    action->setText(i18nc( "@action", "Fetch Project..." ) );
    action->setIcon( QIcon::fromTheme( QStringLiteral("edit-download") ) );
    action->setToolTip( i18nc( "@info:tooltip", "Fetch project" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Guides the user through the project fetch "
                                                    "and then imports it into KDevelop 4." ) );
//     action->setIcon(QIcon::fromTheme("project-open"));
    connect( action, &QAction::triggered, this, &ProjectController::fetchProject );

//    action = ac->addAction( "project_close" );
//    action->setText( i18n( "C&lose Project" ) );
//    connect( action, SIGNAL(triggered(bool)), SLOT(closeProject()) );
//    action->setToolTip( i18n( "Close project" ) );
//    action->setWhatsThis( i18n( "Closes the current project." ) );
//    action->setEnabled( false );

    d->m_closeProject = action = ac->addAction( QStringLiteral("project_close") );
    connect( action, &QAction::triggered, this, [&] { d->closeSelectedProjects(); } );
    action->setText( i18nc( "@action", "Close Project(s)" ) );
    action->setIcon( QIcon::fromTheme( QStringLiteral("project-development-close") ) );
    action->setToolTip( i18nc( "@info:tooltip", "Closes all currently selected projects" ) );
    action->setEnabled( false );

    d->m_openConfig = action = ac->addAction( QStringLiteral("project_open_config") );
    connect( action, &QAction::triggered, this, [&] { d->openProjectConfig(); } );
    action->setText( i18n( "Open Configuration..." ) );
    action->setIcon( QIcon::fromTheme(QStringLiteral("configure")) );
    action->setEnabled( false );

    action = ac->addAction( QStringLiteral("commit_current_project") );
    connect( action, &QAction::triggered, this, &ProjectController::commitCurrentProject );
    action->setText( i18n( "Commit Current Project..." ) );
    action->setIconText( i18n( "Commit..." ) );
    action->setIcon( QIcon::fromTheme(QStringLiteral("svn-commit")) );
    connect(d->m_core->uiControllerInternal()->defaultMainWindow(), &MainWindow::areaChanged,
            this, [&] (Sublime::Area* area) { d->areaChanged(area); });
    d->m_core->uiControllerInternal()->area(0, QStringLiteral("code"))->addAction(action);

    KSharedConfig * config = KSharedConfig::openConfig().data();
//     KConfigGroup group = config->group( "General Options" );

    d->m_recentAction = KStandardAction::openRecent(this, SLOT(openProject(QUrl)), this);
    ac->addAction( QStringLiteral("project_open_recent"), d->m_recentAction );
    d->m_recentAction->setText( i18n( "Open Recent Project" ) );
    d->m_recentAction->setWhatsThis( i18nc( "@info:whatsthis", "Opens recently opened project." ) );
    d->m_recentAction->loadEntries( KConfigGroup(config, "RecentProjects") );

    QAction* openProjectForFileAction = new QAction( this );
    ac->addAction(QStringLiteral("project_open_for_file"), openProjectForFileAction);
    openProjectForFileAction->setText(i18n("Open Project for Current File"));
    connect( openProjectForFileAction, &QAction::triggered, this, &ProjectController::openProjectForUrlSlot);
}

ProjectController::~ProjectController()
{
    delete d->model;
    delete d->dialog;
    delete d;
}

void ProjectController::cleanup()
{
    if ( d->m_currentlyOpening.isEmpty() ) {
        d->saveListOfOpenedProjects();
    }

    d->m_cleaningUp = true;
    if( buildSetModel() ) {
        buildSetModel()->storeToSession( Core::self()->activeSession() );
    }

    closeAllProjects();
}

void ProjectController::initialize()
{
    d->buildset = new ProjectBuildSetModel( this );
    buildSetModel()->loadFromSession( Core::self()->activeSession() );
    connect( this, &ProjectController::projectOpened,
             d->buildset, &ProjectBuildSetModel::loadFromProject );
    connect( this, &ProjectController::projectClosing,
             d->buildset, &ProjectBuildSetModel::saveToProject );
    connect( this, &ProjectController::projectClosed,
             d->buildset, &ProjectBuildSetModel::projectClosed );

    loadSettings(false);
    d->dialog = new ProjectDialogProvider(d);

    QDBusConnection::sessionBus().registerObject( QStringLiteral("/org/kdevelop/ProjectController"),
        this, QDBusConnection::ExportScriptableSlots );

    KSharedConfigPtr config = Core::self()->activeSession()->config();
    KConfigGroup group = config->group( "General Options" );
    QList<QUrl> openProjects = group.readEntry( "Open Projects", QList<QUrl>() );

    QMetaObject::invokeMethod(this, "openProjects", Qt::QueuedConnection, Q_ARG(QList<QUrl>, openProjects));

    connect( Core::self()->selectionController(), &ISelectionController::selectionChanged,
             this, [&] () { d->updateActionStates(); } );
    connect(this, &ProjectController::projectOpened,
            this, [&] () { d->updateActionStates(); });
    connect(this, &ProjectController::projectClosing,
            this, [&] () { d->updateActionStates(); });
}

void ProjectController::openProjects(const QList<QUrl>& projects)
{
    foreach (const QUrl& url, projects)
        openProject(url);
}

void ProjectController::loadSettings( bool projectIsLoaded )
{
    Q_UNUSED(projectIsLoaded)
}

void ProjectController::saveSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
}


int ProjectController::projectCount() const
{
    return d->m_projects.count();
}

IProject* ProjectController::projectAt( int num ) const
{
    if( !d->m_projects.isEmpty() && num >= 0 && num < d->m_projects.count() )
        return d->m_projects.at( num );
    return nullptr;
}

QList<IProject*> ProjectController::projects() const
{
    return d->m_projects;
}

void ProjectController::eventuallyOpenProjectFile(KIO::Job* _job, KIO::UDSEntryList entries ) {
    KIO::SimpleJob* job(dynamic_cast<KIO::SimpleJob*>(_job));
    Q_ASSERT(job);
    foreach(const KIO::UDSEntry& entry, entries) {
        if(d->m_foundProjectFile)
            break;
        if(!entry.isDir()) {
            QString name = entry.stringValue( KIO::UDSEntry::UDS_NAME );

            if(name.endsWith(QLatin1String(".kdev4"))) {
                //We have found a project-file, open it
                openProject(Path(Path(job->url()), name).toUrl());
                d->m_foundProjectFile = true;
            }
        }
    }
}

void ProjectController::openProjectForUrlSlot(bool) {
    if(ICore::self()->documentController()->activeDocument()) {
        QUrl url = ICore::self()->documentController()->activeDocument()->url();
        IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
        if(!project) {
            openProjectForUrl(url);
        }else{
            KMessageBox::error(Core::self()->uiController()->activeMainWindow(), i18n("Project already open: %1", project->name()));
        }
    }else{
        KMessageBox::error(Core::self()->uiController()->activeMainWindow(), i18n("No active document"));
    }
}


void ProjectController::openProjectForUrl(const QUrl& sourceUrl) {
    Q_ASSERT(!sourceUrl.isRelative());
    QUrl dirUrl = sourceUrl.adjusted(QUrl::RemoveFilename);
    QUrl testAt = dirUrl;

    d->m_foundProjectFile = false;

    while(!testAt.path().isEmpty()) {
        KIO::ListJob* job = KIO::listDir(testAt);

        connect(job, &KIO::ListJob::entries, this, &ProjectController::eventuallyOpenProjectFile);
        KJobWidgets::setWindow(job, ICore::self()->uiController()->activeMainWindow());
        job->exec();
        if(d->m_foundProjectFile) {
            //Fine! We have directly opened the project
            d->m_foundProjectFile = false;
            return;
        }
        QUrl oldTest = testAt.adjusted(QUrl::RemoveFilename);
        if(oldTest == testAt)
            break;
    }

    QUrl askForOpen = d->dialog->askProjectConfigLocation(false, dirUrl);

    if(askForOpen.isValid())
        openProject(askForOpen);
}

void ProjectController::openProject( const QUrl &projectFile )
{
    QUrl url = projectFile;

    if ( url.isEmpty() ) {
        url = d->dialog->askProjectConfigLocation(false);
        if ( url.isEmpty() ) {
            return;
        }
    }

    Q_ASSERT(!url.isRelative());

    QList<const Session*> existingSessions;
    if(!Core::self()->sessionController()->activeSession()->containedProjects().contains(url))
    {
        foreach( const Session* session, Core::self()->sessionController()->sessions())
        {
            if(session->containedProjects().contains(url))
            {
                existingSessions << session;
#if 0
                    ///@todo Think about this! Problem: The session might already contain files, the debugger might be active, etc.
                    //If this session is empty, close it
                    if(Core::self()->sessionController()->activeSession()->description().isEmpty())
                    {
                        //Terminate this instance of kdevelop if the user agrees
                        foreach(Sublime::MainWindow* window, Core::self()->uiController()->controller()->mainWindows())
                            window->close();
                    }
#endif
            }
        }
    }

    if ( ! existingSessions.isEmpty() ) {
        QDialog dialog(Core::self()->uiControllerInternal()->activeMainWindow());
        dialog.setWindowTitle(i18n("Project Already Open"));

        auto mainLayout = new QVBoxLayout(&dialog);
        mainLayout->addWidget(new QLabel(i18n("The project you're trying to open is already open in at least one "
                                                     "other session.<br>What do you want to do?")));
        QGroupBox sessions;
        sessions.setLayout(new QVBoxLayout);
        QRadioButton* newSession = new QRadioButton(i18n("Add project to current session"));
        sessions.layout()->addWidget(newSession);
        newSession->setChecked(true);
        foreach ( const Session* session, existingSessions ) {
            QRadioButton* button = new QRadioButton(i18n("Open session %1", session->description()));
            button->setProperty("sessionid", session->id().toString());
            sessions.layout()->addWidget(button);
        }
        sessions.layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        mainLayout->addWidget(&sessions);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Abort);
        auto okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        mainLayout->addWidget(buttonBox);

        bool success = dialog.exec();
        if (!success)
            return;

        foreach ( const QObject* obj, sessions.children() ) {
            if ( const QRadioButton* button = qobject_cast<const QRadioButton*>(obj) ) {
                QString sessionid = button->property("sessionid").toString();
                if ( button->isChecked() && ! sessionid.isEmpty() ) {
                    Core::self()->sessionController()->loadSession(sessionid);
                    return;
                }
            }
        }
    }

    if ( url.isEmpty() )
    {
        url = d->dialog->askProjectConfigLocation(false);
    }

    if ( !url.isEmpty() )
    {
        d->importProject(url);
    }
}

void ProjectController::fetchProjectFromUrl(const QUrl& repoUrl, IPlugin* vcsOrProviderPlugin)
{
    const QUrl url = d->dialog->askProjectConfigLocation(true, QUrl(), repoUrl, vcsOrProviderPlugin);

    if (!url.isEmpty()) {
        d->importProject(url);
    }
}

void ProjectController::fetchProject()
{
    QUrl url = d->dialog->askProjectConfigLocation(true);

    if ( !url.isEmpty() )
    {
        d->importProject(url);
    }
}

void ProjectController::projectImportingFinished( IProject* project )
{
    if( !project )
    {
        qWarning() << "OOOPS: 0-pointer project";
        return;
    }
    IPlugin *managerPlugin = project->managerPlugin();
    QList<IPlugin*> pluglist;
    pluglist.append( managerPlugin );
    d->m_projectPlugins.insert( project, pluglist );
    d->m_projects.append( project );

    if ( d->m_currentlyOpening.isEmpty() ) {
        d->saveListOfOpenedProjects();
    }

    if (Core::self()->setupFlags() != Core::NoUi)
    {
        d->m_recentAction->addUrl( project->projectFile().toUrl() );
        KSharedConfig * config = KSharedConfig::openConfig().data();
        KConfigGroup recentGroup = config->group("RecentProjects");
        d->m_recentAction->saveEntries( recentGroup );

        config->sync();
    }

    Q_ASSERT(d->m_currentlyOpening.contains(project->projectFile().toUrl()));
    d->m_currentlyOpening.removeAll(project->projectFile().toUrl());
    emit projectOpened( project );

    reparseProject(project);
}

// helper method for closeProject()
void ProjectController::unloadUnusedProjectPlugins(IProject* proj)
{
    QList<IPlugin*> pluginsForProj = d->m_projectPlugins.value( proj );
    d->m_projectPlugins.remove( proj );

    QList<IPlugin*> otherProjectPlugins;
    Q_FOREACH( const QList<IPlugin*>& _list, d->m_projectPlugins )
    {
        otherProjectPlugins << _list;
    }

    QSet<IPlugin*> pluginsForProjSet = QSet<IPlugin*>::fromList( pluginsForProj );
    QSet<IPlugin*> otherPrjPluginsSet = QSet<IPlugin*>::fromList( otherProjectPlugins );
    // loaded - target = tobe unloaded.
    QSet<IPlugin*> tobeRemoved = pluginsForProjSet.subtract( otherPrjPluginsSet );
    Q_FOREACH( IPlugin* _plugin, tobeRemoved )
    {
        KPluginMetaData _plugInfo = Core::self()->pluginController()->pluginInfo( _plugin );
        if( _plugInfo.isValid() )
        {
            QString _plugName = _plugInfo.pluginId();
            qCDebug(SHELL) << "about to unloading :" << _plugName;
            Core::self()->pluginController()->unloadPlugin( _plugName );
        }
    }
}

// helper method for closeProject()
void ProjectController::closeAllOpenedFiles(IProject* proj)
{
    foreach(IDocument* doc, Core::self()->documentController()->openDocuments()) {
        if (proj->inProject(IndexedString(doc->url()))) {
            doc->close();
        }
    }
}

// helper method for closeProject()
void ProjectController::initializePluginCleanup(IProject* proj)
{
    // Unloading (and thus deleting) these plugins is not a good idea just yet
    // as we're being called by the view part and it gets deleted when we unload the plugin(s)
    // TODO: find a better place to unload
    connect(proj, &IProject::destroyed, this, [&] { d->unloadAllProjectPlugins(); });
}

void ProjectController::takeProject(IProject* proj)
{
    if (!proj) {
        return;
    }

    // loading might have failed
    d->m_currentlyOpening.removeAll(proj->projectFile().toUrl());
    d->m_projects.removeAll(proj);
    emit projectClosing(proj);
    //Core::self()->saveSettings();     // The project file is being closed.
                                        // Now we can save settings for all of the Core
                                        // objects including this one!!
    unloadUnusedProjectPlugins(proj);
    closeAllOpenedFiles(proj);
    proj->close();
    if (d->m_projects.isEmpty())
    {
        initializePluginCleanup(proj);
    }

    if(!d->m_cleaningUp)
        d->saveListOfOpenedProjects();

    emit projectClosed(proj);
}

void ProjectController::closeProject(IProject* proj)
{
    takeProject(proj);
    proj->deleteLater(); // be safe when deleting
}

void ProjectController::closeAllProjects()
{
    foreach (auto project, d->m_projects) {
        closeProject(project);
    }
}

void ProjectController::abortOpeningProject(IProject* proj)
{
    d->m_currentlyOpening.removeAll(proj->projectFile().toUrl());
    emit projectOpeningAborted(proj);
}

ProjectModel* ProjectController::projectModel()
{
    return d->model;
}

IProject* ProjectController::findProjectForUrl( const QUrl& url ) const
{
    if (d->m_projects.isEmpty()) {
        return nullptr;
    }

    ProjectBaseItem* item = d->model->itemForPath(IndexedString(url));
    if (item) {
        return item->project();
    }
    return nullptr;
}

IProject* ProjectController::findProjectByName( const QString& name )
{
    Q_FOREACH( IProject* proj, d->m_projects )
    {
        if( proj->name() == name )
        {
            return proj;
        }
    }
    return nullptr;
}


void ProjectController::configureProject( IProject* project )
{
    d->projectConfig( project );
}

void ProjectController::addProject(IProject* project)
{
    Q_ASSERT(project);
    if (d->m_projects.contains(project)) {
        qWarning() << "Project already tracked by this project controller:" << project;
        return;
    }

    // fake-emit signals so listeners are aware of a new project being added
    emit projectAboutToBeOpened(project);
    project->setParent(this);
    d->m_projects.append(project);
    emit projectOpened(project);
}

bool ProjectController::isProjectNameUsed( const QString& name ) const
{
    foreach( IProject* p, projects() )
    {
        if( p->name() == name )
        {
            return true;
        }
    }
    return false;
}

QUrl ProjectController::projectsBaseDirectory() const
{
    KConfigGroup group = ICore::self()->activeSession()->config()->group( "Project Manager" );
    return group.readEntry( "Projects Base Directory", QUrl::fromLocalFile( QDir::homePath() + "/projects" ) );
}

QString ProjectController::prettyFilePath(const QUrl& url, FormattingOptions format) const
{
    IProject* project = Core::self()->projectController()->findProjectForUrl(url);

    if(!project)
    {
        // Find a project with the correct base directory at least
        foreach(IProject* candidateProject, Core::self()->projectController()->projects())
        {
            if(candidateProject->path().toUrl().isParentOf(url))
            {
                project = candidateProject;
                break;
            }
        }
    }

    Path parent = Path(url).parent();
    QString prefixText;
    if (project) {
        if (format == FormatHtml) {
            prefixText = "<i>" +  project->name() + "</i>/";
        } else {
            prefixText = project->name() + ':';
        }
        QString relativePath = project->path().relativePath(parent);
        if(relativePath.startsWith(QLatin1String("./"))) {
            relativePath = relativePath.mid(2);
        }
        if (!relativePath.isEmpty()) {
            prefixText += relativePath + '/';
        }
    } else {
        prefixText = parent.pathOrUrl() + '/';
    }
    return prefixText;
}

QString ProjectController::prettyFileName(const QUrl& url, FormattingOptions format) const
{
    IProject* project = Core::self()->projectController()->findProjectForUrl(url);
    if(project && project->path() == Path(url))
    {
        if (format == FormatHtml) {
            return "<i>" +  project->name() + "</i>";
        } else {
            return project->name();
        }
    }

    QString prefixText = prettyFilePath( url, format );
    if (format == FormatHtml) {
        return prefixText + "<b>" + url.fileName() + "</b>";
    } else {
        return prefixText + url.fileName();
    }
}

ContextMenuExtension ProjectController::contextMenuExtension ( Context* ctx )
{
    ContextMenuExtension ext;
    if ( ctx->type() != Context::ProjectItemContext || !static_cast<ProjectItemContext*>(ctx)->items().isEmpty() ) {
        return ext;
    }
    ext.addAction(ContextMenuExtension::ProjectGroup, d->m_openProject);
    ext.addAction(ContextMenuExtension::ProjectGroup, d->m_fetchProject);
    ext.addAction(ContextMenuExtension::ProjectGroup, d->m_recentAction);
    return ext;
}

ProjectBuildSetModel* ProjectController::buildSetModel()
{
    return d->buildset;
}

ProjectChangesModel* ProjectController::changesModel()
{
    if(!d->m_changesModel)
        d->m_changesModel=new ProjectChangesModel(this);

    return d->m_changesModel;
}

void ProjectController::commitCurrentProject()
{
    IDocument* doc=ICore::self()->documentController()->activeDocument();
    if(!doc)
        return;

    QUrl url=doc->url();
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);

    if(project && project->versionControlPlugin()) {
        IPlugin* plugin = project->versionControlPlugin();
        IBasicVersionControl* vcs=plugin->extension<IBasicVersionControl>();

        if(vcs) {
            ICore::self()->documentController()->saveAllDocuments(KDevelop::IDocument::Silent);

            const Path basePath = project->path();
            VCSCommitDiffPatchSource* patchSource = new VCSCommitDiffPatchSource(new VCSStandardDiffUpdater(vcs, basePath.toUrl()));

            bool ret = showVcsDiff(patchSource);

            if(!ret) {
                VcsCommitDialog *commitDialog = new VcsCommitDialog(patchSource);
                commitDialog->setCommitCandidates(patchSource->infos());
                commitDialog->exec();
            }
        }
    }
}

QString ProjectController::mapSourceBuild( const QString& path_, bool reverse, bool fallbackRoot ) const
{
    Path path(path_);
    IProject* sourceDirProject = nullptr, *buildDirProject = nullptr;
    Q_FOREACH(IProject* proj, d->m_projects)
    {
        if(proj->path().isParentOf(path) || proj->path() == path)
            sourceDirProject = proj;
        if(proj->buildSystemManager())
        {
            Path buildDir = proj->buildSystemManager()->buildDirectory(proj->projectItem());
            if(buildDir.isValid() && (buildDir.isParentOf(path) || buildDir == path))
                buildDirProject = proj;
        }
    }

    if(!reverse)
    {
        // Map-target is the build directory
        if(sourceDirProject && sourceDirProject->buildSystemManager())
        {
            // We're in the source, map into the build directory
            QString relativePath = sourceDirProject->path().relativePath(path);

            Path build = sourceDirProject->buildSystemManager()->buildDirectory(sourceDirProject->projectItem());
            build.addPath(relativePath);
            while(!QFile::exists(build.path()))
                build = build.parent();
            return build.pathOrUrl();
        }else if(buildDirProject && fallbackRoot)
        {
            // We're in the build directory, map to the build directory root
            return buildDirProject->buildSystemManager()->buildDirectory(buildDirProject->projectItem()).pathOrUrl();
        }
    }else{
        // Map-target is the source directory
        if(buildDirProject)
        {
            Path build = buildDirProject->buildSystemManager()->buildDirectory(buildDirProject->projectItem());
            // We're in the source, map into the build directory
            QString relativePath = build.relativePath(path);

            Path source = buildDirProject->path();
            source.addPath(relativePath);
            while(!QFile::exists(source.path()))
                source = source.parent();
            return source.pathOrUrl();
        }else if(sourceDirProject && fallbackRoot)
        {
            // We're in the source directory, map to the root
            return sourceDirProject->path().pathOrUrl();
        }
    }
    return QString();
}

void ProjectController::reparseProject( IProject* project, bool forceUpdate )
{
    if (auto job = d->m_parseJobs.value(project)) {
        job->kill();
    }

    d->m_parseJobs[project] = new KDevelop::ParseProjectJob(project, forceUpdate);
    ICore::self()->runController()->registerJob(d->m_parseJobs[project]);
}

}

#include "moc_projectcontroller.cpp"
