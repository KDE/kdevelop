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
#include "kdevcodeproxy.h"

#include "kdevcodeaggregate_p.h"

KDevCodeProxy::KDevCodeProxy( QObject* parent )
        : QSortFilterProxyModel( parent )
{
    setSourceModel( new KDevCodeAggregate( this ) );
    sort( 0 );
}

KDevCodeProxy::~KDevCodeProxy()
{}

CodeModelList KDevCodeProxy::codeModels() const
{
    return codeAggregate() ->codeModels();
}

KDevCodeModel *KDevCodeProxy::codeModel( const KUrl &url ) const
{
    return codeAggregate() ->codeModel( url );
}

void KDevCodeProxy::insertModel( const KUrl &url,
                                 KDevCodeModel *model )
{
    codeAggregate() ->insertModel( url, model );
}

void KDevCodeProxy::deleteModel( const KUrl &url )
{
    codeAggregate() ->deleteModel( url );
}

void KDevCodeProxy::setMode( KDevCodeProxy::Mode mode )
{
    codeAggregate() ->setMode( mode );
}

void KDevCodeProxy::setFilterDocument( const KUrl & url )
{
    codeAggregate() ->setFilterDocument( url );
}

QMap<QString, int> KDevCodeProxy::kindFilterList() const
{
    return m_kindFilterList;
}

void KDevCodeProxy::setKindFilterList( const QMap<QString, int> list )
{
    m_kindFilterList = list;
}

KDevCodeItem *KDevCodeProxy::proxyToItem( const QModelIndex &proxy_index ) const
{
    return codeAggregate() ->proxyToItem( mapToSource( proxy_index ) );
}

KDevCodeItem *KDevCodeProxy::sourceToItem( const QModelIndex &source_index ) const
{
    return codeAggregate() ->sourceToItem( source_index );
}

void KDevCodeProxy::setSourceModel( QAbstractItemModel * sourceModel )
{
    QSortFilterProxyModel::setSourceModel( sourceModel );
}

KDevCodeAggregate *KDevCodeProxy::codeAggregate() const
{
    return qobject_cast<KDevCodeAggregate*>( sourceModel() );
}

void KDevCodeProxy::forceReset()
{
    reset();
}

void KDevCodeProxy::forceClear()
{
    clear();
}

KDevCodeProxy::Mode KDevCodeProxy::mode( ) const
{
    return codeAggregate() ->mode();
}

#include "kdevcodeproxy.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
