/* This file is part of KDevelop
    Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Aleix Pol <aleixpol@gmail.com>

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

#include <QApplication>
#include <QPalette>
#include <QBrush>
#include <QColor>
#include <kdebug.h>

#include "iproject.h"

namespace KDevelop
{

class ProjectBaseItemPrivate
{
public:
    ProjectBaseItemPrivate() : project(0) {}
    IProject* project;
};

class ProjectFolderItemPrivate : public ProjectBaseItemPrivate
{
public:
    KUrl m_url;
    bool m_isProjectRoot;
};

class ProjectBuildFolderItemPrivate : public ProjectFolderItemPrivate
{
public:
    KUrl::List m_includeDirs; ///include directories
    QHash<QString, QString> m_env;
};

class ProjectFileItemPrivate : public ProjectBaseItemPrivate
{
public:
    KUrl m_url;
};

class ProjectTargetItemPrivate : public ProjectBaseItemPrivate
{
};

ProjectBaseItem::ProjectBaseItem( IProject* project, const QString &name, QStandardItem *parent )
        : QStandardItem( name ), d_ptr(new ProjectBaseItemPrivate)
{
    Q_D(ProjectBaseItem);
    d->project = project;
    setParent( parent );
}

ProjectBaseItem::ProjectBaseItem( ProjectBaseItemPrivate& dd)
    : d_ptr(&dd)
{
}

ProjectBaseItem::~ProjectBaseItem()
{
    Q_D(ProjectBaseItem);
    delete d;
}

IProject* ProjectBaseItem::project() const
{
    Q_D(const ProjectBaseItem);
    return d->project;
}

void ProjectBaseItem::setParent( QStandardItem* parent )
{
    if( parent )
        parent->setChild( parent->rowCount(), this );
}

void ProjectBaseItem::setIcon()
{
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

ProjectModel::ProjectModel( QObject *parent )
        : QStandardItemModel( parent ), d(0)
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

void ProjectModel::fetchMore( const QModelIndex &parent )
{
    QStandardItem *parentItem = itemFromIndex( parent );
    if( !parentItem )
        return;
    int rowcount = parentItem->rowCount();
    for( int i=0; i<rowcount; i++ )
    {
        ProjectBaseItem *childItem = dynamic_cast<ProjectBaseItem*>(parentItem->child(i));
        if( childItem && childItem->icon().isNull() )
            childItem->setIcon();
    }
}

bool ProjectModel::canFetchMore( const QModelIndex & parent ) const
{
    QStandardItem *parentItem = itemFromIndex( parent );
    if( !parentItem )
        return false;
    return true;
}


ProjectFolderItem::ProjectFolderItem( IProject* project, const KUrl & dir, QStandardItem * parent )
        : ProjectBaseItem( *new ProjectFolderItemPrivate )
{
    Q_D(ProjectFolderItem);
    d->project = project;
    d->m_url = dir;
    setParent(parent);
    setText( dir.fileName() );
}

ProjectFolderItem::ProjectFolderItem( ProjectFolderItemPrivate& dd)
    : ProjectBaseItem( dd )
{
}

ProjectFolderItem::~ProjectFolderItem()
{
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
    Q_D(const ProjectFolderItem);
    return d->m_url;
}

void ProjectFolderItem::setUrl( const KUrl& url )
{
    Q_D(ProjectFolderItem);
    d->m_url = url;
    setText( url.fileName() );
}

void ProjectFolderItem::setIcon()
{
    QStandardItem::setIcon( KIO::pixmapForUrl( url(), 0, KIconLoader::Small ) );
}

ProjectBuildFolderItem::ProjectBuildFolderItem( ProjectBuildFolderItemPrivate& dd )
    : ProjectFolderItem( dd )
{
}

ProjectBuildFolderItem::ProjectBuildFolderItem( IProject* project, const KUrl &dir, QStandardItem *parent)
    : ProjectFolderItem( *new ProjectBuildFolderItemPrivate )
{
    Q_D(ProjectBuildFolderItem);
    d->project = project;
    setUrl( dir );
    setParent( parent );
}

int ProjectBuildFolderItem::type() const
{
    return ProjectBaseItem::BuildFolder;
}

void ProjectBuildFolderItem::setIncludeDirectories( const KUrl::List& dirList )
{
    Q_D(ProjectBuildFolderItem);
    d->m_includeDirs = dirList;
}

KUrl::List ProjectBuildFolderItem::includeDirectories() const
{
    Q_D(const ProjectBuildFolderItem);
    return d->m_includeDirs;
}

QHash<QString, QString> ProjectBuildFolderItem::environment() const
{
    Q_D(const ProjectBuildFolderItem);
    return d->m_env;
}

void ProjectBuildFolderItem::setIcon()
{
    QStandardItem::setIcon( KIcon("folder-development") );
}

void ProjectFolderItem::setProjectRoot(bool isRoot)
{
	Q_D(ProjectFolderItem);
	d->m_isProjectRoot=isRoot;
	setText(project()->name());
}

bool ProjectFolderItem::isProjectRoot() const
{
	Q_D(const ProjectFolderItem);
	return d->m_isProjectRoot;
}

ProjectFileItem::ProjectFileItem( ProjectFileItemPrivate& dd)
    : ProjectBaseItem(dd)
{
}

ProjectFileItem::ProjectFileItem( IProject* project, const KUrl & file, QStandardItem * parent )
        : ProjectBaseItem( *new ProjectFileItemPrivate )
{
    Q_D(ProjectFileItem);
    d->project = project;
    d->m_url = file;
    setText( file.fileName() );
    setParent( parent );
}

const KUrl & ProjectFileItem::url( ) const
{
    Q_D(const ProjectFileItem);
    return d->m_url;
}

void ProjectFileItem::setUrl( const KUrl& url )
{
    Q_D(ProjectFileItem);
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

void ProjectFileItem::setIcon()
{
    QStandardItem::setIcon( KIO::pixmapForUrl( url(), 0, KIconLoader::Small ) );
}

ProjectTargetItem::ProjectTargetItem( ProjectTargetItemPrivate& dd)
    : ProjectBaseItem( dd )
{
}

ProjectTargetItem::ProjectTargetItem( IProject* project, const QString &name, QStandardItem *parent )
                : ProjectBaseItem( *new ProjectTargetItemPrivate )
{
    Q_D(ProjectTargetItem);
    d->project = project;
    setText( name );
    setParent( parent );
}

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
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
