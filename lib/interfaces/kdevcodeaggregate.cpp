/*
 * This file is part of KDevelop
 *
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

#include "kdevcodeaggregate_p.h"

KDevCodeAggregate::KDevCodeAggregate( QObject *parent )
        : QAbstractProxyModel( parent ),
        m_mode( KDevCodeProxy::Normalize ),
        m_filter( KUrl() )
{}

KDevCodeAggregate::~KDevCodeAggregate()
{}

CodeModelList KDevCodeAggregate::codeModels() const
{
    return m_codeModels.values();
}

KDevCodeModel *KDevCodeAggregate::codeModel( const KUrl &url ) const
{
    if ( m_codeModels.contains( url ) )
        return m_codeModels[ url ];
    else
        return 0;
}

void KDevCodeAggregate::insertModel( const KUrl &url,
                                     KDevCodeModel *model )
{
    if ( m_codeModels.contains( url ) )
        deleteModel( url );

    m_codeModels.insert( url, model );
    mapCodeModels();
}

void KDevCodeAggregate::deleteModel( const KUrl &url )
{
    KDevCodeModel * model = m_codeModels.take( url );
    model->deleteLater();
}

KDevCodeProxy::Mode KDevCodeAggregate::mode( ) const
{
    return m_mode;
}

void KDevCodeAggregate::setMode( KDevCodeProxy::Mode mode )
{
    m_mode = mode;
    mapCodeModels();
}

void KDevCodeAggregate::setFilterDocument( const KUrl & url )
{
    m_filter = url;
    emit reset();   //Changing the file radically alters the structure
    setMode( KDevCodeProxy::Document );
}

KDevCodeItem *KDevCodeAggregate::proxyToItem( const QModelIndex &proxy_index ) const
{
    if ( !proxy_index.parent().isValid() )
    {
        if ( m_mode == KDevCodeProxy::Normalize )
            return m_normalizeRoot[ proxy_index.row() ];
        else
        {
            QModelIndex source_index =
                m_modelHash[ proxy_index.row() ] ->source_index;
            return sourceToItem( source_index );
        }
    }
    else if ( KDevCodeItem * codeItem = sourceToItem( proxy_index ) )
    {
        if ( m_mode == KDevCodeProxy::Normalize )
            if ( m_substituteHash.contains( codeItem ) )
                return m_substituteHash[ codeItem ];

        return codeItem;
    }
    return 0;
}

KDevCodeItem *KDevCodeAggregate::sourceToItem( const QModelIndex &source_index ) const
{
    return reinterpret_cast<KDevCodeItem*>( source_index.internalPointer() );
}

QModelIndex KDevCodeAggregate::index( int row, int column,
                                      const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
    {
        if ( m_mode == KDevCodeProxy::Normalize )
        {
            KDevCodeItem * codeItem = m_normalizeRoot[ row ];
            return createIndex( row, column, codeItem );
        }
        else
        {
            KDevCodeModel *source_model = m_modelHash[ row ] ->source_model;
            QModelIndex source_index = m_modelHash[ row ] ->source_index;
            KDevCodeItem *item = source_model->item( source_index );

            return createIndex( row, column, item );
        }
    }
    else if ( KDevCodeItem * codeItem = sourceToItem( parent ) )
    {
        if ( row >= codeItem->itemCount() )
        {
            Q_ASSERT( m_mode == KDevCodeProxy::Normalize );
            KDevCodeItem * childItem = m_stepchildHash[ codeItem ][ row ];
            return createIndex( row, column, childItem );
        }

        KDevCodeModel *model = codeItem->model();
        Q_ASSERT( model );
        QModelIndex child = model->index( row, column,
                                          model->indexOf( codeItem ) );
        KDevCodeItem *childItem = sourceToItem( child );

        if ( m_mode == KDevCodeProxy::Normalize )
        {
            if ( m_substituteHash.contains( childItem ) )
                childItem = m_substituteHash[ childItem ];
        }

        return createIndex( positionOf( childItem ), column, childItem );
    }
    return QModelIndex();
}

QModelIndex KDevCodeAggregate::parent( const QModelIndex &child ) const
{
    if ( !child.isValid() )
        return QModelIndex();
    else if ( KDevCodeItem * codeItem = sourceToItem( child ) )
    {
        if ( codeItem->parent() == codeItem->model() ->root() )
        {
            return QModelIndex();
        }
        else if ( KDevCodeItem * parentItem =
                      dynamic_cast<KDevCodeItem*>( codeItem->parent() ) )
        {
            if ( m_mode == KDevCodeProxy::Normalize )
            {
                if ( m_substituteHash.contains( parentItem ) )
                    parentItem = m_substituteHash[ parentItem ];
            }
            return createIndex( positionOf( parentItem ), 0, parentItem );
        }
    }
    return QModelIndex();
}

QVariant KDevCodeAggregate::data( const QModelIndex &index, int role ) const
{
    if ( !index.parent().isValid() )
    {
        if ( m_mode == KDevCodeProxy::Normalize )
        {
            KDevCodeItem * codeItem = m_normalizeRoot[ index.row() ];
            return codeItem->model() ->data(
                       codeItem->model() ->indexOf( codeItem ), role );
        }
        else
        {
            KDevCodeModel *source_model =
                m_modelHash[ index.row() ] ->source_model;
            QModelIndex source_index =
                m_modelHash[ index.row() ] ->source_index;
            return source_model->data( source_index, role );
        }
    }
    else if ( KDevCodeItem * codeItem = sourceToItem( index ) )
    {
        if ( m_mode == KDevCodeProxy::Normalize )
        {
            if ( m_substituteHash.contains( codeItem ) )
                codeItem = m_substituteHash[ codeItem ];
        }

        KDevCodeModel *model = codeItem->model();
        Q_ASSERT( model );
        return model->data( model ->indexOf( codeItem ), role );
    }

    return QVariant();
}

int KDevCodeAggregate::rowCount( const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
    {
        if ( m_mode == KDevCodeProxy::Normalize )
            return m_normalizeRoot.count();
        else
            return m_modelHash.count();
    }
    else if ( KDevCodeItem * codeItem = sourceToItem( parent ) )
    {
        if ( m_mode == KDevCodeProxy::Normalize )
        {
            if ( m_substituteHash.contains( codeItem ) )
                codeItem = m_substituteHash[ codeItem ];
            return m_normalizeRowCount[ codeItem ];
        }
        else
            return codeItem->model() ->rowCount(
                       codeItem->model() ->indexOf( codeItem ) );
    }
    return 0;
}

int KDevCodeAggregate::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return 1;
}

QModelIndex KDevCodeAggregate::mapToSource( const QModelIndex &proxyIndex ) const
{
    Q_UNUSED( proxyIndex );
    return QModelIndex();
}

QModelIndex KDevCodeAggregate::mapFromSource( const QModelIndex &sourceIndex ) const
{
    Q_UNUSED( sourceIndex );
    return QModelIndex();
}

void KDevCodeAggregate::setSourceModel( QAbstractItemModel *sourceModel )
{
    Q_UNUSED( sourceModel );
}

QAbstractItemModel *KDevCodeAggregate::sourceModel() const
{
    return 0;
}

void KDevCodeAggregate::mapCodeModels()
{
    if ( m_mode != KDevCodeProxy::Normalize )
        createModelMap();
    else
        normalizeModels();
}

void KDevCodeAggregate::createModelMap()
{
    CodeModelList codeModelList = m_mode != KDevCodeProxy::Document ?
                                  m_codeModels.values() :
                                  m_codeModels.values( m_filter );

    int row = 0;
    QHash<int, ModelMap*> tempHash;
    CodeModelList::ConstIterator it = codeModelList.begin();
    for ( ; it != codeModelList.end(); ++it )
    {
        int rowCount = ( *it ) ->rowCount( QModelIndex() );
        for ( int i = 0; i < rowCount; ++i )
        {
            ModelMap *m = new ModelMap;
            m->source_model = ( *it );
            m->source_index = m->source_model->index( i, 0, QModelIndex() );
            tempHash.insert( row, m );
            row++;
        }
    }

    for ( int i = 0; i < m_modelHash.count(); ++i )
    {
        QModelIndex from = m_modelHash[ i ] ->source_index;
        QModelIndex to = tempHash.contains( i ) ?
                         tempHash[ i ] ->source_index : QModelIndex();
        if ( from != to )
        {
            changePersistentIndex( from, to );
        }
    }

    qDeleteAll( m_modelHash );
    m_modelHash.clear();
    m_modelHash = tempHash;
    emit layoutChanged();
}

int KDevCodeAggregate::positionOf( KDevCodeItem *item ) const
{
    return item->parent() ->indexOf( item );
}

const KDevCodeModel *KDevCodeAggregate::model( const QModelIndex &index ) const
{
    Q_ASSERT( index.isValid() && index.model() );
    return qobject_cast<const KDevCodeModel*>( index.model() );
}

void KDevCodeAggregate::normalizeModels()
{
    m_normalizeHash.clear();
    m_normalizeRowCount.clear();
    m_substituteHash.clear();
    m_stepchildHash.clear();

    int rowCount = 0;
    QHash<int, KDevCodeItem*> normalizeRoot;
    CodeModelList codeModelList = m_codeModels.values();
    CodeModelList::ConstIterator it = codeModelList.begin();
    for ( ; it != codeModelList.end(); ++it )
    {
        QList<KDevItem *> children = ( *it ) ->root() ->items();
        QList<KDevItem *>::ConstIterator child = children.begin();
        for ( ; child != children.end(); ++child )
        {
            KDevCodeItem* childItem =
                static_cast<KDevCodeItem*>( *child );
            if ( normalizeItem( childItem ) )
            {
                normalizeRoot.insert( rowCount, childItem );
                rowCount++;
            }
        }
    }

    m_normalizeHash.clear();
    m_normalizeRoot.clear();
    m_normalizeRoot = normalizeRoot;
    emit layoutChanged();
}

bool KDevCodeAggregate::normalizeItem( KDevCodeItem *item )
{
    bool isDuplicate = false;

    uint hash = item->hashKey();
    if ( isDuplicate = m_normalizeHash.contains( hash ) )
        m_substituteHash.insert( item, m_normalizeHash.value( hash ) );
    else
        m_normalizeHash.insert( hash, item );

    int rowCount = 0;
    QList<KDevItem *> children = item->items();
    QList<KDevItem *>::ConstIterator child = children.begin();
    for ( ; child != children.end(); ++child )
    {
        KDevCodeItem* childItem =
            dynamic_cast<KDevCodeItem*>( *child );
        if ( normalizeItem( childItem ) )
        {
            if ( isDuplicate )
            {
                KDevCodeItem * step_parent = m_substituteHash[ item ];
                int step_row = m_normalizeRowCount[ step_parent ];
                m_normalizeRowCount[ step_parent ] = step_row + 1;
                m_stepchildHash[ step_parent ][ step_row ] = childItem;
            }
            else
                rowCount++;
        }
    }

    if ( !isDuplicate )
        m_normalizeRowCount.insert( item, rowCount );

    return !isDuplicate;
}

#include "kdevcodeaggregate_p.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
