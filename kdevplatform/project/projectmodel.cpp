/*
    SPDX-FileCopyrightText: 2005 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectmodel.h"

#include <QIcon>
#include <QMimeDatabase>
#include <QMimeType>
#include <QMutex>
#include <QMutexLocker>

#include <KIO/StatJob>
#include <KLocalizedString>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include "interfaces/iprojectfilemanager.h"
#include <serialization/indexedstring.h>
#include <serialization/indexedstringview.h>

#include "debug.h"
#include "path.h"

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
        return result.mid(basePath.count());
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

class ProjectModelPrivate
{
public:
    explicit ProjectModelPrivate( ProjectModel* model ): model( model )
    {
    }
    ProjectBaseItem* rootItem;
    ProjectModel* model;
    ProjectBaseItem* itemFromIndex(const QModelIndex& idx) const
    {
        if( !idx.isValid() ) {
            return rootItem;
        }
        if( idx.model() != model ) {
            return nullptr;
        }
        return model->itemFromIndex( idx );
    }

    // a hash of IndexedStringView{path} -> ProjectBaseItem for fast lookup
    QMultiHash<IndexedStringView, ProjectBaseItem*> pathLookupTable;
};

class ProjectBaseItemPrivate
{
public:
    ProjectBaseItemPrivate() {}
    ProjectModel* model = nullptr;
    IProject* project = nullptr;
    ProjectBaseItem* parent = nullptr;
    QList<ProjectBaseItem*> children;
    QString text;
    Path m_path;
    QString iconName;
    int row = -1;
    IndexedStringView m_indexedPath;
    Qt::ItemFlags flags;

    ProjectBaseItem::RenameStatus renameBaseItem(ProjectBaseItem* item, const QString& newName)
    {
        if (item->parent()) {
            const auto siblings = item->parent()->children();
            for (ProjectBaseItem* sibling : siblings) {
                if (sibling->text() == newName) {
                    return ProjectBaseItem::ExistingItemSameName;
                }
            }
        }
        item->setText( newName );
        return ProjectBaseItem::RenameOk;
    }

    ProjectBaseItem::RenameStatus renameFileOrFolder(ProjectBaseItem* item, const QString& newName)
    {
        Q_ASSERT(item->file() || item->folder());

        if (newName.contains(QLatin1Char('/'))) {
            return ProjectBaseItem::InvalidNewName;
        }

        if (item->text() == newName) {
            return ProjectBaseItem::RenameOk;
        }

        Path newPath = item->path();
        newPath.setLastPathSegment(newName);

        auto job = KIO::stat(newPath.toUrl(), KIO::StatJob::SourceSide, KIO::StatNoDetails, KIO::HideProgressInfo);
        if (job->exec()) {
            // file/folder exists already
            return ProjectBaseItem::ExistingItemSameName;
        }

        if( !item->project() || !item->project()->projectFileManager() ) {
            return renameBaseItem(item, newName);
        } else if( item->folder() && item->project()->projectFileManager()->renameFolder(item->folder(), newPath) ) {
            return ProjectBaseItem::RenameOk;
        } else if ( item->file() && item->project()->projectFileManager()->renameFile(item->file(), newPath) ) {
            return ProjectBaseItem::RenameOk;
        } else {
            return ProjectBaseItem::ProjectManagerRenameFailed;
        }
    }
};


ProjectBaseItem::ProjectBaseItem( IProject* project, const QString &name, ProjectBaseItem *parent )
        : d_ptr(new ProjectBaseItemPrivate)
{
    Q_ASSERT(!name.isEmpty() || !parent);
    Q_D(ProjectBaseItem);
    d->project = project;
    d->text = name;
    d->flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if( parent ) {
        parent->appendRow( this );
    }
}

ProjectBaseItem::~ProjectBaseItem()
{
    Q_D(ProjectBaseItem);

    if (model() && !d->m_indexedPath.isEmpty()) {
        model()->d_func()->pathLookupTable.remove(d->m_indexedPath, this);
    }

    if( parent() ) {
        parent()->takeRow( d->row );
    } else if( model() ) {
        model()->takeRow( d->row );
    }
    removeRows(0, d->children.size());
}

ProjectBaseItem* ProjectBaseItem::child( int row ) const
{
    Q_D(const ProjectBaseItem);
    if( row < 0 || row >= d->children.length() ) {
        return nullptr;
    }
    return d->children.at( row );
}

QList< ProjectBaseItem* > ProjectBaseItem::children() const
{
    Q_D(const ProjectBaseItem);
    return d->children;
}

ProjectBaseItem* ProjectBaseItem::takeRow(int row)
{
    Q_D(ProjectBaseItem);
    Q_ASSERT(row >= 0 && row < d->children.size());

    if( model() ) {
        model()->beginRemoveRows(index(), row, row);
    }
    ProjectBaseItem* olditem = d->children.takeAt( row );
    olditem->d_func()->parent = nullptr;
    olditem->d_func()->row = -1;
    olditem->setModel( nullptr );

    for (int i = row, count = d->children.size(); i < count; i++) {
        auto sibling = d->children.at(i);
        Q_ASSERT(sibling->d_func()->row == i + 1);
        sibling->d_func()->row = i;
    }

    if( model() ) {
        model()->endRemoveRows();
    }
    return olditem;
}

void ProjectBaseItem::removeRow( int row )
{
    delete takeRow( row );
}

void ProjectBaseItem::removeRows(int row, int count)
{
    if (!count) {
        return;
    }

    Q_D(ProjectBaseItem);
    Q_ASSERT(row >= 0 && row + count <= d->children.size());

    if( model() ) {
        model()->beginRemoveRows(index(), row, row + count - 1);
    }

    //NOTE: we unset parent, row and model manually to speed up the deletion
    if (row == 0 && count == d->children.size()) {
        // optimize if we want to delete all
        for (ProjectBaseItem* item : std::as_const(d->children)) {
            item->d_func()->parent = nullptr;
            item->d_func()->row = -1;
            item->setModel( nullptr );
            delete item;
        }
        d->children.clear();
    } else {
        for (int i = row; i < count; ++i) {
            ProjectBaseItem* item = d->children.at(i);
            item->d_func()->parent = nullptr;
            item->d_func()->row = -1;
            item->setModel( nullptr );
            delete d->children.takeAt( row );
        }
        for(int i = row; i < d->children.size(); ++i) {
            d->children.at(i)->d_func()->row--;
            Q_ASSERT(child(i)->d_func()->row==i);
        }
    }

    if( model() ) {
        model()->endRemoveRows();
    }
}

QModelIndex ProjectBaseItem::index() const
{
    if( model() ) {
        return model()->indexFromItem( this );
    }
    return QModelIndex();
}

int ProjectBaseItem::rowCount() const
{
    Q_D(const ProjectBaseItem);
    return d->children.count();
}

int ProjectBaseItem::type() const
{
    return ProjectBaseItem::BaseItem;
}

ProjectModel* ProjectBaseItem::model() const
{
    Q_D(const ProjectBaseItem);
    return d->model;
}

ProjectBaseItem* ProjectBaseItem::parent() const
{
    Q_D(const ProjectBaseItem);
    if( model() && model()->d_func()->rootItem == d->parent ) {
        return nullptr;
    }
    return d->parent;
}

int ProjectBaseItem::row() const
{
    Q_D(const ProjectBaseItem);
    return d->row;
}

QString ProjectBaseItem::text() const
{
    Q_D(const ProjectBaseItem);
    if( project() && !parent() ) {
        return project()->name();
    } else {
        return d->text;
    }
}

void ProjectBaseItem::setModel( ProjectModel* model )
{
    Q_D(ProjectBaseItem);

    if (model == d->model) {
        return;
    }

    if (d->model && !d->m_indexedPath.isEmpty()) {
        d->model->d_func()->pathLookupTable.remove(d->m_indexedPath, this);
    }

    d->model = model;

    if (model && !d->m_indexedPath.isEmpty()) {
        model->d_func()->pathLookupTable.insert(d->m_indexedPath, this);
    }

    for (ProjectBaseItem* item : std::as_const(d->children)) {
        item->setModel( model );
    }
}

void ProjectBaseItem::setRow( int row )
{
    Q_D(ProjectBaseItem);
    d->row = row;
}

void ProjectBaseItem::setText( const QString& text )
{
    Q_ASSERT(!text.isEmpty() || !parent());
    Q_D(ProjectBaseItem);
    d->text = text;
    if( d->model ) {
        QModelIndex idx = index();
        emit d->model->dataChanged(idx, idx);
    }
}

ProjectBaseItem::RenameStatus ProjectBaseItem::rename(const QString& newName)
{
    Q_D(ProjectBaseItem);
    return d->renameBaseItem(this, newName);
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
        return text().compare(item->text(), Qt::CaseInsensitive) < 0;
    }
    else
    {
        return leftType<rightType;
    }

    return false;
}

bool ProjectBaseItem::pathLessThan(ProjectBaseItem* item1, ProjectBaseItem* item2)
{
    return item1->path() < item2->path();
}

IProject* ProjectBaseItem::project() const
{
    Q_D(const ProjectBaseItem);
    return d->project;
}

void ProjectBaseItem::appendRow( ProjectBaseItem* item )
{
    Q_D(ProjectBaseItem);
    if( !item ) {
        return;
    }
    if( item->parent() ) {
        // Proper way is to first removeRow() on the original parent, then appendRow on this one
        qCWarning(PROJECT) << "Ignoring double insertion of item" << item;
        return;
    }
    // this is too slow... O(n) and thankfully not a problem anyways
//     Q_ASSERT(!d->children.contains(item));
    int startrow,endrow;
    if( model() ) {
        startrow = endrow = d->children.count();
        model()->beginInsertRows(index(), startrow, endrow);
    }
    d->children.append( item );
    item->setRow( d->children.count() - 1 );
    item->d_func()->parent = this;
    item->setModel( model() );
    if( model() ) {
        model()->endInsertRows();
    }
}

Path ProjectBaseItem::path() const
{
    Q_D(const ProjectBaseItem);
    return d->m_path;
}

IndexedString ProjectBaseItem::indexedPath() const
{
    return indexedPathView().toString();
}

IndexedStringView ProjectBaseItem::indexedPathView() const
{
    Q_D(const ProjectBaseItem);
    return d->m_indexedPath;
}

QString ProjectBaseItem::baseName() const
{
    return text();
}

void ProjectBaseItem::setPath( const Path& path)
{
    Q_D(ProjectBaseItem);

    if (model() && !d->m_indexedPath.isEmpty()) {
        model()->d_func()->pathLookupTable.remove(d->m_indexedPath, this);
    }

    d->m_path = path;
    d->m_indexedPath = IndexedStringView{path.pathOrUrl()};
    setText( path.lastPathSegment() );

    if (model() && !d->m_indexedPath.isEmpty()) {
        model()->d_func()->pathLookupTable.insert(d->m_indexedPath, this);
    }
}

Qt::ItemFlags ProjectBaseItem::flags()
{
    Q_D(ProjectBaseItem);
    return d->flags;
}

Qt::DropActions ProjectModel::supportedDropActions() const
{
    return (Qt::DropActions)(Qt::MoveAction);
}

void ProjectBaseItem::setFlags(Qt::ItemFlags flags)
{
    Q_D(ProjectBaseItem);
    d->flags = flags;
    if(d->model)
        emit d->model->dataChanged(index(), index());
}

QString ProjectBaseItem::iconName() const
{
    return QString();
}

ProjectFolderItem *ProjectBaseItem::folder() const
{
    return nullptr;
}

ProjectTargetItem *ProjectBaseItem::target() const
{
    return nullptr;
}

ProjectExecutableTargetItem *ProjectBaseItem::executable() const
{
    return nullptr;
}

ProjectFileItem *ProjectBaseItem::file() const
{
    return nullptr;
}

QList<ProjectFolderItem*> ProjectBaseItem::folderList() const
{
    QList<ProjectFolderItem*> lst;
    for ( int i = 0; i < rowCount(); ++i )
    {
        ProjectBaseItem* item = child( i );
        if ( item->type() == Folder || item->type() == BuildFolder )
        {
            auto *kdevitem = dynamic_cast<ProjectFolderItem*>( item );
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
        ProjectBaseItem* item = child( i );

        if ( item->type() == Target || item->type() == LibraryTarget || item->type() == ExecutableTarget )
        {
            auto *kdevitem = dynamic_cast<ProjectTargetItem*>( item );
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
        ProjectBaseItem* item = child( i );
        Q_ASSERT(item);
        if ( item && item->type() == File )
        {
            auto *kdevitem = dynamic_cast<ProjectFileItem*>( item );
            if ( kdevitem )
                lst.append( kdevitem );
        }

    }
    return lst;
}

void ProjectModel::clear()
{
    Q_D(ProjectModel);

    d->rootItem->removeRows(0, d->rootItem->rowCount());
}


ProjectFolderItem::ProjectFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
    : ProjectBaseItem( project, path.lastPathSegment(), parent )
{
    setPath( path );

    setFlags(flags() | Qt::ItemIsDropEnabled);
    if (project && project->path() != path)
        setFlags(flags() | Qt::ItemIsDragEnabled);
}

ProjectFolderItem::ProjectFolderItem( const QString & name, ProjectBaseItem * parent )
        : ProjectBaseItem( parent->project(), name, parent )
{
    setPath( Path(parent->path(), name) );

    setFlags(flags() | Qt::ItemIsDropEnabled);
    if (project() && project()->path() != path())
        setFlags(flags() | Qt::ItemIsDragEnabled);
}

ProjectFolderItem::~ProjectFolderItem()
{
}

void ProjectFolderItem::setPath( const Path& path )
{
    ProjectBaseItem::setPath(path);

    propagateRename(path);
}

ProjectFolderItem *ProjectFolderItem::folder() const
{
    return const_cast<ProjectFolderItem*>(this);
}

int ProjectFolderItem::type() const
{
    return ProjectBaseItem::Folder;
}

QString ProjectFolderItem::folderName() const
{
    return baseName();
}

void ProjectFolderItem::propagateRename( const Path& newBase ) const
{
    Path path = newBase;
    path.addPath(QStringLiteral("dummy"));
    const auto children = this->children();
    for (KDevelop::ProjectBaseItem* child : children) {
        path.setLastPathSegment( child->text() );
        child->setPath( path );

        const ProjectFolderItem* folder = child->folder();
        if ( folder ) {
            folder->propagateRename( path );
        }
    }
}

ProjectBaseItem::RenameStatus ProjectFolderItem::rename(const QString& newName)
{
    return d_ptr->renameFileOrFolder(this, newName);
}

bool ProjectFolderItem::hasFileOrFolder(const QString& name) const
{
    const auto children = this->children();
    return std::any_of(children.begin(), children.end(), [&](ProjectBaseItem* item) {
        return ((item->type() == Folder || item->type() == File || item->type() == BuildFolder)
                && name == item->baseName());
    });
}

bool ProjectBaseItem::isProjectRoot() const
{
    return parent()==nullptr;
}

ProjectBuildFolderItem::ProjectBuildFolderItem(IProject* project, const Path& path, ProjectBaseItem *parent)
    : ProjectFolderItem( project, path, parent )
{
}

ProjectBuildFolderItem::ProjectBuildFolderItem( const QString& name, ProjectBaseItem* parent )
    : ProjectFolderItem( name, parent )
{

}

QString ProjectFolderItem::iconName() const
{
    return QStringLiteral("folder");
}

int ProjectBuildFolderItem::type() const
{
    return ProjectBaseItem::BuildFolder;
}

QString ProjectBuildFolderItem::iconName() const
{
    return QStringLiteral("folder-development");
}

ProjectFileItem::ProjectFileItem( IProject* project, const Path& path, ProjectBaseItem* parent )
    : ProjectBaseItem( project, path.lastPathSegment(), parent )
{
    setFlags(flags() | Qt::ItemIsDragEnabled);
    setPath( path );
}

ProjectFileItem::ProjectFileItem( const QString& name, ProjectBaseItem* parent )
    : ProjectBaseItem( parent->project(), name, parent )
{
    setFlags(flags() | Qt::ItemIsDragEnabled);
    setPath( Path(parent->path(), name) );
}

ProjectFileItem::~ProjectFileItem()
{
    if (project() && !d_ptr->m_indexedPath.isEmpty()) {
        project()->removeFromFileSet( this );
    }
}

ProjectBaseItem::RenameStatus ProjectFileItem::rename(const QString& newName)
{
    return d_ptr->renameFileOrFolder(this, newName);
}

QString ProjectFileItem::fileName() const
{
    return baseName();
}

namespace {

// Maximum length of a string to still consider it as a file extension which we cache
// This has to be a slow value, so that we don't fill our file extension cache with crap
static const int maximumCacheExtensionLength = 3;

bool isNumeric(QStringView str)
{
    if (str.isEmpty()) {
        return false;
    }

    return std::all_of(str.begin(), str.end(), [](const QChar c) {
        return c.isNumber();
    });
}

class IconNameCache
{
public:
    QString iconNameForPath(const Path& path, QStringView fileName)
    {
        // find icon name based on file extension, if possible
        QString extension;
        int extensionStart = fileName.lastIndexOf(QLatin1Char('.'));
        if( extensionStart != -1 && fileName.length() - extensionStart - 1 <= maximumCacheExtensionLength ) {
            auto extRef = fileName.mid(extensionStart + 1);
            if( isNumeric(extRef) ) {
                // don't cache numeric extensions
                extRef = {};
            }
            if( !extRef.isEmpty() ) {
                extension = extRef.toString();
                QMutexLocker lock(&mutex);
                QHash< QString, QString >::const_iterator it = fileExtensionToIcon.constFind( extension );
                if( it != fileExtensionToIcon.constEnd() ) {
                    return *it;
                }
            }
        }

        QMimeType mime = QMimeDatabase().mimeTypeForFile(path.lastPathSegment(), QMimeDatabase::MatchExtension); // no I/O
        QMutexLocker lock(&mutex);
        QHash< QString, QString >::const_iterator it = mimeToIcon.constFind(mime.name());
        QString iconName;
        if ( it == mimeToIcon.constEnd() ) {
            iconName = mime.iconName();
            if (iconName.isEmpty()) {
                iconName = QStringLiteral("none");
            }
            mimeToIcon.insert(mime.name(), iconName);
        } else {
            iconName = *it;
        }
        if ( !extension.isEmpty() ) {
            fileExtensionToIcon.insert(extension, iconName);
        }
        return iconName;
    }
    QMutex mutex;
    QHash<QString, QString> mimeToIcon;
    QHash<QString, QString> fileExtensionToIcon;
};

Q_GLOBAL_STATIC(IconNameCache, s_cache)

} // unnamed namespace

QString ProjectFileItem::iconName() const
{
    // think of d_ptr->iconName as mutable, possible since d_ptr is not const
    if (d_ptr->iconName.isEmpty()) {
        // lazy load implementation of icon lookup
        d_ptr->iconName = s_cache->iconNameForPath( d_ptr->m_path, d_ptr->text );
        // we should always get *some* icon name back
        Q_ASSERT(!d_ptr->iconName.isEmpty());
    }
    return d_ptr->iconName;
}

void ProjectFileItem::setPath( const Path& path )
{
    if (path == d_ptr->m_path) {
        return;
    }

    if (project() && !d_ptr->m_indexedPath.isEmpty()) {
        // remove from fileset if we are in there
        project()->removeFromFileSet( this );
    }

    ProjectBaseItem::setPath( path );

    if (project() && !d_ptr->m_indexedPath.isEmpty()) {
        // add to fileset with new path
        project()->addToFileSet( this );
    }

    // invalidate icon name for future lazy-loaded updated
    d_ptr->iconName.clear();
}

int ProjectFileItem::type() const
{
    return ProjectBaseItem::File;
}

ProjectFileItem *ProjectFileItem::file() const
{
    return const_cast<ProjectFileItem*>( this );
}

ProjectTargetItem::ProjectTargetItem( IProject* project, const QString &name, ProjectBaseItem *parent )
    : ProjectBaseItem( project, name, parent )
{
    setFlags(flags() | Qt::ItemIsDropEnabled);
}

QString ProjectTargetItem::iconName() const
{
    return QStringLiteral("system-run");
}

void ProjectTargetItem::setPath( const Path& path )
{
    // don't call base class, it calls setText with the new path's filename
    // which we do not want for target items
    d_ptr->m_path = path;
}

int ProjectTargetItem::type() const
{
    return ProjectBaseItem::Target;
}

ProjectTargetItem *ProjectTargetItem::target() const
{
    return const_cast<ProjectTargetItem*>( this );
}

ProjectExecutableTargetItem::ProjectExecutableTargetItem( IProject* project, const QString &name, ProjectBaseItem *parent )
    : ProjectTargetItem(project, name, parent)
{
}

ProjectExecutableTargetItem *ProjectExecutableTargetItem::executable() const
{
    return const_cast<ProjectExecutableTargetItem*>( this );
}

int ProjectExecutableTargetItem::type() const
{
    return ProjectBaseItem::ExecutableTarget;
}

ProjectLibraryTargetItem::ProjectLibraryTargetItem( IProject* project, const QString &name, ProjectBaseItem *parent )
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
        tofetch.removeLast();

    QModelIndex current=index(0,0, QModelIndex());

    QModelIndex ret;
    for(int a = 0; a < tofetch.size(); ++a)
    {
        const QString& currentName = tofetch[a];

        bool matched = false;
        const QModelIndexList l = match(current, Qt::DisplayRole, currentName, -1, Qt::MatchExactly);
        for (const QModelIndex& idx : l) {
            //If this is not the last item, only match folders, as there may be targets and folders with the same name
            if(a == tofetch.size()-1 || itemFromIndex(idx)->folder()) {
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
        QString t = data(idx, Qt::DisplayRole).toString();
        list.prepend(t);
        QModelIndex parent = idx.parent();
        idx = parent.sibling(parent.row(), index.column());
    } while (idx.isValid());

    return list;
}

int ProjectModel::columnCount( const QModelIndex& ) const
{
    return 1;
}

int ProjectModel::rowCount( const QModelIndex& parent ) const
{
    Q_D(const ProjectModel);

    ProjectBaseItem* item = d->itemFromIndex( parent );
    return item ? item->rowCount() : 0;
}

QModelIndex ProjectModel::parent( const QModelIndex& child ) const
{
    if( child.isValid() ) {
        auto* item = static_cast<ProjectBaseItem*>( child.internalPointer() );
        return indexFromItem( item );
    }
    return QModelIndex();
}

QModelIndex ProjectModel::indexFromItem( const ProjectBaseItem* item ) const
{
    if( item && item->d_func()->parent ) {
        return createIndex( item->row(), 0, item->d_func()->parent );
    }
    return QModelIndex();
}

ProjectBaseItem* ProjectModel::itemFromIndex( const QModelIndex& index ) const
{
    if( index.row() >= 0 && index.column() == 0  && index.model() == this ) {
        auto* parent = static_cast<ProjectBaseItem*>( index.internalPointer() );
        if( parent ) {
            return parent->child( index.row() );
        }
    }
    return nullptr;
}

QVariant ProjectModel::data( const QModelIndex& index, int role ) const
{
    static const QSet<int> allowedRoles = {
        Qt::DisplayRole,
        Qt::ToolTipRole,
        Qt::DecorationRole,
        ProjectItemRole,
        ProjectRole,
        UrlRole
    };
    if( allowedRoles.contains(role) && index.isValid() ) {
        ProjectBaseItem* item = itemFromIndex( index );
        if( item ) {
            switch(role) {
                case Qt::DecorationRole:
                    return QIcon::fromTheme(item->iconName());
                case Qt::ToolTipRole:
                    return item->path().pathOrUrl();
                case Qt::DisplayRole:
                    return item->text();
                case ProjectItemRole:
                    return QVariant::fromValue<ProjectBaseItem*>(item);
                case UrlRole:
                    return item->path().toUrl();
                case ProjectRole:
                    return QVariant::fromValue<QObject*>(item->project());
            }
        }
    }
    return QVariant();
}

ProjectModel::ProjectModel( QObject *parent )
    : QAbstractItemModel(parent)
    , d_ptr(new ProjectModelPrivate(this))
{
    Q_D(ProjectModel);

    d->rootItem = new ProjectBaseItem( nullptr, QString(), nullptr );
    d->rootItem->setModel( this );
}

ProjectModel::~ProjectModel()
{
    Q_D(ProjectModel);

    d->rootItem->setModel(nullptr);
    delete d->rootItem;
}


ProjectVisitor::ProjectVisitor()
{
}

QModelIndex ProjectModel::index( int row, int column, const QModelIndex& parent ) const
{
    Q_D(const ProjectModel);

    ProjectBaseItem* parentItem = d->itemFromIndex( parent );
    if( parentItem && row >= 0 && row < parentItem->rowCount() && column == 0 ) {
        return createIndex( row, column, parentItem );
    }
    return QModelIndex();
}

void ProjectModel::appendRow( ProjectBaseItem* item )
{
    Q_D(ProjectModel);

    d->rootItem->appendRow( item );
}

void ProjectModel::removeRow( int row )
{
    Q_D(ProjectModel);

    d->rootItem->removeRow( row );
}

ProjectBaseItem* ProjectModel::takeRow( int row )
{
    Q_D(ProjectModel);

    return d->rootItem->takeRow( row );
}

ProjectBaseItem* ProjectModel::itemAt(int row) const
{
    Q_D(const ProjectModel);

    return d->rootItem->child(row);
}

QList< ProjectBaseItem* > ProjectModel::topItems() const
{
    Q_D(const ProjectModel);

    return d->rootItem->children();
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex& index) const
{
    ProjectBaseItem* item = itemFromIndex( index );
    if(item)
        return item->flags();

    return Qt::NoItemFlags;
}

bool ProjectModel::insertColumns(int, int, const QModelIndex&)
{
    // Not supported
    return false;
}

bool ProjectModel::insertRows(int, int, const QModelIndex&)
{
    // Not supported
    return false;
}

bool ProjectModel::setData(const QModelIndex&, const QVariant&, int)
{
    // Not supported
    return false;
}

QList<ProjectBaseItem*> ProjectModel::itemsForPath(const IndexedString& path) const
{
    Q_D(const ProjectModel);

    return d->pathLookupTable.values(IndexedStringView::fromString(path));
}

ProjectBaseItem* ProjectModel::itemForPath(IndexedStringView path) const
{
    Q_D(const ProjectModel);

    return d->pathLookupTable.value(path);
}

void ProjectVisitor::visit( ProjectModel* model )
{
    const auto topItems = model->topItems();
    for (ProjectBaseItem* item : topItems) {
        visit( item->project() );
    }
}

void ProjectVisitor::visit ( IProject* prj )
{
    visit( prj->projectItem() );
}

void ProjectVisitor::visit ( ProjectBuildFolderItem* folder )
{
    visit(static_cast<ProjectFolderItem*>(folder));
}

void ProjectVisitor::visit ( ProjectExecutableTargetItem* exec )
{
    const auto fileItems = exec->fileList();
    for (ProjectFileItem* item : fileItems) {
        visit( item );
    }
}

void ProjectVisitor::visit ( ProjectFolderItem* folder )
{
    const auto fileItems = folder->fileList();
    for (ProjectFileItem* item : fileItems) {
        visit( item );
    }
    const auto targetItems = folder->targetList();
    for (ProjectTargetItem* item : targetItems) {
        if( item->type() == ProjectBaseItem::LibraryTarget )
        {
            visit( dynamic_cast<ProjectLibraryTargetItem*>( item ) );
        } else if( item->type() == ProjectBaseItem::ExecutableTarget )
        {
            visit( dynamic_cast<ProjectExecutableTargetItem*>( item ) );
        }
    }
    const auto folderItems = folder->folderList();
    for (ProjectFolderItem* item : folderItems) {
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
    const auto fileItems = lib->fileList();
    for (ProjectFileItem* item : fileItems) {
        visit( item );
    }
}

ProjectVisitor::~ProjectVisitor()
{
}


}

#include "moc_projectmodel.cpp"
