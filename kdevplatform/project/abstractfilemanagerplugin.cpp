/*
    SPDX-FileCopyrightText: 2010-2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "abstractfilemanagerplugin.h"

#include "filemanagerlistjob.h"
#include "projectmodel.h"
#include "helper.h"

#include <QHashIterator>
#include <QFileInfo>
#include <QApplication>
#include <QTimer>
#ifdef TIME_IMPORT_JOB
#include <QElapsedTimer>
#endif

#include <KMessageBox>
#include <KLocalizedString>
#include <KDirWatch>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <serialization/indexedstring.h>

#include "projectfiltermanager.h"
#include "debug.h"

#define ifDebug(x)

using namespace KDevelop;

//BEGIN Helper

namespace {

/**
 * Returns the parent folder item for a given item or the project root item if there is no parent.
 */
ProjectFolderItem* parentFolder(ProjectBaseItem* item)
{
    if ( item->parent() ) {
        return static_cast<ProjectFolderItem*>(item->parent());
    } else {
        return item->project()->projectItem();
    }
}

}

//END Helper

//BEGIN Private

class KDevelop::AbstractFileManagerPluginPrivate
{
public:
    explicit AbstractFileManagerPluginPrivate(AbstractFileManagerPlugin* qq)
        : q(qq)
    {
    }

    AbstractFileManagerPlugin* q;

    /**
     * The just returned must be started in one way or another for this method
     * to have any affect. The job will then auto-delete itself upon completion.
     */
    [[nodiscard]] KJob* eventuallyReadFolder(ProjectFolderItem* item);
    void addJobItems(FileManagerListJob* job,
                     ProjectFolderItem* baseItem,
                     const KIO::UDSEntryList& entries);

    void deleted(const QString &path);
    void created(const QString &path);

    void projectClosing(IProject* project);
    void jobFinished(KJob* job);

    /// Stops watching the given folder for changes, only useful for local files.
    void stopWatcher(ProjectFolderItem* folder);
    /// Continues watching the given folder for changes.
    void continueWatcher(ProjectFolderItem* folder);
    /// Common renaming function.
    bool rename(ProjectBaseItem* item, const Path& newPath);

    QHash<IProject*, KDirWatch*> m_watchers;
    QHash<IProject*, QList<FileManagerListJob*> > m_projectJobs;
    QVector<QString> m_stoppedFolders;
    ProjectFilterManager m_filters;
};

void AbstractFileManagerPluginPrivate::projectClosing(IProject* project)
{
    const auto projectJobIt = m_projectJobs.constFind(project);
    if (projectJobIt != m_projectJobs.constEnd()) {
        // make sure the import job does not live longer than the project
        // see also addLotsOfFiles test
        for (FileManagerListJob* job : *projectJobIt) {
            qCDebug(FILEMANAGER) << "killing project job:" << job;
            job->kill();
        }
        m_projectJobs.remove(project);
    }
#ifdef TIME_IMPORT_JOB
    QElapsedTimer timer;
    if (m_watchers.contains(project)) {
        timer.start();
    }
#endif
    delete m_watchers.take(project);
#ifdef TIME_IMPORT_JOB
    if (timer.isValid()) {
        qCDebug(FILEMANAGER) << "Deleting dir watcher took" << timer.elapsed() / 1000.0 << "seconds for project" << project->name();
    }
#endif
    m_filters.remove(project);
}

KJob* AbstractFileManagerPluginPrivate::eventuallyReadFolder(ProjectFolderItem* item)
{
    auto* listJob = new FileManagerListJob( item );
    m_projectJobs[ item->project() ] << listJob;
    qCDebug(FILEMANAGER) << "adding job" << listJob << item << item->path() << "for project" << item->project();

    q->connect( listJob, &FileManagerListJob::finished,
                q, [&] (KJob* job) { jobFinished(job); } );

    q->connect( listJob, &FileManagerListJob::entries,
                q, [&] (FileManagerListJob* job, ProjectFolderItem* baseItem, const KIO::UDSEntryList& entries) {
                    addJobItems(job, baseItem, entries); } );

    return listJob;
}

void AbstractFileManagerPluginPrivate::jobFinished(KJob* job)
{
    // ensure we don't keep a dangling point in our list
    // NOTE: job is potentially emitting its finished signal from its destructor
    // or the item that was used internally may have been deleted already
    for (auto& jobs : m_projectJobs) {
        if (jobs.removeOne(reinterpret_cast<FileManagerListJob*>(job))) {
            break;
        }
    }
}

