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
#include <kio/global.h>
#include <kmessagebox.h>
#include <kio/jobclasses.h>

#include "core.h"
#include "iplugin.h"
#include "mainwindow.h"
#include "iprojectfilemanager.h"
#include "iprojectcontroller.h"
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
    IPlugin* manager;
//     IPersistentHash persistentHash;
    ProjectItem* topItem;
    QString name;
    KUrl localFile;
    KUrl globalFile;
    KSharedConfig::Ptr m_cfg;
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

    void importDone( KJob* job )
    {
        job->deleteLater();
    }

};

Project::Project( QObject *parent )
        : IProject( parent )
        , d( new ProjectPrivate )
{
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/Project", this, QDBusConnection::ExportScriptableSlots );

    d->manager = 0;
    d->topItem = 0;
}

Project::~Project()
{
    delete d;
}

QString Project::name() const
{
    return d->name;
}

KUrl Project::projectConfigFile() const
{
    return d->localFile;
}

KUrl Project::projectDefaultsConfigFile() const
{
    return d->globalFile;
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

    KConfig projectConfig( projectFileUrl.pathOrUrl() );
    KConfigGroup projectGroup( &projectConfig, "General Options" );

    d->name = projectGroup.readEntry( "Name", projectFileUrl.fileName() );
    d->folder = projectFileUrl.directory();
    QString managerSetting = projectGroup.readEntry( "Manager", "KDevGenericManager" );

    //Get our importer
    IPluginController* pluginManager = Core::self()->pluginController();
    d->manager = pluginManager->pluginForExtension( "IProjectFileManager", managerSetting );
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
        ProjectModel* model = Core::self()->projectController()->projectModel();
        d->topItem = iface->import( this );
        model->insertRow( model->rowCount(), d->topItem );

        ImportProjectJob* importJob = new ImportProjectJob( d->topItem, iface );
        connect( importJob, SIGNAL( result( KJob* ) ), this, SLOT( importDone( KJob* ) ) );
        importJob->start(); //be asynchronous
    }
    else
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "project importing plugin doesn't support the IProjectFileManager interface." ) );
        delete d->manager;
        d->manager = 0;
        return false;
    }
    d->globalFile = projectFileUrl;
    d->localFile = KUrl( projectFileUrl.directory( KUrl::AppendTrailingSlash )
                                  + ".kdev4/"
                                  + projectFileUrl.fileName() );
    d->m_cfg = KSharedConfig::openConfig( d->globalFile.path() );
    d->m_cfg->setExtraConfigFiles( QStringList() << d->localFile.path() );
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

}
bool Project::inProject( const KUrl& url ) const
{
    KUrl u = d->topItem->url();
    if ( u.protocol() != url.protocol() || u.host() != url.host() )
        return false;

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
                            return true; //we found it
                            break;
                        }
                    }
                    return false; //not in the project
                }
                top = parent;
            }
        }
    }
    return false;
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
    QList<ProjectFileItem*> files;
    if ( d->topItem )
        files = d->recurseFiles( d->topItem );

    foreach (ProjectFileItem *file, files)
    {
        if (file->url() == url)
            return file;
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
    if ( KUrl::isRelativeUrl( relativeUrl.path() ) )
        return KUrl( folder(), relativeUrl.path() );

    return relativeUrl;
}

IProjectFileManager* Project::fileManager() const
{
    return d->manager->extension<IProjectFileManager>();
}

void Project::setFileManager( IPlugin* newManager )
{
    d->manager = newManager;
}

// PersistentHash * Project::persistentHash() const
// {
//     return &d->persistentHash;
// }

}

#include "project.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
