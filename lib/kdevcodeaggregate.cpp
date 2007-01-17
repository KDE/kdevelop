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
#include "kdevcodemodel.h"

namespace Koncrete
{

CodeAggregate::CodeAggregate( QObject *parent )
        : QAbstractProxyModel( parent ),
        m_mode( CodeProxy::Document ),
        m_filter( KUrl() )
{}

CodeAggregate::~CodeAggregate()
{}

CodeModelList CodeAggregate::codeModels() const
{
    return m_codeModels.values();
}

CodeModel *CodeAggregate::codeModel( const KUrl &url ) const
{
    if ( m_codeModels.contains( url ) )
        return m_codeModels[ url ];
    else
        return 0;
}

void CodeAggregate::insertModel( const KUrl &url,
                                     CodeModel *model )
{
    if ( m_codeModels.contains( url ) )
        deleteModel( url );

    m_codeModels.insert( url, model );
    mapCodeModels();
}

void CodeAggregate::insertModelCache( const CodeModelCache &list )
{
    CodeModelCache::const_iterator it = list.begin();
    for ( ; it != list.end(); it++ )
    {
        KUrl url = ( *it ).first;
        CodeModel *model = ( *it ).second;

        if ( m_codeModels.contains( url ) )
            deleteModel( url );

        m_codeModels.insert( url, model );
    }
    mapCodeModels();
}

void CodeAggregate::deleteModel( const KUrl &url )
{
    CodeModel * model = m_codeModels.take( url );
    model->deleteLater();
}

CodeProxy::Mode CodeAggregate::mode( ) const
{
    return m_mode;
}

void CodeAggregate::setMode( CodeProxy::Mode mode )
{
    m_mode = mode;
    mapCodeModels();
}

void CodeAggregate::setFilterDocument( const KUrl & url )
{
    m_filter = url;
    setMode( CodeProxy::Document );
}

CodeItem *CodeAggregate::proxyToItem( const QModelIndex &proxy_index ) const
{
    if ( !proxy_index.parent().isValid() )
    {
        if ( m_mode == CodeProxy::Normalize )
            return m_normalizeRoot[ proxy_index.row() ];
        else
        {
            QModelIndex source_index =
                m_modelHash[ proxy_index.row() ] ->source_index;
            return sourceToItem( source_index );
        }
    }
    else if ( CodeItem * codeItem = sourceToItem( proxy_index ) )
    {
        if ( m_mode == CodeProxy::Normalize )
            if ( m_substituteHash.contains( codeItem ) )
                return m_substituteHash[ codeItem ];

        return codeItem;
    }
    return 0;
}

CodeItem *CodeAggregate::sourceToItem( const QModelIndex &source_index ) const
{
    return reinterpret_cast<CodeItem*>( source_index.internalPointer() );
}

QModelIndex CodeAggregate::index( int row, int column,
                                      const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
    {
        if ( m_mode == CodeProxy::Normalize )
        {
            CodeItem * codeItem = m_normalizeRoot[ row ];
            return createIndex( row, column, codeItem );
        }
        else
        {
            if ( m_modelHash[ row ] )
            {
                CodeModel *source_model = m_modelHash[ row ] ->source_model;
                QModelIndex source_index = m_modelHash[ row ] ->source_index;
                CodeItem *item = source_model->item( source_index );

                return createIndex( row, column, item );
            }
        }
    }
    else if ( CodeItem * codeItem = sourceToItem( parent ) )
    {
        if ( row >= codeItem->itemCount()
             && m_mode == CodeProxy::Normalize )
        {
            CodeItem * childItem = m_stepchildHash[ codeItem ][ row ];
            return createIndex( row, column, childItem );
        }

        CodeModel *model = codeItem->model();
        Q_ASSERT( model );
        QModelIndex child = model->index( row, column,
                                          model->indexOf( codeItem ) );
        CodeItem *childItem = sourceToItem( child );

        if ( m_mode == CodeProxy::Normalize )
        {
            if ( m_substituteHash.contains( childItem ) )
                childItem = m_substituteHash[ childItem ];
        }

        return createIndex( positionOf( childItem ), column, childItem );
    }
    return QModelIndex();
}

QModelIndex CodeAggregate::parent( const QModelIndex &child ) const
{
    if ( !child.isValid() )
        return QModelIndex();
    else if ( CodeItem * codeItem = sourceToItem( child ) )
    {
        if ( codeItem->parent() == codeItem->model() ->root() )
        {
            return QModelIndex();
        }
        else if ( CodeItem * parentItem =
                      dynamic_cast<CodeItem*>( codeItem->parent() ) )
        {
            if ( m_mode == CodeProxy::Normalize )
            {
                if ( m_substituteHash.contains( parentItem ) )
                    parentItem = m_substituteHash[ parentItem ];
            }
            return createIndex( positionOf( parentItem ), 0, parentItem );
        }
    }
    return QModelIndex();
}

QVariant CodeAggregate::data( const QModelIndex &index, int role ) const
{
    if (!index.isValid())
        return QVariant();

    if ( !index.parent().isValid() )
    {
        if ( m_mode == CodeProxy::Normalize )
        {
            CodeItem * codeItem = m_normalizeRoot[ index.row() ];
            return codeItem->model() ->data(
                       codeItem->model() ->indexOf( codeItem ), role );
        }
        else
        {
            CodeModel *source_model =
                m_modelHash[ index.row() ] ->source_model;
            QModelIndex source_index =
                m_modelHash[ index.row() ] ->source_index;
            return source_model->data( source_index, role );
        }
    }
    else if ( CodeItem * codeItem = sourceToItem( index ) )
    {
        if ( m_mode == CodeProxy::Normalize )
        {
            if ( m_substituteHash.contains( codeItem ) )
                codeItem = m_substituteHash[ codeItem ];
        }

        CodeModel *model = codeItem->model();
        Q_ASSERT( model );
        return model->data( model ->indexOf( codeItem ), role );
    }

    return QVariant();
}

int CodeAggregate::rowCount( const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
    {
        if ( m_mode == CodeProxy::Normalize )
            return m_normalizeRoot.count();
        else
            return m_modelHash.count();
    }
    else if ( CodeItem * codeItem = sourceToItem( parent ) )
    {
        if ( m_mode == CodeProxy::Normalize )
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

int CodeAggregate::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return 1;
}

QModelIndex CodeAggregate::mapToSource( const QModelIndex &proxyIndex ) const
{
    Q_UNUSED( proxyIndex );
    return QModelIndex();
}

QModelIndex CodeAggregate::mapFromSource( const QModelIndex &sourceIndex ) const
{
    Q_UNUSED( sourceIndex );
    return QModelIndex();
}

void CodeAggregate::setSourceModel( QAbstractItemModel *sourceModel )
{
    Q_UNUSED( sourceModel );
}

QAbstractItemModel *CodeAggregate::sourceModel() const
{
    return 0;
}

void CodeAggregate::mapCodeModels()
{
    if ( m_mode != CodeProxy::Normalize )
        createModelMap();
    else
        normalizeModels();
}

void CodeAggregate::createModelMap()
{
    CodeModelList codeModelList = m_mode != CodeProxy::Document ?
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

int CodeAggregate::positionOf( CodeItem *item ) const
{
    return item->parent() ->indexOf( item );
}

const CodeModel *CodeAggregate::model( const QModelIndex &index ) const
{
    Q_ASSERT( index.isValid() && index.model() );
    return qobject_cast<const CodeModel*>( index.model() );
}

void CodeAggregate::normalizeModels()
{
    m_normalizeHash.clear();
    m_normalizeRowCount.clear();
    m_substituteHash.clear();
    m_stepchildHash.clear();

    int rowCount = 0;
    QHash<int, CodeItem*> normalizeRoot;
    CodeModelList codeModelList = m_codeModels.values();
    CodeModelList::ConstIterator it = codeModelList.begin();
    for ( ; it != codeModelList.end(); ++it )
    {
        QList<Item *> children = ( *it ) ->root() ->items();
        QList<Item *>::ConstIterator child = children.begin();
        for ( ; child != children.end(); ++child )
        {
            CodeItem* childItem =
                static_cast<CodeItem*>( *child );
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

bool CodeAggregate::normalizeItem( CodeItem *item )
{
    bool isDuplicate = false;

    uint hash = item->hashKey();
    if ( isDuplicate = m_normalizeHash.contains( hash ) )
        m_substituteHash.insert( item, m_normalizeHash.value( hash ) );
    else
        m_normalizeHash.insert( hash, item );

    int rowCount = 0;
    QList<Item *> children = item->items();
    QList<Item *>::ConstIterator child = children.begin();
    for ( ; child != children.end(); ++child )
    {
        CodeItem* childItem =
            dynamic_cast<CodeItem*>( *child );
        if ( normalizeItem( childItem ) )
        {
            if ( isDuplicate )
            {
                CodeItem * step_parent = m_substituteHash[ item ];
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

}

#include "kdevcodeaggregate_p.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