void AbstractFileManagerPluginPrivate::addJobItems(FileManagerListJob* job,
                                                     ProjectFolderItem* baseItem,
                                                     const KIO::UDSEntryList& entries)
{
    qCDebug(FILEMANAGER) << "reading entries of" << baseItem->path();

    // build lists of valid files and folders with paths relative to the project folder
    Path::List files;
    Path::List folders;
    for (const KIO::UDSEntry& entry : entries) {
        QString name = entry.stringValue( KIO::UDSEntry::UDS_NAME );
        if (name == QLatin1String(".") || name == QLatin1String("..")) {
            continue;
        }

        Path path(baseItem->path(), name);

        if ( !q->isValid( path, entry.isDir(), baseItem->project() ) ) {
            continue;
        } else {
            if ( entry.isDir() ) {
                if( entry.isLink() ) {
                    const Path linkedPath = baseItem->path().cd(entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST ));
                    // make sure we don't end in an infinite loop
                    if( linkedPath.isParentOf( baseItem->project()->path() ) ||
                        baseItem->project()->path().isParentOf( linkedPath ) ||
                        linkedPath == baseItem->project()->path() )
                    {
                        continue;
                    }
                }
                folders << path;
            } else {
                files << path;
            }
        }
    }

    ifDebug(qCDebug(FILEMANAGER) << "valid folders:" << folders;)
    ifDebug(qCDebug(FILEMANAGER) << "valid files:" << files;)

    // remove obsolete rows
    for ( int j = 0; j < baseItem->rowCount(); ++j ) {
        if ( ProjectFolderItem* f = baseItem->child(j)->folder() ) {
            // check if this is still a valid folder
            int index = folders.indexOf( f->path() );
            if ( index == -1 ) {
                // folder got removed or is now invalid
                delete f;
                --j;
            } else {
                // this folder already exists in the view
                folders.remove( index );
                // no need to add this item, but we still want to recurse into it
                job->addSubDir( f );
                emit q->reloadedFolderItem( f );
            }
        } else if ( ProjectFileItem* f =  baseItem->child(j)->file() ) {
            // check if this is still a valid file
            int index = files.indexOf( f->path() );
            if ( index == -1 ) {
                // file got removed or is now invalid
                ifDebug(qCDebug(FILEMANAGER) << "removing file:" << f << f->path();)
                delete f;
                --j;
            } else {
                // this file already exists in the view
                files.remove( index );
                emit q->reloadedFileItem( f );
            }
        }
    }

    // add new rows
    for (const Path& path : std::as_const(files)) {
        ProjectFileItem* file = q->createFileItem( baseItem->project(), path, baseItem );
        if (file) {
            emit q->fileAdded( file );
        }
    }
    for (const Path& path : std::as_const(folders)) {
        ProjectFolderItem* folder = q->createFolderItem( baseItem->project(), path, baseItem );
        if (folder) {
            emit q->folderAdded( folder );
            job->addSubDir( folder );
        }
    }
}

void AbstractFileManagerPluginPrivate::created(const QString& path_)
{
    qCDebug(FILEMANAGER) << "created:" << path_;
    QFileInfo info(path_);
    if (!info.exists()) {
        // we delay handling of the signal, so maybe the path actually got removed again
        return;
    }

    ///FIXME: share memory with parent
    const Path path(path_);
    const IndexedString indexedPath(path.pathOrUrl());
    const IndexedString indexedParent(path.parent().pathOrUrl());

    QHashIterator<IProject*, KDirWatch*> it(m_watchers);
    while (it.hasNext()) {
        const auto p = it.next().key();
        if ( !p->projectItem()->model() ) {
            // not yet finished with loading
            // FIXME: how should this be handled? see unit test
            continue;
        }
        if ( !q->isValid(path, info.isDir(), p) ) {
            continue;
        }
        if ( info.isDir() ) {
            bool found = false;
            const auto folderItems = p->foldersForPath(indexedPath);
            for (ProjectFolderItem* folder : folderItems) {
                // exists already in this project, happens e.g. when we restart the dirwatcher
                // or if we delete and remove folders consecutively https://bugs.kde.org/show_bug.cgi?id=260741
                qCDebug(FILEMANAGER) << "force reload of" << path << folder;
                auto job = eventuallyReadFolder( folder );
                job->start();
                found = true;
            }
            if ( found ) {
                continue;
            }
        } else if (!p->filesForPath(indexedPath).isEmpty()) {
            // also gets triggered for kate's backup files
            continue;
        }
        const auto parentItems = p->foldersForPath(indexedParent);
        for (ProjectFolderItem* parentItem : parentItems) {
            if ( info.isDir() ) {
                ProjectFolderItem* folder = q->createFolderItem( p, path, parentItem );
                if (folder) {
                    emit q->folderAdded( folder );
                    auto job = eventuallyReadFolder( folder );
                    job->start();
                }
            } else {
                ProjectFileItem* file = q->createFileItem( p, path, parentItem );
                if (file) {
                    emit q->fileAdded( file );
                }
            }
        }
    }
}

