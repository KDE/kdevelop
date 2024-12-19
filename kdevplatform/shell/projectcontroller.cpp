/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QTemporaryFile>
#include <QVBoxLayout>
#include <QTimer>

#include <KActionCollection>
#include <KConfigGroup>
#include <KIO/DeleteJob>
#include <KIO/FileCopyJob>
#include <KIO/ListJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KSharedConfig>
#include <KStandardAction>

#include <sublime/area.h>
#include <sublime/message.h>
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
#include <language/backgroundparser/parseprojectjob.h>
#include <interfaces/iruncontroller.h>
#include <serialization/indexedstringview.h>
#include <util/scopeddialog.h>
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
    QPointer<KRecentFilesAction> m_recentProjectsAction;
    Core* const m_core;
//     IProject* m_currentProject;
    ProjectModel* const model;
    QPointer<QAction> m_openProject;
    QPointer<QAction> m_fetchProject;
    QPointer<QAction> m_closeProject;
    QPointer<QAction> m_openConfig;
    IProjectDialogProvider* dialog;
    QList<QUrl> m_currentlyOpening; // project-file urls that are being opened
    ProjectController* const q;
    ProjectBuildSetModel* buildset;
    bool m_foundProjectFile; //Temporary flag used while searching the hierarchy for a project file
    bool m_cleaningUp; //Temporary flag enabled while destroying the project-controller
    ProjectChangesModel* m_changesModel = nullptr;
    QHash<IProject*, KJob*> m_parseJobs; // parse jobs that add files from the project to the background parser.

    ProjectControllerPrivate(Core* core, ProjectController* p)
        : m_core(core)
        , model(new ProjectModel())
        , dialog(nullptr)
        , q(p)
        , buildset(nullptr)
        , m_foundProjectFile(false)
        , m_cleaningUp(false)
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
        auto* proj = qobject_cast<Project*>(obj);
        if( !proj )
            return;

        auto cfgDlg = new KDevelop::ConfigDialog(m_core->uiController()->activeMainWindow());
        cfgDlg->setAttribute(Qt::WA_DeleteOnClose);
        cfgDlg->setModal(true);

        QVector<KDevelop::ConfigPage*> configPages;

        ProjectConfigOptions options;
        options.developerFile = proj->developerFile();
        options.developerTempFile = proj->developerTempFile();
        options.projectTempFile = proj->projectTempFile();
        options.project = proj;

        const auto plugins = findPluginsForProject(proj);
        for (IPlugin* plugin : plugins) {
            const int perProjectConfigPagesCount = plugin->perProjectConfigPages();
            configPages.reserve(configPages.size() + perProjectConfigPagesCount);
            for (int i = 0; i < perProjectConfigPagesCount; ++i) {
                configPages.append(plugin->perProjectConfigPage(i, options, cfgDlg));
            }
        }

        std::sort(configPages.begin(), configPages.end(),
                  [](const ConfigPage* a, const ConfigPage* b) {
            return a->name() < b->name();
        });

        for (auto page : configPages) {
            cfgDlg->appendConfigPage(page);
        }

        QObject::connect(cfgDlg, &ConfigDialog::configSaved, proj, [this, proj] {
            emit q->projectConfigurationChanged(proj);
        });
        cfgDlg->setWindowTitle(i18nc("@title:window", "Configure Project %1", proj->name()));
        QObject::connect(cfgDlg, &KDevelop::ConfigDialog::finished, proj, [proj]() {
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

        for (IProject* project : std::as_const(m_projects)) {
            openProjects.append(project->projectFile().toUrl());
        }

        activeSession->setContainedProjects( openProjects );
    }

    // Recursively collects builder dependencies for a project.
    static void collectBuilders( QList< IProjectBuilder* >& destination, IProjectBuilder* topBuilder, IProject* project )
    {
        const QList<IProjectBuilder*> auxBuilders = topBuilder->additionalBuilderPlugins(project);
        destination.append( auxBuilders );
        for (IProjectBuilder* auxBuilder : auxBuilders ) {
            collectBuilders( destination, auxBuilder, project );
        }
    }

    QVector<IPlugin*> findPluginsForProject( IProject* project ) const
    {
        const QList<IPlugin*> plugins = m_core->pluginController()->loadedPlugins();
        const IBuildSystemManager* const buildSystemManager = project->buildSystemManager();
        QVector<IPlugin*> projectPlugins;
        QList<IProjectBuilder*> buildersForKcm;
        // Important to also include the "top" builder for the project, so
        // projects with only one such builder are kept working. Otherwise the project config
        // dialog is empty for such cases.
        if (buildSystemManager) {
            buildersForKcm << buildSystemManager->builder();
            collectBuilders( buildersForKcm, buildSystemManager->builder(), project );
        }

        for (auto plugin : plugins) {
            auto info = m_core->pluginController()->pluginInfo(plugin);
            auto* manager = plugin->extension<KDevelop::IProjectFileManager>();
            if( manager && manager != project->projectFileManager() )
            {
                // current plugin is a manager but does not apply to given project, skip
                continue;
            }
            auto* builder = plugin->extension<KDevelop::IProjectBuilder>();
            if ( builder && !buildersForKcm.contains( builder ) )
            {
                continue;
            }
            // Do not show config pages for analyzer tools which need a buildSystemManager
            // TODO: turn into generic feature to disable plugin config pages which do not apply for a project
            if (!buildSystemManager) {
                const auto required = info.value(QStringLiteral("X-KDevelop-IRequired"), QStringList());
                if (required.contains(QLatin1String("org.kdevelop.IBuildSystemManager"))) {
                    continue;
                }
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
            auto* itemContext = dynamic_cast<ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
            if (itemContext) {
                itemCount = itemContext->items().count();
            }
        }

        m_openConfig->setEnabled(itemCount == 1);
        m_closeProject->setEnabled(itemCount > 0);
    }

    QSet<IProject*> selectedProjects()
    {
        QSet<IProject*> projects;

        // if only one project loaded, this is our target
        if (m_projects.count() == 1) {
            projects.insert(m_projects.at(0));
        } else {
            // otherwise base on selection
            auto* ctx = dynamic_cast<ProjectItemContext*>(ICore::self()->selectionController()->currentSelection());
            if (ctx) {
                const auto items = ctx->items();
                for (ProjectBaseItem* item : items) {
                    projects.insert(item->project());
                }
            }
        }
        return projects;
    }

    void openProjectConfig()
    {
        auto projects = selectedProjects();

        if (projects.count() == 1) {
            q->configureProject(*projects.constBegin());
        }
    }

    void closeSelectedProjects()
    {
        const auto projects = selectedProjects();
        for (IProject* project : projects) {
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
            const QString messageText =  i18n("Invalid Location: %1", url.toDisplayString(QUrl::PreferLocalFile));
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
            return;
        }

        if ( m_currentlyOpening.contains(url))
        {
            qCDebug(SHELL) << "Already opening " << url << ". Aborting.";
            const QString messageText =
                i18n("Already opening %1, not opening again", url.toDisplayString(QUrl::PreferLocalFile));
            auto* message = new Sublime::Message(messageText, Sublime::Message::Information);
            message->setAutoHide(0);
            ICore::self()->uiController()->postMessage(message);
            return;
        }

        const auto projects = m_projects;
        for (IProject* project : projects) {
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

        auto* project = new Project();
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

ProjectDialogProvider::ProjectDialogProvider(ProjectControllerPrivate* p) : d(p)
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
    KConfigGroup grp = cfg->group(QStringLiteral("Project"));
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
        auto statJob = KIO::stat(u, KIO::StatJob::DestinationSide, KIO::StatNoDetails, KIO::HideProgressInfo);
        KJobWidgets::setWindow(statJob, Core::self()->uiControllerInternal()->activeMainWindow());
        return statJob->exec();
    }
}

bool equalProjectFile( const QString& configPath, OpenProjectDialog* dlg )
{
    KSharedConfigPtr cfg = KSharedConfig::openConfig( configPath, KConfig::SimpleConfig );
    KConfigGroup grp = cfg->group(QStringLiteral("Project"));
    QString defaultName = dlg->projectFileUrl().adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).fileName();
    return (grp.readEntry( "Name", QString() ) == dlg->projectName() || dlg->projectName() == defaultName) &&
           grp.readEntry( "Manager", QString() ) == dlg->projectManager();
}

QUrl ProjectDialogProvider::askProjectConfigLocation(bool fetch, const QUrl& startUrl,
                                                     const QUrl& repoUrl, IPlugin* vcsOrProviderPlugin)
{
    Q_ASSERT(d);
    ScopedDialog<OpenProjectDialog> dlg(fetch, startUrl, repoUrl, vcsOrProviderPlugin,
                                         Core::self()->uiController()->activeMainWindow());
    if(dlg->exec() == QDialog::Rejected) {
        return QUrl();
    }

    QUrl projectFileUrl = dlg->projectFileUrl();
    qCDebug(SHELL) << "selected project:" << projectFileUrl << dlg->projectName() << dlg->projectManager();
    if ( dlg->projectManager() == QLatin1String("<built-in>") ) {
        return projectFileUrl;
    }

    // controls if existing project file should be saved
    bool writeProjectConfigToFile = true;
    if( projectFileExists( projectFileUrl ) )
    {
        // check whether config is equal
        bool shouldAsk = true;
        if( projectFileUrl == dlg->selectedUrl() )
        {
            if( projectFileUrl.isLocalFile() )
            {
                shouldAsk = !equalProjectFile( projectFileUrl.toLocalFile(), dlg );
            } else {
                shouldAsk = false;

                QTemporaryFile tmpFile;
                if (tmpFile.open()) {
                    auto downloadJob = KIO::file_copy(projectFileUrl, QUrl::fromLocalFile(tmpFile.fileName()));
                    KJobWidgets::setWindow(downloadJob, qApp->activeWindow());
                    if (downloadJob->exec()) {
                        shouldAsk = !equalProjectFile(tmpFile.fileName(), dlg);
                    }
                }
            }
        }

        if ( shouldAsk )
        {
            KGuiItem yes(i18nc("@action:button", "Override"));
            yes.setToolTip(i18nc("@info:tooltip", "Continue to open the project and use the just provided project configuration"));
            KGuiItem no(i18nc("@action:button", "Open Existing File"));
            no.setToolTip(i18nc("@info:tooltip", "Continue to open the project but use the existing project configuration"));
            KGuiItem cancel = KStandardGuiItem::cancel();
            cancel.setToolTip(i18nc("@info:tooltip", "Cancel and do not open the project"));
            int ret = KMessageBox::questionTwoActionsCancel(
                qApp->activeWindow(),
                i18n("There already exists a project configuration file at %1.\n"
                     "Do you want to override it or open the existing file?",
                     projectFileUrl.toDisplayString(QUrl::PreferLocalFile)),
                i18nc("@title:window", "Override Existing Project Configuration"), yes, no, cancel);
            if (ret == KMessageBox::SecondaryAction) {
                writeProjectConfigToFile = false;
            } else if (ret == KMessageBox::Cancel) {
                return QUrl();
            } // else fall through and write new file
        } else {
            writeProjectConfigToFile = false;
        }
    }

    if (writeProjectConfigToFile) {
        Path projectConfigDir(projectFileUrl);
        projectConfigDir.setLastPathSegment(QStringLiteral(".kdev4"));
        auto delJob = KIO::del(projectConfigDir.toUrl());
        delJob->exec();

        if (!writeProjectSettingsToConfigFile(projectFileUrl, dlg)) {
            const QString messageText = i18n("Unable to create configuration file %1", projectFileUrl.url());
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
            return QUrl();
        }
    }

    return projectFileUrl;
}

bool ProjectDialogProvider::userWantsReopen()
{
    Q_ASSERT(d);
    return (KMessageBox::questionTwoActions(
                d->m_core->uiControllerInternal()->defaultMainWindow(), i18n("Reopen the current project?"), {},
                KGuiItem(i18nc("@action:button", "Reopen"), QStringLiteral("view-refresh")), KStandardGuiItem::cancel())
            == KMessageBox::SecondaryAction)
        ? false
        : true;
}

void ProjectController::setDialogProvider(IProjectDialogProvider* dialog)
{
    Q_D(ProjectController);

    Q_ASSERT(d->dialog);
    delete d->dialog;
    d->dialog = dialog;
}

ProjectController::ProjectController( Core* core )
    : IProjectController(core)
    , d_ptr(new ProjectControllerPrivate(core, this))
{
    qRegisterMetaType<QList<QUrl>>();

    setObjectName(QStringLiteral("ProjectController"));

    //NOTE: this is required to be called here, such that the
    //      actions are available when the UI controller gets
    //      initialized *before* the project controller
    if (Core::self()->setupFlags() != Core::NoUi) {
        setupActions();
    }
}

void ProjectController::setupActions()
{
    Q_D(ProjectController);

    KActionCollection * ac =
        d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    QAction*action;

    d->m_openProject = action = ac->addAction( QStringLiteral("project_open") );
    action->setText(i18nc( "@action", "Open / Import Project..." ) );
    action->setToolTip( i18nc( "@info:tooltip", "Open or import project" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Open an existing KDevelop project or import "
                                                    "an existing Project into KDevelop. This entry "
                                                    "allows one to select a KDevelop project file "
                                                    "or an existing directory to open it in KDevelop. "
                                                    "When opening an existing directory that does "
                                                    "not yet have a KDevelop project file, the file "
                                                    "will be created." ) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("project-open")));
    connect(action, &QAction::triggered, this, [&] { openProject(); });

    d->m_fetchProject = action = ac->addAction( QStringLiteral("project_fetch") );
    action->setText(i18nc( "@action", "Fetch Project..." ) );
    action->setIcon( QIcon::fromTheme( QStringLiteral("edit-download") ) );
    action->setToolTip( i18nc( "@info:tooltip", "Fetch project" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Guides the user through the project fetch "
                                                    "and then imports it into KDevelop." ) );
//     action->setIcon(QIcon::fromTheme("project-open"));
    connect( action, &QAction::triggered, this, &ProjectController::fetchProject );

//    action = ac->addAction( "project_close" );
//    action->setText( i18n( "C&lose Project" ) );
//    connect( action, SIGNAL(triggered(bool)), SLOT(closeProject()) );
//    action->setToolTip( i18n( "Close project" ) );
//    action->setWhatsThis( i18n( "Closes the current project." ) );
//    action->setEnabled( false );

    d->m_closeProject = action = ac->addAction( QStringLiteral("project_close") );
    connect(action, &QAction::triggered,
            this, [this] { Q_D(ProjectController); d->closeSelectedProjects(); } );
    action->setText( i18nc( "@action", "Close Project(s)" ) );
    action->setIcon( QIcon::fromTheme( QStringLiteral("project-development-close") ) );
    action->setToolTip( i18nc( "@info:tooltip", "Closes all currently selected projects" ) );
    action->setEnabled( false );

    d->m_openConfig = action = ac->addAction( QStringLiteral("project_open_config") );
    connect(action, &QAction::triggered,
            this, [this] { Q_D(ProjectController); d->openProjectConfig(); } );
    action->setText( i18nc("@action", "Open Configuration..." ) );
    action->setIcon( QIcon::fromTheme(QStringLiteral("configure")) );
    action->setEnabled( false );

    action = ac->addAction( QStringLiteral("commit_current_project") );
    connect( action, &QAction::triggered, this, &ProjectController::commitCurrentProject );
    action->setText( i18nc("@action", "Commit Current Project..." ) );
    action->setIconText( i18nc("@action", "Commit..." ) );
    action->setIcon( QIcon::fromTheme(QStringLiteral("svn-commit")) );
    connect(d->m_core->uiControllerInternal()->defaultMainWindow(), &MainWindow::areaChanged,
            this, [this] (Sublime::Area* area) { Q_D(ProjectController); d->areaChanged(area); });
    d->m_core->uiControllerInternal()->area(0, QStringLiteral("code"))->addAction(action);

    KSharedConfig * config = KSharedConfig::openConfig().data();
//     KConfigGroup group = config->group(QStringLiteral("General Options"));

    d->m_recentProjectsAction = KStandardAction::openRecent(this, SLOT(openProject(QUrl)), this);
    ac->addAction( QStringLiteral("project_open_recent"), d->m_recentProjectsAction );
    d->m_recentProjectsAction->setText( i18nc("@action", "Open Recent Project" ) );
    d->m_recentProjectsAction->setWhatsThis( i18nc( "@info:whatsthis", "Opens recently opened project." ) );
    d->m_recentProjectsAction->loadEntries(KConfigGroup(config, QStringLiteral("RecentProjects")));

    auto* openProjectForFileAction = new QAction( this );
    ac->addAction(QStringLiteral("project_open_for_file"), openProjectForFileAction);
    openProjectForFileAction->setText(i18nc("@action", "Open Project for Current File"));
    openProjectForFileAction->setIcon(QIcon::fromTheme(QStringLiteral("project-open")));
    connect( openProjectForFileAction, &QAction::triggered, this, &ProjectController::openProjectForUrlSlot);
}

