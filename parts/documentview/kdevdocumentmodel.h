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
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#ifndef KDEVDOCUMENTMODEL_H
#define KDEVDOCUMENTMODEL_H

#include <kdevitemmodel.h>
#include <kdevdocumentcontroller.h>

#include <kurl.h>
#include <kiconloader.h>

class KURL;
class KDevDocumentItem;
class KDevMimeTypeItem;
class KDevFileItem;

class KDevDocumentItem: public KDevItemCollection
{
public:
    KDevDocumentItem( const QString &name, KDevItemGroup *parent = 0 );
    virtual ~KDevDocumentItem();

    virtual KDevDocumentItem *itemAt( int index ) const;
    virtual KDevMimeTypeItem *mimeTypeItem() const
    {
        return 0;
    }
    virtual KDevFileItem *fileItem() const
    {
        return 0;
    }

    QIcon icon() const
    {
        switch ( m_documentState )
        {
        case Clean:
            return QIcon();
        case Modified:
            return QIcon( SmallIcon( "filesave" ) );
        case Dirty:
            return QIcon( SmallIcon( "revert" ) );
        case DirtyAndModified:
            return QIcon( SmallIcon( "stop" ) );
        default:
            return QIcon();
        }
    }

    DocumentState documentState() const
    {
        return m_documentState;
    }

    void setDocumentState( DocumentState state )
    {
        m_documentState = state;
    }

private:
    DocumentState m_documentState;
};

class KDevMimeTypeItem: public KDevDocumentItem
{
public:
    KDevMimeTypeItem( const QString &name, KDevItemGroup *parent = 0 );
    virtual ~KDevMimeTypeItem();

    virtual KDevMimeTypeItem *mimeTypeItem() const
    {
        return const_cast<KDevMimeTypeItem*>( this );
    }

    QList<KDevFileItem*> fileList() const;
    KDevFileItem* file( const KURL &url ) const;
};

class KDevFileItem: public KDevDocumentItem
{
public:
    KDevFileItem( const KURL &url, KDevItemGroup *parent = 0 );
    virtual ~KDevFileItem();

    virtual KDevFileItem *fileItem() const
    {
        return const_cast<KDevFileItem*>( this );
    }

    const KURL &URL() const
    {
        return m_url;
    }

    void setURL( const KURL &url )
    {
        m_url = url;
    }

private:
    KURL m_url;
};

class KDevDocumentModel: public KDevItemModel
{
    Q_OBJECT
public:
    KDevDocumentModel( QObject *parent = 0 );
    virtual ~KDevDocumentModel();

    virtual KDevDocumentItem *item( const QModelIndex &index ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex &parent ) const;
    virtual int rowCount( const QModelIndex &parent ) const;

    QList<KDevMimeTypeItem*> mimeTypeList() const;
    KDevMimeTypeItem* mimeType( const QString& mimeType ) const;
};

#endif // KDEVDOCUMENTMODEL_H

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
