/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "projectpathsmodel.h"

#include <klocale.h>
#include <interfaces/iproject.h>
#include <language/duchain/indexedstring.h>
#include <util/path.h>

namespace{
//TODO: Port the model to the Path api.
KUrl projectFolder( KDevelop::IProject* project )
{
    auto url = project->path().toUrl();
    url.adjustPath( KUrl::AddTrailingSlash );
    return url;
}
}

ProjectPathsModel::ProjectPathsModel( QObject* parent )
    : QAbstractListModel( parent ), project( 0 )
{
}

void ProjectPathsModel::setProject(KDevelop::IProject* w_project)
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
        break;
    case DefinesDataRole:
        return pathConfig.defines;
        break;
    case Qt::EditRole:
        return sanitizePath( pathConfig.path, true, false );
        break;
    case Qt::DisplayRole: {
        const QString& path = pathConfig.path;
        return (path == ".") ? "(project root)" : path;
        break;
    }
    case FullUrlDataRole:
        return QVariant::fromValue(KUrl( sanitizePath( pathConfig.path, true, false ) ));
        break;
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
        foreach( const ConfigEntry& existingConfig, projectPaths ) {
            if( addedPath == existingConfig.path ) {
                return false;
            }
        }
        projectPaths.insert( 1, sanitizePath( value.toString(), false ) );
        emit dataChanged( this->index( 1, 0 ), this->index( projectPaths.count() - 1, 0 ) );
        return true;
    }

    ConfigEntry& pathConfig = projectPaths[ index.row() ];
    switch( role ) {
    case IncludesDataRole:
        pathConfig.includes = value.toStringList();
        break;
    case DefinesDataRole:
        pathConfig.defines = value.toHash();
        break;
    case Qt::EditRole:
        pathConfig.path = sanitizePath( value.toString(), false );
        break;
    case Qt::DisplayRole:
        pathConfig.path = sanitizePath( value.toString(), true );
        break;
    case FullUrlDataRole:
        pathConfig.path = sanitizeUrl( value.value<KUrl>() );
        break;
    default:
        return false;
        break;
    }
    emit dataChanged( index, index );
    return true;
}

Qt::ItemFlags ProjectPathsModel::flags( const QModelIndex& index ) const
{
    if( !index.isValid() ) {
        return 0;
    }

    if( index.row() == 0 ) {
        return Qt::ItemFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
    }

    return Qt::ItemFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
}

QList< ConfigEntry > ProjectPathsModel::paths() const
{
    return projectPaths;
}

void ProjectPathsModel::setPaths(const QList< ConfigEntry >& paths )
{
    beginResetModel();
    projectPaths.clear();
    foreach( const ConfigEntry& existingPathConfig, paths ) {
        // Sanitize the path of loaded config
        ConfigEntry config = existingPathConfig;
        config.path = sanitizePath( config.path == "." ? QString() : config.path );
        addPathInternal( config, false );
    }
    addPathInternal( sanitizePath( QString() ), true ); // add an empty "root" config entry if one does not exist
    endResetModel();
}

bool ProjectPathsModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( row >= 0 && count > 0 && row < rowCount() ) {
        beginRemoveRows( parent, row, row + count - 1 );

        for( int i = 0; i < count; ++i ) {
            if( projectPaths.at(row).path == "." ) {
                continue; // we won't remove the root item
            }
            projectPaths.removeAt(row);
        }

        endRemoveRows();
        return true;
    }
    return false;
}

void ProjectPathsModel::addPath( const KUrl& url )
{
    if( !projectFolder(project).isParentOf(url) ) {
        return;
    }

    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    addPathInternal( sanitizeUrl(url), false );
    endInsertRows();
}

void ProjectPathsModel::addPathInternal( const ConfigEntry& config, bool prepend )
{
    // Do not allow duplicates
    foreach( const ConfigEntry& existingConfig, projectPaths ) {
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

QString ProjectPathsModel::sanitizeUrl( KUrl url, bool needRelative ) const
{
    Q_ASSERT( project );

    url.cleanPath();
    if( needRelative )
        url = KUrl::relativeUrl( projectFolder(project), url );
    return url.pathOrUrl( KUrl::RemoveTrailingSlash );
}

QString ProjectPathsModel::sanitizePath( const QString& path, bool expectRelative, bool needRelative ) const
{
    Q_ASSERT( project );
    Q_ASSERT( expectRelative || project->inProject(KDevelop::IndexedString(path)) );

    KUrl url;
    if( expectRelative ) {
        url = projectFolder(project);
        url.addPath(path);
    } else {
        url = path;
    }
    return sanitizeUrl( url, needRelative );
}

#include "projectpathsmodel.moc"
