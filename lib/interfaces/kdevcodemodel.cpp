/*
 * This file is part of KDevelop
 *
 * Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kdevcodemodel.h"

#include <QMutexLocker>

KDevCodeItem::KDevCodeItem( const QString &name, KDevItemGroup *parent )
        : KDevItemCollection( name, parent )
{}

KDevCodeItem::~KDevCodeItem()
{}

KDevCodeItem *KDevCodeItem::itemAt( int index ) const
{
    return static_cast<KDevCodeItem*>( KDevItemCollection::itemAt( index ) );
}

KDevCodeModel::KDevCodeModel( QObject *parent )
        : KDevItemModel( parent )
{}

KDevCodeModel::~KDevCodeModel()
{}

QVariant KDevCodeModel::data( const QModelIndex &index, int role ) const
{
    if ( KDevCodeItem * code_item = item( index ) )
    {
        switch ( role )
        {
        case Qt::DisplayRole:
            return code_item->display();
        case Qt::DecorationRole:
            return code_item->decoration();
        case Qt::ToolTipRole:
            return code_item->toolTip();
        case Qt::WhatsThisRole:
            return code_item->whatsThis();
        case Qt::UserRole:
            return code_item->kind();
        default:
            break;
        }
    }

    return QVariant();
}

void KDevCodeModel::beginAppendItem( KDevCodeItem *item,
                                     KDevItemCollection *collection )
{
    QMutexLocker locker( &m_mutex );
    Q_ASSERT( item != root() );
    QModelIndex parent;

    if ( !collection )
        collection = root();

    parent = indexOf( collection );

    int row = collection->itemCount();

    beginInsertRows( parent, row, row );
    collection->add
    ( item );
}

void KDevCodeModel::endAppendItem()
{
    QMutexLocker locker( &m_mutex );
    endInsertRows();
}

void KDevCodeModel::beginRemoveItem( KDevCodeItem *item )
{
    QMutexLocker locker( &m_mutex );
    Q_ASSERT( item != 0 && item->parent() != 0 );
    Q_ASSERT( item->parent() ->collection() != 0 );

    KDevItemCollection *parent_collection = item->parent() ->collection();

    int row = positionOf( item );

    beginRemoveRows( indexOf( parent_collection ), row, row );
    parent_collection->remove
    ( row );
}

void KDevCodeModel::endRemoveItem()
{
    QMutexLocker locker( &m_mutex );
    endRemoveRows();
}

KDevCodeItem *KDevCodeModel::item( const QModelIndex &index ) const
{
    return reinterpret_cast<KDevCodeItem*>( KDevItemModel::item( index ) );
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
