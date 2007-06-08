/* This file is part of KDevelop
    Copyright (C) 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#include <projectmodel.h>
#include "projectfilesystemwatcher.h"
#include <QHash>
#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include <QQueue>
#include "kdebug.h"

class ProjectFileSystemWatcherPrivate
{
public:
    QFileSystemWatcher *m_watch;
    QHash< QString, KDevelop::ProjectFolderItem* > m_folderHash;
    QHash< QString, KDevelop::ProjectFileItem* > m_fileHash;
//     KDevelop::IProjectFileManager *m_manager;
};

ProjectFileSystemWatcher::ProjectFileSystemWatcher( QObject* parent  )
    : QObject( parent )
    , d( new ProjectFileSystemWatcherPrivate )
{
    d->m_watch = new QFileSystemWatcher(this);
    connect( d->m_watch, SIGNAL(directoryChanged ( const QString &)),
             this, SLOT(slotDirChanged(const QString&)) );
    connect( d->m_watch, SIGNAL(fileChanged ( const QString &)),
             this, SLOT(slotFileChanged(const QString&)) );
}

ProjectFileSystemWatcher::~ProjectFileSystemWatcher()
{
    delete d->m_watch;
    delete d;
}

void ProjectFileSystemWatcher::addDirectory( const QString &path, KDevelop::ProjectFolderItem *folderItem )
{
    d->m_watch->addPath( path );
    d->m_folderHash.insert( path, folderItem );
}

void ProjectFileSystemWatcher::addFile( const QString &path, KDevelop::ProjectFileItem *fileItem )
{
    d->m_watch->addPath( path );
    d->m_fileHash.insert( path, fileItem );
}

void ProjectFileSystemWatcher::removeDirectory( const QString & path, bool recurse )
{
    kDebug() << "Removing Directory from Watcher " << path << endl;
    QStringList tobeRemovedPaths;
    tobeRemovedPaths.append( path );
    d->m_folderHash.remove( path );

    if( recurse )
    {
    // ### note: Sometimes, when directories are being deleted, signal for parent directory is
    // emitted first, and signal for child directories are emitted at next, which results in crash.
    // So, when removing a directory, remove all of its children files and directories
        QList<QString> dirkeys = d->m_folderHash.keys();
        Q_FOREACH( QString _key, dirkeys )
        {
            if( _key.contains(path) )
            {
                kDebug() << "Removing Directory from Watcher: SubDir " << _key << endl;
                d->m_folderHash.remove( _key );
                tobeRemovedPaths << _key;
            }
        }

        // and same in case of files.
        QList<QString> filekeys = d->m_fileHash.keys();
        Q_FOREACH( QString _filekey, filekeys )
        {
            if( _filekey.contains(path) )
            {
                kDebug() << "Removing File      from Watcher: SubFile " << _filekey << endl;
                d->m_fileHash.remove( _filekey );
                tobeRemovedPaths << _filekey;
            }
        }
    }

    d->m_watch->removePaths( tobeRemovedPaths );
}

void ProjectFileSystemWatcher::removeFile( const QString & path )
{
    if( d->m_fileHash.contains( path ) )
    {
        kDebug() << "Removing file from Watcher " << path << endl;
        d->m_watch->removePath( path );
        d->m_fileHash.remove( path );
    }
}

// void ProjectFileSystemWatcher::removePaths( const QStringList & paths );

void ProjectFileSystemWatcher::slotDirChanged( const QString& path )
{
//     Q_ASSERT( d->m_folderHash.contains(path) );
    if( d->m_folderHash.contains(path) )
    {
        KDevelop::ProjectFolderItem *folder = d->m_folderHash.value( path );
        slotDirChangedInternal( path, folder );
    }
}

void ProjectFileSystemWatcher::slotFileChanged( const QString& path )
{
//     Q_ASSERT( d->m_fileHash.contains(path) );
    if( d->m_fileHash.contains(path) )
    {
        KDevelop::ProjectFileItem *file = d->m_fileHash.value( path );
        emit fileChanged( path, file );
    }
//     else
//     {
//         emit fileChanged( path, 0 );
//     }
}

void ProjectFileSystemWatcher::slotDirChangedInternal( const QString &dir, KDevelop::ProjectFolderItem* folderItem )
{
    QDir changedDir( dir );

    if( !changedDir.exists() )
    {
        //directory itself deleted
//         int row = folderItem->row();
//         QStandardItem *parent = folderItem->parent();
//         parent->removeRow( row );
//
//         this->removeDirectory( dir );

        QList<KDevelop::ProjectFolderItem*> deletedList;
        QStandardItem *parent = folderItem->parent();
        KDevelop::ProjectFolderItem *parentItem = dynamic_cast<KDevelop::ProjectFolderItem*>( parent );

        deletedList << folderItem;
        emit directoriesDeleted( deletedList, parentItem );

        return;
    }
    else //subdirectory or file is created or deleted.
    {
        // retrieve current disk info
        QFileInfoList fileEntries = changedDir.entryInfoList(QDir::Files);
        QFileInfoList dirEntries = changedDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);

        // convert disk info into QStringList
        QStringList fileList;
        for ( int i = 0; i < fileEntries.count(); ++i )
        {
            QFileInfo fileInfo = fileEntries.at( i );
            QString absFilePath = fileInfo.absoluteFilePath();
            fileList << absFilePath;
        }
        QStringList dirList;
        for ( int i = 0; i < dirEntries.count(); ++i )
        {
            QFileInfo fileInfo = dirEntries.at( i );
            QString absFilePath = fileInfo.absoluteFilePath();
            dirList << absFilePath;
        }

        // retrieve model item info, and convert into QStringList
        QList<KDevelop::ProjectFileItem*> itemFileList = folderItem->fileList();
        QStringList itemFileListString;
        Q_FOREACH( KDevelop::ProjectFileItem* _item, itemFileList )
        {
            itemFileListString << _item->url().toLocalFile();
        }

        QList<KDevelop::ProjectFolderItem*> itemFolderList = folderItem->folderList();
        QStringList itemFolderListString;
        Q_FOREACH( KDevelop::ProjectFolderItem *_item, itemFolderList )
        {
            itemFolderListString << _item->url().toLocalFile();
        }

        // Compare the difference between disk file and model items

        // round 1 -- file
//         KUrl::List deletedFileUrl;
        QList< KDevelop::ProjectFileItem* > deletedItems;
        Q_FOREACH( KDevelop::ProjectFileItem* _fileItem, itemFileList )
        {
            if( fileList.contains( _fileItem->url().toLocalFile() ) == false )
            {
                // disk file was deleted, so project file items should be also deleted

                // ### note: if some file, previously added to watching list, was deleted,
                // than QFSW::directoryChanged() is not emitted. Rather fileChanged() is emitted.
//                 int row = _fileItem->row();
//                 folderItem->removeRow( row );

                deletedItems << _fileItem;
            }
        }
        if( !deletedItems.isEmpty() )
            emit filesDeleted( deletedItems, folderItem );

        KUrl::List createdFileUrl;
        Q_FOREACH( QString diskFile, fileList )
        {
            if( itemFileListString.contains( diskFile ) == false )
            {
                // disk file was created, file items should be also created
//                 KDevelop::ProjectFileItem *newitem = new KDevelop::ProjectFileItem(
//                         folderItem->project(), KUrl(diskFile), folderItem );
//                 // if Makefile, parse new targets and add to watcher
//                 if( diskFile.endsWith( "/Makefile" ) ) // TODO portable, setting aware
//                 {
//                     QStringList newTargets = parseCustomMakeFile( KUrl(diskFile) );
//                     Q_FOREACH( QString newTarget, newTargets )
//                     {
//                         new CustomMakeTargetItem( folderItem->project(), newTarget, folderItem );
//                     }
//                     cmpi->fsWatcher()->addFile( diskFile, newitem );
//                 }
                createdFileUrl << KUrl(diskFile);
            }
        }
        if( !createdFileUrl.isEmpty() )
            emit filesCreated( createdFileUrl, folderItem );

        // round 2 -- directory
//         KUrl::List deletedDirs;
        QList< KDevelop::ProjectFolderItem* > deletedDirs;
        Q_FOREACH( KDevelop::ProjectFolderItem* _folderItem, itemFolderList )
        {
            if( dirList.contains( _folderItem->url().toLocalFile() ) == false)
            {
//                 int row = _folderItem->row();
//                 QString tobeRemovedDir = _folderItem->url().toLocalFile();
//                 folderItem->removeRow( row );
//
//                 this->removeDirectory( tobeRemovedDir );
                deletedDirs << _folderItem;
            }
        }
        if( !deletedDirs.isEmpty() )
            emit directoriesDeleted( deletedDirs, folderItem );

        KUrl::List createdDirs;
        Q_FOREACH( QString diskDir, dirList )
        {
            if( itemFolderListString.contains( diskDir ) == false )
            {
//                 KDevelop::ProjectBuildFolderItem *newitem =new KDevelop::ProjectBuildFolderItem(
//                         folderItem->project(), KUrl(diskDir), folderItem );
//
//                 this->addDirectory( diskDir, newitem );
//                 this->parseDirectoryRecursively( newitem, d->m_manager );
                createdDirs << KUrl(diskDir);
            }
        }
        if( !createdDirs.isEmpty() )
            emit directoriesCreated( createdDirs, folderItem );
    }
}


#include "projectfilesystemwatcher.moc"
