/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>
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

#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/global.h>
#include <kmessagebox.h>
#include <kio/jobclasses.h>
#include <ktemporaryfile.h>

#include "iprojectfilemanager.h"

#include "core.h"
#include "iplugin.h"
#include "mainwindow.h"
#include "projectcontroller.h"
#include "importprojectjob.h"
#include "projectmodel.h"
// #include "ipersistenthash.h"
#include "iplugincontroller.h"
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
//     IPersistentHash persistentHash;
    ProjectItem* topItem;
    QString name;
    KSharedConfig::Ptr m_cfg;
    IProject *project;

    QList<ProjectFileItem*> recurseFiles( ProjectBaseItem * projectItem )
    {
        QList<ProjectFileItem*> files;
        if ( ProjectFolderItem * folder = projectItem->folder() )
        {
            QList<ProjectFolderItem*> folder_list = folder->folderList();
            for ( QList<ProjectFolderItem*>::Iterator it = folder_list.begin(); it != folder_list.end(); ++it )
                files += recurseFiles( ( *it ) );

            QList<ProjectTargetItem*> target_list = folder->targetList();
            for ( QList<ProjectTargetItem*>::Iterator it = target_list.begin(); it != target_list.end(); ++it )
                files += recurseFiles( ( *it ) );

            QList<ProjectFileItem*> file_list = folder->fileList();
            for ( QList<ProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it )
                files += recurseFiles( ( *it ) );
        }
        else if ( ProjectTargetItem * target = projectItem->target() )
        {
            QList<ProjectFileItem*> file_list = target->fileList();
            for ( QList<ProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it )
                files += recurseFiles( ( *it ) );
        }
        else if ( ProjectFileItem * file = projectItem->file() )
        {
            files.append( file );
        }
        return files;
    }

    void importDone( KJob* /*job*/ )
    {
        Core::self()->projectControllerInternal()->projectImportingFinished( project );
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

// void Project::setLocalFile( const KUrl& u )
// {
//     d->localFile = u;
// }

// void Project::setGlobalFile( const KUrl& u )
// {
//     d->globalFile = u;
// }

const KUrl& Project::folder() const
{
    return d->folder;
}

bool Project::open( const KUrl& projectFileUrl )
{
    KIO::StatJob* statJob = KIO::stat( projectFileUrl );
    if ( !statJob->exec() ) //be sync for right now
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Unable to load the project file %1",
                                  projectFileUrl.pathOrUrl() ) );
        return false;
    }

    d->projectFileUrl = projectFileUrl;
    d->developerFileUrl = KUrl( projectFileUrl.directory( KUrl::AppendTrailingSlash )
                                  + ".kdev4/"
                                  + projectFileUrl.fileName() );

    statJob = KIO::stat( d->developerFileUrl );
    if( !statJob->exec() )
    {
        KUrl dir = KUrl( projectFileUrl.directory( KUrl::AppendTrailingSlash ) + ".kdev4");
        statJob = KIO::stat( projectFileUrl );
        if( !statJob->exec() )
        {
            KIO::SimpleJob* mkdirJob = KIO::mkdir( dir );
            if( !mkdirJob->exec() )
            {
                KMessageBox::sorry(
                        Core::self()->uiControllerInternal()->activeMainWindow(),
                        i18n("Unable to create hidden dir (%1) for developer file",
                        dir.pathOrUrl() )
                        );
                return false;
            }
        }
    }

    if( !KIO::NetAccess::download( d->projectFileUrl, d->projectTempFile,
                        Core::self()->uiControllerInternal()->activeMainWindow() ) )
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->activeMainWindow(),
                            i18n("Unable to get project file: %1",
                            d->projectFileUrl.pathOrUrl() ) );
        return false;

    }

    statJob = KIO::stat( d->developerFileUrl );
    if( !statJob->exec() || !KIO::NetAccess::download( d->developerFileUrl, d->developerTempFile,
            Core::self()->uiControllerInternal()->activeMainWindow() ) )
    {

        d->tmp = new KTemporaryFile();
        d->tmp->open();
        d->tmp->close();
        d->developerTempFile = d->tmp->fileName();
    }

    d->m_cfg = KSharedConfig::openConfig( d->projectTempFile );
    d->m_cfg->setExtraConfigFiles( QStringList() << d->developerTempFile );

    KConfigGroup projectGroup( d->m_cfg, "Project" );

    d->name = projectGroup.readEntry( "Name", projectFileUrl.fileName() );
    d->folder = projectFileUrl.directory();
    QString managerSetting = projectGroup.readEntry( "Manager", "KDevGenericManager" );

    //Get our importer
    IPluginController* pluginManager = Core::self()->pluginController();
    d->manager = pluginManager->pluginForExtension( "org.kdevelop.IProjectFileManager", managerSetting );
    IProjectFileManager* iface = 0;
    if ( d->manager )
        iface = d->manager->extension<IProjectFileManager>();
    else
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Could not load project management plugin %1.",
                                  managerSetting ) );
        d->manager = 0;
        return false;
    }
    if ( d->manager && iface )
    {
//         ProjectModel* model = Core::self()->projectController()->projectModel();
        d->topItem = iface->import( this );
//         model->insertRow( model->rowCount(), d->topItem );

        ImportProjectJob* importJob = new ImportProjectJob( d->topItem, iface );
        connect( importJob, SIGNAL( result( KJob* ) ), this, SLOT( importDone( KJob* ) ) );
        importJob->start(); //be asynchronous
    }
    else
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "project importing plugin (%1) does not support the IProjectFileManager interface.", managerSetting ) );
        delete d->manager;
        d->manager = 0;
        return false;
    }

    return true;
}

