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

#include <shell/core.h>
#include <shell/uicontroller.h>
#include <shell/projectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>
#include <project/importprojectjob.h>
#include <project/projectmodel.h>

namespace KDevelop
{

class DumbProjectPrivate
{
public:
    KUrl folder;
    KUrl projectFileUrl;
    KUrl developerFileUrl;
    QString developerTempFile;
    QString projectTempFile;
    KTemporaryFile* tmp;
    IPlugin* manager;
    ProjectFolderItem* topItem;

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

    QList<ProjectFileItem*> filesForUrlInternal( const KUrl& url, ProjectFolderItem* folder ) const
    {
        QList<ProjectFileItem*> files;
        if( !folder )
            return files;
        // Check top level files
        const Path path(url);
        foreach( ProjectFileItem* file, folder->fileList() )
        {
            if( file->path() == path )
            {
                files << file;
            }
        }

        // Check top level targets
        foreach( ProjectTargetItem* target, folder->targetList() )
        {
            foreach( ProjectFileItem* file, target->fileList() )
            {
                if( file->path() == path )
                {
                    files << file;
                }
            }
        }

        foreach( ProjectFolderItem* top, folder->folderList() )
        {
            files += filesForUrlInternal( url, top );
        }
        return files;
    }

};

DumbProject::DumbProject( QObject *parent )
        : Project( parent )
        , d( new DumbProjectPrivate )
{
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/Project", this, QDBusConnection::ExportScriptableSlots );
}

DumbProject::~DumbProject()
{
    delete d;
}

QString DumbProject::name() const
{
    return QString("dumbProject");
}

QString DumbProject::developerTempFile() const
{
    return QString();
}

QString DumbProject::projectTempFile() const
{
    return QString();
}

KSharedConfig::Ptr DumbProject::projectConfiguration() const
{
    return KSharedPtr<KSharedConfig>();
}

const KUrl DumbProject::folder() const
{
    return KUrl();
}

void DumbProject::reloadModel()
{}

bool DumbProject::open( const KUrl& projectFileUrl_ )
{
    Q_UNUSED( projectFileUrl_ );
    return true;
}

void DumbProject::close()
{}

bool DumbProject::inProject( const KUrl& url ) const
{
    return ( !filesForUrl( url ).isEmpty() || url.equals( d->topItem->path().toUrl(), KUrl::CompareWithoutTrailingSlash ) );
}

ProjectFileItem* DumbProject::fileAt( int num ) const
{
    QList<ProjectFileItem*> files;
    if ( d->topItem )
        files = d->recurseFiles( d->topItem );

    if( !files.isEmpty() && num >= 0 && num < files.count() )
        return files.at( num );
    return 0;
}

QList<ProjectFileItem *> KDevelop::DumbProject::files() const
{
    QList<ProjectFileItem *> files;
    if ( d->topItem )
        files = d->recurseFiles( d->topItem );
    return files;
}

QList<ProjectFileItem*> DumbProject::filesForUrl(const KUrl& url) const
{
    const KUrl u = d->topItem->path().toUrl();
    if ( u.protocol() != url.protocol() || u.host() != url.host() )
        return QList<ProjectFileItem*>();

    return d->filesForUrlInternal( url, d->topItem );
}

int DumbProject::fileCount() const
{
    QList<ProjectFileItem*> files;
    if ( d->topItem )
        files = d->recurseFiles( d->topItem );
    return files.count();
}

KUrl DumbProject::relativeUrl( const KUrl& absolute ) const
{
    kDebug() << "Creating relative url between: " << folder() << absolute;
    return KUrl::relativeUrl( folder(), absolute );
}

IProjectFileManager* DumbProject::projectFileManager() const
{
    return d->manager->extension<IProjectFileManager>();
}

IBuildSystemManager* DumbProject::buildSystemManager() const
{
    return dynamic_cast<IBuildSystemManager*>(d->manager);
}

IPlugin* DumbProject::managerPlugin() const
{
  return d->manager;
}

void DumbProject::setManagerPlugin( IPlugin* manager )
{
    d->manager = manager;
    d->topItem = dynamic_cast<IProjectFileManager*>(manager)->import( this );
}

// PersistentHash * DumbProject::persistentHash() const
// {
//     return &d->persistentHash;
// }

KUrl DumbProject::projectFileUrl() const
{
    return d->projectFileUrl;
}

KUrl DumbProject::developerFileUrl() const
{
    return d->developerFileUrl;
}

ProjectFolderItem* DumbProject::projectItem() const
{
    return d->topItem;
}

IPlugin* DumbProject::versionControlPlugin() const
{
    return 0;
}

}

#include "project.moc"

