
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

#include <QtCore/QDir>
#include <QtCore/QSignalMapper>
#include <QtGui/QAction>
#include <QtCore/QSet>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>

#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kpassivepopup.h>
#include <kservice.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kxmlguiwindow.h>
#include <kactioncollection.h>
#include <ktemporaryfile.h>
#include <kservicetypetrader.h>
#include <krecentfilesaction.h>
#include <kactionmenu.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <ksettings/dialog.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>

#include <ksettings/dispatcher.h>

#include <sublime/area.h>
#include <interfaces/iplugin.h>
#include <interfaces/isession.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iselectioncontroller.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/projectmodel.h>
#include <project/projectbuildsetmodel.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>

#include "core.h"
#include "project.h"
#include "mainwindow.h"
#include "shellextension.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "openprojectdialog.h"
#include <interfaces/iruncontroller.h>
#include <language/backgroundparser/parseprojectjob.h>
#include <kio/job.h>
#include "sessioncontroller.h"
#include "session.h"
#include <QDBusConnection>
#include <QApplication>
#include <vcs/models/projectchangesmodel.h>
#include <vcs/widgets/vcsdiffpatchsources.h>
#include <vcs/widgets/vcscommitdialog.h>

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
    QItemSelectionModel* selectionModel;
    QPointer<KAction> m_openProject;
    QPointer<KAction> m_fetchProject;
    QPointer<KAction> m_closeProject;
    QPointer<KAction> m_openConfig;
    IProjectDialogProvider* dialog;
    QList<KUrl> m_currentlyOpening; // project-file urls that are being opened
    IProject* m_configuringProject;
    ProjectController* q;
    ProjectBuildSetModel* buildset;
    bool m_foundProjectFile; //Temporary flag used while searching the hierarchy for a project file
    bool m_cleaningUp; //Temporary flag enabled while destroying the project-controller
    QPointer<ProjectChangesModel> m_changesModel;

    ProjectControllerPrivate( ProjectController* p )
        : m_core(0), model(0), selectionModel(0), dialog(0), m_configuringProject(0), q(p), buildset(0), m_foundProjectFile(false), m_cleaningUp(false)
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

        //@FIXME: Currently it is important to set a parentApp on the kcms
        //that's different from the component name of the application, else
        //the plugin will show up on all projects settings dialogs.

        QStringList pluginsForPrj = findPluginsForProject( proj );
        kDebug() << "Using pluginlist:" << pluginsForPrj;
        pluginsForPrj << "kdevplatformproject"; // for project-wide env settings.
        KSettings::Dialog cfgDlg( pluginsForPrj, m_core->uiController()->activeMainWindow() );
        cfgDlg.setKCMArguments( QStringList()
                                    << proj->developerTempFile()
                                    << proj->projectTempFile()
                                    << proj->projectFileUrl().url()
                                    << proj->developerFileUrl().url()
                                    << proj->name() );
        m_configuringProject = proj;
        cfgDlg.setWindowTitle( i18n("Configure Project %1", proj->name()) );
        cfgDlg.exec();
        proj->projectConfiguration()->sync();
        m_configuringProject = 0;
    }
    void saveListOfOpenedProjects()
    {
        KSharedConfig::Ptr config = Core::self()->activeSession()->config();
        KConfigGroup group = config->group( "General Options" );
    
        KUrl::List openProjects;
    
        foreach( IProject* project, m_projects ) {
            openProjects.append(project->projectFileUrl());
        }
    
        group.writeEntry( "Open Projects", openProjects.toStringList() );
        group.sync();
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

    QStringList findPluginsForProject( IProject* project )
    {
        QList<IPlugin*> plugins = m_core->pluginController()->loadedPlugins();
        QStringList pluginnames;
        QList< IProjectBuilder* > buildersForKcm;
        // Important to also include the "top" builder for the project, so
        // projects with only one such builder are kept working. Otherwise the project config
        // dialog is empty for such cases.
        if( IBuildSystemManager* buildSystemManager = project->buildSystemManager() ) {
            buildersForKcm << buildSystemManager->builder();
            collectBuilders( buildersForKcm, buildSystemManager->builder(), project );
        }

        for( QList<IPlugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it )
        {
            IPlugin* plugin = *it;
            const KPluginInfo info = m_core->pluginController()->pluginInfo( plugin );
            if (info.property("X-KDevelop-Category").toString() != "Project")
                continue;
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
            pluginnames << info.pluginName();
        }

        return pluginnames;
    }

    void notifyProjectConfigurationChanged()
    {
        if( m_configuringProject )
        {
            emit q->projectConfigurationChanged( m_configuringProject );
        }
    }

    void updateActionStates( Context* ctx )
    {
        ProjectItemContext* itemctx = dynamic_cast<ProjectItemContext*>(ctx);
        m_openConfig->setEnabled( itemctx && itemctx->items().count() == 1 );
        m_closeProject->setEnabled( itemctx && itemctx->items().count() > 0 );
    }
    
    void openProjectConfig()
    {
        ProjectItemContext* ctx = dynamic_cast<ProjectItemContext*>( Core::self()->selectionController()->currentSelection() );
        if( ctx && ctx->items().count() == 1 )
        {
            q->configureProject( ctx->items().at(0)->project() );
        }
    }
    
    void closeSelectedProjects()
    {
        ProjectItemContext* ctx =  dynamic_cast<ProjectItemContext*>( Core::self()->selectionController()->currentSelection() );
        if( ctx && ctx->items().count() > 0 )
        {
            QSet<IProject*> projects;
            foreach( ProjectBaseItem* item, ctx->items() )
            {
                projects.insert( item->project() );
            }
            foreach( IProject* project, projects )
            {
                q->closeProject( project );
            }
        }
    
    }
    
    void importProject(const KUrl& url_)
    {
        KUrl url(url_);
        if ( url.isLocalFile() )
        {
            QString path = QFileInfo( url.toLocalFile() ).canonicalFilePath();
            if ( !path.isEmpty() )
                url.setPath( path );
        }

        if ( !url.isValid() )
        {
            KMessageBox::error(Core::self()->uiControllerInternal()->activeMainWindow(),
                            i18n("Invalid Location: %1", url.prettyUrl()));
            return;
        }

        if ( m_currentlyOpening.contains(url))
        {
            kDebug() << "Already opening " << url << ". Aborting.";
            KPassivePopup::message( i18n( "Project already being opened"), 
                                    i18n( "Already opening %1, not opening again", 
                                        url.prettyUrl() ), 
                                    m_core->uiController()->activeMainWindow() );
            return;
        }
        m_currentlyOpening += url;

        foreach( IProject* project, m_projects )
        {
            if ( url == project->projectFileUrl() )
            {
                if ( dialog->userWantsReopen() )
                { // close first, then open again by falling through
                    q->closeProject(project);
                } else { // abort
                    return;
                }
            }
        }

        m_core->pluginControllerInternal()->loadProjectPlugins();

        Project* project = new Project();
        emit q->projectAboutToBeOpened( project );
        if ( !project->open( url ) )
        {
            m_currentlyOpening.removeAll(url);
            q->abortOpeningProject(project);
            project->deleteLater();
        }
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

bool writeNewProjectFile( const QString& localConfigFile, const QString& name, const QString& manager )
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig( localConfigFile, KConfig::SimpleConfig );
    if (!cfg->isConfigWritable(true)) {
        kDebug() << "can't write to configfile";
        return false;
    }
    KConfigGroup grp = cfg->group( "Project" );
    grp.writeEntry( "Name", name );
    grp.writeEntry( "Manager", manager );
    cfg->sync();
    return true;
}

bool writeProjectSettingsToConfigFile(const KUrl& projectFileUrl, const QString& projectName, const QString& projectManager)
{
    if ( !projectFileUrl.isLocalFile() ) {
        KTemporaryFile tmp;
        if ( !tmp.open() ) {
            return false;
        }
        if ( !writeNewProjectFile( tmp.fileName(), projectName, projectManager ) ) {
            return false;
        }
        // explicitly close file before uploading it, see also: https://bugs.kde.org/show_bug.cgi?id=254519
        tmp.close();
        return KIO::NetAccess::upload( tmp.fileName(), projectFileUrl, Core::self()->uiControllerInternal()->defaultMainWindow() );
    }
    return writeNewProjectFile( projectFileUrl.toLocalFile(),projectName, projectManager );
}


bool projectFileExists( const KUrl& u )
{
    if( u.isLocalFile() ) 
    {
        return QFileInfo( u.toLocalFile() ).exists();
    } else
    {
        return KIO::NetAccess::exists( u, KIO::NetAccess::DestinationSide, Core::self()->uiControllerInternal()->activeMainWindow() );
    }
}

bool equalProjectFile( const QString& configPath, OpenProjectDialog* dlg )
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig( configPath, KConfig::SimpleConfig );
    KConfigGroup grp = cfg->group( "Project" );
    QString defaultName = dlg->projectFileUrl().upUrl().fileName();
    return (grp.readEntry( "Name", QString() ) == dlg->projectName() || dlg->projectName() == defaultName) &&
           grp.readEntry( "Manager", QString() ) == dlg->projectManager();
}