void AbstractFileManagerPluginPrivate::deleted(const QString& path_)
{
    if ( QFile::exists(path_) ) {
        // we delay handling of the signal, so maybe the path actually exists again
        return;
    }
    // ensure that the path is not inside a stopped folder
    for (const QString& folder : std::as_const(m_stoppedFolders)) {
        if (path_.startsWith(folder)) {
            return;
        }
    }
    qCDebug(FILEMANAGER) << "deleted:" << path_;

    const Path path(QUrl::fromLocalFile(path_));
    const IndexedString indexed(path.pathOrUrl());

    QHashIterator<IProject*, KDirWatch*> it(m_watchers);
    while (it.hasNext()) {
        const auto p = it.next().key();
        if (path == p->path()) {
            KMessageBox::error(qApp->activeWindow(),
                               i18n("The base folder of project <b>%1</b>"
                                    " got deleted or moved outside of KDevelop.\n"
                                    "The project has to be closed.", p->name()),
                               i18nc("@title:window", "Project Folder Deleted") );
            ICore::self()->projectController()->closeProject(p);
            continue;
        }
        if ( !p->projectItem()->model() ) {
            // not yet finished with loading
            // FIXME: how should this be handled? see unit test
            continue;
        }
        const auto folderItems = p->foldersForPath(indexed);
        for (ProjectFolderItem* item : folderItems) {
            delete item;
        }
        const auto fileItems = p->filesForPath(indexed);
        for (ProjectFileItem* item : fileItems) {
            emit q->fileRemoved(item);
            ifDebug(qCDebug(FILEMANAGER) << "removing file" << item;)
            delete item;
        }
    }
}

bool AbstractFileManagerPluginPrivate::rename(ProjectBaseItem* item, const Path& newPath)
{
    if ( !q->isValid(newPath, true, item->project()) ) {
        int cancel = KMessageBox::warningContinueCancel( qApp->activeWindow(),
            i18n("You tried to rename '%1' to '%2', but the latter is filtered and will be hidden.\n"
                 "Do you want to continue?", item->text(), newPath.lastPathSegment()),
            QString(), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), QStringLiteral("GenericManagerRenameToFiltered")
        );
        if ( cancel == KMessageBox::Cancel ) {
            return false;
        }
    }
    const auto parentItems = item->project()->foldersForPath(IndexedString(newPath.parent().pathOrUrl()));
    for (ProjectFolderItem* parent : parentItems) {
        if ( parent->folder() ) {
            stopWatcher(parent);
            const Path source = item->path();
            bool success = renameUrl( item->project(), source.toUrl(), newPath.toUrl() );
            if ( success ) {
                item->setPath( newPath );
                item->parent()->takeRow( item->row() );
                parent->appendRow( item );
                if (item->file()) {
                    emit q->fileRenamed(source, item->file());
                } else {
                    Q_ASSERT(item->folder());
                    emit q->folderRenamed(source, item->folder());
                }
            }
            continueWatcher(parent);
            return success;
        }
    }
    return false;
}

void AbstractFileManagerPluginPrivate::stopWatcher(ProjectFolderItem* folder)
{
    if ( !folder->path().isLocalFile() ) {
        return;
    }
    Q_ASSERT(m_watchers.contains(folder->project()));
    const QString path = folder->path().toLocalFile();
    m_watchers[folder->project()]->stopDirScan(path);
    m_stoppedFolders.append(path);
}

void AbstractFileManagerPluginPrivate::continueWatcher(ProjectFolderItem* folder)
{
    if ( !folder->path().isLocalFile() ) {
        return;
    }
    auto watcher = m_watchers.value(folder->project(), nullptr);
    Q_ASSERT(watcher);
    const QString path = folder->path().toLocalFile();
    if (!watcher->restartDirScan(path)) {
        // path wasn't being watched yet - can we be 100% certain of that will never happen?
        qCWarning(FILEMANAGER) << "Folder" << path << "in project" << folder->project()->name() << "wasn't yet being watched";
        watcher->addDir(path);
    }
    const int idx = m_stoppedFolders.indexOf(path);
    if (idx != -1) {
        m_stoppedFolders.remove(idx);
    }
}
//END Private

//BEGIN Plugin

