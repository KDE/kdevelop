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

#include "definesmodel.h"

#include <klocale.h>

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

    // Only show the hint for display, once the user goes into edit mode leave an empty line
    // makes the setData check easier and follows common behaviour of this in lineedits etc.
    if( index.row() == m_defines.count() && index.column() == 0 && role == Qt::DisplayRole ) {
        return i18n( "Double-click here to insert a new define to be used for the path" );
    } else if( index.row() < m_defines.count() ) {
        switch( index.column() ) {
        case 0:
            return m_defines.at( index.row() ).first;
        case 1:
            return m_defines.at( index.row() ).second;
        default:
            Q_ASSERT_X( 0, "DefinesModel::data", "Invalid column requested" );
            break;
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
    if( parent.isValid() ) {
        return 0;
    }
    return 2;
}

QVariant DefinesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        switch( section ) {
        case 0:
            return i18n("Define");
        case 1:
            return i18n("Value");
        default:
            Q_ASSERT_X( 0, "DefinesModel::headerData", "Invalid column requested" );
            break;
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
        if( index.column() == 0 && !value.toString().isEmpty() ) {
            beginInsertRows( QModelIndex(), m_defines.count(), m_defines.count() );
            m_defines << qMakePair<QString,QVariant>( value.toString(), "" );
            endInsertRows();
        }
    } else {
        switch( index.column() ) {
        case 0:
            m_defines[ index.row() ].first = value.toString();
            break;
        case 1:
            m_defines[ index.row() ].second = QVariant( value.toString() );
            break;
        default:
            Q_ASSERT_X( 0, "DefinesModel::setData", "Invalid column requested" );
            return false;
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
    typedef QPair<QString, QVariant> DefinePair;
    QHash<QString,QVariant> tmp;
    foreach(const DefinePair& pair, m_defines) {
        tmp[pair.first] = pair.second;
    }
    return tmp;
}

void DefinesModel::setDefines(const QHash<QString,QVariant>& includes )
{
    beginResetModel();
    m_defines.clear();
    foreach( const QString& k, includes.keys() ) {
        m_defines << qMakePair<QString,QVariant>( k, includes[k] );
    }
    endResetModel();
}

bool DefinesModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( row >= 0 && count > 0 && row < m_defines.count() ) {
        beginRemoveRows( parent, row, row + count - 1 );
        for( int i = 0; i < count; ++i ) {
            m_defines.removeAt( row );
        }
        endRemoveRows();
        return true;
    }
    return false;
}

