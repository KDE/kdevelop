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

#ifndef KDEVDOCUMENTMODEL_H
#define KDEVDOCUMENTMODEL_H

#include <kdevitemmodel.h>
#include <kdevdocumentcontroller.h>

#include <kurl.h>
#include <kicon.h>

class KUrl;
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
        case KDevDocument::Clean:
            return QIcon();
        case KDevDocument::Modified:
            return KIcon( "filesave" );
        case KDevDocument::Dirty:
            return KIcon( "revert" );
        case KDevDocument::DirtyAndModified:
            return KIcon( "stop" );
        default:
            return QIcon();
        }
    }

    KDevDocument::DocumentState documentState() const
    {
        return m_documentState;
    }

    void setDocumentState( KDevDocument::DocumentState state )
    {
        m_documentState = state;
    }

private:
    KDevDocument::DocumentState m_documentState;
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
    KDevFileItem* file( const KUrl &url ) const;
};

class KDevFileItem: public KDevDocumentItem
{
public:
    KDevFileItem( const KUrl &url, KDevItemGroup *parent = 0 );
    virtual ~KDevFileItem();

    virtual KDevFileItem *fileItem() const
    {
        return const_cast<KDevFileItem*>( this );
    }

    const KUrl &URL() const
    {
        return m_url;
    }

    void setURL( const KUrl &url )
    {
        m_url = url;
    }

private:
    KUrl m_url;
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