AbstractFileManagerPlugin::AbstractFileManagerPlugin(const QString& componentName, QObject* parent,
                                                     const KPluginMetaData& metaData, const QVariantList& /*args*/)
    : IProjectFileManager()
    , IPlugin(componentName, parent, metaData)
    , d_ptr(new AbstractFileManagerPluginPrivate(this))
{
    connect(core()->projectController(), &IProjectController::projectClosing,
            this, [this] (IProject* project) { Q_D(AbstractFileManagerPlugin); d->projectClosing(project); });
    connect(core()->projectController()->projectModel(), &ProjectModel::rowsAboutToBeRemoved,
            this, [this] (const QModelIndex& parent, int first, int last) {
                Q_D(AbstractFileManagerPlugin);
                // cleanup list jobs to remove about-to-be-dangling pointers
                auto* model = core()->projectController()->projectModel();
                for (int i = first; i <= last; ++i) {
                    const auto index = model->index(i, 0, parent);
                    auto* item = index.data(ProjectModel::ProjectItemRole).value<ProjectBaseItem*>();
                    Q_ASSERT(item);
                    for (auto* job : d->m_projectJobs.value(item->project())) {
                        job->handleRemovedItem(item);
                    }
                }
            });
}

AbstractFileManagerPlugin::~AbstractFileManagerPlugin() = default;

IProjectFileManager::Features AbstractFileManagerPlugin::features() const
{
    return Features( Folders | Files );
}

QList<ProjectFolderItem*> AbstractFileManagerPlugin::parse( ProjectFolderItem *item )
{
    // we are async, can't return anything here
    qCDebug(FILEMANAGER) << "note: parse will always return an empty list";
    Q_UNUSED(item);
    return QList<ProjectFolderItem*>();
}

ProjectFolderItem *AbstractFileManagerPlugin::import( IProject *project )
{
    Q_D(AbstractFileManagerPlugin);

    ProjectFolderItem *projectRoot = createFolderItem( project, project->path(), nullptr );
    emit folderAdded( projectRoot );
    qCDebug(FILEMANAGER) << "imported new project" << project->name() << "at" << projectRoot->path();

    ///TODO: check if this works for remote files when something gets changed through another KDE app
    if ( project->path().isLocalFile() ) {
        auto watcher = new KDirWatch( project );

        // set up the signal handling
        // NOTE: We delay handling of the creation/deletion events here by one second to prevent
        //       useless or even outright wrong handling of events during common git workflows.
        //       I.e. sometimes we used to get a 'delete' event during a rebase which was never
        //       followed up by a 'created' signal, even though the file actually exists after
        //       the rebase.
        //       see also: https://bugs.kde.org/show_bug.cgi?id=404184
        connect(watcher, &KDirWatch::created,
                this, [this] (const QString& path) {
                    QTimer::singleShot(1000, this, [this, path]() {
                        Q_D(AbstractFileManagerPlugin);
                        d->created(path);
                    });
                });
        connect(watcher, &KDirWatch::deleted,
                this, [this] (const QString& path) {
                    QTimer::singleShot(1000, this, [this, path]() {
                        Q_D(AbstractFileManagerPlugin);
                        d->deleted(path);
                    });
                });
        watcher->addDir(project->path().toLocalFile(), KDirWatch::WatchSubDirs | KDirWatch:: WatchFiles );
        d->m_watchers[project] = watcher;
    }

    d->m_filters.add(project);

    return projectRoot;
}

KJob* AbstractFileManagerPlugin::createImportJob(ProjectFolderItem* item)
{
    Q_D(AbstractFileManagerPlugin);

    return d->eventuallyReadFolder(item);
}

bool AbstractFileManagerPlugin::reload( ProjectFolderItem* item )
{
    Q_D(AbstractFileManagerPlugin);

    qCDebug(FILEMANAGER) << "reloading item" << item->path();
    auto job = d->eventuallyReadFolder( item->folder() );
    job->start();
    return true;
}

ProjectFolderItem* AbstractFileManagerPlugin::addFolder( const Path& folder,
        ProjectFolderItem * parent )
{
    Q_D(AbstractFileManagerPlugin);

    qCDebug(FILEMANAGER) << "adding folder" << folder << "to" << parent->path();
    ProjectFolderItem* created = nullptr;
    d->stopWatcher(parent);
    if ( createFolder(folder.toUrl()) ) {
        created = createFolderItem( parent->project(), folder, parent );
        if (created) {
            emit folderAdded(created);
        }
    }
    d->continueWatcher(parent);
    return created;
}


