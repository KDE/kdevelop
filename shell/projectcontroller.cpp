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
#include <QSet>
#include <QList>
#include <QMap>

#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kservice.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kxmlguiwindow.h>
#include <kfiledialog.h>
#include <kactioncollection.h>
#include <kservicetypetrader.h>
#include <krecentfilesaction.h>
#include <kactionmenu.h>
#include <ksettings/dialog.h>
#include <kstandarddirs.h>

#include "sublime/area.h"

#include "core.h"
#include "iplugin.h"
#include "iprojectfilemanager.h"
#include "project.h"
#include "mainwindow.h"
#include "shellextension.h"
#include "projectmodel.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "ilanguagecontroller.h"
#include "backgroundparser.h"

namespace KDevelop
{

class ProjectControllerPrivate
{
public:
    QList<IProject*> m_projects;
    QMap< IProject*, QList<IPlugin*> > m_projectPlugins;
    QMap< IProject*, QPointer<QAction> > m_configActions;
    IPlugin* m_projectPart;
    QPointer<KRecentFilesAction> m_recentAction;
    KActionMenu *m_projectConfigAction;
    QSignalMapper *m_signalMapper;
    Core* m_core;
//     IProject* m_currentProject;
    ProjectModel* model;
    QMap<IProject*, QPointer<KSettings::Dialog> > m_cfgDlgs;

    WorkspaceItem* workspaceitem;

    QPointer<QAction> m_closeAllProjects;

    bool reopenProjectsOnStartup;
    bool parseAllProjectSources;

    void unloadAllProjectPlugins()
    {
        if( m_projects.isEmpty() )
            m_core->pluginController()->unloadPlugins( IPluginController::Project );
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
        m_cfgDlgs[proj]->show();
    }
    QStringList findPluginsForProject( IProject* project )
    {
        QList<IPlugin*> plugins = m_core->pluginController()->loadedPlugins();
        QStringList pluginnames;
        kDebug() << "managerplugin:" << project->managerPlugin();
        for( QList<IPlugin*>::iterator it = plugins.begin(); it != plugins.end(); it++ )
        {
            IPlugin* plugin = *it;
            IProjectFileManager* iface = plugin->extension<KDevelop::IProjectFileManager>();
            kDebug() << "Checking plugin:" << plugin << "with iface" << iface;
            if( !iface || plugin == project->managerPlugin() )
                pluginnames << m_core->pluginController()->pluginInfo( plugin ).pluginName();
        }

        return pluginnames;
    }
};

ProjectController::ProjectController( Core* core )
        : IProjectController( core ), d( new ProjectControllerPrivate )
{
    d->reopenProjectsOnStartup = false;
    d->parseAllProjectSources = false;
    d->m_core = core;
    d->m_projectPart = 0;
    d->m_signalMapper = new QSignalMapper( this );
    connect( d->m_signalMapper, SIGNAL( mapped( QObject* ) ),
            this, SLOT( projectConfig( QObject* ) ) );
//     d->m_currentProject = 0;
    d->model = new ProjectModel();
    QString workspacename = KGlobal::config()->group("Workspace").readEntry( "Name", "default" );
    QString workspaceMetadataFile = KGlobal::dirs()->findResource("data", workspacename+"Workspace/"+workspacename);
    d->workspaceitem = new WorkspaceItem( workspacename, workspaceMetadataFile );
    d->model->insertRow( 0, d->workspaceitem );
    setupActions();

    loadSettings(false);
}

void ProjectController::setupActions()
{
    KActionCollection * ac =
        d->m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    QAction *action;

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

    KSharedConfig * config = KGlobal::config().data();
//     KConfigGroup group = config->group( "General Options" );

    d->m_recentAction =
        KStandardAction::openRecent( this, SLOT( openProject( const KUrl& ) ),
                                ac);
    ac->addAction( "project_open_recent", d->m_recentAction );
    d->m_recentAction->setToolTip( i18n( "Open recent project" ) );
    d->m_recentAction->setWhatsThis(
        i18n( "<b>Open recent project</b><p>Opens recently opened project.</p>" ) );
    d->m_recentAction->loadEntries( KConfigGroup(config, "RecentProjects") );

    d->m_projectConfigAction = new KActionMenu( i18n("Configure Project"), ac );
    ac->addAction( "project_config_menu", d->m_projectConfigAction );
    d->m_projectConfigAction->setIcon(KIcon("configure"));
}

ProjectController::~ProjectController()
{
    delete d->model;
    delete d;
}

void ProjectController::cleanup()
{
    KSharedConfig * config = KGlobal::config().data();
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
    if (d->reopenProjectsOnStartup) {
        KSharedConfig * config = KGlobal::config().data();
        KConfigGroup group = config->group( "General Options" );
        KUrl::List openProjects = group.readEntry( "Open Projects", QStringList() );

        foreach (const KUrl& url, openProjects)
            openProject(url);
    }
}

void ProjectController::loadSettings( bool projectIsLoaded )
{
    Q_UNUSED(projectIsLoaded)

    KConfigGroup config(KGlobal::config(), "Project Manager");

    d->reopenProjectsOnStartup = config.readEntry("Reopen Projects On Startup", false);
    d->parseAllProjectSources = config.readEntry("Parse All Project Sources", false);
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
        KSharedConfig * config = KGlobal::config().data();
        KConfigGroup group = config->group( "General Options" );
        QString dir = group.readEntry( "DefaultProjectsDirectory",
                                             QDir::homePath() );

        QString projectFileInfo = ShellExtension::getInstance()->projectFileExtension()
                + "|" + ShellExtension::getInstance()->projectFileDescription()
                + "\n";
        url = KFileDialog::getOpenUrl( dir, projectFileInfo,
                                       d->m_core->uiControllerInternal()->defaultMainWindow(),
                                       i18n( "Open Project" ) );
    }