ProjectController::~ProjectController()
{
    Q_D(ProjectController);

    delete d->model;
    delete d->dialog;
}

void ProjectController::cleanup()
{
    Q_D(ProjectController);

    if ( d->m_currentlyOpening.isEmpty() ) {
        d->saveListOfOpenedProjects();
    }

    saveRecentProjectsActionEntries();

    d->m_cleaningUp = true;
    if( buildSetModel() ) {
        buildSetModel()->storeToSession( Core::self()->activeSession() );
    }

    closeAllProjects();
}

void ProjectController::saveRecentProjectsActionEntries()
{
    Q_D(ProjectController);

    if (!d->m_recentProjectsAction)
        return;

    auto config = KSharedConfig::openConfig();
    KConfigGroup recentGroup = config->group(QStringLiteral("RecentProjects"));
    d->m_recentProjectsAction->saveEntries( recentGroup );
    config->sync();
}

void ProjectController::initialize()
{
    Q_D(ProjectController);

    d->buildset = new ProjectBuildSetModel( this );
    buildSetModel()->loadFromSession( Core::self()->activeSession() );
    connect( this, &ProjectController::projectOpened,
             d->buildset, &ProjectBuildSetModel::loadFromProject );
    connect( this, &ProjectController::projectClosing,
             d->buildset, &ProjectBuildSetModel::saveToProject );
    connect( this, &ProjectController::projectClosed,
             d->buildset, &ProjectBuildSetModel::projectClosed );

    d->m_changesModel = new ProjectChangesModel(this);

    loadSettings(false);
    d->dialog = new ProjectDialogProvider(d);

    QDBusConnection::sessionBus().registerObject( QStringLiteral("/org/kdevelop/ProjectController"),
        this, QDBusConnection::ExportScriptableSlots );

    KSharedConfigPtr config = Core::self()->activeSession()->config();
    KConfigGroup group = config->group(QStringLiteral("General Options"));
    const auto projects = group.readEntry( "Open Projects", QList<QUrl>() );

    connect( Core::self()->selectionController(), &ISelectionController::selectionChanged,
             this, [this]() { Q_D(ProjectController); d->updateActionStates(); } );
    connect(this, &ProjectController::projectOpened,
            this, [this]() { Q_D(ProjectController); d->updateActionStates(); });
    connect(this, &ProjectController::projectClosing,
            this, [this]() { Q_D(ProjectController); d->updateActionStates(); });

    QTimer::singleShot(0, this, [this, projects](){
        openProjects(projects);
        emit initialized();
    });
}

