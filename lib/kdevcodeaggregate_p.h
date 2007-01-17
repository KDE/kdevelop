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

namespace Koncrete
{

class CodeAggregate : public QAbstractProxyModel
{
    Q_OBJECT
public:
    CodeAggregate( QObject *parent = 0 );
    virtual ~CodeAggregate();

    CodeModelList codeModels() const;
    CodeModel *codeModel( const KUrl &url ) const;
    void insertModel( const KUrl &url,
                      CodeModel *model );
    void insertModelCache( const CodeModelCache &list );
    void deleteModel( const KUrl &url );

    CodeProxy::Mode mode() const;
    void setMode( CodeProxy::Mode mode = CodeProxy::Normalize );
    void setFilterDocument( const KUrl &url = KUrl() );

    CodeItem *proxyToItem( const QModelIndex &proxy_index ) const;
    CodeItem *sourceToItem( const QModelIndex &source_index ) const;

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
    int positionOf( CodeItem *item ) const;
    const CodeModel* model( const QModelIndex &index ) const;
    void mapCodeModels();
    void createModelMap();
    void normalizeModels();
    bool normalizeItem( CodeItem *item );

private:
    bool m_updatingMap;

    CodeProxy::Mode m_mode;
    KUrl m_filter;
    QMap<KUrl, CodeModel*> m_codeModels;

    struct ModelMap
    {
        CodeModel* source_model;
        QModelIndex source_index;
    };
    QHash<int, ModelMap*> m_modelHash;
    QHash<int, CodeItem*> m_normalizeRoot;
    QHash<uint, CodeItem*> m_normalizeHash;
    QHash<CodeItem*, int> m_normalizeRowCount;
    QHash<CodeItem*, CodeItem*> m_substituteHash;
    QHash<CodeItem*, QHash<int, CodeItem*> > m_stepchildHash;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
