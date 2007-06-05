/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <projectmodel.h>
#include "projectfilesystemwatcher.h"
#include <QHash>
#include <QFileSystemWatcher>

ProjectFileSystemWatcher::ProjectFileSystemWatcher( QObject* parent  )
    : QObject( parent )
{
    m_watch = new QFileSystemWatcher(this);
    connect( m_watch, SIGNAL(directoryChanged ( const QString &)),
             this, SLOT(slotDirChanged(const QString&)) );
    connect( m_watch, SIGNAL(fileChanged ( const QString &)),
             this, SLOT(slotFileChanged(const QString&)) );
}

ProjectFileSystemWatcher::~ProjectFileSystemWatcher()
{
    delete m_watch;
}

void ProjectFileSystemWatcher::addDirectory( const QString &path, KDevelop::ProjectFolderItem *folderItem )
{
    m_watch->addPath( path );
    m_folderHash.insert( path, folderItem );
}

void ProjectFileSystemWatcher::addFile( const QString &path, KDevelop::ProjectFileItem *fileItem )
{
    m_watch->addPath( path );
    if( fileItem )
    {
        m_fileHash.insert( path, fileItem );
    }
}

void ProjectFileSystemWatcher::removeDirectory( const QString & path )
{
    m_watch->removePath( path );
//     if( m_folderHash.contains( path ) )
//     {
    m_folderHash.remove( path );
//     }

}

void ProjectFileSystemWatcher::removeFile( const QString & path )
{
    m_watch->removePath( path );
    m_fileHash.remove( path );
}

// void ProjectFileSystemWatcher::removePaths( const QStringList & paths );

void ProjectFileSystemWatcher::slotDirChanged( const QString& path )
{
    Q_ASSERT( m_folderHash.contains(path) );
    if( m_folderHash.contains(path) )
    {
        KDevelop::ProjectFolderItem *folder = m_folderHash.value( path );
        emit directoryChanged( path, folder );
    }
}

void ProjectFileSystemWatcher::slotFileChanged( const QString& path )
{
    if( m_fileHash.contains(path) )
    {
        KDevelop::ProjectFileItem *file = m_fileHash.value( path );
        emit fileChanged( path, file );
    }
    else
    {
        emit fileChanged( path, 0 );
    }
}

#include "projectfilesystemwatcher.moc"
