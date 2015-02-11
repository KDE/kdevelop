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

#ifndef KDEVPLATFORM_PLUGIN_QUICKOPENMODEL_H
#define KDEVPLATFORM_PLUGIN_QUICKOPENMODEL_H

#include <QMultiMap>
#include <QString>
#include <QAbstractItemModel>
#include <QSet>

#include <serialization/indexedstring.h>

#include <language/interfaces/quickopendataprovider.h>
#include "expandingtree/expandingwidgetmodel.h"

class QuickOpenModel : public ExpandingWidgetModel {
  Q_OBJECT
  public:
    QuickOpenModel( QWidget* parent );
    
    void registerProvider( const QStringList& scopes, const QStringList& type, KDevelop::QuickOpenDataProviderBase* provider );

    /**
     * Remove provider.
     * @param provider The provider to remove
     * @return Whether a provider was removed. If false, the provider was not attached.
     * */
    bool removeProvider( KDevelop::QuickOpenDataProviderBase* provider );

    ///Returns a list of all scopes that a registered through some providers
    QStringList allScopes() const;
    ///Returns a list of all types that a registered through some providers
    QStringList allTypes() const;

    /**
     * @param items The list of items that should be used.
     * @param scopesThe list of scopes that should be used.
     
    * When this is called, the state is restart()ed.
     * */
    void enableProviders( const QStringList& items, const QStringList& scopes );
    
    ///Reset all providers, unexpand everything, empty caches.
    void restart(bool keepFilterText = false);

    QModelIndex index( int, int, const QModelIndex& parent ) const override;
    QModelIndex parent( const QModelIndex& ) const override;
    int rowCount( const QModelIndex& ) const override;
    int unfilteredRowCount() const;
    int columnCount() const;
    int columnCount( const QModelIndex& ) const override;
    QVariant data( const QModelIndex&, int ) const override;

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
    
    virtual QTreeView* treeView() const override;

    virtual QSet<KDevelop::IndexedString> fileSet() const;

    ///This value will be added to the height of all created expanding-widgets
    void setExpandingWidgetHeightIncrease(int pixels);

  public slots:
    void textChanged( const QString& str );
  private slots:
    void destroyed( QObject* obj );
    void resetTimer();
    void restart_internal( bool keepFilterText );
  
  private:
    virtual bool indexIsItem(const QModelIndex& index) const override;
    
    virtual int contextMatchQuality(const QModelIndex & index) const override;

    KDevelop::QuickOpenDataPointer getItem( int row, bool noReset = false ) const;
    
    typedef QHash<int, KDevelop::QuickOpenDataPointer> DataList;
    mutable DataList m_cachedData;

    QTreeView* m_treeView;
    QTimer* m_resetTimer;

    struct ProviderEntry {
      ProviderEntry() : enabled(false) {
      }
      bool enabled;
      QSet<QString> scopes;
      QSet<QString> types;
      KDevelop::QuickOpenDataProviderBase* provider;
    };
    
  //typedef QMultiMap< QString, ProviderEntry > ProviderMap;
    typedef QList<ProviderEntry> ProviderList;
    QList<ProviderEntry> m_providers;
    QString m_filterText;
    int m_expandingWidgetHeightIncrease;
    mutable int m_resetBehindRow;

    QSet<QString> m_enabledItems;
    QSet<QString> m_enabledScopes;
};

#endif

