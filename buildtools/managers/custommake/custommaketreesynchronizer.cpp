/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "custommaketreesynchronizer.h"
#include "projectfilesystemwatcher.h"
#include "custommakemanager.h"
#include "custommakemodelitems.h"
#include "iprojectcontroller.h"
#include "iproject.h"
#include "icore.h"
#include <kdebug.h>
#include <QFileInfo>
#include <QQueue>

CustomMakeTreeSynchronizer::CustomMakeTreeSynchronizer(CustomMakeManager* manager, QObject* parent)
    : QObject( parent )
{
    m_customMan = manager;
    m_watch = new ProjectFileSystemWatcher( this );

    connect( m_watch, SIGNAL(fileChanged(const QString&, KDevelop::ProjectFileItem*)),
             this, SLOT(fileChanged(const QString&, KDevelop::ProjectFileItem*)) );

    connect( m_watch, SIGNAL(directoriesCreated( const KUrl::List&, KDevelop::ProjectFolderItem* )),
             this, SLOT(directoriesCreated(const KUrl::List&, KDevelop::ProjectFolderItem*)) );
    connect( m_watch, SIGNAL(directoriesDeleted( const QList<KDevelop::ProjectFolderItem*> &,
                          KDevelop::ProjectFolderItem *)),
             this,    SLOT(directoriesDeleted(const QList< KDevelop :: ProjectFolderItem * >&,
                          KDevelop::ProjectFolderItem*)) );

    connect( m_watch, SIGNAL(filesCreated( const KUrl::List&, KDevelop::ProjectFolderItem* )),
             this, SLOT(filesCreated(const KUrl::List&, KDevelop::ProjectFolderItem*)) );
    connect( m_watch, SIGNAL(filesDeleted( const QList<KDevelop::ProjectFileItem*>&, KDevelop::ProjectFolderItem* ) ),
             this, SLOT(filesDeleted(const QList< KDevelop :: ProjectFileItem * >&, KDevelop::ProjectFolderItem*)) );

}
CustomMakeTreeSynchronizer::~CustomMakeTreeSynchronizer()
{}

void CustomMakeTreeSynchronizer::addDirectory( const QString &path, KDevelop::ProjectFolderItem *folderItem )
{
    m_watch->addDirectory( path, folderItem );
}

void CustomMakeTreeSynchronizer::addFile( const QString &path, KDevelop::ProjectFileItem *fileItem )
{
    m_watch->addFile( path, fileItem );
}

void CustomMakeTreeSynchronizer::removeDirectory( const QString & path, bool recurse )
{
    m_watch->removeDirectory( path, recurse );
}

void CustomMakeTreeSynchronizer::removeFile( const QString & path )
{
    m_watch->removeFile( path );
}

void CustomMakeTreeSynchronizer::filesCreated( const KUrl::List &files,
                                               KDevelop::ProjectFolderItem *parentFolder )
{
    Q_FOREACH( KUrl _file, files )
    {
        KDevelop::ProjectFileItem *newitem = new KDevelop::ProjectFileItem(
            parentFolder->project(), _file, parentFolder );
        // if Makefile, parse new targets and add to watcher
        if( _file.fileName() == QString( "Makefile" ) ) // TODO portable, setting aware
        {
            QStringList newTargets = m_customMan->parseCustomMakeFile( _file );
            Q_FOREACH( QString newTarget, newTargets )
            {
                new CustomMakeTargetItem( parentFolder->project(), newTarget, parentFolder );
            }
            addFile( _file.toLocalFile(), newitem );
        }
    }
}
void CustomMakeTreeSynchronizer::filesDeleted( const QList<KDevelop::ProjectFileItem*> &files,
                            KDevelop::ProjectFolderItem *parentFolder )
{
    Q_FOREACH( KDevelop::ProjectFileItem *_item, files )
    {
        int row = _item->row();
        removeFile( _item->url().toLocalFile() );
        parentFolder->removeRow( row );
    }
}

