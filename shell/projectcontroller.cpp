/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Anreas Pakulat <apaku@gmx.de>

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

#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kservice.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kxmlguiwindow.h>
#include <kfiledialog.h>
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
#include <knotification.h>

#include <ksettings/dispatcher.h>

#include <sublime/area.h>
#include <interfaces/iplugin.h>
#include <interfaces/isession.h>
#include <interfaces/context.h>
#include <interfaces/iselectioncontroller.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>
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

namespace KDevelop
{

bool reopenProjectsOnStartup()
{
    KConfigGroup group = Core::self()->activeSession()->config()->group( "Project Manager" );
    return group.readEntry( "Reopen Projects On Startup", false );
}

bool parseAllProjectSources()
{
    KConfigGroup group = Core::self()->activeSession()->config()->group( "Project Manager" );
    return group.readEntry( "Parse All Project Sources", false );
}

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
    QMap<IProject*, QPointer<KSettings::Dialog> > m_cfgDlgs;
    QPointer<KAction> m_closeAllProjects;
    QPointer<KAction> m_closeProject;
    QPointer<KAction> m_openConfig;
    IProjectDialogProvider* dialog;
    QList<KUrl> m_currentlyOpening; // project-file urls that are being opened
    IProject* m_configuringProject;
    ProjectController* q;

