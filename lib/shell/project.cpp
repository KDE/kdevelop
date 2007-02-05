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
#include <ksimpleconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kio/job.h>
#include <kio/global.h>
#include <kmessagebox.h>
#include <kio/jobclasses.h>

#include "core.h"
#include "iplugin.h"
#include "filetemplate.h"
#include "mainwindow.h"
#include "ifilemanager.h"
#include "importprojectjob.h"
#include "projectmodel.h"
// #include "ipersistenthash.h"
#include "iplugincontroller.h"
#include "uicontroller.h"


namespace Koncrete
{

class ProjectPrivate
{
public:
    KUrl folder;
    IPlugin* manager;
//     IPersistentHash persistentHash;
    ProjectModel* model;
    QString name;
    QList<ProjectFileItem*> recurseFiles( ProjectItem * projectItem )
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
        model->resetModel();
    }

};

Project::Project( QObject *parent )
        : IProject( parent )
        , d( new ProjectPrivate )
{
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/Project", this, QDBusConnection::ExportScriptableSlots );

    d->manager = 0;
    d->model = new ProjectModel( this );
}

Project::~Project()
{
    delete d;
}

ProjectModel* Project::model() const
{
    return d->model;
}

QString Project::name() const
{
    return d->name;
}

KUrl Project::folder() const
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

    KSimpleConfig projectConfig( projectFileUrl.pathOrUrl(), true /*read only*/ );
    KConfigGroup projectGroup( &projectConfig, "General Options" );

    d->name = projectGroup.readEntry( "Name", projectFileUrl.fileName() );
    d->folder = projectFileUrl.directory();
    QString importerSetting = projectGroup.readEntry( "Importer", "GenericImporter" );

    //Get our importer
    IPluginController* pluginManager = Core::self()->pluginController();
    d->manager = pluginManager->pluginForExtension( "IFileManager", importerSetting );
    IFileManager* iface = 0;
    if ( d->manager )
        iface = d->manager->extension<IFileManager>();
    else
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "Could not load project importing plugin %1.",
                                  importerSetting ) );
        d->manager = 0;
        return false;
    }
    if ( d->manager && iface )
    {
        QStandardItem* topItem = iface->import( d->model, d->folder );
        d->model->insertRow( 0, topItem );

        ImportProjectJob* importJob = new ImportProjectJob( d->model->item( 0, 0 ), iface );
        connect( importJob, SIGNAL( result( KJob* ) ), this, SLOT( importDone( KJob* ) ) );
        importJob->start(); //be asynchronous
    }
    else
    {
        KMessageBox::sorry( Core::self()->uiControllerInternal()->defaultMainWindow(),
                            i18n( "project importing plugin doesn't support the IFileManager interface." ) );
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
    QList<QStandardItem*> itemList = d->model->takeRow( 0 );
    qDeleteAll( itemList );

}
bool Project::inProject( const KUrl& url ) const
{
    IFileManager* iface = d->manager->extension<IFileManager>();
    ProjectFolderItem *top = iface->top();
    KUrl u = top->url();
    if ( u.protocol() != url.protocol() || u.host() != url.host() )
        return false;
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

    return false;
}

QList<ProjectFileItem*> Project::allFiles()
{
    QStandardItem* rootItem = d->model->item( 0, 0 );
    ProjectItem* projectItem = dynamic_cast<ProjectItem*>( rootItem );
    QList<ProjectFileItem*> files;
    if ( projectItem )
        files = d->recurseFiles( projectItem );

    return files;
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

IFileManager* Project::fileManager() const
{
    return d->manager->extension<IFileManager>();
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
