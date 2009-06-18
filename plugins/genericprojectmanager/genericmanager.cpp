/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>

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
#include "genericmanager.h"
#include <project/projectmodel.h>

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <language/duchain/indexedstring.h>
#include <project/importprojectjob.h>
#include <interfaces/iuicontroller.h>

#include <kparts/mainwindow.h>

#include <kdebug.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kaboutdata.h>
#include <kdirwatch.h>

#include <QtDesigner/QExtensionFactory>
#include <QtCore/QRegExp>

#include <KIO/Job>
#include <KIO/NetAccess>

K_PLUGIN_FACTORY(GenericSupportFactory, registerPlugin<GenericProjectManager>(); )
K_EXPORT_PLUGIN(GenericSupportFactory(KAboutData("kdevgenericmanager","kdevgenericprojectmanager",ki18n("Generic Project Manager"), "0.1", ki18n("A plugin to support basic project management on a filesystem level"), KAboutData::License_GPL)))

class GenericProjectManagerPrivate
{
};

GenericProjectManager::GenericProjectManager( QObject *parent, const QVariantList & args )
        : KDevelop::IPlugin( GenericSupportFactory::componentData(), parent ), KDevelop::IProjectFileManager(), d( new GenericProjectManagerPrivate )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    Q_UNUSED( args )
    
    connect( KDevelop::ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
             this, SLOT(waitForProjectOpen(KDevelop::IProject*)) );
}

GenericProjectManager::~GenericProjectManager()
{
    delete d;
}

bool GenericProjectManager::isValid( const KUrl &url, const bool isFolder, KDevelop::IProject* project,
                                     const QStringList &includes, const QStringList &excludes ) const
{
    if ( isFolder && ( url.fileName() == "." || url.fileName() == ".."
            || ( url.fileName() == ".kdev4" && url.upUrl() == project->folder() ) ) ) {
        return false;
    } else if ( url == project->projectFileUrl() ) {
        return false;
    }

    bool ok = isFolder;

    for ( QStringList::ConstIterator it = includes.begin(); !ok && it != includes.end(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( url.fileName() ) ) {
            ok = true;
            break;
        }
    }

    if ( !ok ) {
        return false;
    }

    for ( QStringList::ConstIterator it = excludes.begin(); it != excludes.end(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( url.fileName() ) ) {
            return false;
        }
    }

    return true;
}

QList<KDevelop::ProjectFolderItem*> GenericProjectManager::parse( KDevelop::ProjectFolderItem *item )
{
    // we are async, can't return anything here
    kDebug() << "note: parse will always return an empty list";
    Q_UNUSED(item);
    return QList<KDevelop::ProjectFolderItem*>();
}

void GenericProjectManager::waitForProjectOpen(KDevelop::IProject* project)
{
    if ( project->managerPlugin() == this ) {
        kDebug() << "generic project got opened:" << project->folder();
        eventuallyReadFolder( project->projectItem() );
    }
}

void GenericProjectManager::eventuallyReadFolder( KDevelop::ProjectFolderItem* item )
{
    ///TODO: listRecursive does not follow directory links!
    KIO::ListJob* job = KIO::listRecursive( item->url() );
    kDebug() << "adding job" << job << job->url() << "for project" << item->project();

    KDevelop::ICore::self()->runController()->registerJob( job );

    connect( job, SIGNAL(entries(KIO::Job*, KIO::UDSEntryList)),
                this, SLOT(addJobItems(KIO::Job*, KIO::UDSEntryList)) );
}

void GenericProjectManager::addJobItems(KIO::Job* j, KIO::UDSEntryList entries)
{
    if ( entries.empty() ) {
        return;
    }

    KIO::SimpleJob* job(dynamic_cast<KIO::SimpleJob*>(j));
    Q_ASSERT(job);

    // the folder we are currently adding items to
    // the UDS_NAME is essentially a path relative to the job->url()
    KUrl curUrl = job->url();
    curUrl.addPath( entries.first().stringValue( KIO::UDSEntry::UDS_NAME ) );
    curUrl.setFileName( "" );

    kDebug() << "reading contents of " << curUrl << "total entries:" << entries.count();

    KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->findProjectForUrl( job->url() );
    Q_ASSERT(project);

    QList<KDevelop::ProjectFolderItem*> fitems = project->foldersForUrl( curUrl );
    if ( fitems.isEmpty() ) {
        kDebug() << "parent was filtered";
        return;
    }
    KDevelop::ProjectFolderItem* item = fitems.first();

    KConfigGroup filtersConfig = project->projectConfiguration()->group("Filters");
    QStringList includes = filtersConfig.readEntry("Includes", QStringList("*"));
    ///TODO: filter hidden files by default
    QStringList excludes = filtersConfig.readEntry("Excludes", QStringList());

    // build lists of valid files and folders with relative urls to the project folder
    KUrl::List files;
    KUrl::List folders;
    foreach ( KIO::UDSEntry entry, entries ) {
        KUrl url = job->url();
        url.addPath( entry.stringValue( KIO::UDSEntry::UDS_NAME ) );

        if ( !isValid( url, entry.isDir(), project, includes, excludes ) ) {
            continue;
        } else {
            if ( entry.isDir() ) {
                folders << url;
            } else {
                files << url;
            }
        }
    }

    kDebug() << "valid folders:" << folders;
    kDebug() << "valid files:" << files;

    // remove obsolete rows
    for ( int j = 0; j < item->rowCount(); ++j ) {
        if ( item->child(j)->type() == KDevelop::ProjectBaseItem::Folder ) {
            KDevelop::ProjectFolderItem* f = static_cast<KDevelop::ProjectFolderItem*>( item->child(j) );
            // check if this is still a valid folder
            int index = folders.indexOf( f->folderName() );
            if ( index == -1 ) {
                // folder got removed or is now invalid
                item->removeRow( j );
                --j;
            } else {
                // this folder already exists in the view
                folders.removeAt( index );
            }
        } else if ( item->child(j)->type() == KDevelop::ProjectBaseItem::File ) {
            KDevelop::ProjectFileItem* f = static_cast<KDevelop::ProjectFileItem*>( item->child(j) );
            // check if this is still a valid file
            int index = files.indexOf( f->fileName() );
            if ( index == -1 ) {
                // file got removed or is now invalid
                project->removeFromFileSet( KDevelop::IndexedString( f->url() ) );
                item->removeRow( j );
                --j;
            } else {
                // this file already exists in the view
                files.removeAt( index );
            }
        }
    }

    // add new rows
    ///TODO: why do the fileitems look like remote folders, and why is the sorting fubar?
    foreach ( const KUrl& url, files ) {
        new KDevelop::ProjectFileItem( project, url, item );
        project->addToFileSet( KDevelop::IndexedString( url ) );
    }
    foreach ( const KUrl& url, folders ) {
        new KDevelop::ProjectFolderItem( project, url, item );
    }
}

