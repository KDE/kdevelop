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
//     void removePaths ( const QStringList & paths );

Q_SIGNALS:
    void directoryChanged( const QString & path, KDevelop::ProjectFolderItem* folderItem );
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
