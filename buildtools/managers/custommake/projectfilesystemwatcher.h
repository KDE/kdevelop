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

class QFileSystemWatcher;
// template <typename T1, typename T2> class QHash;
#include <QHash>
namespace KDevelop
{
class ProjectFolderItem;
class ProjectFileItem;
}

/**
 * Simple QFileSystemWatcher wrapper. Designed to be used in custom make manager.
 * Emits directory/file changed signal with ProjectFolderItem or ProjectFileItem
 */
class ProjectFileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    ProjectFileSystemWatcher( QObject* parent = 0 );
    virtual ~ProjectFileSystemWatcher();

    void addDirectory( const QString &path, KDevelop::ProjectFolderItem *folderItem );
    void addFile( const QString &path, KDevelop::ProjectFileItem *fileItem = 0 );

    void removeDirectory( const QString & path );
    void removeFile( const QString & path );
//     void removePaths ( const QStringList & paths );

Q_SIGNALS:
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
    void directoryChanged( const QString & path, KDevelop::ProjectFolderItem* folderItem );

    /**
     * Emitted when the contents of file were modified, or the file itself was deleted on disk.
     */
    void fileChanged( const QString & path, KDevelop::ProjectFileItem* fileItem );

private Q_SLOTS:
    void slotDirChanged( const QString& path );
    void slotFileChanged( const QString& path );

private:
    QFileSystemWatcher *m_watch;
    QHash< QString, KDevelop::ProjectFolderItem* > m_folderHash;
    QHash< QString, KDevelop::ProjectFileItem* > m_fileHash;

};

#endif