void CustomMakeTreeSynchronizer::directoriesCreated( const KUrl::List &files,
                                        KDevelop::ProjectFolderItem *parentFolder )
{
    Q_FOREACH( KUrl _file, files )
    {
        KDevelop::ProjectBuildFolderItem *newitem = new KDevelop::ProjectBuildFolderItem(
                parentFolder->project(), _file, parentFolder );

        addDirectory( _file.toLocalFile(), newitem );
        this->parseDirectoryRecursively( newitem, m_customMan );
    }
}

void CustomMakeTreeSynchronizer::directoriesDeleted( const QList<KDevelop::ProjectFolderItem*> &dirs,
                                        KDevelop::ProjectFolderItem *parentFolder )
{
    Q_FOREACH( KDevelop::ProjectFolderItem *_item, dirs )
    {
        int row = _item->row();
        QString tobeRemovedDir = _item->url().toLocalFile();
        parentFolder->removeRow( row );

        removeDirectory( tobeRemovedDir, true );
    }
}

void CustomMakeTreeSynchronizer::fileChanged( const QString& file, KDevelop::ProjectFileItem* fileItem)
{
    kDebug() << "File" << file << "changed";

    QFileInfo info( file );
    if( info.fileName() != QString("Makefile") )
        return;

    // find Makefile item, because it is allowed to be null
    KDevelop::ProjectFileItem *makefileItem=0;
    KDevelop::IProject *project=0;
    if( !fileItem )
    {
        KUrl url(file);
        project = m_customMan->core()->projectController()->findProjectForUrl( url );
        Q_ASSERT(project);
        makefileItem = project->fileForUrl( KUrl(file) );
    }
    else
    {
        makefileItem = fileItem;
        project = makefileItem->project();
    }
    Q_ASSERT(makefileItem);

    // find parent folder item
    QStandardItem *stditem = makefileItem->parent();
    KDevelop::ProjectBuildFolderItem *parentFolder =
            dynamic_cast<KDevelop::ProjectBuildFolderItem*>( stditem );
    if( !parentFolder )
        return;

    // delete every targets in the fileItem's parent directory
    QList<KDevelop::ProjectTargetItem*> targets = parentFolder->targetList();
    Q_FOREACH( KDevelop::ProjectTargetItem* _deletingTarget, targets )
    {
        int targetrow = _deletingTarget->row();
        parentFolder->removeRow( targetrow );
    }

    // determine whether the file contents were modified or the entire file itself was deleted.
    if( info.exists() == false )
    {
        // Makefile deleted
        KDevelop::ProjectItem *prjitem = project->projectItem();
        CustomMakeProjectItem *cmpi = dynamic_cast<CustomMakeProjectItem*>( prjitem );
        cmpi->fsWatcher()->removeFile( file );
        int makefileRow = makefileItem->row();
        parentFolder->removeRow( makefileRow );
    }
    else
    {
        // Makefile contents modified
        QStringList newTargets = m_customMan->parseCustomMakeFile( KUrl(file) );
        Q_FOREACH( QString newTarget, newTargets )
        {
            new CustomMakeTargetItem( project, newTarget, parentFolder );
        }
    }

}

void CustomMakeTreeSynchronizer::parseDirectoryRecursively( KDevelop::ProjectFolderItem* dir,
        KDevelop::IProjectFileManager* manager )
{
    QQueue< QList<KDevelop::ProjectFolderItem*> > workQueue;
    QList<KDevelop::ProjectFolderItem*> initial;
    initial.append( dir );
    workQueue.enqueue( initial );

    while( workQueue.count() > 0 )
    {
        QList<KDevelop::ProjectFolderItem*> front = workQueue.dequeue();
        Q_FOREACH( KDevelop::ProjectFolderItem* _item, front )
        {
            QList<KDevelop::ProjectFolderItem*> workingList = m_customMan->parse( _item );
            if( workingList.count() > 0 )
                workQueue.enqueue( workingList );
        }
    }
}


#include "custommaketreesynchronizer.moc"
