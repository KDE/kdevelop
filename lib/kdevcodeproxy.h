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
#ifndef KDEVCODEPROXY_H
#define KDEVCODEPROXY_H

#include <QSortFilterProxyModel>

#include <kurl.h>

#include "kdevcodemodel.h"

typedef QList<KDevCodeModel*> CodeModelList;
typedef QList< QPair<KUrl, KDevCodeModel* > > CodeModelCache;

class KDevCodeAggregate;

/**
 * \short A model which assembles multiple CodeModel%s into a global model and filters them as requested.
 *
 * This proxy is used to combine together the results of parsing multiple source
 * files into one coherent model.  It allows you to add a KDevCodeModel for each
 * url that the project encompasses.  These models are then joined together in
 * the manner specified by mode(), and filtered if requested.
 */
class KDEVPLATFORM_EXPORT KDevCodeProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum Mode
    {
        Normalize = 0,
        Aggregate = 1,
        Document = 2
    };

    KDevCodeProxy( QObject *parent = 0 );
    virtual ~KDevCodeProxy();

    CodeModelList codeModels() const;
    KDevCodeModel *codeModel( const KUrl &url ) const;
    void insertModel( const KUrl &url,
                      KDevCodeModel *model );
    void insertModelCache( const CodeModelCache &list );
    void deleteModel( const KUrl &url );

    KDevCodeProxy::Mode mode() const;
    void setMode( KDevCodeProxy::Mode mode = Normalize );
    void setFilterDocument( const KUrl &url = KUrl() );

    QMap<QString, int> kindFilterList() const;

    /** Language parts should inherit this class, publish the kind filter list,
        implement this method and filter accordingly. The default value does
        nothing. Note: The filter should be exclusive.*/
    virtual void setKindFilter( int kind = 0 ) = 0;

    KDevCodeItem *proxyToItem( const QModelIndex &proxy_index ) const;
    KDevCodeItem *sourceToItem( const QModelIndex &source_index ) const;

protected:
    void setKindFilterList( const QMap<QString, int> list );

private:
    virtual void setSourceModel( QAbstractItemModel *sourceModel );
    KDevCodeAggregate *codeAggregate() const;

private slots:
    void forceReset();
    void forceClear();

private:
    QMap<QString, int> m_kindFilterList;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
