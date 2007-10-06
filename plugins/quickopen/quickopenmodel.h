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

#include "quickopendataprovider.h"
#include "expandingtree/expandingwidgetmodel.h"

class QuickOpenModel : public ExpandingWidgetModel {
  Q_OBJECT
  public:
    QuickOpenModel( QWidget* parent );
    
    void registerProvider( const QString& scope, const QString& type, KDevelop::QuickOpenDataProviderBase* provider );

    /**
     * Remove provider.
     * @param provider The provider to remove
     * @return Whether a provider was removed. If false, the provider was not attached.
     * */
    bool removeProvider( KDevelop::QuickOpenDataProviderBase* provider );

    void restart();

    QModelIndex index( int, int, const QModelIndex& parent ) const;
    QModelIndex parent( const QModelIndex& ) const;
    int rowCount( const QModelIndex& ) const;
    int columnCount( const QModelIndex& ) const;
    QVariant data( const QModelIndex&, int ) const;

    /**
     * Tries to execute the item currently selected.
     * Returns true if the quickopen-dialog should be closed.
     * @param filterText Should be the current content of the filter line-edit.
     *
     * If this returns false, and filterText was changed, the change must be put
     * into the line-edit. That way items may execute by changing the content
     * of the line-edit.
     * */
    bool execute( const QModelIndex& index, QString& filterText );
    
    //The expandingwidgetmodel needs access to the tree-view
    void setTreeView( QTreeView* view );
  public slots:
    void textChanged( const QString& str );
  private slots:
    void destroyed( QObject* obj );
  
  private:
    virtual QTreeView* treeView() const;
    
    virtual bool indexIsItem(const QModelIndex& index) const;
    
    virtual int contextMatchQuality(const QModelIndex & index) const;

    KDevelop::QuickOpenDataPointer getItem( int row ) const;
    
    typedef QList<KDevelop::QuickOpenDataPointer> DataList;
    mutable DataList m_cachedData;

    QTreeView* m_treeView;
    
    typedef QMultiMap< QString, KDevelop::QuickOpenDataProviderBase* > ProviderMap;
    ProviderMap m_providers;
};

#endif