KDevelop::ProjectFolderItem *GenericProjectManager::import( KDevelop::IProject *project )
{
    KDevelop::ProjectFolderItem *projectRoot=new KDevelop::ProjectFolderItem( project, project->folder(), 0 );
    kDebug() << "imported new project" << project->name() << "at" << projectRoot->url();
    projectRoot->setProjectRoot(true);

    ///TODO: check if this works for remote files when something gets changed through another KDE app
    if ( project->folder().isLocalFile() ) {
        m_watchers[project] = new KDirWatch( project );

        connect(m_watchers[project], SIGNAL(dirty(QString)), this, SLOT(dirty(QString)));

        m_watchers[project]->addDir(project->folder().toLocalFile(), KDirWatch::WatchSubDirs );
    }

    return projectRoot;
}

bool GenericProjectManager::reload( KDevelop::ProjectBaseItem* item )
{
    kDebug() << "reloading project" << item->project()->name() << item->project()->folder();

    eventuallyReadFolder( item->project()->projectItem() );
    return true;
}

void GenericProjectManager::dirty( const QString &fileName )
{
    foreach ( KDevelop::IProject* p, KDevelop::ICore::self()->projectController()->projects() ) {
        foreach ( KDevelop::ProjectFolderItem* item, p->foldersForUrl( KUrl(fileName) ) ) {
            kDebug() << "reloading item" << item->url();

            eventuallyReadFolder( item );
        }
    }
}


KDevelop::ProjectFolderItem* GenericProjectManager::addFolder( const KUrl& url,
        KDevelop::ProjectFolderItem * folder )
{
    // dirwatcher cares about local folders, yet remote ones have to be added by hand
    KDevelop::ProjectFolderItem* newFolder = 0;
    if ( !url.isLocalFile() ) {
        kDebug() << "adding remote folder" << url << "to" << folder->url();
        newFolder = new KDevelop::ProjectFolderItem( folder->project(), url, folder );
    }
    return newFolder;
}


KDevelop::ProjectFileItem* GenericProjectManager::addFile( const KUrl& url,
        KDevelop::ProjectFolderItem * folder )
{
    // dirwatcher cares about local files, yet remote ones have to be added by hand
    KDevelop::ProjectFileItem* file = 0;
    if ( !url.isLocalFile() ) {
        kDebug() << "adding remote file" << url << "to" << folder->url();
        file = new KDevelop::ProjectFileItem( folder->project(), url, folder );
    }
    return file;
}

bool GenericProjectManager::renameFolder( KDevelop::ProjectFolderItem * folder, const KUrl& url )
{
    kDebug() << "trying to rename a folder:" << folder->url() << url;
    Q_UNUSED( folder )
    Q_UNUSED( url )
    return false;
}

bool GenericProjectManager::renameFile( KDevelop::ProjectFileItem * file, const KUrl& url )
{
    kDebug() << "trying to rename a file:" << file->url() << url;
    Q_UNUSED(file)
    Q_UNUSED(url)
    return false;
}

bool GenericProjectManager::removeFolder( KDevelop::ProjectFolderItem * folder )
{
    // dirwatcher cares about local folders, yet remote ones have to be removed by hand
    if ( !folder->url().isLocalFile() ) {
        kDebug() << "removing folder" << folder->url();
        folder->parent()->removeRow( folder->row() );
    }
    return true;
}

bool GenericProjectManager::removeFile( KDevelop::ProjectFileItem * file )
{
    // dirwatcher cares about local files, yet remote ones have to be removed by hand
    if ( !file->url().isLocalFile() ) {
        kDebug() << "removing file" << file->url();
        file->parent()->removeRow( file->row() );
    }
    return true;
}


#include "genericmanager.moc"
