/* This file is part of the KDE libraries
   Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef QUICKOPENMODEL_H
#define QUICKOPENMODEL_H

#include <QMultiMap>
#include <QString>
#include <QAbstractItemModel>

#include "iquickopendataprovider.h"

class QuickOpenModel : public QAbstractItemModel {
  Q_OBJECT;
  public:
    /**
     * Registers a new provider under a specified name.
     * There may be multiple models with the same name.
     * @param name Name of the provider, Example: "Files". The name will be shown in the GUI, so should be translated.
     * @param provider The provider. It does not need to be explicitly removed before its destruction.
     * */
    void registerProvider( const QString& name, KDevelop::IQuickOpenDataProvider* provider );

    /**
     * Remove provider.
     * @param provider The provider to remove
     * @return Whether a provider was removed. If false, the provider was not attached.
     * */
    bool removeProvider( KDevelop::IQuickOpenDataProvider* provider );

    void restart();

    QModelIndex index( int, int, const QModelIndex& parent ) const;
    QModelIndex parent( const QModelIndex& ) const;
    int rowCount( const QModelIndex& ) const;
    int columnCount( const QModelIndex& ) const;
    QVariant data( const QModelIndex&, int ) const;
  private slots:
    void destroyed( QObject* obj );
  
  private:
    typedef QList<KDevelop::QuickOpenDataPointer> DataList;
    mutable DataList m_cachedData;
    
    typedef QMultiMap< QString, KDevelop::IQuickOpenDataProvider* > ProviderMap;
    ProviderMap m_providers;
};

#endif

