/*
    SPDX-FileCopyrightText: 2005 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdevdocumentmodel.h"

#include <QIcon>

KDevDocumentItem::KDevDocumentItem( const QString &name )
    : QStandardItem(name)
{
}

KDevDocumentItem::~KDevDocumentItem()
{}

const QUrl KDevDocumentItem::url() const
{
    return m_url;
}

void KDevDocumentItem::setUrl(const QUrl& url)
{
    m_url = url;
}

QVariant KDevDocumentItem::data(int role) const
{
    if (role == UrlRole) {
        return m_url;
    }
    return QStandardItem::data(role);
}

KDevCategoryItem::KDevCategoryItem( const QString &name )
    : KDevDocumentItem( name )
{
    setFlags(Qt::ItemIsEnabled);
    setToolTip( name );
    setIcon( QIcon::fromTheme( QStringLiteral( "folder") ) );
}

KDevCategoryItem::~KDevCategoryItem()
{}

QList<KDevFileItem*> KDevCategoryItem::fileList() const
{
    QList<KDevFileItem*> lst;

    for ( int i = 0; i < rowCount(); ++i )
    {
        if (KDevFileItem* item = static_cast<KDevDocumentItem*>(child(i))->fileItem())
            lst.append( item );
    }

    return lst;
}

KDevFileItem* KDevCategoryItem::file( const QUrl &url ) const
{
    const auto fileList = this->fileList();
    for (KDevFileItem* item : fileList) {
        if ( item->url() == url )
            return item;
    }

    return nullptr;
}

KDevFileItem::KDevFileItem( const QUrl &url )
        : KDevDocumentItem( url.fileName() )
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    setUrl( url );
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

QList<KDevCategoryItem*> KDevDocumentModel::categoryList() const
{
    QList<KDevCategoryItem*> lst;
    for ( int i = 0; i < rowCount() ; ++i )
    {
        if (KDevCategoryItem* categoryitem = static_cast<KDevDocumentItem*>(item(i))->categoryItem()) {
            lst.append( categoryitem );
        }
    }

    return lst;
}

KDevCategoryItem* KDevDocumentModel::category( const QString& category ) const
{
    const auto categoryList = this->categoryList();
    for (KDevCategoryItem* item : categoryList) {
        if ( item->toolTip() == category )
            return item;
    }

    return nullptr;
}

#include "moc_kdevdocumentmodel.cpp"
