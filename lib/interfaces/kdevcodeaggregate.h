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
#ifndef KDEVCODEAGGREGATE_H
#define KDEVCODEAGGREGATE_H

#include <QAbstractProxyModel>

#include "kdevcodeproxy.h"

class KDevCodeAggregate : public QAbstractProxyModel
{
    Q_OBJECT
public:
    KDevCodeAggregate( QObject *parent = 0 );
    virtual ~KDevCodeAggregate();

    CodeModelList codeModels() const;
    KDevCodeModel *codeModel( const KUrl &url ) const;
    void insertModel( const KUrl &url,
                      KDevCodeModel *model );
    void deleteModel( const KUrl &url );

    void setMode( KDevCodeProxy::Mode mode = KDevCodeProxy::Normalize );
    void setFilterDocument( const KUrl &url = KUrl() );

    KDevCodeItem *proxyToItem( const QModelIndex &proxy_index ) const;
    KDevCodeItem *sourceToItem( const QModelIndex &source_index ) const;

    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &child ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QModelIndex mapToSource( const QModelIndex &proxyIndex ) const;
    QModelIndex mapFromSource( const QModelIndex &sourceIndex ) const;

private:
    void setSourceModel( QAbstractItemModel * sourceModel );
    QAbstractItemModel *sourceModel() const;

private:
    int positionOf( KDevCodeItem *item ) const;
    const KDevCodeModel* model( const QModelIndex &index ) const;
    void mapCodeModels();
    void createModelMap();
    void normalizeModels();
    bool normalizeItem( KDevCodeItem *item );

private:
    bool m_updatingMap;

    KDevCodeProxy::Mode m_mode;
    KUrl m_filter;
    QMap<KUrl, KDevCodeModel*> m_codeModels;

    struct ModelMap
    {
        KDevCodeModel* source_model;
        QModelIndex source_index;
    };
    QHash<int, ModelMap*> m_modelHash;
    QHash<int, KDevCodeItem*> m_normalizeRoot;
    QHash<uint, KDevCodeItem*> m_normalizeHash;
    QHash<KDevCodeItem*, int> m_normalizeRowCount;
    QHash<KDevCodeItem*, KDevCodeItem*> m_substituteHash;
    QHash<KDevCodeItem*, QHash<int, KDevCodeItem*> > m_stepchildHash;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
