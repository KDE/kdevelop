/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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

#include "projectmodel.h"

#include <kmimetype.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kio/global.h>

#include "iproject.h"

namespace KDevelop
{

class ProjectBaseItemPrivate
{
    IProject* project;
};

ProjectBaseItem::ProjectBaseItem( IProject* project, const QString &name, QStandardItem *parent )
        : QStandardItem( name ), d(new ProjectBaseItemPrivate)
{
    d->project = project;
    if ( parent )
        parent->setChild( parent->rowCount(), this );
}

ProjectBaseItem::~ProjectBaseItem()
{
    delete d;
}

IProject* ProjectBaseItem::project() const
{
    return d->project;
}

void ProjectBaseItem::add( ProjectBaseItem* item )
{
    setChild( rowCount(), item );
}

ProjectFolderItem *ProjectBaseItem::folder() const
{
    return 0;
}

ProjectTargetItem *ProjectBaseItem::target() const
{
    return 0;
}

ProjectFileItem *ProjectBaseItem::file() const
{
    return 0;
}

ProjectItem *ProjectBaseItem::projectItem() const
{
    return 0;
}

QList<ProjectFolderItem*> ProjectBaseItem::folderList() const
{
    QList<ProjectFolderItem*> lst;
    for ( int i = 0; i < rowCount(); ++i )
    {
        QStandardItem* item = child( i );
        if ( item->type() == Folder || item->type() == BuildFolder )
        {
            ProjectFolderItem *kdevitem = dynamic_cast<ProjectFolderItem*>( item );
            if ( kdevitem )
                lst.append( kdevitem );
        }
    }

    return lst;
}

QList<ProjectTargetItem*> ProjectBaseItem::targetList() const
{
    QList<ProjectTargetItem*> lst;
    for ( int i = 0; i < rowCount(); ++i )
    {
        QStandardItem* item = child( i );
        if ( item->type() == Target )
        {
            ProjectTargetItem *kdevitem = dynamic_cast<ProjectTargetItem*>( item );
            if ( kdevitem )
                lst.append( kdevitem );
        }
    }

    return lst;
}

QList<ProjectFileItem*> ProjectBaseItem::fileList() const
{
    QList<ProjectFileItem*> lst;
    for ( int i = 0; i < rowCount(); ++i )
    {
        QStandardItem* item = child( i );
        if ( item->type() == File )
        {
            ProjectFileItem *kdevitem = dynamic_cast<ProjectFileItem*>( item );
            if ( kdevitem )
                lst.append( kdevitem );
        }

    }

    return lst;
}


ProjectItem::ProjectItem( IProject * project, const QString & name, QStandardItem * parent )
    : ProjectBuildFolderItem( project, project->folder(), parent )
{
    setText( name );
}

ProjectItem::~ ProjectItem( )
{
}

ProjectItem *ProjectItem::projectItem() const
{
    return const_cast<ProjectItem*>(this);
}

int ProjectItem::type( ) const
{
    return Project;
}

ProjectModel::ProjectModel( QObject *parent )
        : QStandardItemModel( parent )
{}

ProjectModel::~ProjectModel()
{}


ProjectBaseItem *ProjectModel::item( const QModelIndex &index ) const
{
    return reinterpret_cast<ProjectBaseItem*>( itemFromIndex( index ) );
}

void ProjectModel::resetModel()
{
    reset();
}

class ProjectFolderItemPrivate
{
public:
    KUrl m_url;
};

ProjectFolderItem::ProjectFolderItem( IProject* project, const KUrl & dir, QStandardItem * parent )
        : ProjectBaseItem( project, dir.directory(), parent ), d(new ProjectFolderItemPrivate)
{
    d->m_url = dir;
    setText( dir.fileName() );
    setIcon( KIO::pixmapForUrl( url(), 0, K3Icon::Small ) );
}

ProjectFolderItem::~ProjectFolderItem()
{
    delete d;
}

ProjectFolderItem *ProjectFolderItem::folder() const
{
    return const_cast<ProjectFolderItem*>(this);
}

int ProjectFolderItem::type() const
{
    return ProjectBaseItem::Folder;
}

const KUrl& ProjectFolderItem::url( ) const
{
    return d->m_url;
}

void ProjectFolderItem::setUrl( const KUrl& url )
{
    d->m_url = url;
    setText( url.fileName() );
}

class ProjectBuildFolderItemPrivate
{
public:
    KUrl m_url;
    KUrl::List m_includeDirs; ///include directories
    QHash<QString, QString> m_env;
};

ProjectBuildFolderItem::ProjectBuildFolderItem( IProject* project, const KUrl &dir, QStandardItem *parent)
    : ProjectFolderItem( project, dir, parent ), d(new ProjectBuildFolderItemPrivate)
{
    d->m_url = dir;
    setIcon( KIcon("folder-development") );
}

int ProjectBuildFolderItem::type() const
{
    return ProjectBaseItem::BuildFolder;
}

void ProjectBuildFolderItem::setIncludeDirectories( const KUrl::List& dirList )
{
    d->m_includeDirs = dirList;
}

const KUrl::List& ProjectBuildFolderItem::includeDirectories() const
{
    return d->m_includeDirs;
}

const QHash<QString, QString>& ProjectBuildFolderItem::environment() const
{
    return d->m_env;
}

class ProjectFileItemPrivate
{
    public:
        KUrl m_url;
};

ProjectFileItem::ProjectFileItem( IProject* project, const KUrl & file, QStandardItem * parent )
        : ProjectBaseItem( project, file.fileName(), parent ), d(new ProjectFileItemPrivate)
{
    d->m_url = file;
    setIcon( KIO::pixmapForUrl( url(), 0, K3Icon::Small ) );
}

const KUrl & ProjectFileItem::url( ) const
{
    return d->m_url;
}

void ProjectFileItem::setUrl( const KUrl& url )
{
    d->m_url = url;
}

int ProjectFileItem::type() const
{
    return ProjectBaseItem::File;
}

ProjectFileItem *ProjectFileItem::file() const
{
    return const_cast<ProjectFileItem*>( this );
}

ProjectTargetItem::ProjectTargetItem( IProject* project, const QString &name, QStandardItem *parent )
                : ProjectBaseItem( project, name, parent )
{}

int ProjectTargetItem::type() const
{
    return ProjectBaseItem::Target;
}

ProjectTargetItem *ProjectTargetItem::target() const
{
    return const_cast<ProjectTargetItem*>( this );
}

}
#include "projectmodel.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
