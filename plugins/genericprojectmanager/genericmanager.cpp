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
#include <project/helper.h>
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
#include <QtCore/QFileInfo>

#include <KIO/Job>
#include <KIO/NetAccess>
#include <KIO/CopyJob>

#include <KMessageBox>

#include "genericmanagerlistjob.h"

#define ifDebug(x)

K_PLUGIN_FACTORY(GenericSupportFactory, registerPlugin<GenericProjectManager>(); )
K_EXPORT_PLUGIN(GenericSupportFactory(KAboutData("kdevgenericmanager","kdevgenericprojectmanager",ki18n("Generic Project Manager"), "0.1", ki18n("A plugin to support basic project management on a filesystem level"), KAboutData::License_GPL)))


/**
 * Reads project config and returns lists for include and exclude rules.
 */
GenericProjectManager::IncludeRules getIncludeRules(KDevelop::IProject* project) {
    KConfigGroup filtersConfig = project->projectConfiguration()->group("Filters");
    QStringList includes = filtersConfig.readEntry("Includes", QStringList("*"));
    QStringList excludes = filtersConfig.readEntry("Excludes", QStringList() << "*/.*" << "*~");

    return qMakePair(includes, excludes);
}

/**
 * Returns the parent folder item for a given item or the project root item if there is no parent.
 */
KDevelop::ProjectFolderItem* getParentFolder(KDevelop::ProjectBaseItem* item)
{
    if ( item->parent() ) {
        return static_cast<KDevelop::ProjectFolderItem*>(item->parent());
    } else {
        return item->project()->projectItem();
    }
}

GenericProjectManager::GenericProjectManager( QObject *parent, const QVariantList & args )
        : KDevelop::IPlugin( GenericSupportFactory::componentData(), parent ), KDevelop::IGenericProjectManager()
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IGenericProjectManager )
    Q_UNUSED( args )

    connect(core()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
            this, SLOT(projectClosing(KDevelop::IProject*)));
}

void GenericProjectManager::projectClosing(KDevelop::IProject* project)
{
    delete m_watchers.take(project);
}

GenericProjectManager::~GenericProjectManager()
{
}

