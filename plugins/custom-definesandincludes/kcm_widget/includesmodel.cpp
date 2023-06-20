/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "includesmodel.h"

#include <KLocalizedString>

IncludesModel::IncludesModel( QObject* parent )
    : QAbstractListModel( parent )
{
}

QVariant IncludesModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || ( role != Qt::DisplayRole && role != Qt::EditRole ) ) {
        return QVariant();
    }

    if( index.row() < 0 || index.row() >= rowCount() || index.column() != 0 ) {
        return QVariant();
    }

    return m_includes.at( index.row() );
}

int IncludesModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : m_includes.count();
}

bool IncludesModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if( !index.isValid() || role != Qt::EditRole ) {
        return false;
    }
    if( index.row() < 0 || index.row() >= rowCount() || index.column() != 0 ) {
        return false;
    }

    m_includes[index.row()] = value.toString().trimmed();
    emit dataChanged( index, index );
    return true;
}

Qt::ItemFlags IncludesModel::flags( const QModelIndex& index ) const
{
    if( !index.isValid() ) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
}

QStringList IncludesModel::includes() const
{
    return m_includes;
}

void IncludesModel::setIncludes(const QStringList& includes )
{
    beginResetModel();
    m_includes.clear();
    for (const QString& includePath : includes) {
        addIncludeInternal( includePath.trimmed() );
    }
    endResetModel();
}

bool IncludesModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( row >= 0 && count > 0 && row < m_includes.count() ) {
        beginRemoveRows( parent, row, row + count - 1 );
        for( int i = 0; i < count; ++i ) {
            m_includes.removeAt( row );
        }
        endRemoveRows();
        return true;
    }
    return false;
}

void IncludesModel::addInclude( const QString& includePath )
{
    if( !includePath.isEmpty() ) {
        beginInsertRows( QModelIndex(), rowCount(), rowCount() );
        addIncludeInternal( includePath );
        endInsertRows();
    }
}

void IncludesModel::addIncludeInternal( const QString& includePath )
{
    if ( includePath.isEmpty() ) {
        return;
    }

    // Do not allow duplicates
    if (m_includes.contains(includePath)) {
        return;
    }

    m_includes << includePath;
}

#include "moc_includesmodel.cpp"
