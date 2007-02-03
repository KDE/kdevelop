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
#include <kio/global.h>

namespace Koncrete
{

ProjectItem::ProjectItem( const QString &name, QStandardItem *parent )
        : QStandardItem( name )
{
    if ( parent )
        parent->setChild( parent->rowCount(), this );
}

void ProjectItem::add( ProjectItem* item )
{
    setChild( rowCount(), item );
}
ProjectFolderItem *ProjectItem::folder() const
{
    return 0;
}

ProjectTargetItem *ProjectItem::target() const
{
    return 0;
}

ProjectFileItem *ProjectItem::file() const
{
    return 0;
}

QList<ProjectFolderItem*> ProjectItem::folderList() const
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

QList<ProjectTargetItem*> ProjectItem::targetList() const
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

QList<ProjectFileItem*> ProjectItem::fileList() const
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

ProjectModel::ProjectModel( QObject *parent )
        : QStandardItemModel( parent )
{}

ProjectModel::~ProjectModel()
{}


ProjectItem *ProjectModel::item( const QModelIndex &index ) const
{
    return reinterpret_cast<ProjectItem*>( itemFromIndex( index ) );
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

ProjectFolderItem::ProjectFolderItem( const KUrl & dir, QStandardItem * parent )
        : ProjectItem( dir.directory(), parent ), d(new ProjectFolderItemPrivate)
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
    return ProjectItem::Folder;
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

ProjectBuildFolderItem::ProjectBuildFolderItem( const KUrl &dir, QStandardItem *parent)
    : ProjectFolderItem( dir, parent ), d(new ProjectBuildFolderItemPrivate)
{
    d->m_url = dir;
}

int ProjectBuildFolderItem::type() const
{
    return ProjectItem::BuildFolder;
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

ProjectFileItem::ProjectFileItem( const KUrl & file, QStandardItem * parent )
        : ProjectItem( file.fileName(), parent ), d(new ProjectFileItemPrivate)
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
    return ProjectItem::File;
}

ProjectFileItem *ProjectFileItem::file() const
{
    return const_cast<ProjectFileItem*>( this );
}

ProjectTargetItem::ProjectTargetItem( const QString &name, QStandardItem *parent )
                : ProjectItem( name, parent )
{}

int ProjectTargetItem::type() const
{
    return ProjectItem::Target;
}

ProjectTargetItem *ProjectTargetItem::target() const
{
    return const_cast<ProjectTargetItem*>( this );
}

}
#include "projectmodel.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

