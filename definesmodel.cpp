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

#include "definesmodel.h"

#include <klocale.h>

#include "custombuildsystemconfig.h"

DefinesModel::DefinesModel( QObject* parent )
    : QAbstractTableModel( parent )
{
}

QVariant DefinesModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || ( role != Qt::DisplayRole && role != Qt::EditRole ) ) {
        return QVariant();
    }

    if( index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= columnCount() ) {
        return QVariant();
    }

    if( index.row() == m_defines.count() && index.column() == 0 ) {
        return i18n( "Double-Click here to insert a new define to be used for the path" );
    } else if( index.row() < m_defines.count() ) {
        if( index.column() == 0 ) {
            return m_defines.at( index.row() ).first;
        } else {
            return m_defines.at( index.row() ).second;
        }
    }
    return QVariant();
}

int DefinesModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() ) {
        return 0;
    }
    return m_defines.count() + 1;
}

int DefinesModel::columnCount(const QModelIndex& parent) const
{
    if( !parent.isValid() ) {
        return 2;
    }
    return 0;
}

QVariant DefinesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        if( section == 0 ) {
            return i18n("Define");
        } else if( section == 1 ) {
            return i18n("Value");
        }
    }
    return QVariant();
}


bool DefinesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if( !index.isValid() || role != Qt::EditRole ) {
        return false;
    }
    if( index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= columnCount() ) {
        return false;
    }

    if( index.row() == m_defines.count() ) {
        if( value.toString() != data( index ).toString() && index.column() == 0 ) {
            beginInsertRows( QModelIndex(), m_defines.count(), m_defines.count() );
            m_defines << qMakePair<QString,QVariant>( value.toString(), "" );
            endInsertRows();
        }
    } else {
        if( index.column() == 0 ) {
            m_defines[ index.row() ].first = value.toString();
        } else {
            m_defines[ index.row() ].second = QVariant( value.toString() );
        }
        emit dataChanged( index, index );
        return true;

    }

    return false;
}

Qt::ItemFlags DefinesModel::flags( const QModelIndex& index ) const
{
    if( !index.isValid() ) {
        return 0;
    }

    if( index.row() == m_defines.count() && index.column() == 1 ) {
        return 0;
    }
    return Qt::ItemFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
}

QHash<QString,QVariant> DefinesModel::defines() const
{
    QHash<QString,QVariant> tmp;
    QList<QPair<QString, QVariant> >::const_iterator it, end = m_defines.constEnd();
    for( it = m_defines.constBegin() ; it != end; it++ ) {
        tmp[(*it).first] = (*it).second;
    }
    return tmp;
}

void DefinesModel::setDefines(const QHash<QString,QVariant>& includes )
{
    m_defines.clear();
    foreach( const QString& k, includes.keys() ) {
        m_defines << qMakePair<QString,QVariant>( k, includes[k] );
    }
    reset();
}

bool DefinesModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( row >= 0 && count > 0 && row < rowCount() - 1 ) {
        beginRemoveRows( parent, row, row + count - 1 );
        for( int i = row + count - 1; i >= row; i-- ) {
            m_defines.removeAt( i );
        }
        endRemoveRows();
    }
    return false;
}

#include "definesmodel.moc"
