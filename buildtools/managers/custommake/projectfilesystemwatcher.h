/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef PROJECTFILESYSTEMWATCHER_H
#define PROJECTFILESYSTEMWATCHER_H

#include <QObject>

// template <typename T1, typename T2> class QHash;
#include "kurl.h"
namespace KDevelop
{
class ProjectFolderItem;
class ProjectFileItem;
class IProjectFileManager;
}

class ProjectFileSystemWatcherPrivate;

/**
 * Simple QFileSystemWatcher wrapper.
 * Emits directory/file changed signal with ProjectFolderItem or ProjectFileItem
 */
class ProjectFileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    ProjectFileSystemWatcher( /*KDevelop::IProjectFileManager* manager, */QObject* parent = 0 );
    virtual ~ProjectFileSystemWatcher();

    void addDirectory( const QString &path, KDevelop::ProjectFolderItem *folderItem );
    void addFile( const QString &path, KDevelop::ProjectFileItem *fileItem );

    /** Note: Sometimes, when directory trees are being deleted, directoriesDeleted() signal for
     * parent directory is emitted first, and signal for child directories are emitted next.
     * If you delete project**items in a slot connected to dirsDeleted() for parent directory,
     * every child items will be also deleted.
     * This results in crash when you attempt to delete child directries in second slots
     *
     * So, when directories are being deleted, set @arg recurse TRUE.
     */
    void removeDirectory( const QString & path, bool recurse );

    void removeFile( const QString & path );
//     void removePaths ( const QStringList & paths );

// Q_SIGNALS:
    /**
     * Emitted when the files or subdirectories under @arg path were created or deleted.
     *
     * Note that if some file, which was being monitored by addFile(), is deleted,
     * directoryChanged() is not emitted. Rather only fileChanged() will be emitted in that case.
     *
     * Also, if both the parent and *empty* child directory are being watched, and child directory
     * is deleted, the directoryChanged( parentDir ) will be emitted. But if child directory
     * is not empty, then directoryChanged( childDir ) will be emitted.
     */
//     void directoryChanged( const QString & path, KDevelop::ProjectFolderItem* folderItem );

    /**
     * Emitted when the contents of file were modified, or the file itself was deleted on disk.
     */
//     void fileChanged( const QString & path, KDevelop::ProjectFileItem* fileItem );

Q_SIGNALS:
    /**
     * Emitted when the contents of file were modified, or the file itself was deleted on disk.
     * This signal is emitted only for files being watched.
     */
    virtual void fileChanged( const QString & path, KDevelop::ProjectFileItem* fileItem );

    /**
     * Emitted when files under @arg parentFolder, which is being monitored, were created.
     */
    virtual void filesCreated( const KUrl::List &files, KDevelop::ProjectFolderItem *parentFolder );

    /**
     * Emitted when files under @arg parentFolder, which is being monitored, were deleted.
     */
    virtual void filesDeleted( const QList<KDevelop::ProjectFileItem*> &files,
                               KDevelop::ProjectFolderItem *parentFolder );

    /**
     * Emitted when subdirectories under @arg parentFolder, which is being monitored, were created.
     */
    virtual void directoriesCreated( const KUrl::List &files, KDevelop::ProjectFolderItem *parentFolder );

    /**
     * Emitted when subdirectories under @arg parentFolder, which is being monitored, were deleted.
     */
    virtual void directoriesDeleted( const QList<KDevelop::ProjectFolderItem*> &dirs,
                                     KDevelop::ProjectFolderItem *parentFolder );

private Q_SLOTS:
    void slotDirChanged( const QString& path );
    void slotDirChangedInternal( const QString &dir, KDevelop::ProjectFolderItem* folderItem );
    void slotFileChanged( const QString& path );

private:
    ProjectFileSystemWatcherPrivate *d;
};

#endif
