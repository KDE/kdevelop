/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef CUSTOMMAKETREESYNCHRONIZER_H
#define CUSTOMMAKETREESYNCHRONIZER_H

#include "projectfilesystemwatcher.h"

class KUrl;
class KUrl::List;

namespace KDevelop
{
class ProjectFileItem;
class ProjectFolderItem;
class IProjectFileManager;
}

class CustomMakeManager;
class ProjectFileSystemWatcher;

class CustomMakeTreeSynchronizer : public QObject
{
    Q_OBJECT
public:
    explicit CustomMakeTreeSynchronizer( CustomMakeManager* manager, QObject* parent = 0 );
    virtual ~CustomMakeTreeSynchronizer();

    void addDirectory( const QString &path, KDevelop::ProjectFolderItem *folderItem );
    void addFile( const QString &path, KDevelop::ProjectFileItem *fileItem = 0 );

    void removeDirectory( const QString & path, bool recurse );
    void removeFile( const QString & path );

protected Q_SLOTS:
    void filesCreated( const KUrl::List &files, KDevelop::ProjectFolderItem *parentFolder );
    void filesDeleted( const QList<KDevelop::ProjectFileItem*> &files,
                               KDevelop::ProjectFolderItem *parentFolder );

    void directoriesCreated( const KUrl::List &files, KDevelop::ProjectFolderItem *parentFolder );
    void directoriesDeleted( const QList<KDevelop::ProjectFolderItem*> &dirs,
                                     KDevelop::ProjectFolderItem *parentFolder );

    void fileChanged( const QString& file, KDevelop::ProjectFileItem* fileItem);

private:
    void parseDirectoryRecursively( KDevelop::ProjectFolderItem* dir,
                                    KDevelop::IProjectFileManager* manager );

private:
    CustomMakeManager *m_customMan;
    ProjectFileSystemWatcher *m_watch;
};

#endif