KUrl ProjectDialogProvider::askProjectConfigLocation(bool fetch, const KUrl& startUrl)
{
    Q_ASSERT(d);
    OpenProjectDialog dlg( fetch, startUrl, Core::self()->uiController()->activeMainWindow() );
    if(dlg.exec() == QDialog::Rejected)
        return KUrl();

    KUrl projectFileUrl = dlg.projectFileUrl();
    kDebug() << "selected project:" << projectFileUrl << dlg.projectName() << dlg.projectManager();

    // controls if existing project file should be saved
    bool writeProjectConfigToFile = true;
    if( projectFileExists( projectFileUrl ) )
    {
        // check whether config is equal
        bool shouldAsk = true;
        if( projectFileUrl.isLocalFile() )
        {
            shouldAsk = !equalProjectFile( projectFileUrl.toLocalFile(), &dlg );
        } else {
            QString tmpFile;
            if ( KIO::NetAccess::download( projectFileUrl, tmpFile, qApp->activeWindow() ) ) {
                shouldAsk = !equalProjectFile( tmpFile, &dlg );
                QFile::remove(tmpFile);
            } else {
                shouldAsk = false;
            }
        }

        if ( shouldAsk )
        {
            KGuiItem yes = KStandardGuiItem::yes();
            yes.setText(i18n("Override"));
            yes.setToolTip(i18nc("@info:tooltip", "Continue to open the project and use the just provided project configuration."));
            yes.setIcon(KIcon());
            KGuiItem no = KStandardGuiItem::no();
            no.setText(i18n("Open Existing File"));
            no.setToolTip(i18nc("@info:tooltip", "Continue to open the project but use the existing project configuration."));
            no.setIcon(KIcon());
            KGuiItem cancel = KStandardGuiItem::cancel();
            cancel.setToolTip(i18nc("@info:tooltip", "Cancel and do not open the project."));
            int ret = KMessageBox::questionYesNoCancel(qApp->activeWindow(),
                i18n("There already exists a project configuration file at %1.\n"
                     "Do you want to override it or open the existing file?", projectFileUrl.pathOrUrl()),
                i18n("Override existing project configuration"), yes, no, cancel );
            if ( ret == KMessageBox::No )
            {
                writeProjectConfigToFile = false;
            } else if ( ret == KMessageBox::Cancel )
            {
                return KUrl();
            } // else fall through and write new file
        } else {
            writeProjectConfigToFile = false;
        }
    }

    if (writeProjectConfigToFile) {
        if (!writeProjectSettingsToConfigFile(projectFileUrl, dlg.projectName(), dlg.projectManager())) {
            KMessageBox::error(d->m_core->uiControllerInternal()->defaultMainWindow(),
                i18n("Unable to create configuration file %1", projectFileUrl.url()));
            return KUrl();
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
    setObjectName("ProjectController");
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

    KAction *action;

    d->m_openProject = action = ac->addAction( "project_open" );
    action->setText(i18nc( "@action", "Open / Import Project..." ) );
    action->setToolTip( i18nc( "@info:tooltip", "Open or import project" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Open an existing KDevelop 4 project or import "
                                                    "an existing Project into KDevelop 4. This entry "
                                                    "allows to select a KDevelop4 project file or an "
                                                    "existing directory to open it in KDevelop. "
                                                    "When opening an existing directory that does "
                                                    "not yet have a KDevelop4 project file, the file "
                                                    "will be created." ) );
    action->setIcon(KIcon("project-open"));
    connect( action, SIGNAL(triggered(bool)), SLOT(openProject()) );
    
    d->m_fetchProject = action = ac->addAction( "project_fetch" );
    action->setText(i18nc( "@action", "Fetch Project..." ) );
    action->setIcon( KIcon( "download" ) );
    action->setToolTip( i18nc( "@info:tooltip", "Fetch project" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Guides the user through the project fetch "
                                                    "and then imports it into KDevelop 4." ) );
//     action->setIcon(KIcon("project-open"));
    connect( action, SIGNAL(triggered(bool)), SLOT(fetchProject()) );

//    action = ac->addAction( "project_close" );
//    action->setText( i18n( "C&lose Project" ) );
//    connect( action, SIGNAL(triggered(bool)), SLOT(closeProject()) );
//    action->setToolTip( i18n( "Close project" ) );
//    action->setWhatsThis( i18n( "Closes the current project." ) );
//    action->setEnabled( false );

    d->m_closeProject = action = ac->addAction( "project_close" );
    connect( action, SIGNAL(triggered(bool)), SLOT(closeSelectedProjects()) );
    action->setText( i18nc( "@action", "Close Project(s)" ) );
    action->setIcon( KIcon( "project-development-close" ) );
    action->setToolTip( i18nc( "@info:tooltip", "Closes all currently selected projects" ) );
    action->setEnabled( false );

    d->m_openConfig = action = ac->addAction( "project_open_config" );
    connect( action, SIGNAL(triggered(bool)), SLOT(openProjectConfig()) );
    action->setText( i18n( "Open Configuration..." ) );
    action->setIcon( KIcon("configure") );
    action->setEnabled( false );
    
    action = ac->addAction( "commit_current_project" );
    connect( action, SIGNAL(triggered(bool)), SLOT(commitCurrentProject()) );
    action->setText( i18n( "Commit Current Project..." ) );
    action->setIconText( i18n( "Commit..." ) );
    action->setIcon( KIcon("svn-commit") );

    KSharedConfig * config = KGlobal::config().data();
//     KConfigGroup group = config->group( "General Options" );

    d->m_recentAction = KStandardAction::openRecent(this, SLOT(openProject(KUrl)), this);
    ac->addAction( "project_open_recent", d->m_recentAction );
    d->m_recentAction->setText( i18n( "Open Recent Project" ) );
    d->m_recentAction->setWhatsThis( i18nc( "@info:whatsthis", "Opens recently opened project." ) );
    d->m_recentAction->loadEntries( KConfigGroup(config, "RecentProjects") );
    
    KAction* openProjectForFileAction = new KAction( this );
    ac->addAction("project_open_for_file", openProjectForFileAction);
    openProjectForFileAction->setText(i18n("Open Project for Current File"));
    connect( openProjectForFileAction, SIGNAL(triggered(bool)), SLOT(openProjectForUrlSlot(bool)));
}

ProjectController::~ProjectController()
{
    delete d->model;
    delete d->dialog;
    delete d;
}

void ProjectController::cleanup()
{
    d->m_cleaningUp = true;
    if( buildSetModel() ) {
        buildSetModel()->storeToSession( Core::self()->activeSession() );
    }

    foreach( IProject* project, d->m_projects ) {
        closeProject( project );
    }
}

void ProjectController::initialize()
{
    d->buildset = new ProjectBuildSetModel( this );
    buildSetModel()->loadFromSession( Core::self()->activeSession() );
    connect( this, SIGNAL(projectOpened(KDevelop::IProject*)),
             d->buildset, SLOT(loadFromProject(KDevelop::IProject*)) );
    connect( this, SIGNAL(projectClosing(KDevelop::IProject*)),
             d->buildset, SLOT(saveToProject(KDevelop::IProject*)) );
    connect( this, SIGNAL(projectClosed(KDevelop::IProject*)),
             d->buildset, SLOT(projectClosed(KDevelop::IProject*)) );

    d->selectionModel = new QItemSelectionModel(d->model);

    loadSettings(false);
    d->dialog = new ProjectDialogProvider(d);
    KSettings::Dispatcher::registerComponent( KComponentData("kdevplatformproject"),
                                              this,
                                              "notifyProjectConfigurationChanged" );

    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/ProjectController",
        this, QDBusConnection::ExportScriptableSlots );

    KSharedConfig::Ptr config = Core::self()->activeSession()->config();
    KConfigGroup group = config->group( "General Options" );
    KUrl::List openProjects = group.readEntry( "Open Projects", QStringList() );

    QMetaObject::invokeMethod(this, "openProjects", Qt::QueuedConnection, Q_ARG(KUrl::List, openProjects));
    
    connect( Core::self()->selectionController(), SIGNAL(selectionChanged(KDevelop::Context*)),
             SLOT(updateActionStates(KDevelop::Context*)) );
}

void ProjectController::openProjects(const KUrl::List& projects)
{
    foreach (const KUrl& url, projects)
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
    return 0;
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
            
            if(name.endsWith(".kdev4")) {
                //We have found a project-file, open it
                KUrl u(job->url());
                u.addPath(name);
                openProject(u);
                d->m_foundProjectFile = true;
            }
        }
    }
}

