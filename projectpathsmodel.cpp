/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
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

#include "custombuildsystemconfig.h"

ProjectPathsModel::ProjectPathsModel( QObject* parent )
    : QAbstractListModel( parent )
{
}

QVariant ProjectPathsModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || ( role != Qt::DisplayRole && role != Qt::EditRole && role != IncludesDataRole && role != DefinesDataRole ) ) {
        return QVariant();
    }

    if( index.row() < 0 || index.row() >= rowCount() || index.column() != 0 ) {
        return QVariant();
    }

    if( index.row() == projectPaths.count() ) {
        return i18n( "Double-Click here to insert a new path to a directory or file." );
    } else {
        switch( role ) {
        case IncludesDataRole:
            return projectPaths.at( index.row() ).includes;
            break;
        case DefinesDataRole:
            return projectPaths.at( index.row() ).defines;
            break;
        default:
            return projectPaths.at( index.row() ).path;
            break;
        }
    }
}

int ProjectPathsModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() ) {
        return 0;
    }
    return projectPaths.count() + 1;
}

bool ProjectPathsModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if( !index.isValid() || ( role != Qt::EditRole && role != SetIncludesRole && role != SetDefinesRole ) ) {
        return false;
    }
    if( index.row() < 0 || index.row() >= rowCount() || index.column() != 0 ) {
        return false;
    }

    if( index.row() == projectPaths.count() ) {
        if( value.toString() != data( index ).toString() ) {
            beginInsertRows( QModelIndex(), projectPaths.count(), projectPaths.count() );
            CustomBuildSystemProjectPathConfig c;
            c.path = value.toString();
            projectPaths << c;
            endInsertRows();
        }
    } else {
        switch( role ) {
            case SetIncludesRole:
                projectPaths[index.row()].includes = value.toStringList();
                break;
            case SetDefinesRole:
                projectPaths[index.row()].defines = value.toHash();
                break;
            default:
                projectPaths[index.row()].path = value.toString();
                break;
        }
        emit dataChanged( index, index );
        return true;
    }

    return false;
}

Qt::ItemFlags ProjectPathsModel::flags( const QModelIndex& index ) const
{
    if( !index.isValid() ) {
        return 0;
    }

    return Qt::ItemFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
}

QList< CustomBuildSystemProjectPathConfig > ProjectPathsModel::paths() const
{
    return projectPaths;
}

void ProjectPathsModel::setPaths(const QList< CustomBuildSystemProjectPathConfig >& paths )
{
    projectPaths = paths;
    this->reset();
}

bool ProjectPathsModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( row >= 0 && count > 0 && row < rowCount() - 1 ) {
        beginRemoveRows( parent, row, row + count - 1 );
        for( int i = row + count - 1; i >= row; i-- ) {
            projectPaths.removeAt( i );
        }
        endRemoveRows();
    }
    return false;
}


#include "projectpathsmodel.moc"