void Project::close()
{
    KPluginInfo* pluginInfo = Core::self()->pluginController()->pluginInfo( d->manager );
    if ( pluginInfo )
    {
        Core::self()->pluginController()->unloadPlugin( pluginInfo->pluginName() );
    }

    //the manager plugin will be deleted in the plugin controller, so just set
    //the manager to zero.
    d->manager = 0;
    QList<QStandardItem*> itemList = Core::self()->projectController()->projectModel()->takeRow( d->topItem->row() );
    qDeleteAll( itemList );

    if( d->tmp )
    {
        d->tmp->close();
    }

    if( !KIO::NetAccess::upload( d->developerTempFile, d->developerFileUrl,
                Core::self()->uiControllerInternal()->activeMainWindow() ) )
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->activeMainWindow(),
                    i18n("Could not store developer specific project configuration.\n"
                         "Attention: The project settings you changed will be lost."
                    ) );
    }
    delete d->tmp;
}

bool Project::inProject( const KUrl& url ) const
{
    return fileForUrl( url ) != 0;
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

ProjectFileItem *Project::fileForUrl(const KUrl& url) const
{
    // TODO: This is moderately efficient, but could be much faster with a
    // QHash<QString, ProjectFolderItem> member. Would it be worth it?

    KUrl u = d->topItem->url();
    if ( u.protocol() != url.protocol() || u.host() != url.host() )
        return 0;

    foreach( ProjectFolderItem* top, d->topItem->folderList() )
    {
        while ( top )
        {
            u = top->url();
            if ( u.isParentOf( url ) )
            {
                ProjectFolderItem *parent = 0L;
                QList<ProjectFolderItem*> folder_list = top->folderList();
                foreach( ProjectFolderItem *folder, folder_list )
                {
                    if ( folder->url().isParentOf( url ) )
                    {
                        parent = folder;
                        break;
                    }
                }
                if ( !parent ) //the subfolders are not parent of url
                {
                    QList<ProjectFileItem*> file_list = top->fileList();
                    foreach( ProjectFileItem *file, file_list )
                    {
                        if ( file->url() == url )
                        {
                            return file; //we found it
                            break;
                        }
                    }
                    return 0; //not in the project
                }
                top = parent;
            }
        }
    }
    return 0;
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
    return KUrl::relativeUrl( folder(), absolute );
}

KUrl Project::urlRelativeToProject( const KUrl & relativeUrl ) const
{
    if ( KUrl::isRelativeUrl( relativeUrl.toLocalFile() ) )
        return KUrl( folder(), relativeUrl.toLocalFile() );

    return relativeUrl;
}

IProjectFileManager* Project::fileManager() const
{
    return d->manager->extension<IProjectFileManager>();
}

IPlugin* Project::managerPlugin() const
{
  return d->manager;
}

void Project::setFileManager( IPlugin* newManager )
{
    d->manager = newManager;
}

// PersistentHash * Project::persistentHash() const
// {
//     return &d->persistentHash;
// }

KUrl Project::projectFileUrl() const
{
    return d->projectFileUrl;
}

KUrl Project::developerFileUrl() const
{
    return d->developerFileUrl;
}

ProjectItem* Project::projectItem() const
{
    return d->topItem;
}

}

#include "project.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
