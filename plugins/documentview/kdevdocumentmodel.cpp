/* This file is part of KDevelop
  Copyright (C) 2005 Adam Treat <treat@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kdevdocumentmodel.h"
#include <QtCore/qdebug.h>

KDevDocumentItem::KDevDocumentItem( const QString &name, KDevItemGroup *parent )
        : KDevItemCollection( name, parent ),
        m_documentState( KDevDocument::Clean )
{}

KDevDocumentItem::~KDevDocumentItem()
{}

KDevDocumentItem *KDevDocumentItem::itemAt( int index ) const
{
    return static_cast<KDevDocumentItem*>( KDevItemCollection::itemAt( index ) );
}

KDevMimeTypeItem::KDevMimeTypeItem( const QString &name, KDevItemGroup *parent )
        : KDevDocumentItem( name, parent )
{}

KDevMimeTypeItem::~KDevMimeTypeItem()
{}

QList<KDevFileItem*> KDevMimeTypeItem::fileList() const
{
    QList<KDevFileItem*> lst;

    for ( int i = 0; i < itemCount(); ++i )
    {
        if ( KDevFileItem * item = itemAt( i ) ->fileItem() )
            lst.append( item );
    }

    return lst;
}

KDevFileItem* KDevMimeTypeItem::file( const KUrl &url ) const
{
    foreach( KDevFileItem * item, fileList() )
    {
        if ( item->URL() == url )
            return item;
    }

    return 0;
}

KDevFileItem::KDevFileItem( const KUrl &url, KDevItemGroup *parent )
        : KDevDocumentItem( url.fileName(), parent ),
        m_url( url )
{}

KDevFileItem::~KDevFileItem()
{}

KDevDocumentModel::KDevDocumentModel( QObject *parent )
        : KDevItemModel( parent )
{}

KDevDocumentModel::~KDevDocumentModel()
{}

KDevDocumentItem *KDevDocumentModel::item( const QModelIndex &index ) const
{
    return reinterpret_cast<KDevDocumentItem*>( KDevItemModel::item( index ) );
}

QModelIndex KDevDocumentModel::index( int row, int column, const QModelIndex &parent ) const
{
    return KDevItemModel::index( row, column, parent );
}

int KDevDocumentModel::rowCount( const QModelIndex &parent ) const
{
    return KDevItemModel::rowCount( parent );
}

QList<KDevMimeTypeItem*> KDevDocumentModel::mimeTypeList() const
{
    KDevDocumentItem * collection = reinterpret_cast<KDevDocumentItem*>( root() );

    QList<KDevMimeTypeItem*> lst;

    for ( int i = 0; i < collection->itemCount(); ++i )
    {
        if ( KDevMimeTypeItem * item = collection->itemAt( i ) ->mimeTypeItem() )
            lst.append( item );
    }

    return lst;
}

KDevMimeTypeItem* KDevDocumentModel::mimeType( const QString& mimeType ) const
{
    foreach( KDevMimeTypeItem * item, mimeTypeList() )
    {
        if ( item->name() == mimeType )
            return item;
    }

    return 0;
}

#include "kdevdocumentmodel.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