    if ( !url.isValid() )
        return false;

    foreach( IProject* project, d->m_projects )
    {
        if ( url == project->projectFileUrl() )
        {
            if ( KMessageBox::questionYesNo( d->m_core->uiControllerInternal()->defaultMainWindow(),
                                             i18n( "Reopen the current project?" ) )
                    == KMessageBox::No )
            {
                return false;
            } else
            {
                closeProject( project );
            }
        }
    }

    //FIXME Create the hidden directory if it doesn't exist
    if ( loadProjectPart() )
    {
        //The project file has been opened.
        //Now we can load settings for all of the Core objects including this one!!
//         Core::loadSettings();
        d->m_core->pluginController()->loadPlugins( IPluginController::Project );
    }
    else
        return false;

    Project* project = new Project();
    if ( !project->open( url ) )
    {
        delete project;
        return false;
    }

    d->m_closeAllProjects->setEnabled(true);

    return true;
}

bool ProjectController::projectImportingFinished( IProject* project )
{
    IPlugin *managerPlugin = project->managerPlugin();
    QList<IPlugin*> pluglist;
    pluglist.append( managerPlugin );
    d->m_projectPlugins.insert( project, pluglist );

    ProjectFolderItem *topItem = project->projectItem();
    d->workspaceitem->insertRow( d->workspaceitem->rowCount(), topItem );

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
    QAction* qa = new QAction( project->name(), d->m_projectConfigAction );
    d->m_configActions.insert( project, qa );
    connect( qa, SIGNAL( triggered() ), d->m_signalMapper, SLOT( map() ) );
    d->m_signalMapper->setMapping( qa, project );
    d->m_projectConfigAction->addAction( qa );
    emit projectOpened( project );

    if (d->parseAllProjectSources) {
        // Add the project files to the background parser to be parsed.
        KUrl::List urlList;
        QList<ProjectFileItem*> files = project->files();
        foreach ( ProjectFileItem* file, files )
        {
            urlList.append( file->url() );
        }
        Core::self()->languageController()->backgroundParser()->addDocumentList( urlList );
    }

    return true;
}

bool ProjectController::closeProject( IProject* proj )
{

    if( !proj )
        return false;
    if( d->m_projects.indexOf( proj ) == -1 )
        return false;
//     if ( !d->m_isLoaded )
//         return false;

    emit projectClosing( proj );

    //The project file is being closed.

    //Now we can save settings for all of the Core objects including this one!!
//     Core::self()->saveSettings();

//     Core::self()->documentController() ->closeAllDocuments();

    // save the the project to open it automatically on startup if needed
//     d->m_lastProject = d->m_globalFile;

//     d->m_name = QString();
//     d->m_localFile.clear();
//     d->m_globalFile.clear();
//     d->m_projectsDir.clear();

//     if (d->m_core->uiControllerInternal()->defaultMainWindow())
//     {
//         if (MainWindow* mw = d->m_core->uiControllerInternal()->defaultMainWindow()) {
//             KActionCollection * ac = mw->actionCollection();
//             QAction * action;
// 
//             action = ac->action( "project_close" );
//             if( action )
//                 action->setEnabled( false );
//         }
//     }

    // close all opened files.
    Q_FOREACH( ProjectFileItem *fileItem, proj->files() )
    {
        Core::self()->documentControllerInternal()->closeDocument( fileItem->url() );
    }

    // delete project setting menu and its dialog.
    QPointer<QAction> configAction = d->m_configActions.take( proj );
    delete configAction;
    if( d->m_cfgDlgs.contains(proj) )
        delete d->m_cfgDlgs.take(proj);

    // start project manager that is used by this project. If that manager is being used
    // by other project also, don't unload that manager.
    // TODO remove all the plugins unique to this project.
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

    proj->close();
    proj->deleteLater(); //be safe when deleting
    d->m_projects.removeAll( proj );
    if (d->m_recentAction)
        d->m_recentAction->setCurrentAction( 0 );
//     d->m_isLoaded = false;

    // Unloading (and thus deleting) these plugins is not a good idea just yet
    // as we're being called by the view part and it gets deleted when we unload the plugin(s)
    // TODO: find a better place to unload
    if( d->m_projects.isEmpty() )
    {
        connect( proj, SIGNAL(destroyed(QObject*) ), this, SLOT(unloadAllProjectPlugins()) );

        if (d->m_closeAllProjects)
            d->m_closeAllProjects->setEnabled(false);
    }


    emit projectClosed( proj );

    //FIXME
    //     PluginController::self() ->changeProfile( m_oldProfileName );

    return true;
}

bool ProjectController::loadProjectPart()
{

    if( !d->m_projectPart )
    {
        KSharedConfig* config = KGlobal::config().data();

        KConfigGroup group = config->group( "General Options" );

        QString projectManager =
                group.readEntry( "ManagementView", "KDevProjectManagerView" );

        d->m_projectPart = d->m_core->pluginController()->loadPlugin( projectManager );
        if ( !d->m_projectPart )
        {
            KMessageBox::sorry( d->m_core->uiControllerInternal()->defaultMainWindow(),
                                i18n( "Could not load project management view plugin %1.",
                                      projectManager ) );
            return false;
        }
    }
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

}

void KDevelop::ProjectController::closeAllProjects()
{
    foreach (IProject* project, projects())
    {
        closeProject(project);
    }
}

#include "projectcontroller.moc"