bool GenericProjectManager::isValid( const KUrl &url, const bool isFolder, KDevelop::IProject* project,
                                     const IncludeRules &rules ) const
{
    if ( isFolder && ( url.fileName() == "." || url.fileName() == ".."
            || ( url.fileName() == ".kdev4" && url.upUrl() == project->folder() ) ) ) {
        return false;
    } else if ( url == project->projectFileUrl() ) {
        return false;
    }

    bool ok = isFolder;

    // we operate on the path of this url relative to the project base
    // by prepending a slash we can filter hidden files with the pattern "*/.*"
    // by appending a slash to folders we can filter them with "*/"
    const QString relativePath = "/" + project->relativeUrl( url ).path(
        isFolder ? KUrl::AddTrailingSlash : KUrl::RemoveTrailingSlash
    );

    for ( QStringList::ConstIterator it = rules.first.constBegin(); !ok && it != rules.first.constEnd(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( relativePath ) ) {
            ok = true;
            break;
        }
    }

    if ( !ok ) {
        return false;
    }

    for ( QStringList::ConstIterator it = rules.second.constBegin(); it != rules.second.constEnd(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( relativePath ) ) {
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

KJob* GenericProjectManager::eventuallyReadFolder( KDevelop::ProjectFolderItem* item, const bool forceResursion )
{
    GenericManagerListJob* listJob = new GenericManagerListJob( item, forceResursion );
    kDebug() << "adding job" << listJob << item->url() << "for project" << item->project();

    KDevelop::ICore::self()->runController()->registerJob( listJob );

    connect( listJob, SIGNAL(entries(KDevelop::ProjectFolderItem*, KIO::UDSEntryList, bool)),
             this, SLOT(addJobItems(KDevelop::ProjectFolderItem*, KIO::UDSEntryList, bool)) );

    connect( this, SIGNAL(appendSubDir(KDevelop::ProjectFolderItem*)),
             listJob, SLOT(addSubDir(KDevelop::ProjectFolderItem*)));

    return listJob;
}

void GenericProjectManager::addJobItems(KDevelop::ProjectFolderItem* baseItem, const KIO::UDSEntryList& entries,
                                        const bool forceRecursion)
{
    if ( entries.empty() ) {
        return;
    }

    kDebug() << "reading entries of" << baseItem->url();

    const IncludeRules& rules = getIncludeRules(baseItem->project());

    // build lists of valid files and folders with relative urls to the project folder
    KUrl::List files;
    KUrl::List folders;
    foreach ( const KIO::UDSEntry& entry, entries ) {
        KUrl url = baseItem->url();
        url.addPath( entry.stringValue( KIO::UDSEntry::UDS_NAME ) );

        if ( !isValid( url, entry.isDir(), baseItem->project(), rules ) ) {
            continue;
        } else {
            if ( entry.isDir() ) {
                if( entry.isLink() ) {
                    KUrl linkedUrl = baseItem->url();
                    linkedUrl.cd(entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST ));
                    // make sure we don't end in an infinite loop
                    if( linkedUrl.isParentOf( baseItem->project()->folder() ) ||
                        baseItem->project()->folder().isParentOf( linkedUrl ) ||
                        linkedUrl == baseItem->project()->folder() )
                    {
                        continue;
                    }
                }
                folders << url;
            } else {
                files << url;
            }
        }
    }

    ifDebug(kDebug() << "valid folders:" << folders;)
    ifDebug(kDebug() << "valid files:" << files;)

    // remove obsolete rows
    for ( int j = 0; j < baseItem->rowCount(); ++j ) {
        if ( baseItem->child(j)->type() == KDevelop::ProjectBaseItem::Folder ) {
            KDevelop::ProjectFolderItem* f = static_cast<KDevelop::ProjectFolderItem*>( baseItem->child(j) );
            // check if this is still a valid folder
            int index = folders.indexOf( f->url() );
            if ( index == -1 ) {
                // folder got removed or is now invalid
                ifDebug(kDebug() << "removing folder:" << f->url();)
                baseItem->removeRow( j );
                --j;
            } else {
                // this folder already exists in the view
                folders.removeAt( index );
                if ( forceRecursion ) {
                    //no need to add this item, but we still want to recurse into it
                    emit appendSubDir( f );
                }
            }
        } else if ( baseItem->child(j)->type() == KDevelop::ProjectBaseItem::File ) {
            KDevelop::ProjectFileItem* f = static_cast<KDevelop::ProjectFileItem*>( baseItem->child(j) );
            // check if this is still a valid file
            int index = files.indexOf( f->url() );
            if ( index == -1 ) {
                // file got removed or is now invalid
                ifDebug(kDebug() << "removing file:" << f->url();)
                baseItem->removeRow( j );
                --j;
            } else {
                // this file already exists in the view
                files.removeAt( index );
            }
        }
    }

    // add new rows
    foreach ( const KUrl& url, files ) {
        new KDevelop::ProjectFileItem( baseItem->project(), url, baseItem );
    }
    foreach ( const KUrl& url, folders ) {
        emit appendSubDir( new KDevelop::ProjectFolderItem( baseItem->project(), url, baseItem ) );
    }
}

KDevelop::ProjectFolderItem *GenericProjectManager::import( KDevelop::IProject *project )
{
    KDevelop::ProjectFolderItem *projectRoot = new KDevelop::ProjectFolderItem( project, project->folder(), 0 );
    kDebug() << "imported new project" << project->name() << "at" << projectRoot->url();
    projectRoot->setProjectRoot(true);

    ///TODO: check if this works for remote files when something gets changed through another KDE app
    if ( project->folder().isLocalFile() ) {
        m_watchers[project] = new KDirWatch( project );

        connect(m_watchers[project], SIGNAL(created(QString)),
                    this, SLOT(created(QString)));
        connect(m_watchers[project], SIGNAL(deleted(QString)),
                    this, SLOT(deleted(QString)));

        m_watchers[project]->addDir(project->folder().toLocalFile(), KDirWatch::WatchSubDirs | KDirWatch:: WatchFiles );
    }

    return projectRoot;
}

KJob* GenericProjectManager::createImportJob(KDevelop::ProjectFolderItem* item)
{
    return eventuallyReadFolder(item);
}

bool GenericProjectManager::reload( KDevelop::ProjectFolderItem* item )
{
    kDebug() << "reloading item" << item->url();
    eventuallyReadFolder( item->folder(), true );
    return true;
}

void GenericProjectManager::created(const QString &path)
{
    kDebug() << "created:" << path;
    QFileInfo info(path);

    KUrl url = KUrl(path);
    KUrl parent = url.upUrl();

    foreach ( KDevelop::IProject* p, m_watchers.keys() ) {
        if ( !isValid(url, info.isDir(), p, getIncludeRules(p)) ) {
            continue;
        }
        if ( !p->foldersForUrl(url).isEmpty() || !p->filesForUrl(url).isEmpty() ) {
            // exists already in this project, happens e.g. when we restart the dirwatcher
            // for files it also gets triggered for kate's backup files
            continue;
        }
        foreach ( KDevelop::ProjectFolderItem* parentItem, p->foldersForUrl(parent) ) {
            if ( info.isDir() ) {
                eventuallyReadFolder(new KDevelop::ProjectFolderItem( p, url, parentItem ));
            } else {
                new KDevelop::ProjectFileItem( p, url, parentItem );
            }
        }
    }
}

void GenericProjectManager::deleted(const QString &path)
{
    if ( QFile::exists(path) ) {
        // stopDirScan...
        return;
    }
    kDebug() << "deleted:" << path;

    KUrl url = KUrl(path);
    foreach ( KDevelop::IProject* p, m_watchers.keys() ) {
        if ( url.equals(p->folder(), KUrl::CompareWithoutTrailingSlash) ) {
            KMessageBox::error(KDevelop::ICore::self()->uiController()->activeMainWindow(),
                               i18n("The base folder of project <b>%1</b> got deleted or moved outside of KDevelop.\n"
                                    "The project has to be closed.", p->name()), i18n("Project Folder Deleted") );
            KDevelop::ICore::self()->projectController()->closeProject(p);
            continue;
        }
        foreach ( KDevelop::ProjectFolderItem* item, p->foldersForUrl(url) ) {
            item->parent()->removeRow(item->row());
        }
        foreach ( KDevelop::ProjectFileItem* item, p->filesForUrl(url) ) {
            item->parent()->removeRow(item->row());
        }
    }
}

KDevelop::ProjectFolderItem* GenericProjectManager::addFolder( const KUrl& url,
        KDevelop::ProjectFolderItem * parent )
{
    kDebug() << "adding folder" << url << "to" << parent->url();
    KDevelop::ProjectFolderItem* created = 0;
    stopWatcher(parent);
    if ( KDevelop::createFolder(url) ) {
        created = new KDevelop::ProjectFolderItem( parent->project(), url, parent );
    }
    continueWatcher(parent);
    return created;
}


KDevelop::ProjectFileItem* GenericProjectManager::addFile( const KUrl& url,
        KDevelop::ProjectFolderItem * parent )
{
    kDebug() << "adding file" << url << "to" << parent->url();
    KDevelop::ProjectFileItem* created = 0;
    stopWatcher(parent);
    if ( KDevelop::createFile(url) ) {
        created = new KDevelop::ProjectFileItem( parent->project(), url, parent );
    }
    continueWatcher(parent);
    return created;
}

bool GenericProjectManager::renameFolder( KDevelop::ProjectFolderItem * folder, const KUrl& url )
{
    kDebug() << "trying to rename a folder:" << folder->url() << url;
    if ( rename(folder, folder->url(), url) ) {
        folder->setUrl(url);
        return true;
    } else {
        return false;
    }
}

bool GenericProjectManager::renameFile( KDevelop::ProjectFileItem * file, const KUrl& url )
{
    kDebug() << "trying to rename a file:" << file->url() << url;

    if ( rename(file, file->url(), url) ) {
        file->setUrl(url);
        return true;
    } else {
        return false;
    }
}

bool GenericProjectManager::rename(KDevelop::ProjectBaseItem* item, const KUrl& source, const KUrl& destination)
{
    if ( !isValid(destination, true, item->project(), getIncludeRules(item->project())) ) {
        int cancel = KMessageBox::warningContinueCancel( KDevelop::ICore::self()->uiController()->activeMainWindow(),
            i18n("You tried to rename '%1' to '%2', but the latter is filtered and will be hidden.\n"
                 "Do you want to continue?", item->text(), destination.fileName()),
            QString(), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), "GenericManagerRenameToFiltered"
        );
        if ( cancel == KMessageBox::Cancel ) {
            return false;
        }
    }
    foreach ( KDevelop::ProjectFolderItem* parent, item->project()->foldersForUrl(destination.upUrl()) ) {
        if ( parent->type() == KDevelop::ProjectBaseItem::Folder ) {
            stopWatcher(parent);
            KIO::CopyJob* job = KIO::move(source, destination);
            bool success = KIO::NetAccess::synchronousRun( job, KDevelop::ICore::self()->uiController()->activeMainWindow() );
            continueWatcher(parent);
            if ( success ) {
                item->setParent(parent);
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

bool GenericProjectManager::removeFolder( KDevelop::ProjectFolderItem * folder )
{
    kDebug() << "removing folder" << folder->url();
    KDevelop::ProjectFolderItem* parent = getParentFolder(folder);
    stopWatcher(parent);
    const bool success = KDevelop::removeUrl(parent->project(), folder->url(), true);
    if ( success ) {
        folder->parent()->removeRow( folder->row() );
    }
    continueWatcher(parent);
    return success;
}

bool GenericProjectManager::removeFile( KDevelop::ProjectFileItem * file )
{
    kDebug() << "removing file" << file->url();
    KDevelop::ProjectFolderItem* parent = getParentFolder(file);
    stopWatcher(parent);
    const bool success = KDevelop::removeUrl(file->project(), file->url(), false);
    if ( success ) {
        file->parent()->removeRow( file->row() );
    }
    continueWatcher(parent);
    return success;
}

void GenericProjectManager::stopWatcher(KDevelop::ProjectFolderItem* folder)
{
    if ( !folder->url().isLocalFile() ) {
        return;
    }
    Q_ASSERT(m_watchers.contains(folder->project()));
    m_watchers[folder->project()]->stopDirScan(folder->url().toLocalFile());
}

void GenericProjectManager::continueWatcher(KDevelop::ProjectFolderItem* folder)
{
    if ( !folder->url().isLocalFile() ) {
        return;
    }
    Q_ASSERT(m_watchers.contains(folder->project()));
    m_watchers[folder->project()]->restartDirScan(folder->url().toLocalFile());
}


#include "genericmanager.moc"
