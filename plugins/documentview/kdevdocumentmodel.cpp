/* This file is part of KDevelop
  Copyright 2005 Adam Treat <treat@kde.org>

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
#include <KFileItem>
#include <QtCore/qdebug.h>

KDevDocumentItem::KDevDocumentItem( const QString &name )
    : QStandardItem( name ),
        m_documentState( KDevelop::IDocument::Clean )
{
    setIcon(icon());
}

KDevDocumentItem::~KDevDocumentItem()
{}

// KDevDocumentItem *KDevDocumentItem::itemAt( int index ) const
// {
//     return static_cast<KDevDocumentItem*>( QStandardItem::itemFromIndex( index ) );
// }

KDevMimeTypeItem::KDevMimeTypeItem( const QString &name )
        : KDevDocumentItem( name )
{
    setToolTip(name);
}

KDevMimeTypeItem::~KDevMimeTypeItem()
{}

QList<KDevFileItem*> KDevMimeTypeItem::fileList() const
{
    QList<KDevFileItem*> lst;

    for ( int i = 0; i < rowCount(); ++i )
    {
        if ( KDevFileItem * item = dynamic_cast<KDevDocumentItem*>( child( i ) ) ->fileItem() )
            lst.append( item );
    }

    return lst;
}

KDevFileItem* KDevMimeTypeItem::file( const KUrl &url ) const
{
    foreach( KDevFileItem * item, fileList() )
    {
        if ( item->url() == url )
            return item;
    }

    return 0;
}

KDevFileItem::KDevFileItem( const KUrl &url )
        : KDevDocumentItem( url.fileName() ),
        m_url( url )
{
    KFileItem fi = KFileItem( url, QString(), 0 );
    m_fileIcon = fi.iconName();
    setIcon( KIcon( m_fileIcon ) );
}

KDevFileItem::~KDevFileItem()
{}

KDevDocumentModel::KDevDocumentModel( QObject *parent )
    : QStandardItemModel( parent )
{
    setRowCount(0);
    setColumnCount(1);
}

KDevDocumentModel::~KDevDocumentModel()
{}

QList<KDevMimeTypeItem*> KDevDocumentModel::mimeTypeList() const
{

    QList<KDevMimeTypeItem*> lst;
    for ( int i = 0; i < rowCount() ; ++i )
    {
        if ( KDevMimeTypeItem * mimeitem = dynamic_cast<KDevDocumentItem*>( item( i ) ) ->mimeTypeItem() )
        {

            lst.append( mimeitem );
        }
    }

    return lst;
}

KDevMimeTypeItem* KDevDocumentModel::mimeType( const QString& mimeType ) const
{
    foreach( KDevMimeTypeItem * item, mimeTypeList() )
    {
        if ( item->text() == mimeType )
            return item;
    }

    return 0;
}

#include "kdevdocumentmodel.moc"

