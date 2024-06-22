/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "projectpathsmodel.h"

#include <KLocalizedString>
#include <interfaces/iproject.h>
#include <serialization/indexedstring.h>
#include <util/path.h>

using namespace KDevelop;

ProjectPathsModel::ProjectPathsModel( QObject* parent )
    : QAbstractListModel( parent )
{
}

void ProjectPathsModel::setProject(IProject* w_project)
{
    project = w_project;
}

QVariant ProjectPathsModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || index.row() < 0 || index.row() >= rowCount() || index.column() != 0 ) {
        return QVariant();
    }

    const ConfigEntry& pathConfig = projectPaths.at( index.row() );
    switch( role ) {
    case IncludesDataRole:
        return pathConfig.includes;
    case DefinesDataRole:
        return QVariant::fromValue(pathConfig.defines);
    case Qt::EditRole:
        return sanitizePath( pathConfig.path, true, false );
    case Qt::DisplayRole: {
        const QString& path = pathConfig.path;
        return (path == QLatin1String(".")) ? QStringLiteral("(project root)") : path;
    }
    case FullUrlDataRole:
        return QVariant::fromValue(QUrl::fromUserInput( sanitizePath( pathConfig.path, true, false ) ));
    case CompilerDataRole:
        return QVariant::fromValue(pathConfig.compiler);
    case ParserArgumentsRole:
        return QVariant::fromValue(pathConfig.parserArguments);
    default:
        break;
    }
    return QVariant();
}

int ProjectPathsModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() ) {
        return 0;
    }
    return projectPaths.count();
}

bool ProjectPathsModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if( !index.isValid() || index.row() < 0 || index.row() >= rowCount() || index.column() != 0 ) {
        return false;
    }

    // Do not allow to change path of the first entry; instead, add a new one in that case
    if( index.row() == 0 && ( role == Qt::EditRole || role == Qt::DisplayRole || role == FullUrlDataRole ) ) {
        QString addedPath = sanitizePath( value.toString(), false );

        // Do not allow duplicates
        for (const ConfigEntry& existingConfig : std::as_const(projectPaths)) {
            if( addedPath == existingConfig.path ) {
                return false;
            }
        }
        projectPaths.insert( 1, ConfigEntry(sanitizePath( value.toString(), false ) ));
        emit dataChanged( this->index( 1, 0 ), this->index( projectPaths.count() - 1, 0 ) );
        return true;
    }

    ConfigEntry& pathConfig = projectPaths[ index.row() ];
    switch( role ) {
    case IncludesDataRole:
        pathConfig.includes = value.toStringList();
        break;
    case DefinesDataRole:
        pathConfig.defines = value.value<Defines>();
        break;
    case Qt::EditRole:
        pathConfig.path = sanitizePath( value.toString(), false );
        break;
    case Qt::DisplayRole:
        pathConfig.path = sanitizePath( value.toString(), true );
        break;
    case FullUrlDataRole:
        pathConfig.path = sanitizeUrl(value.toUrl());
        break;
    case CompilerDataRole:
        pathConfig.compiler = value.value<CompilerPointer>();
        break;
    case ParserArgumentsRole:
        pathConfig.parserArguments = value.value<ParserArguments>();
        break;
    default:
        return false;
    }
    emit dataChanged( index, index );
    return true;
}

Qt::ItemFlags ProjectPathsModel::flags( const QModelIndex& index ) const
{
    if( !index.isValid() ) {
        return Qt::NoItemFlags;
    }

    if( index.row() == 0 ) {
        return Qt::ItemFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    }

    return Qt::ItemFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
}

QVector< ConfigEntry > ProjectPathsModel::paths() const
{
    return projectPaths;
}

void ProjectPathsModel::setPaths(const QVector< ConfigEntry >& paths )
{
    beginResetModel();
    projectPaths.clear();
    for (const ConfigEntry& existingPathConfig : paths) {
        // Sanitize the path of loaded config
        ConfigEntry config = existingPathConfig;
        bool rootPath = config.path == QLatin1String(".") ? true : false;
        config.path = sanitizePath(rootPath ? QString() : config.path );
        addPathInternal(config, rootPath);
    }
    addPathInternal( ConfigEntry(sanitizePath( QString() )), true ); // add an empty "root" config entry if one does not exist
    endResetModel();
}

bool ProjectPathsModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( row >= 0 && count > 0 && row < rowCount() ) {
        beginRemoveRows( parent, row, row + count - 1 );

        for( int i = 0; i < count; ++i ) {
            if( projectPaths.at(row).path == QLatin1String(".") ) {
                continue; // we won't remove the root item
            }
            projectPaths.removeAt(row);
        }

        endRemoveRows();
        return true;
    }
    return false;
}

void ProjectPathsModel::addPath( const QUrl &url )
{
    if( !project->path().isParentOf(KDevelop::Path(url)) ) {
        return;
    }

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    addPathInternal( ConfigEntry(sanitizeUrl(url)), false );
    endInsertRows();
}

void ProjectPathsModel::addPathInternal( const ConfigEntry& config, bool prepend )
{
    Q_ASSERT(!config.parserArguments.isAnyEmpty());

    // Do not allow duplicates
    for (const ConfigEntry& existingConfig : std::as_const(projectPaths)) {
        if( config.path == existingConfig.path ) {
            return;
        }
    }
    if( prepend ) {
        projectPaths.prepend( config );
    } else {
        projectPaths.append( config );
    }
}

QString ProjectPathsModel::sanitizeUrl( const QUrl& url, bool needRelative ) const
{
    Q_ASSERT( project );

    if (needRelative) {
        const auto relativePath = project->path().relativePath(KDevelop::Path(url));
        return relativePath.isEmpty() ? QStringLiteral(".") : relativePath;
    }
    return url.adjusted(QUrl::StripTrailingSlash | QUrl::NormalizePathSegments).toString(QUrl::PreferLocalFile);
}

QString ProjectPathsModel::sanitizePath( const QString& path, bool expectRelative, bool needRelative ) const
{
    Q_ASSERT( project );
    Q_ASSERT( expectRelative || project->inProject(IndexedString(path)) );

    QUrl url;
    if( expectRelative ) {
        url = KDevelop::Path(project->path(), path).toUrl();
    } else {
        url = QUrl::fromUserInput(path);
    }
    return sanitizeUrl( url, needRelative );
}

#include "moc_projectpathsmodel.cpp"
