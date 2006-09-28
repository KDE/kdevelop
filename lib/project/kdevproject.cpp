/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>
   Copyright     2006 Matt Rogers <mattr@kde.org>

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

#include "kdevproject.h"

#include <QSet>
#include <QtGlobal>
#include <QFileInfo>
#include <QtDBus/QtDBus>
#include <QStandardItemModel>

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kio/job.h>
#include <kinstance.h>
#include <kio/global.h>
#include <kmessagebox.h>
#include <kio/jobclasses.h>

#include "kdevcore.h"
#include "filetemplate.h"
#include "kdevmainwindow.h"
#include "kdevfilemanager.h"
#include "importprojectjob.h"
#include "kdevprojectmodel.h"
#include "kdevpersistenthash.h"
#include "kdevplugincontroller.h"



class KDevProject::Private
{
public:
    KUrl folder;
    KDevFileManager* manager;
    KDevPersistentHash persistentHash;
    KDevProjectModel* model;
    QString name;
};

KDevProject::KDevProject(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    QDBusConnection::sessionBus().registerObject("/org/kdevelop/Project", this, QDBusConnection::ExportScriptableSlots);

    d->manager = 0;
    d->model = new KDevProjectModel( this );
}

KDevProject::~KDevProject()
{
    delete d;
}

KDevProjectModel* KDevProject::model() const
{
    return d->model;
}

QString KDevProject::name() const
{
    return d->name;
}

KUrl KDevProject::folder() const
{
    return d->folder;
}

bool KDevProject::open( const KUrl& projectFileUrl )
{
    KIO::StatJob* statJob = KIO::stat( projectFileUrl );
    if ( !statJob->exec() ) //be sync for right now
    {
        KMessageBox::sorry( KDevCore::mainWindow(),
                            i18n( "Unable to load the project file %1",
                                  projectFileUrl.pathOrUrl() ) );
        return false;
    }

    KSimpleConfig projectConfig( projectFileUrl.pathOrUrl(), true /*read only*/);
    KConfigGroup projectGroup( &projectConfig, "General Options");

    d->name = projectGroup.readEntry("Name", projectFileUrl.fileName());
    d->folder = projectFileUrl.directory();
    QString importerSetting = projectGroup.readEntry("Importer", "KDevGenericImporter");

    //Get our importer
    KDevPluginController* pluginManager = KDevPluginController::self();
    d->manager = qobject_cast<KDevFileManager*>( pluginManager->loadPlugin( importerSetting ) );

    if ( d->manager )
    {
        QStandardItem* topItem = d->manager->import( d->model, d->folder );
        d->model->insertRow(0, topItem);

        ImportProjectJob* importJob = new ImportProjectJob( d->model->item(0,0), d->manager );
        connect( importJob, SIGNAL(result(KJob*)), this, SLOT(importDone(KJob*)));
        importJob->start(); //be asynchronous
    }
    return true;
}

void KDevProject::close()
{
    KPluginInfo* pluginInfo = KDevPluginController::self()->pluginInfo( d->manager );
    if ( pluginInfo )
    {
        KDevPluginController::self()->unloadPlugin( pluginInfo->pluginName() );
    }

    //the manager plugin will be deleted in the plugin controller, so just set
    //the manager to zero.
    d->manager = 0;
    QList<QStandardItem*> itemList= d->model->takeRow(0);
    qDeleteAll(itemList);

}
bool KDevProject::inProject( const KUrl& url ) const
{
  KDevProjectFolderItem *top = d->manager->top();
  KUrl u = top->url();
  if (u.protocol() != url.protocol() || u.host() != url.host())
    return false;
  while (top)
  {
    u = top->url();
    if (u.isParentOf(url))
    {
      KDevProjectFolderItem *parent = 0L;
      QList<KDevProjectFolderItem*> folder_list = top->folderList();
      foreach (KDevProjectFolderItem *folder, folder_list)
      {
        if (folder->url().isParentOf(url))
        {
          parent = folder;
          break;
        }
      }
      if (!parent) //the subfolders are not parent of url
      {
        QList<KDevProjectFileItem*> file_list= top->fileList();
        foreach (KDevProjectFileItem *file, file_list)
        {
          if (file->url() == url)
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

QList<KDevProjectFileItem*> KDevProject::allFiles()
{
    QStandardItem* rootItem = d->model->item(0,0);
    KDevProjectItem* projectItem = dynamic_cast<KDevProjectItem*>(rootItem);
    QList<KDevProjectFileItem*> files;
    if ( projectItem )
        files = recurseFiles( projectItem );

    return files;
}

KUrl KDevProject::relativeUrl( const KUrl& absolute ) const
{
    return KUrl::relativeUrl(folder(), absolute);
}

KUrl KDevProject::urlRelativeToProject( const KUrl & relativeUrl ) const
{
    if (KUrl::isRelativeUrl(relativeUrl.path()))
        return KUrl(folder(), relativeUrl.path());

    return relativeUrl;
}

KDevFileManager* KDevProject::fileManager() const
{
    return d->manager;
}

void KDevProject::setFileManager( KDevFileManager* newManager )
{
    d->manager = newManager;
}

KDevPersistentHash * KDevProject::persistentHash() const
{
    return &d->persistentHash;
}

void KDevProject::importDone( KJob* job )
{
    job->deleteLater();
    d->model->resetModel();
}

QList<KDevProjectFileItem*> KDevProject::recurseFiles(KDevProjectItem * projectItem)
{
    QList<KDevProjectFileItem*> files;
    if ( KDevProjectFolderItem * folder = projectItem->folder() )
    {
        QList<KDevProjectFolderItem*> folder_list = folder->folderList();
        for ( QList<KDevProjectFolderItem*>::Iterator it = folder_list.begin(); it != folder_list.end(); ++it )
            files += recurseFiles( ( *it ) );

        QList<KDevProjectTargetItem*> target_list = folder->targetList();
        for ( QList<KDevProjectTargetItem*>::Iterator it = target_list.begin(); it != target_list.end(); ++it )
            files += recurseFiles( ( *it ) );

        QList<KDevProjectFileItem*> file_list = folder->fileList();
        for ( QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it )
            files += recurseFiles( ( *it ) );
    }
    else if ( KDevProjectTargetItem * target = projectItem->target() )
    {
        QList<KDevProjectFileItem*> file_list = target->fileList();
        for ( QList<KDevProjectFileItem*>::Iterator it = file_list.begin(); it != file_list.end(); ++it )
            files += recurseFiles( ( *it ) );
    }
    else if ( KDevProjectFileItem * file = projectItem->file() )
    {
        files.append( file );
    }
    return files;
}


#include "kdevproject.moc"
