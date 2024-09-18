/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVDOCUMENTMODEL_H
#define KDEVPLATFORM_PLUGIN_KDEVDOCUMENTMODEL_H

#include <QStandardItem>
#include <QStandardItemModel>
#include <QUrl>

class KDevDocumentItem;
class KDevCategoryItem;
class KDevFileItem;

class KDevDocumentItem: public QStandardItem
{
public:
    explicit KDevDocumentItem( const QString &name );
    ~KDevDocumentItem() override;

    virtual KDevCategoryItem *categoryItem() const
    {
        return nullptr;
    }
    virtual KDevFileItem *fileItem() const
    {
        return nullptr;
    }

    const QUrl url() const;
    void setUrl(const QUrl &url);

    QVariant data(int role) const override;

    enum Roles {
        UrlRole = Qt::UserRole + 1
    };

private:
    QUrl m_url;
};

class KDevCategoryItem: public KDevDocumentItem
{
public:
    explicit KDevCategoryItem( const QString &name );
    ~KDevCategoryItem() override;

    KDevCategoryItem *categoryItem() const override
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
    ~KDevFileItem() override;

    KDevFileItem *fileItem() const override
    {
        return const_cast<KDevFileItem*>( this );
    }
};

class KDevDocumentModel: public QStandardItemModel
{
    Q_OBJECT

public:
    explicit KDevDocumentModel( QObject *parent = nullptr );
    ~KDevDocumentModel() override;

    QList<KDevCategoryItem*> categoryList() const;
    KDevCategoryItem* category( const QString& category ) const;
};

#endif // KDEVPLATFORM_PLUGIN_KDEVDOCUMENTMODEL_H
