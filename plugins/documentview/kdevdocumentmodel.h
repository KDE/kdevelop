/* This file is part of KDevelop
  Copyright 2005 Adam Treat <treat@kde.org>
  Copyright 2013 Sebastian Kügler <sebas@kde.org>

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

#ifndef KDEVPLATFORM_PLUGIN_KDEVDOCUMENTMODEL_H
#define KDEVPLATFORM_PLUGIN_KDEVDOCUMENTMODEL_H

#include <QStandardItem>
#include <QStandardItemModel>
#include <QUrl>

#include <interfaces/idocument.h>

#include <QIcon>

class KDevDocumentItem;
class KDevCategoryItem;
class KDevFileItem;

class KDevDocumentItem: public QStandardItem
{
public:
    explicit KDevDocumentItem( const QString &name );
    virtual ~KDevDocumentItem();

    virtual KDevCategoryItem *categoryItem() const
    {
        return 0;
    }
    virtual KDevFileItem *fileItem() const
    {
        return 0;
    }

    QIcon icon() const;

    KDevelop::IDocument::DocumentState documentState() const;
    void setDocumentState( KDevelop::IDocument::DocumentState state );

    const QUrl url() const;
    void setUrl(const QUrl &url);

protected:
    QString m_fileIcon;

private:
    QUrl m_url;
    KDevelop::IDocument::DocumentState m_documentState;
};

class KDevCategoryItem: public KDevDocumentItem
{
public:
    explicit KDevCategoryItem( const QString &name );
    virtual ~KDevCategoryItem();

    virtual KDevCategoryItem *categoryItem() const override
    {
        return const_cast<KDevCategoryItem*>( this );
    }

    QList<KDevFileItem*> fileList() const;
    KDevFileItem* file( const QUrl &url ) const;
};

class KDevFileItem: public KDevDocumentItem
{
public:
    explicit KDevFileItem( const QUrl &url );
    virtual ~KDevFileItem();

    virtual KDevFileItem *fileItem() const override
    {
        return const_cast<KDevFileItem*>( this );
    }
};

class KDevDocumentModel: public QStandardItemModel
{
    Q_OBJECT

public:
    KDevDocumentModel( QObject *parent = 0 );
    virtual ~KDevDocumentModel();

    QList<KDevCategoryItem*> categoryList() const;
    KDevCategoryItem* category( const QString& category ) const;
};

#endif // KDEVPLATFORM_PLUGIN_KDEVDOCUMENTMODEL_H
