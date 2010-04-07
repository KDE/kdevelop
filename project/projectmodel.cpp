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
#include <QFileInfo>
#include <kdebug.h>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include "interfaces/iprojectfilemanager.h"
#include <KIO/NetAccess>
#include <language/duchain/indexedstring.h>
#include <KLocalizedString>
#include <KMessageBox>

namespace KDevelop
{

QStringList removeProjectBasePath( const QStringList& fullpath, KDevelop::ProjectBaseItem* item )
{
    QStringList result = fullpath;
    if( item )
    {
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        QStringList basePath = model->pathFromIndex( model->indexFromItem( item ) );
        if( basePath.count() >= fullpath.count() )
        {
            return QStringList();
        }
        for( int i = 0; i < basePath.count(); i++ )
        {
            result.takeFirst();
        }
    }
    return result;
}

QStringList joinProjectBasePath( const QStringList& partialpath, KDevelop::ProjectBaseItem* item )
{
    QStringList basePath;
    if( item )
    {
        KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
        basePath = model->pathFromIndex( model->indexFromItem( item ) );
    }
    return basePath + partialpath;
}


class ProjectBaseItemPrivate
{
public:
    ProjectBaseItemPrivate() : project(0) {}
    IProject* project;
};

class ProjectFolderItemPrivate : public ProjectBaseItemPrivate
{
public:
    ProjectFolderItemPrivate() : m_isProjectRoot(false) {
    }
    KUrl m_url;
    QString m_folderName;
    bool m_isProjectRoot; ///@todo Why is this needed at all? Shouldn't it equal "parent() == 0" ?
};

class ProjectBuildFolderItemPrivate : public ProjectFolderItemPrivate
{
public:
};

class ProjectFileItemPrivate : public ProjectBaseItemPrivate
{
public:
    KUrl m_url;
    QString m_fileName;
};

class ProjectTargetItemPrivate : public ProjectBaseItemPrivate
{
};

class ProjectModelPrivate
{
};

ProjectBaseItem::ProjectBaseItem( IProject* project, const QString &name, QStandardItem *parent )
        : QStandardItem( name ), d_ptr(new ProjectBaseItemPrivate)
{
    Q_D(ProjectBaseItem);
    d->project = project;
    setParent( parent );
    setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
}

ProjectBaseItem::ProjectBaseItem( ProjectBaseItemPrivate& dd)
    : d_ptr(&dd)
{
    setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
}

ProjectBaseItem::~ProjectBaseItem()
{
    Q_D(ProjectBaseItem);
    delete d;
}

KUrl ProjectBaseItem::url() const
{
    return KUrl();
}

KDevelop::ProjectBaseItem::ProjectItemType baseType( int type )
{
    if( type == KDevelop::ProjectBaseItem::Folder || type == KDevelop::ProjectBaseItem::BuildFolder )
        return KDevelop::ProjectBaseItem::Folder;
    if( type == KDevelop::ProjectBaseItem::Target || type == KDevelop::ProjectBaseItem::ExecutableTarget
        || type == KDevelop::ProjectBaseItem::LibraryTarget)
        return KDevelop::ProjectBaseItem::Target;

    return static_cast<KDevelop::ProjectBaseItem::ProjectItemType>( type );
}

bool ProjectBaseItem::lessThan( const KDevelop::ProjectBaseItem* item ) const
{
    if(item->type() >= KDevelop::ProjectBaseItem::CustomProjectItemType ) {
        // For custom types we want to make sure that if they override lessThan, then we
        // prefer their lessThan implementation
        return !item->lessThan( this );
    }
    KDevelop::ProjectBaseItem::ProjectItemType leftType=baseType(type()), rightType=baseType(item->type());
    if(leftType==rightType)
    {
        if(leftType==KDevelop::ProjectBaseItem::File)
        {
            return file()->fileName().compare(item->file()->fileName(), Qt::CaseInsensitive) < 0;
        }
        return *this<*item;
    }
    else
    {
        return leftType<rightType;
    }

    return false;
}

IProject* ProjectBaseItem::project() const
{
    Q_D(const ProjectBaseItem);
    return d->project;
}

void ProjectBaseItem::setParent( QStandardItem* newParent )
{
    if( newParent && parent() != newParent )
        newParent->setChild( newParent->rowCount(), this );
}

void ProjectBaseItem::add( ProjectBaseItem* item )
{
    appendRow( item );
}

ProjectFolderItem *ProjectBaseItem::folder() const
{
    return 0;
}

ProjectTargetItem *ProjectBaseItem::target() const
{
    return 0;
}

ProjectExecutableTargetItem *ProjectBaseItem::executable() const
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
        Q_ASSERT(item);
        if ( item && ( item->type() == Target || item->type() == LibraryTarget ||
             item->type() == ExecutableTarget ) )
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
        Q_ASSERT(item);
        if ( item && item->type() == File )
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
{
}

ProjectModel::~ProjectModel()
{}


ProjectBaseItem *ProjectModel::item( const QModelIndex &index ) const
{
    return dynamic_cast<ProjectBaseItem*>( itemFromIndex( index ) );
}

void ProjectModel::resetModel()
{
    reset();
}

ProjectFolderItem::ProjectFolderItem( IProject* project, const KUrl & dir, QStandardItem * parent )
        : ProjectBaseItem( *new ProjectFolderItemPrivate )
{
    Q_D(ProjectFolderItem);
    d->project = project;
    setUrl(dir);
    setParent(parent);
    setIcon(KIcon("folder"));
    setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
}

ProjectFolderItem::ProjectFolderItem( ProjectFolderItemPrivate& dd)
    : ProjectBaseItem( dd )
{
    setIcon(KIcon("folder"));
    setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
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

KUrl ProjectFolderItem::url( ) const
{
    Q_D(const ProjectFolderItem);
    return d->m_url;
}

const QString& ProjectFolderItem::folderName() const
{
    Q_D(const ProjectFolderItem);
    return d->m_folderName;
}

void ProjectFolderItem::setUrl( const KUrl& url )
{
    Q_D(ProjectFolderItem);
    d->m_url = url;
    d->m_folderName = d->m_url.fileName();
    setText( d->m_folderName );
    setToolTip( text() );
}

bool ProjectFolderItem::hasFileOrFolder(const QString& name) const
{
    for ( int i = 0; i < rowCount(); ++i )
    {
        QStandardItem* item = child( i );
        if ( ProjectFileItem* file = dynamic_cast<ProjectFileItem*>(item))
            if (file->fileName() == name)
                return true;

        if ( ProjectFolderItem* folder = dynamic_cast<ProjectFolderItem*>(item))
            if (folder->folderName() == name)
                return true;
    }
    return false;
}

ProjectBuildFolderItem::ProjectBuildFolderItem( ProjectBuildFolderItemPrivate& dd )
    : ProjectFolderItem( dd )
{
    setIcon(KIcon("folder-development"));
}

ProjectBuildFolderItem::ProjectBuildFolderItem( IProject* project, const KUrl &dir, QStandardItem *parent)
    : ProjectFolderItem( *new ProjectBuildFolderItemPrivate )
{
    Q_D(ProjectBuildFolderItem);
    d->project = project;
    setUrl( dir );
    setParent( parent );
    setIcon(KIcon("folder-development"));
}

int ProjectBuildFolderItem::type() const
{
    return ProjectBaseItem::BuildFolder;
}

void ProjectFolderItem::setProjectRoot(bool isRoot)
{
	Q_D(ProjectFolderItem);
	d->m_isProjectRoot=isRoot;
	setText(project()->name());
        setToolTip( text() );
}

bool ProjectFolderItem::isProjectRoot() const
{
	Q_D(const ProjectFolderItem);
	return d->m_isProjectRoot;
}

void ProjectFolderItem::setData(const QVariant& value, int role)
{
    if(role==Qt::EditRole && value != data(role)) {
        KUrl dest = url().upUrl();
        dest.addPath(value.toString());
        bool ret=!value.toString().contains('/');
        
        KIO::UDSEntry entry;
        ret = ret && !KIO::NetAccess::stat(dest, entry, 0); //There exists a file with that name
        ret = ret && project()->projectFileManager()->renameFolder(this, dest);
        if(ret)
            emitDataChanged();
        else
            KMessageBox::error(0, i18n("The name for '%1' could not be changed", url().prettyUrl()), i18n("Project Management"));
    } else
        ProjectBaseItem::setData(value, role);
}

ProjectFileItem::ProjectFileItem( ProjectFileItemPrivate& dd)
    : ProjectBaseItem(dd)
{
    setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
}

ProjectFileItem::ProjectFileItem( IProject* project, const KUrl & file, QStandardItem * parent )
        : ProjectBaseItem( *new ProjectFileItemPrivate )
{
    Q_D(ProjectFileItem);
    d->project = project;
    setUrl( file );
    setParent( parent );
    setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
}

ProjectFileItem::~ProjectFileItem()
{
    Q_D(ProjectFileItem);
    project()->removeFromFileSet(KDevelop::IndexedString(d->m_url));
}

void ProjectFileItem::setData(const QVariant& value, int role)
{
    if(role==Qt::EditRole && value != data(role)) {
        KUrl dest = url().upUrl();
        dest.addPath(value.toString());
        bool ret=!value.toString().contains('/');
        
        KIO::UDSEntry entry;
        ret = ret && !KIO::NetAccess::stat(dest, entry, 0); //There exists a file with that name
        ret = ret && project()->projectFileManager()->renameFile(this, dest);
        if(ret)
            emitDataChanged();
        else
            KMessageBox::error(0, i18n("The name for '%1' could not be changed", url().prettyUrl()), i18n("Project Management"));
    } else
        ProjectBaseItem::setData(value, role);
}

KUrl ProjectFileItem::url( ) const
{
    Q_D(const ProjectFileItem);
    return d->m_url;
}

const QString& ProjectFileItem::fileName() const
{
    Q_D(const ProjectFileItem);
    return d->m_fileName;
}

void ProjectFileItem::setUrl( const KUrl& url )
{
    Q_D(ProjectFileItem);
    if(!d->m_url.isEmpty())
        project()->removeFromFileSet( KDevelop::IndexedString(d->m_url) );
    project()->addToFileSet( KDevelop::IndexedString(url) );
    
    d->m_url = url;
    d->m_fileName = d->m_url.fileName();
    setText( d->m_fileName );
    setToolTip( text() );
    setIcon(KIcon(KMimeType::findByUrl(url, 0, false, true)->iconName(url)));
}

int ProjectFileItem::type() const
{
    return ProjectBaseItem::File;
}

ProjectFileItem *ProjectFileItem::file() const
{
    return const_cast<ProjectFileItem*>( this );
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
    setToolTip( text() );
    setParent( parent );
    setIcon( KIcon("system-run") );
}

int ProjectTargetItem::type() const
{
    return ProjectBaseItem::Target;
}

ProjectTargetItem *ProjectTargetItem::target() const
{
    return const_cast<ProjectTargetItem*>( this );
}

ProjectExecutableTargetItem::ProjectExecutableTargetItem( IProject* project, const QString &name, QStandardItem *parent )
    : ProjectTargetItem(project, name, parent)
{}

ProjectExecutableTargetItem *ProjectExecutableTargetItem::executable() const
{
    return const_cast<ProjectExecutableTargetItem*>( this );
}

int ProjectExecutableTargetItem::type() const
{
    return ProjectBaseItem::ExecutableTarget;
}

ProjectLibraryTargetItem::ProjectLibraryTargetItem( IProject* project, const QString &name, QStandardItem *parent )
    : ProjectTargetItem(project, name, parent)
{}

int ProjectLibraryTargetItem::type() const
{
    return ProjectBaseItem::LibraryTarget;
}

QModelIndex ProjectModel::pathToIndex(const QStringList& tofetch_) const
{
    if(tofetch_.isEmpty())
        return QModelIndex();
    QStringList tofetch(tofetch_);
    if(tofetch.last().isEmpty())
        tofetch.takeLast();

    QModelIndex current=index(0,0, QModelIndex());

    QModelIndex ret;
    for(int a = 0; a < tofetch.size(); ++a)
    {
        const QString& currentName = tofetch[a];
        
        bool matched = false;
        QModelIndexList l = match(current, Qt::EditRole, currentName, -1, Qt::MatchExactly);
        foreach(const QModelIndex& idx, l) {
            //If this is not the last item, only match folders, as there may be targets and folders with the same name
            if(a == tofetch.size()-1 || item(idx)->folder()) {
                ret = idx;
                current = index(0,0, ret);
                matched = true;
                break;
            }
        }
        if(!matched) {
            ret = QModelIndex();
            break;
        }
    }
    Q_ASSERT(!ret.isValid() || data(ret).toString()==tofetch.last());
    return ret;
}

QStringList ProjectModel::pathFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return QStringList();

    QModelIndex idx = index;
    QStringList list;
    do {
        QString t = data(idx, Qt::EditRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    } while (idx.isValid());

    return list;
}


ProjectVisitor::ProjectVisitor() 
{
}


void ProjectVisitor::visit ( IProject* prj )
{
    visit( prj->projectItem() );
}


void ProjectVisitor::visit ( ProjectBuildFolderItem* folder )
{
    foreach( ProjectFileItem* item, folder->fileList() )
    {
        visit( item );
    }
    foreach( ProjectTargetItem* item, folder->targetList() )
    {
        if( item->type() == ProjectBaseItem::LibraryTarget )
        {
            visit( dynamic_cast<ProjectLibraryTargetItem*>( item ) );
        } else if( item->type() == ProjectBaseItem::ExecutableTarget )
        {
            visit( dynamic_cast<ProjectExecutableTargetItem*>( item ) );
        }
    }
    foreach( ProjectFolderItem* item, folder->folderList() )
    {
        if( item->type() == ProjectBaseItem::BuildFolder )
        {
            visit( dynamic_cast<ProjectBuildFolderItem*>( item ) );
        } else if( item->type() == ProjectBaseItem::Folder )
        {
            visit( dynamic_cast<ProjectFolderItem*>( item ) );
        }
    }
}


void ProjectVisitor::visit ( ProjectExecutableTargetItem* exec )
{
    foreach( ProjectFileItem* item, exec->fileList() )
    {
        visit( item );
    }
}


void ProjectVisitor::visit ( ProjectFolderItem* folder )
{
    foreach( ProjectFileItem* item, folder->fileList() )
    {
        visit( item );
    }
    foreach( ProjectTargetItem* item, folder->targetList() )
    {
        if( item->type() == ProjectBaseItem::LibraryTarget )
        {
            visit( dynamic_cast<ProjectLibraryTargetItem*>( item ) );
        } else if( item->type() == ProjectBaseItem::ExecutableTarget )
        {
            visit( dynamic_cast<ProjectExecutableTargetItem*>( item ) );
        }
    }
    foreach( ProjectFolderItem* item, folder->folderList() )
    {
        if( item->type() == ProjectBaseItem::BuildFolder )
        {
            visit( dynamic_cast<ProjectBuildFolderItem*>( item ) );
        } else if( item->type() == ProjectBaseItem::Folder )
        {
            visit( dynamic_cast<ProjectFolderItem*>( item ) );
        }
    }
}


void ProjectVisitor::visit ( ProjectFileItem* )
{
}

void ProjectVisitor::visit ( ProjectLibraryTargetItem* lib )
{
    foreach( ProjectFileItem* item, lib->fileList() )
    {
        visit( item );
    }
}

ProjectVisitor::~ProjectVisitor()
{
}


}
#include "projectmodel.moc"