void ProjectController::openProjects(const QList<QUrl>& projects)
{
    for (const QUrl& url : projects) {
        openProject(url);
    }
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
    Q_D(const ProjectController);

    return d->m_projects.count();
}

IProject* ProjectController::projectAt( int num ) const
{
    Q_D(const ProjectController);

    if( !d->m_projects.isEmpty() && num >= 0 && num < d->m_projects.count() )
        return d->m_projects.at( num );
    return nullptr;
}

QList<IProject*> ProjectController::projects() const
{
    Q_D(const ProjectController);

    return d->m_projects;
}

void ProjectController::eventuallyOpenProjectFile(KIO::Job* _job, const KIO::UDSEntryList& entries)
{
    Q_D(ProjectController);

    auto* job = qobject_cast<KIO::SimpleJob*>(_job);
    Q_ASSERT(job);
    for (const KIO::UDSEntry& entry : entries) {
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
            auto* message = new Sublime::Message(i18n("Project already open: %1", project->name()), Sublime::Message::Error);
            Core::self()->uiController()->postMessage(message);
        }
    }else{
        auto* message = new Sublime::Message(i18n("No active document"), Sublime::Message::Error);
        Core::self()->uiController()->postMessage(message);
    }
}


void ProjectController::openProjectForUrl(const QUrl& sourceUrl) {
    Q_D(ProjectController);

    Q_ASSERT(!sourceUrl.isRelative());
    QUrl dirUrl = sourceUrl;
    if (sourceUrl.isLocalFile() && !QFileInfo(sourceUrl.toLocalFile()).isDir()) {
        dirUrl = dirUrl.adjusted(QUrl::RemoveFilename);
    }
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
    Q_D(ProjectController);

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
        const auto sessions = Core::self()->sessionController()->sessions();
        for (const Session* session : sessions) {
            if(session->containedProjects().contains(url))
            {
                existingSessions << session;
#if 0
                    ///@todo Think about this! Problem: The session might already contain files, the debugger might be active, etc.
                    //If this session is empty, close it
                    if(Core::self()->sessionController()->activeSession()->description().isEmpty())
                    {
                        //Terminate this instance of kdevelop if the user agrees
                        const auto windows = Core::self()->uiController()->controller()->mainWindows();
                        for (Sublime::MainWindow* window : windows) {
                            window->close();
                        }
                    }
#endif
            }
        }
    }

    if ( ! existingSessions.isEmpty() ) {
        ScopedDialog<QDialog> dialog(Core::self()->uiControllerInternal()->activeMainWindow());
        dialog->setWindowTitle(i18nc("@title:window", "Project Already Open"));

        auto mainLayout = new QVBoxLayout(dialog);
        mainLayout->addWidget(new QLabel(i18n("The project you're trying to open is already open in at least one "
                                                     "other session.<br>What do you want to do?")));
        QGroupBox sessions;
        sessions.setLayout(new QVBoxLayout);
        auto* newSession = new QRadioButton(i18nc("@option:radio", "Add project to current session"));
        sessions.layout()->addWidget(newSession);
        newSession->setChecked(true);
        for (const Session* session : std::as_const(existingSessions)) {
            auto* button = new QRadioButton(i18nc("@option:radio", "Open session %1", session->description()));
            button->setProperty("sessionid", session->id().toString());
            sessions.layout()->addWidget(button);
        }
        sessions.layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
        mainLayout->addWidget(&sessions);

        auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Abort);
        auto okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        connect(buttonBox, &QDialogButtonBox::accepted, dialog.data(), &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);
        mainLayout->addWidget(buttonBox);

        if (!dialog->exec())
            return;

        for (const QObject* obj : sessions.children()) {
            if ( const auto* button = qobject_cast<const QRadioButton*>(obj) ) {
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

bool ProjectController::fetchProjectFromUrl(const QUrl& repoUrl, FetchFlags fetchFlags)
{
    Q_D(ProjectController);

    IPlugin* vcsOrProviderPlugin = nullptr;

    // TODO: query also projectprovider plugins, and that before plain vcs plugins
    // e.g. KDE provider plugin could catch URLs from mirror or pickup kde:repo things
    auto* pluginController = d->m_core->pluginController();
    const auto& vcsPlugins = pluginController->allPluginsForExtension(QStringLiteral("org.kdevelop.IBasicVersionControl"));

    for (auto* plugin : vcsPlugins) {
        auto* iface = plugin->extension<IBasicVersionControl>();
        if (iface->isValidRemoteRepositoryUrl(repoUrl)) {
            vcsOrProviderPlugin = plugin;
            break;
        }
    }
    if (!vcsOrProviderPlugin) {
        if (fetchFlags.testFlag(FetchShowErrorIfNotSupported)) {
            const QString messageText =
                i18n("No enabled plugin supports this repository URL: %1", repoUrl.toDisplayString());
            auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
            ICore::self()->uiController()->postMessage(message);
        }
        return false;
    }

    const QUrl url = d->dialog->askProjectConfigLocation(true, QUrl(), repoUrl, vcsOrProviderPlugin);

    if (!url.isEmpty()) {
        d->importProject(url);
    }

    return true;
}

void ProjectController::fetchProject()
{
    Q_D(ProjectController);

    QUrl url = d->dialog->askProjectConfigLocation(true);

    if ( !url.isEmpty() )
    {
        d->importProject(url);
    }
}

void ProjectController::projectImportingFinished( IProject* project )
{
    Q_D(ProjectController);

    if( !project )
    {
        qCWarning(SHELL) << "OOOPS: 0-pointer project";
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
        d->m_recentProjectsAction->addUrl( project->projectFile().toUrl() );
        saveRecentProjectsActionEntries();
    }

    Q_ASSERT(d->m_currentlyOpening.contains(project->projectFile().toUrl()));
    d->m_currentlyOpening.removeAll(project->projectFile().toUrl());
    emit projectOpened( project );

    reparseProject(project);
}

// helper method for closeProject()
void ProjectController::unloadUnusedProjectPlugins(IProject* proj)
{
    Q_D(ProjectController);

    const QList<IPlugin*> pluginsForProj = d->m_projectPlugins.value( proj );
    d->m_projectPlugins.remove( proj );

    QList<IPlugin*> otherProjectPlugins;
    for (const QList<IPlugin*>& _list : std::as_const(d->m_projectPlugins)) {
        otherProjectPlugins << _list;
    }

    QSet<IPlugin*> pluginsForProjSet(pluginsForProj.begin(), pluginsForProj.end());
    QSet<IPlugin*> otherPrjPluginsSet(otherProjectPlugins.constBegin(), otherProjectPlugins.constEnd());
    // loaded - target = tobe unloaded.
    const QSet<IPlugin*> tobeRemoved = pluginsForProjSet.subtract( otherPrjPluginsSet );
    for (IPlugin* _plugin : tobeRemoved) {
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
    const auto documents = Core::self()->documentController()->openDocuments();
    for (IDocument* doc : documents) {
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
    connect(proj, &IProject::destroyed,
            this, [this] { Q_D(ProjectController); d->unloadAllProjectPlugins(); });
}

void ProjectController::takeProject(IProject* proj)
{
    Q_D(ProjectController);

    if (!proj) {
        return;
    }

    // loading might have failed
    d->m_currentlyOpening.removeAll(proj->projectFile().toUrl());
    d->m_projects.removeAll(proj);
    if (auto* job = d->m_parseJobs.value(proj)) {
        job->kill(); // Removes job from m_parseJobs.
    }
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
    Q_D(ProjectController);

    const auto projects = d->m_projects;
    for (auto* project : projects) {
        closeProject(project);
    }
}

void ProjectController::abortOpeningProject(IProject* proj)
{
    Q_D(ProjectController);

    d->m_currentlyOpening.removeAll(proj->projectFile().toUrl());
    emit projectOpeningAborted(proj);
}

ProjectModel* ProjectController::projectModel()
{
    Q_D(ProjectController);

    return d->model;
}

IProject* ProjectController::findProjectForUrl( const QUrl& url ) const
{
    Q_D(const ProjectController);

    if (d->m_projects.isEmpty()) {
        return nullptr;
    }

    const auto* const item = d->model->itemForPath(IndexedStringView{url});
    if (item) {
        return item->project();
    }
    return nullptr;
}

IProject* ProjectController::findProjectByName( const QString& name )
{
    Q_D(ProjectController);

    auto it = std::find_if(d->m_projects.constBegin(), d->m_projects.constEnd(), [&](IProject* proj) {
        return (proj->name() == name);
    });
    return (it != d->m_projects.constEnd()) ? *it : nullptr;
}


void ProjectController::configureProject( IProject* project )
{
    Q_D(ProjectController);

    d->projectConfig( project );
}

void ProjectController::addProject(IProject* project)
{
    Q_D(ProjectController);

    Q_ASSERT(project);
    if (d->m_projects.contains(project)) {
        qCWarning(SHELL) << "Project already tracked by this project controller:" << project;
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
    const auto projects = this->projects();
    return std::any_of(projects.begin(), projects.end(), [&](IProject* p) {
        return (p->name() == name);
    });
}

QUrl ProjectController::projectsBaseDirectory() const
{
    KConfigGroup group = ICore::self()->activeSession()->config()->group(QStringLiteral("Project Manager"));
    return group.readEntry("Projects Base Directory", QUrl::fromLocalFile(QDir::homePath() + QLatin1String("/projects")));
}

QString ProjectController::prettyFilePath(const QUrl& url, FormattingOptions format) const
{
    IProject* project = Core::self()->projectController()->findProjectForUrl(url);

    if(!project)
    {
        // Find a project with the correct base directory at least
        const auto projects = Core::self()->projectController()->projects();
        auto it = std::find_if(projects.begin(), projects.end(), [&](IProject* candidateProject) {
            return (candidateProject->path().toUrl().isParentOf(url));
        });
        if (it != projects.end()) {
            project = *it;
        }
    }

    Path parent = Path(url).parent();
    QString prefixText;
    if (project) {
        if (format == FormatHtml) {
            prefixText = QLatin1String("<i>") +  project->name() + QLatin1String("</i>/");
        } else {
            prefixText = project->name() + QLatin1Char(':');
        }
        QString relativePath = project->path().relativePath(parent);
        if(relativePath.startsWith(QLatin1String("./"))) {
            relativePath.remove(0, 2);
        }
        if (!relativePath.isEmpty()) {
            prefixText += relativePath + QLatin1Char('/');
        }
    } else {
        prefixText = parent.pathOrUrl() + QLatin1Char('/');
    }
    return prefixText;
}

QString ProjectController::prettyFileName(const QUrl& url, FormattingOptions format) const
{
    IProject* project = Core::self()->projectController()->findProjectForUrl(url);
    if(project && project->path() == Path(url))
    {
        if (format == FormatHtml) {
            return QLatin1String("<i>") +  project->name() + QLatin1String("</i>");
        } else {
            return project->name();
        }
    }

    QString prefixText = prettyFilePath( url, format );
    if (format == FormatHtml) {
        return prefixText + QLatin1String("<b>") + url.fileName() + QLatin1String("</b>");
    } else {
        return prefixText + url.fileName();
    }
}

ContextMenuExtension ProjectController::contextMenuExtension(Context* ctx, QWidget* parent)
{
    Q_D(ProjectController);

    Q_UNUSED(parent);
    ContextMenuExtension ext;
    if ( ctx->type() != Context::ProjectItemContext) {
        return ext;
    }
    if (!static_cast<ProjectItemContext*>(ctx)->items().isEmpty() ) {

        auto* action = new QAction(i18nc("@action", "Reparse the Entire Project"), this);
        connect(action, &QAction::triggered, this, [this] {
            Q_D(ProjectController);
            const auto projects = d->selectedProjects();
            for (auto* project : projects) {
                reparseProject(project, true, true);
            }
        });

        ext.addAction(ContextMenuExtension::ProjectGroup, action);
        return ext;
    }

    ext.addAction(ContextMenuExtension::ProjectGroup, d->m_openProject);
    ext.addAction(ContextMenuExtension::ProjectGroup, d->m_fetchProject);
    ext.addAction(ContextMenuExtension::ProjectGroup, d->m_recentProjectsAction);

    return ext;
}

ProjectBuildSetModel* ProjectController::buildSetModel()
{
    Q_D(ProjectController);

    return d->buildset;
}

ProjectChangesModel* ProjectController::changesModel()
{
    Q_D(ProjectController);

    return d->m_changesModel;
}

void ProjectController::commitCurrentProject()
{
    IDocument* doc=ICore::self()->documentController()->activeDocument();
    if(!doc)
        return;

    QUrl url=doc->url();
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    if (!project)
        return;

    IPlugin* plugin = project->versionControlPlugin();
    if (!plugin)
        return;
    auto* vcs = plugin->extension<IBasicVersionControl>();
    if (!vcs)
        return;

    ICore::self()->documentController()->saveAllDocuments(IDocumentController::SaveSelectionMode::DontAskUser);

    const Path basePath = project->path();
    auto* patchSource = new VCSCommitDiffPatchSource(new VCSStandardDiffUpdater(vcs, basePath.toUrl()));

    bool ret = showVcsDiff(patchSource);

    if (!ret) {
        ScopedDialog<VcsCommitDialog> commitDialog(patchSource);
        commitDialog->setCommitCandidates(patchSource->infos());
        commitDialog->exec();
    }
}

QString ProjectController::mapSourceBuild( const QString& path_, bool reverse, bool fallbackRoot ) const
{
    Q_D(const ProjectController);

    Path path(path_);
    IProject* sourceDirProject = nullptr, *buildDirProject = nullptr;
    for (IProject* proj : std::as_const(d->m_projects)) {
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

void ProjectController::reparseProject(IProject* project, bool forceUpdate, bool forceAll)
{
    Q_D(ProjectController);

    if (auto* oldJob = d->m_parseJobs.value(project)) {
        oldJob->kill(); // Removes oldJob from m_parseJobs.
    }

    auto& job = d->m_parseJobs[project];
    job = new ParseProjectJob(project, forceUpdate, forceAll || parseAllProjectSources());
    connect(job, &KJob::finished, this, [d, project](KJob* job) {
        const auto it = d->m_parseJobs.constFind(project);
        if (it != d->m_parseJobs.cend() && *it == job) {
            d->m_parseJobs.erase(it);
        }
    });
    ICore::self()->runController()->registerJob(job);
}

}

#include "moc_projectcontroller.cpp"
