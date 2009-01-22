/* This file is part of the KDE project
   Copyright 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2002 Simon Hausmann <hausmann@kde.org>
   Copyright 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright     2006 Matt Rogers <mattr@kde.org>
   Copyright     2007 Andreas Pakulat <apaku@gmx.de>

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

#include "project.h"

#include <QSet>
#include <QtGlobal>
#include <QFileInfo>
#include <QtDBus/QtDBus>
#include <QStandardItemModel>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kmessagebox.h>
#include <kio/jobclasses.h>
#include <ktemporaryfile.h>
#include <kdebug.h>

#include <project/interfaces/iprojectfilemanager.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iplugin.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <project/importprojectjob.h>
#include <project/projectmodel.h>
#include <language/duchain/indexedstring.h>
#include <vcs/interfaces/ibasicversioncontrol.h>

#include "core.h"
#include "mainwindow.h"
#include "projectcontroller.h"
#include "uicontroller.h"

namespace KDevelop
{

class ProjectPrivate
{
public:
    KUrl folder;
    KUrl projectFileUrl;
    KUrl developerFileUrl;
    QString developerTempFile;
    QString projectTempFile;
    KTemporaryFile* tmp;
    IPlugin* manager;
    IPlugin* vcsPlugin;
    ProjectFolderItem* topItem;
    QString name;
    KSharedConfig::Ptr m_cfg;
    IProject *project;
    QSet<KDevelop::IndexedString> fileSet;
    bool reloading;
    bool scheduleReload;

    void reloadDone()
    {
        reloading = false;
        Core::self()->projectController()->projectModel()->appendRow(topItem);
        if (scheduleReload) {
            scheduleReload = false;
            project->reloadModel();
        }
    }

    QList<ProjectFileItem*> recurseFiles( ProjectBaseItem * projectItem )
    {
        QList<ProjectFileItem*> files;
        if ( ProjectFolderItem * folder = projectItem->folder() )
        {
            QList<ProjectFolderItem*> folder_list = folder->folderList();
            for ( QList<ProjectFolderItem*>::Iterator it = folder_list.begin(); it != folder_list.end(); ++it )
            {
                files += recurseFiles( ( *it ) );
            }

            QList<ProjectTargetItem*> target_list = folder->targetList();
            for ( QList<ProjectTargetItem*>::Iterator it = target_list.begin(); it != target_list.end(); ++it )
            {
                files += recurseFiles( ( *it ) );
            }

            files += folder->fileList();
        }
        else if ( ProjectTargetItem * target = projectItem->target() )
        {
            files += target->fileList();
        }
        else if ( ProjectFileItem * file = projectItem->file() )
        {
            files.append( file );
        }
        return files;
    }

    QList<ProjectBaseItem*> itemsForUrlInternal( const KUrl& url, ProjectFolderItem* folder ) const
    {
        QList<ProjectBaseItem*> files;
        if( !folder )
            return files;

        if( folder->url().equals( url, KUrl::CompareWithoutTrailingSlash ) )
        {
            files << folder;
        }

        // Check top level files
        foreach( ProjectFileItem* file, folder->fileList() )
        {
            if( file->url() == url )
            {
                files << file;
            }
        }

        // Check top level targets
        foreach( ProjectTargetItem* target, folder->targetList() )
        {
            foreach( ProjectFileItem* file, target->fileList() )
            {
                if( file->url() == url )
                {
                    files << file;
                }
            }
        }

        foreach( ProjectFolderItem* top, folder->folderList() )
        {
            files += itemsForUrlInternal( url, top );
        }
        return files;
    }
    QList<ProjectBaseItem*> itemsForUrl( const KUrl& url ) const
    {
        // TODO: This is moderately efficient, but could be much faster with a
        // QHash<QString, ProjectFolderItem> member. Would it be worth it?
        KUrl u = topItem->url();
        if ( u.protocol() != url.protocol() || u.host() != url.host() )
            return QList<ProjectBaseItem*>();
    
        return itemsForUrlInternal( url, topItem );
    }


    void importDone( KJob* )
    {
        ProjectController* projCtrl = Core::self()->projectControllerInternal();
        projCtrl->projectModel()->appendRow(topItem);
        projCtrl->projectImportingFinished( project );
    }

    void initProjectUrl(const KUrl& projectFileUrl_)
    {
        // helper method for open()
        projectFileUrl = projectFileUrl_;
        if ( projectFileUrl.isLocalFile() )
        {
            QString path = QFileInfo( projectFileUrl.toLocalFile() ).canonicalFilePath();
            if ( !path.isEmpty() )
                projectFileUrl.setPath( path );
        }
    }

    bool initProjectFiles()
    {
        KIO::StatJob* statJob = KIO::stat( projectFileUrl, KIO::HideProgressInfo );
        if ( !statJob->exec() ) //be sync for right now
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Unable to load the project file %1",
                                  projectFileUrl.pathOrUrl() ) );
            return false;
        }


        developerFileUrl = KUrl( projectFileUrl.directory( KUrl::AppendTrailingSlash ) );
        developerFileUrl.addPath(".kdev4");
        developerFileUrl.addPath( projectFileUrl.fileName() );

        statJob = KIO::stat( developerFileUrl, KIO::HideProgressInfo );
        if( !statJob->exec() )
        {
            KUrl dir = KUrl( projectFileUrl.directory( KUrl::AppendTrailingSlash ) + ".kdev4");
            statJob = KIO::stat( dir, KIO::HideProgressInfo );
            if( !statJob->exec() )
            {
                KIO::SimpleJob* mkdirJob = KIO::mkdir( dir );
                if( !mkdirJob->exec() )
                {
                    KMessageBox::sorry(
                        Core::self()->uiController()->activeMainWindow(),
                        i18n("Unable to create hidden dir (%1) for developer file",
                        dir.pathOrUrl() )
                        );
                    return false;
                }
            }
        }

        if( !KIO::NetAccess::download( projectFileUrl, projectTempFile,
                        Core::self()->uiController()->activeMainWindow() ) )
        {
            KMessageBox::sorry( Core::self()->uiController()->activeMainWindow(),
                            i18n("Unable to get project file: %1",
                            projectFileUrl.pathOrUrl() ) );
            return false;
        }

        statJob = KIO::stat( developerFileUrl, KIO::HideProgressInfo );
        if( !statJob->exec() || !KIO::NetAccess::download( developerFileUrl, developerTempFile,
            Core::self()->uiController()->activeMainWindow() ) )
        {
            tmp = new KTemporaryFile();
            tmp->open();
            developerTempFile = tmp->fileName();
            tmp->close();
        }
        return true;
    }

    KConfigGroup initKConfigObject()
    {
        // helper method for open()
        kDebug() << "Creating KConfig object for project files" << developerTempFile << projectTempFile;
        m_cfg = KSharedConfig::openConfig( developerTempFile );
        m_cfg->addConfigSources( QStringList() << projectTempFile );
        KConfigGroup projectGroup( m_cfg, "Project" );
        return projectGroup;
    }

    bool projectNameUsed(const KConfigGroup& projectGroup)
    {
        // helper method for open()
        name = projectGroup.readEntry( "Name", projectFileUrl.fileName() );
        if( Core::self()->projectController()->isProjectNameUsed( name ) ) 
        {
            kWarning() << "Trying to open a project with a name thats already used by another open project";
            return true;
        }
        return false;
    }

    IProjectFileManager* fetchFileManager(const KConfigGroup& projectGroup)
    {
        if (manager)
        {
            IProjectFileManager* iface = 0;
            iface = manager->extension<KDevelop::IProjectFileManager>();
            Q_ASSERT(iface);
            return iface;
        }

        // helper method for open()
        QString managerSetting = projectGroup.readEntry( "Manager", "KDevGenericManager" );

        //Get our importer
        IPluginController* pluginManager = Core::self()->pluginController();
        manager = pluginManager->pluginForExtension( "org.kdevelop.IProjectFileManager", managerSetting );
        IProjectFileManager* iface = 0;
        if ( manager )
            iface = manager->extension<IProjectFileManager>();
        else
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Could not load project management plugin %1.",
                                  managerSetting ) );
            manager = 0;
        }
        if (iface == 0)
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "project importing plugin (%1) does not support the IProjectFileManager interface.", managerSetting ) );
            delete manager;
            manager = 0;
        }
        return iface;
    }

    void loadVersionControlPlugin(KConfigGroup& projectGroup)
    {
        // helper method for open()
        IPluginController* pluginManager = Core::self()->pluginController();
        if( projectGroup.hasKey( "VersionControlSupport" ) )
        {
            QString vcsPluginName = projectGroup.readEntry("VersionControlSupport", "");
            if( !vcsPluginName.isEmpty() )
            {
                vcsPlugin = pluginManager->pluginForExtension( "org.kdevelop.IBasicVersionControl", vcsPluginName );
            }
        } else 
        {
            foreach( IPlugin* p, pluginManager->allPluginsForExtension( "org.kdevelop.IBasicVersionControl" ) )
            {
                IBasicVersionControl* iface = p->extension<KDevelop::IBasicVersionControl>();
                if( iface && iface->isVersionControlled( topItem->url() ) )
                {
                    vcsPlugin = p;
                    projectGroup.writeEntry("VersionControlSupport", pluginManager->pluginInfo( vcsPlugin ).pluginName() );
                    projectGroup.sync();
                }
            }
        }

    }

    bool importTopItem(IProjectFileManager* fileManager)
    {
        if (!fileManager)
        {
            return false;
        }
        topItem = fileManager->import( project );
        if( !topItem )
        {
            KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                                i18n("Could not open project") );
            return false;
        }
        topItem->setIcon();
        return true;
    }

};

Project::Project( QObject *parent )
        : IProject( parent )
        , d( new ProjectPrivate )
{
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/Project", this, QDBusConnection::ExportScriptableSlots );

    d->project = this;
    d->manager = 0;
    d->topItem = 0;
    d->tmp = 0;
    d->vcsPlugin = 0;
    d->reloading = false;
    d->scheduleReload = false;
}

Project::~Project()
{
    delete d;
}

QString Project::name() const
{
    return d->name;
}

QString Project::developerTempFile() const
{
    return d->developerTempFile;
}

QString Project::projectTempFile() const
{
    return d->projectTempFile;
}

KSharedConfig::Ptr Project::projectConfiguration() const
{
    d->m_cfg->reparseConfiguration();
    return d->m_cfg;
}

const KUrl Project::folder() const
{
    return d->folder;
}

void Project::reloadModel()
{
    if (d->reloading) {
        d->scheduleReload = true;
        return;
    }
    d->reloading = true;
    d->fileSet.clear();

    ProjectModel* model = Core::self()->projectController()->projectModel();
    model->removeRow( d->topItem->row() );

    IProjectFileManager* iface = d->manager->extension<IProjectFileManager>();
    if (!d->importTopItem(iface))
    {
            d->reloading = false;
            d->scheduleReload = false;
            return;
    }

    ImportProjectJob* importJob = new ImportProjectJob( d->topItem, iface );
    connect(importJob, SIGNAL(finished(KJob*)), SLOT(reloadDone()));
    Core::self()->runController()->registerJob( importJob );
}

bool Project::open( const KUrl& projectFileUrl_ )
{
    d->initProjectUrl(projectFileUrl_);
    if (!d->initProjectFiles())
        return false;

    KConfigGroup projectGroup = d->initKConfigObject();
    if (d->projectNameUsed(projectGroup))
        return false;
    d->folder = d->projectFileUrl.directory( KUrl::AppendTrailingSlash );

    IProjectFileManager* iface = d->fetchFileManager(projectGroup);
    if (!iface)
        return false;

    Q_ASSERT(d->manager);
    if (!d->importTopItem(iface) )
        return false;

    d->loadVersionControlPlugin(projectGroup);
    ImportProjectJob* importJob = new ImportProjectJob( d->topItem, iface );
    connect( importJob, SIGNAL( result( KJob* ) ), this, SLOT( importDone( KJob* ) ) );
    Core::self()->runController()->registerJob( importJob );
    return true;
}

void Project::close()
{
    Core::self()->projectController()->projectModel()->removeRow( d->topItem->row() );

    if( d->tmp )
    {
        d->tmp->close();
    }

    if( !KIO::NetAccess::upload( d->developerTempFile, d->developerFileUrl,
                Core::self()->uiController()->activeMainWindow() ) )
    {
        KMessageBox::sorry( Core::self()->uiController()->activeMainWindow(),
                    i18n("Could not store developer specific project configuration.\n"
                         "Attention: The project settings you changed will be lost."
                    ) );
    }
    delete d->tmp;
}

bool Project::inProject( const KUrl& url ) const
{
    if( url.isLocalFile() && QFileInfo( url.path() ).isFile() )
        return d->fileSet.contains( IndexedString( url ) );
    return ( !d->itemsForUrl( url ).isEmpty() );
}

ProjectFileItem* Project::fileAt( int num ) const
{
    QList<ProjectFileItem*> files;
    if ( d->topItem )
        files = d->recurseFiles( d->topItem );

    if( !files.isEmpty() && num >= 0 && num < files.count() )
        return files.at( num );
    return 0;
}

QList<ProjectFileItem *> KDevelop::Project::files() const
{
    QList<ProjectFileItem *> files;
    if ( d->topItem )
        files = d->recurseFiles( d->topItem );
    return files;
}

QList<ProjectFileItem*> Project::filesForUrl(const KUrl& url) const
{
    QList<ProjectFileItem*> items;
    foreach(ProjectBaseItem* item,  d->itemsForUrl( url ) )
    {
        if( item->type() == ProjectBaseItem::File )
            items << dynamic_cast<ProjectFileItem*>( item );
    }
    return items;
}

QList<ProjectFolderItem*> Project::foldersForUrl(const KUrl& url) const
{
    QList<ProjectFolderItem*> items;
    foreach(ProjectBaseItem* item,  d->itemsForUrl( url ) )
    {
        if( item->type() == ProjectBaseItem::Folder )
            items << dynamic_cast<ProjectFolderItem*>( item );
    }
    return items;
}

int Project::fileCount() const
{
    QList<ProjectFileItem*> files;
    if ( d->topItem )
        files = d->recurseFiles( d->topItem );
    return files.count();
}

KUrl Project::relativeUrl( const KUrl& absolute ) const
{
    kDebug() << "Creating relative url between: " << folder() << absolute;
    return KUrl::relativeUrl( folder(), absolute );
}

KUrl Project::urlRelativeToProject( const KUrl & relativeUrl ) const
{
    if ( KUrl::isRelativeUrl( relativeUrl.toLocalFile() ) )
        return KUrl( folder(), relativeUrl.toLocalFile() );

    return relativeUrl;
}

IProjectFileManager* Project::projectFileManager() const
{
    return d->manager->extension<IProjectFileManager>();
}

IBuildSystemManager* Project::buildSystemManager() const
{
    return d->manager->extension<IBuildSystemManager>();
}

IPlugin* Project::managerPlugin() const
{
  return d->manager;
}

void Project::setManagerPlugin( IPlugin* manager )
{
    d->manager = manager;
}

KUrl Project::projectFileUrl() const
{
    return d->projectFileUrl;
}

KUrl Project::developerFileUrl() const
{
    return d->developerFileUrl;
}

ProjectFolderItem* Project::projectItem() const
{
    return d->topItem;
}

IPlugin* Project::versionControlPlugin() const
{
    return d->vcsPlugin;
}


void Project::addToFileSet( const IndexedString& file )
{
    d->fileSet.insert( file );
}

void Project::removeFromFileSet( const IndexedString& file )
{
    d->fileSet.remove( file );
}

QSet<IndexedString> Project::fileSet() const
{
    return d->fileSet;
}

} // namespace KDevelop

#include "project.moc"