    ProjectControllerPrivate( ProjectController* p )
        : m_core(0), model(0), selectionModel(0), dialog(0), m_configuringProject(0), q(p)
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
        if( !m_cfgDlgs.contains( proj ) )
        {
            //@FIXME: compute a blacklist, based on a query for all KDevelop
            //plugins implementing IProjectManager, removing from that the
            //plugin that manages this project. Set this as blacklist on the
            //dialog
            //@FIXME: Currently its important to set a parentApp on the kcm's
            //thats different from the component name of the application, else
            //the plugin will show up on all projects settings dialogs.

            QStringList pluginsForPrj = findPluginsForProject( proj );
            kDebug() << "Using pluginlist:" << pluginsForPrj;
            pluginsForPrj << "kdevplatformproject"; // for project-wide env settings.
            m_cfgDlgs[proj] = new KSettings::Dialog( pluginsForPrj,
                                                     m_core->uiController()->activeMainWindow() );
            m_cfgDlgs[proj]->setKCMArguments( QStringList()
                                              << proj->developerTempFile()
                                              << proj->projectTempFile()
                                              << proj->projectFileUrl().url()
                                              << proj->developerFileUrl().url() );
        }
        m_configuringProject = proj;
        m_cfgDlgs[proj]->exec();
        m_configuringProject = 0;
    }
    QStringList findPluginsForProject( IProject* project )
    {
        QList<IPlugin*> plugins = m_core->pluginController()->loadedPlugins();
        QStringList pluginnames;
        for( QList<IPlugin*>::iterator it = plugins.begin(); it != plugins.end(); ++it )
        {
            IPlugin* plugin = *it;
            IProjectFileManager* iface = plugin->extension<KDevelop::IProjectFileManager>();
            if( !iface || plugin == project->managerPlugin() )
                pluginnames << m_core->pluginController()->pluginInfo( plugin ).pluginName();
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


};

IProjectDialogProvider::IProjectDialogProvider()
{}

IProjectDialogProvider::~IProjectDialogProvider()
{}

ProjectDialogProvider::ProjectDialogProvider(ProjectControllerPrivate* const p) : d(p)
{}

ProjectDialogProvider::~ProjectDialogProvider()
{}

void writeNewProjectFile( KSharedConfig::Ptr cfg, const QString& name, const QString& manager )
{
    KConfigGroup grp = cfg->group( "Project" );
    grp.writeEntry( "Name", name );
    grp.writeEntry( "Manager", manager );
    cfg->sync();
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

KUrl ProjectDialogProvider::askProjectConfigLocation()
{
    Q_ASSERT(d);
    OpenProjectDialog dlg( Core::self()->uiController()->activeMainWindow() );
    if(dlg.exec() == QDialog::Rejected)
        return KUrl();
    
    KUrl projectFileUrl = dlg.projectFileUrl();
    kDebug() << "selected project:" << projectFileUrl << dlg.projectName() << dlg.projectManager();
    if( !projectFileExists( projectFileUrl ) )
    {
        if( projectFileUrl.isLocalFile() )
        {
            writeNewProjectFile( KSharedConfig::openConfig( projectFileUrl.toLocalFile(), KConfig::SimpleConfig ),
                            dlg.projectName(),
                            dlg.projectManager() );
        } else
        {
            KTemporaryFile tmp;
            tmp.setAutoRemove( false );
            tmp.open();
            tmp.close();
            writeNewProjectFile( KSharedConfig::openConfig( tmp.fileName(), KConfig::SimpleConfig ),
                            dlg.projectName(),
                            dlg.projectManager() );
            KIO::NetAccess::upload( tmp.fileName(), projectFileUrl, Core::self()->uiControllerInternal()->defaultMainWindow() );
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
    d->selectionModel = new QItemSelectionModel(d->model);
    if(!(Core::self()->setupFlags() & Core::NoUi)) setupActions();

    loadSettings(false);
    d->dialog = new ProjectDialogProvider(d);
    KSettings::Dispatcher::registerComponent( KComponentData("kdevplatformproject"), 
                                              this, 
                                              "notifyProjectConfigurationChanged" );
}

void ProjectController::setupActions()
{
    KActionCollection * ac =
        d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    KAction *action;

    action = ac->addAction( "project_open" );
    action->setText(i18n( "&Open Project..." ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( openProject() ) );
    action->setToolTip( i18n( "Open project" ) );
    action->setWhatsThis( i18n( "<b>Open project</b><p>Opens a KDevelop 4 project.</p>" ) );
    action->setIcon(KIcon("project-open"));

//    action = ac->addAction( "project_close" );
//    action->setText( i18n( "C&lose Project" ) );
//    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeProject() ) );
//    action->setToolTip( i18n( "Close project" ) );
//    action->setWhatsThis( i18n( "<b>Close project</b><p>Closes the current project." ) );
//    action->setEnabled( false );

    d->m_closeAllProjects = action = ac->addAction( "project_close_all" );
    action->setText( i18n( "Close All Projects" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeAllProjects() ) );
    action->setToolTip( i18n( "Close all currently open projects" ) );
    action->setWhatsThis( i18n( "<b>Close all projects</b><p>Closes all of the currently open projects." ) );
    action->setEnabled( false );
    action->setIcon(KIcon("window-close"));

    d->m_closeProject = action = ac->addAction( "project_close" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( closeSelectedProjects() ) );
    action->setText( i18n( "Close Project(s)" ) );
    action->setToolTip( i18n( "Closes all currently selected projects" ) );
    action->setEnabled( false );

    d->m_openConfig = action = ac->addAction( "project_open_config" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( openProjectConfig() ) );
    action->setText( i18n( "Open Configuration..." ) );
    action->setEnabled( false );

    KSharedConfig * config = KGlobal::config().data();
//     KConfigGroup group = config->group( "General Options" );

    d->m_recentAction = new KRecentFilesAction( this );
    connect( d->m_recentAction, SIGNAL(urlSelected(const KUrl&)), SLOT(
                            openProject( const KUrl& ) ));
    ac->addAction( "project_open_recent", d->m_recentAction );
    d->m_recentAction->setText( i18n( "Open Recent" ) );
    d->m_recentAction->setToolTip( i18n( "Open recent project" ) );
    d->m_recentAction->setWhatsThis(
        i18n( "<b>Open recent project</b><p>Opens recently opened project.</p>" ) );
    d->m_recentAction->loadEntries( KConfigGroup(config, "RecentProjects") );
}

ProjectController::~ProjectController()
{
    delete d->model;
    delete d->dialog;
    delete d;
}

void ProjectController::cleanup()
{
    KSharedConfig::Ptr config = Core::self()->activeSession()->config();
    KConfigGroup group = config->group( "General Options" );

    KUrl::List openProjects;

    foreach( IProject* project, d->m_projects ) {
        openProjects.append(project->projectFileUrl());
        closeProject( project );
    }

    group.writeEntry( "Open Projects", openProjects.toStringList() );
}

void ProjectController::initialize()
{
    if (reopenProjectsOnStartup()) {
        KSharedConfig::Ptr config = Core::self()->activeSession()->config();
        KConfigGroup group = config->group( "General Options" );
        KUrl::List openProjects = group.readEntry( "Open Projects", QStringList() );

        foreach (const KUrl& url, openProjects)
            openProject(url);
    }

    connect( Core::self()->selectionController(), SIGNAL(selectionChanged(KDevelop::Context*)),
             SLOT(updateActionStates(KDevelop::Context*)) );
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

bool ProjectController::openProject( const KUrl &projectFile )
{
    KUrl url = projectFile;

    if ( url.isEmpty() )
    {
        url = d->dialog->askProjectConfigLocation();
    }

    if ( !url.isValid() )
    {
        KMessageBox::error(Core::self()->uiControllerInternal()->activeMainWindow(),
                           i18n("Invalid Location: %1", url.prettyUrl()));
        return false;
    }
    if ( d->m_currentlyOpening.contains(url))
    {
        kDebug() << "Already opening " << url << ". Aborting.";
        return false;
    }

    foreach( IProject* project, d->m_projects )
    {
        if ( url == project->projectFileUrl() )
        {
            if ( d->dialog->userWantsReopen() )
            { // close first, then open again by falling through
                closeProject(project);
            } else { // abort
                return false;
            }
        }
    }

    KNotification* ev=new KNotification("LoadingProject", Core::self()->uiControllerInternal()->activeMainWindow());
    ev->setText(i18n( "Loading Project: %1", url.prettyUrl() ));
    ev->setComponentData(KGlobal::mainComponent());
    ev->sendEvent();

    //FIXME Create the hidden directory if it doesn't exist
    if ( loadProjectPart() )
    {
        //The project file has been opened.
        //Now we can load settings for all of the Core objects including this one!!
//         Core::loadSettings();
        d->m_core->pluginControllerInternal()->loadProjectPlugins();
    } else
        return false;

    Project* project = new Project();
    if ( !project->open( url ) )
    {
        KMessageBox::error(Core::self()->uiControllerInternal()->activeMainWindow(),
                           i18n( "Project could not be opened: %1", url.prettyUrl() ));
        delete project;
        return false;
    }

    d->m_currentlyOpening << url;
    d->m_closeAllProjects->setEnabled(true);
    return true;
}

bool ProjectController::projectImportingFinished( IProject* project )
{
    IPlugin *managerPlugin = project->managerPlugin();
    QList<IPlugin*> pluglist;
    pluglist.append( managerPlugin );
    d->m_projectPlugins.insert( project, pluglist );
    d->m_projects.append( project );

//     KActionCollection * ac = d->m_core->uiController()->defaultMainWindow()->actionCollection();
//     QAction * action;

    //action = ac->action( "project_close" );
    //action->setEnabled( true );

    d->m_recentAction->addUrl( project->projectFileUrl() );
    KSharedConfig * config = KGlobal::config().data();
    KConfigGroup recentGroup = config->group("RecentProjects");
    d->m_recentAction->saveEntries( recentGroup );

    config->sync();

    d->m_currentlyOpening.removeAll(project->projectFileUrl());
    emit projectOpened( project );

    KUrl::List parseList;
    if (parseAllProjectSources())
    {
        // Add the project files to the background parser to be parsed.
        QList<ProjectFileItem*> files = project->files();
        foreach ( ProjectFileItem* file, files )
        {
            parseList.append( file->url() );
        }
        //Add low-priority parse jobs, with only the minimum parsed information
        Core::self()->languageController()->backgroundParser()->addDocumentList( parseList, KDevelop::TopDUContext::VisibleDeclarationsAndContexts, 10000 );
    } else
    {
        // Add all currently open files that belong to the project to the background-parser,
        // since more information may be available for parsing them now(Like include-paths).
        foreach(IDocument* document, Core::self()->documentController()->openDocuments())
        {
            if(!project->filesForUrl(document->url()).isEmpty())
            {
                parseList.append(document->url());
            }
        }
        Core::self()->languageController()->backgroundParser()->addDocumentList( parseList, KDevelop::TopDUContext::AllDeclarationsContextsAndUses, 10 );
    }
    KNotification* ev=new KNotification("ProjectLoadedSuccessfully", Core::self()->uiControllerInternal()->activeMainWindow());
    ev->setText(i18n( "Project loaded: %1", project->name() ));
    ev->setComponentData(KGlobal::mainComponent());
    ev->sendEvent();
    return true;
}

// helper method for closeProject()
void ProjectController::unloadUnusedProjectPlugins(IProject* proj)
{
    QList<IPlugin*> pluginsForProj = d->m_projectPlugins.value( proj );
    d->m_projectPlugins.remove( proj );

    QList<IPlugin*> otherProjectPlugins;
    Q_FOREACH( QList<IPlugin*> _list, d->m_projectPlugins )
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
    if (d->m_closeAllProjects)
    {
        d->m_closeAllProjects->setEnabled(false);
    }
}

bool ProjectController::closeProject(IProject* proj)
{
    if(!proj || d->m_projects.indexOf(proj) == -1)
    {
        return false;
    }
    emit projectClosing(proj);
    //Core::self()->saveSettings();     // The project file is being closed.
                                        // Now we can save settings for all of the Core
                                        // objects including this one!!
    unloadUnusedProjectPlugins(proj);
    closeAllOpenedFiles(proj);
    proj->close();
    proj->deleteLater();                //be safe when deleting
    d->m_projects.removeAll(proj);
    if (d->m_projects.isEmpty())
    {
        initializePluginCleanup(proj);
    }
    emit projectClosed(proj);
    return true;
}

bool ProjectController::loadProjectPart()
{

    return true;
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


bool ProjectController::configureProject( IProject* project )
{
    d->projectConfig( project );
    return true;
}

void ProjectController::addProject(IProject* project)
{
    d->m_projects.append( project );
}



void ProjectController::closeAllProjects()
{
    foreach (IProject* project, projects())
    {
        closeProject(project);
    }
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
    KConfigGroup group = Core::self()->activeSession()->config()->group( "Project Manager" );
    return group.readEntry( "Projects Base Directory",
                                     KUrl( QDir::homePath()+"/projects" ) );
}


}

#include "projectcontroller.moc"