void ProjectController::openProjectForUrlSlot(bool) {
    if(ICore::self()->documentController()->activeDocument()) {
        KUrl url = ICore::self()->documentController()->activeDocument()->url();
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


void ProjectController::openProjectForUrl(const KUrl& sourceUrl) {
    KUrl dirUrl = sourceUrl.upUrl();
    KUrl testAt = dirUrl;
    
    d->m_foundProjectFile = false;
    
    while(!testAt.path().isEmpty()) {
        KUrl testProjectFile(testAt);
        KIO::ListJob* job = KIO::listDir(testAt);
        
        connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)), SLOT(eventuallyOpenProjectFile(KIO::Job*,KIO::UDSEntryList)));
        
        KIO::NetAccess::synchronousRun(job, ICore::self()->uiController()->activeMainWindow());
        if(d->m_foundProjectFile) {
            //Fine! We have directly opened the project
            d->m_foundProjectFile = false;
            return;
        }
        KUrl oldTest = testAt;
        testAt = testAt.upUrl();
        if(oldTest == testAt)
            break;
    }
    
    KUrl askForOpen = d->dialog->askProjectConfigLocation(false, dirUrl);
    
    if(askForOpen.isValid())
        openProject(askForOpen);
}

void ProjectController::openProject( const KUrl &projectFile )
{
    KUrl url = projectFile;
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
        KDialog dialog(Core::self()->uiControllerInternal()->activeMainWindow());
        dialog.setButtons(KDialog::Ok | KDialog::Cancel);
        dialog.setWindowTitle(i18n("Project Already Open"));

        QWidget contents;
        contents.setLayout(new QVBoxLayout);
        contents.layout()->addWidget(new QLabel(i18n("The project you're trying to open is already open in at least one "
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
        contents.layout()->addWidget(&sessions);

        dialog.setMainWidget(&contents);
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

void ProjectController::fetchProject()
{
    KUrl url = d->dialog->askProjectConfigLocation(true);

    if ( !url.isEmpty() )    
    {
        d->importProject(url);
    }
}

void ProjectController::projectImportingFinished( IProject* project )
{
    if( !project )
    {
        kWarning() << "OOOPS: 0-pointer project";
        return;
    }
    IPlugin *managerPlugin = project->managerPlugin();
    QList<IPlugin*> pluglist;
    pluglist.append( managerPlugin );
    d->m_projectPlugins.insert( project, pluglist );
    d->m_projects.append( project );

    if(!Core::self()->sessionController()->activeSession()->containedProjects().contains(project->projectFileUrl()))
        d->saveListOfOpenedProjects();
    
//     KActionCollection * ac = d->m_core->uiController()->defaultMainWindow()->actionCollection();
//     QAction * action;

    //action = ac->action( "project_close" );
    //action->setEnabled( true );

    if (Core::self()->setupFlags() != Core::NoUi)
    {
        d->m_recentAction->addUrl( project->projectFileUrl() );
        KSharedConfig * config = KGlobal::config().data();
        KConfigGroup recentGroup = config->group("RecentProjects");
        d->m_recentAction->saveEntries( recentGroup );

        config->sync();
    }

    Q_ASSERT(d->m_currentlyOpening.contains(project->projectFileUrl()));
    d->m_currentlyOpening.removeAll(project->projectFileUrl());
    emit projectOpened( project );

    if (parseAllProjectSources())
    {
        KJob* parseProjectJob = new KDevelop::ParseProjectJob(project);
        ICore::self()->runController()->registerJob(parseProjectJob);
    }

    // Add all currently open files that belong to the project to the background-parser,
    // since more information may be available for parsing them now(Like include-paths).
    foreach(IDocument* document, Core::self()->documentController()->openDocuments())
    {
        if(!project->filesForUrl(document->url()).isEmpty())
        {
            Core::self()->languageController()->backgroundParser()->addDocument( IndexedString(document->url()), TopDUContext::AllDeclarationsContextsAndUses, 10 );
        }
    }
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
        KPluginInfo _plugInfo = Core::self()->pluginController()->pluginInfo( _plugin );
        if( _plugInfo.isValid() )
        {
            QString _plugName = _plugInfo.pluginName();
            kDebug() << "about to unloading :" << _plugName;
            Core::self()->pluginController()->unloadPlugin( _plugName );
        }
    }
}

// helper method for closeProject()
void ProjectController::closeAllOpenedFiles(IProject* proj)
{
    Q_FOREACH( ProjectFileItem *fileItem, proj->files() )
    {
        Core::self()->documentControllerInternal()->closeDocument( fileItem->url() );
    }
}

// helper method for closeProject()
void ProjectController::initializePluginCleanup(IProject* proj)
{
    // Unloading (and thus deleting) these plugins is not a good idea just yet
    // as we're being called by the view part and it gets deleted when we unload the plugin(s)
    // TODO: find a better place to unload
    connect(proj, SIGNAL(destroyed(QObject*)), this, SLOT(unloadAllProjectPlugins()));
}

void ProjectController::closeProject(IProject* proj_)
{
    if (!proj_)
    {
        return;
    }

    // loading might have failed
    d->m_currentlyOpening.removeAll(proj_->projectFileUrl());

    Project* proj = dynamic_cast<KDevelop::Project*>( proj_ );
    if( !proj ) 
    {
        kWarning() << "Unknown Project subclass found!";
        return;
    }
    d->m_projects.removeAll(proj);
    emit projectClosing(proj);
    //Core::self()->saveSettings();     // The project file is being closed.
                                        // Now we can save settings for all of the Core
                                        // objects including this one!!
    unloadUnusedProjectPlugins(proj);
    closeAllOpenedFiles(proj);
    proj->close();
    proj->deleteLater();                //be safe when deleting
    if (d->m_projects.isEmpty())
    {
        initializePluginCleanup(proj);
    }

    if(!d->m_cleaningUp)
        d->saveListOfOpenedProjects();

    emit projectClosed(proj);
    return;
}

void ProjectController::abortOpeningProject(IProject* proj)
{
    d->m_currentlyOpening.removeAll(proj->projectFileUrl());
    emit projectOpeningAborted(proj);
}

ProjectModel* ProjectController::projectModel()
{
    return d->model;
}

IProject* ProjectController::findProjectForUrl( const KUrl& url ) const
{
    Q_FOREACH( IProject* proj, d->m_projects )
    {
        if( proj->inProject( url ) )
            return proj;
    }
    return 0;
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
    return 0;
}


void ProjectController::configureProject( IProject* project )
{
    d->projectConfig( project );
}

void ProjectController::addProject(IProject* project)
{
    d->m_projects.append( project );
}

QItemSelectionModel* ProjectController::projectSelectionModel()
{
    return d->selectionModel;
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

KUrl ProjectController::projectsBaseDirectory() const
{
    KConfigGroup group = KGlobal::config()->group( "Project Manager" );
    return group.readEntry( "Projects Base Directory",
                                     KUrl( QDir::homePath()+"/projects" ) );
}

QString ProjectController::prettyFilePath(KUrl url, FormattingOptions format) const
{
    IProject* project = Core::self()->projectController()->findProjectForUrl(url);
    
    if(!project)
    {
        // Find a project with the correct base directory at least
        foreach(IProject* candidateProject, Core::self()->projectController()->projects())
        {
            if(candidateProject->folder().isParentOf(url))
            {
                project = candidateProject;
                break;
            }
        }
    }
    
    QString prefixText = url.upUrl().pathOrUrl(KUrl::AddTrailingSlash);
    if (project) {
        if (format == FormatHtml) {
            prefixText = "<i>" +  project->name() + "</i>/";
        } else {
            prefixText = project->name() + '/';
        }
        QString relativePath = project->relativeUrl(url.upUrl()).path(KUrl::AddTrailingSlash);
        if(relativePath.startsWith("./"))
            relativePath = relativePath.mid(2);
        prefixText += relativePath;
    }
    return prefixText;
}

QString ProjectController::prettyFileName(KUrl url, FormattingOptions format) const
{
    IProject* project = Core::self()->projectController()->findProjectForUrl(url);
    if(project && project->folder().equals(url, KUrl::CompareWithoutTrailingSlash))
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
    
    KUrl url=doc->url();
    IProject* project = ICore::self()->projectController()->findProjectForUrl(url);
    
    if(project && project->versionControlPlugin()) {
        KUrl baseUrl=project->projectItem()->url();
        IPlugin* plugin = project->versionControlPlugin();
        IBasicVersionControl* vcs=plugin->extension<IBasicVersionControl>();
        
        if(vcs) {
            ICore::self()->documentController()->saveAllDocuments(KDevelop::IDocument::Silent);

            VCSCommitDiffPatchSource* patchSource = new VCSCommitDiffPatchSource(new VCSStandardDiffUpdater(vcs, baseUrl));

            bool ret = showVcsDiff(patchSource);

            if(!ret) {
                VcsCommitDialog *commitDialog = new VcsCommitDialog(patchSource);
                commitDialog->setCommitCandidates(patchSource->infos());
                commitDialog->exec();
            }
        }
    }
}

QString ProjectController::mapSourceBuild( const QString& path, bool reverse, bool fallbackRoot ) const
{
    KUrl url(path);
    IProject* sourceDirProject = 0, *buildDirProject = 0;
    Q_FOREACH(IProject* proj, d->m_projects)
    {
        if(proj->folder().isParentOf(url))
            sourceDirProject = proj;
        if(proj->buildSystemManager())
        {
            KUrl buildDir = proj->buildSystemManager()->buildDirectory(proj->projectItem());
            if(buildDir.isValid() && buildDir.isParentOf(url))
                buildDirProject = proj;
        }
    }
    
    if(!reverse)
    {
        // Map-target is the build directory
        if(sourceDirProject && sourceDirProject->buildSystemManager())
        {
            // We're in the source, map into the build directory
            QString relativePath = KUrl::relativeUrl(sourceDirProject->folder(), url);
            
            KUrl build = sourceDirProject->buildSystemManager()->buildDirectory(sourceDirProject->projectItem());
            build.addPath(relativePath);
            while(!QFile::exists(build.path()))
                build = build.upUrl();
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
            KUrl build = buildDirProject->buildSystemManager()->buildDirectory(buildDirProject->projectItem());
            // We're in the source, map into the build directory
            QString relativePath = KUrl::relativeUrl(build, url);
            
            KUrl source = buildDirProject->folder();
            source.addPath(relativePath);
            while(!QFile::exists(source.path()))
                source = source.upUrl();
            return source.pathOrUrl();
        }else if(sourceDirProject && fallbackRoot)
        {
            // We're in the source directory, map to the root
            return sourceDirProject->folder().pathOrUrl();
        }
    }
    return QString();
}

}

#include "projectcontroller.moc"
