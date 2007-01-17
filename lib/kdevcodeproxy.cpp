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

namespace Koncrete
{

CodeProxy::CodeProxy( QObject* parent )
        : QSortFilterProxyModel( parent )
{
    setSourceModel( new CodeAggregate( this ) );
    sort( 0 );
}

CodeProxy::~CodeProxy()
{}

CodeModelList CodeProxy::codeModels() const
{
    return codeAggregate() ->codeModels();
}

CodeModel *CodeProxy::codeModel( const KUrl &url ) const
{
    return codeAggregate() ->codeModel( url );
}

void CodeProxy::insertModel( const KUrl &url,
                                 CodeModel *model )
{
    codeAggregate() ->insertModel( url, model );
}

void CodeProxy::insertModelCache( const CodeModelCache &list )
{
    codeAggregate() ->insertModelCache( list );
}

void CodeProxy::deleteModel( const KUrl &url )
{
    codeAggregate() ->deleteModel( url );
}

void CodeProxy::setMode( CodeProxy::Mode mode )
{
    codeAggregate() ->setMode( mode );
}

void CodeProxy::setFilterDocument( const KUrl & url )
{
    codeAggregate() ->setFilterDocument( url );
}

QMap<QString, int> CodeProxy::kindFilterList() const
{
    return m_kindFilterList;
}

void CodeProxy::setKindFilterList( const QMap<QString, int> list )
{
    m_kindFilterList = list;
}

CodeItem *CodeProxy::proxyToItem( const QModelIndex &proxy_index ) const
{
    return codeAggregate() ->proxyToItem( mapToSource( proxy_index ) );
}

CodeItem *CodeProxy::sourceToItem( const QModelIndex &source_index ) const
{
    return codeAggregate() ->sourceToItem( source_index );
}

void CodeProxy::setSourceModel( QAbstractItemModel * sourceModel )
{
    QSortFilterProxyModel::setSourceModel( sourceModel );
}

CodeAggregate *CodeProxy::codeAggregate() const
{
    return qobject_cast<CodeAggregate*>( sourceModel() );
}

void CodeProxy::forceReset()
{
    reset();
}

void CodeProxy::forceClear()
{
    clear();
}

CodeProxy::Mode CodeProxy::mode( ) const
{
    return codeAggregate() ->mode();
}

}
#include "kdevcodeproxy.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
