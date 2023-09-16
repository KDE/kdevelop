/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#include "definesmodel.h"

#include <KLocalizedString>

using namespace KDevelop;

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
            return i18nc("@title:column", "Define");
        case 1:
            return i18nc("@title:column", "Value");
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
            m_defines << qMakePair<QString,QString>(value.toString(), QString());
            endInsertRows();
        }
    } else {
        switch( index.column() ) {
        case 0:
            m_defines[ index.row() ].first = value.toString();
            break;
        case 1:
            m_defines[ index.row() ].second = value.toString();
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
        return Qt::NoItemFlags;
    }

    if( index.row() == m_defines.count() && index.column() == 1 ) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemFlags( Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
}

Defines DefinesModel::defines() const
{
    Defines ret;
    ret.reserve(m_defines.size());
    for (const auto& pair : m_defines) {
        ret[pair.first] = pair.second;
    }
    return ret;
}

void DefinesModel::setDefines(const Defines& includes )
{
    beginResetModel();
    m_defines.clear();
    m_defines.reserve(includes.size());
    for ( auto it = includes.begin(); it != includes.end(); ++it ) {
        m_defines << std::pair<QString, QString>{ it.key(), it.value() };
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

#include "moc_definesmodel.cpp"