ProjectFileItem* AbstractFileManagerPlugin::addFile( const Path& file,
        ProjectFolderItem * parent )
{
    Q_D(AbstractFileManagerPlugin);

    qCDebug(FILEMANAGER) << "adding file" << file << "to" << parent->path();
    ProjectFileItem* created = nullptr;
    d->stopWatcher(parent);
    if ( createFile(file.toUrl()) ) {
        created = createFileItem( parent->project(), file, parent );
        if (created) {
            emit fileAdded(created);
        }
    }
    d->continueWatcher(parent);
    return created;
}

bool AbstractFileManagerPlugin::renameFolder(ProjectFolderItem* folder, const Path& newPath)
{
    Q_D(AbstractFileManagerPlugin);

    qCDebug(FILEMANAGER) << "trying to rename a folder:" << folder->path() << newPath;
    return d->rename(folder, newPath);
}

bool AbstractFileManagerPlugin::renameFile(ProjectFileItem* file, const Path& newPath)
{
    Q_D(AbstractFileManagerPlugin);

    qCDebug(FILEMANAGER) << "trying to rename a file:" << file->path() << newPath;
    return d->rename(file, newPath);
}

bool AbstractFileManagerPlugin::removeFilesAndFolders(const QList<ProjectBaseItem*> &items)
{
    Q_D(AbstractFileManagerPlugin);

    bool success = true;
    for (ProjectBaseItem* item : items) {
        Q_ASSERT(item->folder() || item->file());

        ProjectFolderItem* parent = parentFolder(item);
        d->stopWatcher(parent);

        success &= removeUrl(parent->project(), item->path().toUrl(), true);
        if ( success ) {
            if (item->file()) {
                emit fileRemoved(item->file());
            } else {
                Q_ASSERT(item->folder());
                emit folderRemoved(item->folder());
            }
            delete item;
        }

        d->continueWatcher(parent);
        if ( !success )
            break;
    }
    return success;
}

bool AbstractFileManagerPlugin::moveFilesAndFolders(const QList< ProjectBaseItem* >& items, ProjectFolderItem* newParent)
{
    Q_D(AbstractFileManagerPlugin);

    bool success = true;
    for (ProjectBaseItem* item : items) {
        Q_ASSERT(item->folder() || item->file());

        ProjectFolderItem* oldParent = parentFolder(item);
        d->stopWatcher(oldParent);
        d->stopWatcher(newParent);

        const Path oldPath = item->path();
        const Path newPath(newParent->path(), item->baseName());

        success &= renameUrl(oldParent->project(), oldPath.toUrl(), newPath. toUrl());
        if ( success ) {
            if (item->file()) {
                emit fileRemoved(item->file());
            } else {
                emit folderRemoved(item->folder());
            }
            delete item;
            auto* const readJob = d->eventuallyReadFolder(newParent);
            // reload first level synchronously, deeper levels will run async
            // this is required for code that expects the new item to exist after
            // this method finished
            readJob->exec();
        }

        d->continueWatcher(oldParent);
        d->continueWatcher(newParent);
        if ( !success )
            break;
    }
    return success;
}

bool AbstractFileManagerPlugin::copyFilesAndFolders(const Path::List& items, ProjectFolderItem* newParent)
{
    Q_D(AbstractFileManagerPlugin);

    bool success = true;
    for (const Path& item : items) {
        d->stopWatcher(newParent);

        success &= copyUrl(newParent->project(), item.toUrl(), newParent->path().toUrl());
        if ( success ) {
            auto* const readJob = d->eventuallyReadFolder(newParent);
            // reload first level synchronously, deeper levels will run async
            // this is required for code that expects the new item to exist after
            // this method finished
            readJob->exec();
        }

        d->continueWatcher(newParent);
        if ( !success )
            break;
    }
    return success;
}

bool AbstractFileManagerPlugin::isValid( const Path& path, const bool isFolder,
                                         IProject* project ) const
{
    Q_D(const AbstractFileManagerPlugin);

    return d->m_filters.isValid( path, isFolder, project );
}

ProjectFileItem* AbstractFileManagerPlugin::createFileItem( IProject* project, const Path& path,
                                                            ProjectBaseItem* parent )
{
    return new ProjectFileItem( project, path, parent );
}

ProjectFolderItem* AbstractFileManagerPlugin::createFolderItem( IProject* project, const Path& path,
                                                                ProjectBaseItem* parent )
{
    return new ProjectFolderItem( project, path, parent );
}

KDirWatch* AbstractFileManagerPlugin::projectWatcher( IProject* project ) const
{
    Q_D(const AbstractFileManagerPlugin);

    return d->m_watchers.value( project, nullptr );
}

//END Plugin

#include "moc_abstractfilemanagerplugin.cpp"
