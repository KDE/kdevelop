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

#include "quickopenmodel.h"

#include <QTreeView>

#include <ktexteditor/codecompletionmodel.h>
#include <kdebug.h>

using namespace KDevelop;

QuickOpenModel::QuickOpenModel( QWidget* parent ) : ExpandingWidgetModel( parent )
{
  
}

QStringList QuickOpenModel::allScopes() const
{
  QStringList scopes;
  foreach( const ProviderEntry& provider, m_providers )
    foreach( const QString& scope, provider.scopes )
      if( !scopes.contains( scope ) )
        scopes << scope;
  
  return scopes;
}

QStringList QuickOpenModel::allTypes() const
{
  QStringList types;
  foreach( const ProviderEntry& provider, m_providers )
    if( !types.contains( provider.type ) )
      types << provider.type;
  
  return types;
}

void QuickOpenModel::registerProvider( const QStringList& scopes, const QString& type, KDevelop::QuickOpenDataProviderBase* provider )
{
  ProviderEntry e;
  e.scopes = QSet<QString>::fromList(scopes);
  e.type = type;
  e.provider = provider;
  
  m_providers.insert( type, e );
  
  connect( provider, SIGNAL( destroyed(QObject*) ), this, SLOT( destroyed( QObject* ) ) );

  restart();
}

bool QuickOpenModel::removeProvider( KDevelop::QuickOpenDataProviderBase* provider )
{
  restart();
  for( ProviderMap::iterator it = m_providers.begin(); it != m_providers.end(); ++it ) {
    if( (*it).provider == provider ) {
      m_providers.erase( it );
      disconnect( provider, SIGNAL( destroyed(QObject*) ), this, SLOT( destroyed( QObject* ) ) );
      return true;
    }
  }
  return false;
}

void QuickOpenModel::enableProviders( const QStringList& items, const QStringList& _scopes )
{
  QSet<QString> scopes = QSet<QString>::fromList( _scopes );
  kDebug() << "params " << items << " " << scopes;
  for( ProviderMap::iterator it = m_providers.begin(); it != m_providers.end(); ++it ) {
    if( ( scopes.isEmpty() || !scopes.intersect( (*it).scopes ).isEmpty() ) && ( items.contains( (*it).type ) || items.isEmpty() ) ) {
      kDebug() << "enabling " << (*it).type << " " << (*it).scopes;
      (*it).enabled = true;
      (*it).provider->enableScopes( _scopes );
    } else {
      kDebug() << "disabling " << (*it).type << " " << (*it).scopes;
      (*it).enabled = false;
    }
  }
  
  restart();
}

void QuickOpenModel::textChanged( const QString& str )
{
  foreach( const ProviderEntry& provider, m_providers )
    if( provider.enabled )
      provider.provider->setFilterText( str );
  
  m_cachedData.clear();
  clearExpanding();
  reset();
}

void QuickOpenModel::restart()
{
  foreach( const ProviderEntry& provider, m_providers )
    if( provider.enabled )
      provider.provider->reset();
  
  m_cachedData.clear();
  clearExpanding();

  reset();
}

void QuickOpenModel::destroyed( QObject* obj )
{
  removeProvider( dynamic_cast<KDevelop::QuickOpenDataProviderBase*>(obj) );
}

QModelIndex QuickOpenModel::index( int row, int column, const QModelIndex& /*parent*/) const
{
  if( column >= columnCount() )
    return QModelIndex();
  return createIndex( row, column );
}

QModelIndex QuickOpenModel::parent( const QModelIndex& ) const
{
  return QModelIndex();
}

int QuickOpenModel::rowCount( const QModelIndex& i ) const
{
  if( i.isValid() )
    return 0;
  
  int count = 0;
  foreach( const ProviderEntry& provider, m_providers )
    if( provider.enabled )
      count += provider.provider->itemCount();

  return count;
}

int QuickOpenModel::columnCount() const
{
  return 2;
}

int QuickOpenModel::columnCount( const QModelIndex& index ) const
{
  if( index.parent().isValid() )
    return 0;
  else {
    return columnCount();
  }
}

QVariant QuickOpenModel::data( const QModelIndex& index, int role ) const
{
  QuickOpenDataPointer d = getItem( index.row() );

  if( !d )
    return QVariant();

  switch( role ) {
    case KTextEditor::CodeCompletionModel::ItemSelected:
      return d->htmlDescription();

    case KTextEditor::CodeCompletionModel::IsExpandable:
      return d->isExpandable();

    case KTextEditor::CodeCompletionModel::ExpandingWidget: {
      QVariant v;
      QWidget* w =  d->expandingWidget();
      v.setValue<QWidget*>(w);
      return v;
    }
  }
  
  if( index.column() == 1 )
  {
    switch( role ) {
      case Qt::DecorationRole:
        return d->icon();
      
      case Qt::DisplayRole:
        return d->text();
    }
  } else if( index.column() == 0 )
  {
    switch( role ) {
      case Qt::DecorationRole:
      {
        if( isExpandable(index) ) {
          //Show the expanded/unexpanded handles
          cacheIcons();
          if( isExpanded(index) ) {
            return m_expandedIcon;
          } else {
            return m_collapsedIcon;
          }
        }
      }
    }
  }
  
  return ExpandingWidgetModel::data( index, role );
}

QuickOpenDataPointer QuickOpenModel::getItem( int row ) const {
  ///@todo mix all the models alphabetically here. For now, they are simply ordered.

  if( m_cachedData.contains( row ) )
    return m_cachedData[row];
  
  foreach( const ProviderEntry& provider, m_providers ) {
    if( !provider.enabled )
      continue;
    if( (uint)row < provider.provider->itemCount() )
    {
      QList<QuickOpenDataPointer> items = provider.provider->data( row, row+1 );
      
      if( items.isEmpty() )
      {
        kWarning() << "Provider returned no item";
        return QuickOpenDataPointer();
      } else {
        m_cachedData[row] = items.first();
        return items.first();
      }
    } else {
      row -= provider.provider->itemCount();
    }
  }

  kWarning() << "No item for row " <<  row;

  return QuickOpenDataPointer();
}

QTreeView* QuickOpenModel::treeView() const {
  return m_treeView;
}

bool QuickOpenModel::indexIsItem(const QModelIndex& /*index*/) const {
  return true;
}

void QuickOpenModel::setTreeView( QTreeView* view ) {
  m_treeView = view;
}

int QuickOpenModel::contextMatchQuality(const QModelIndex & /*index*/) const {
  return -1;
}

bool QuickOpenModel::execute( const QModelIndex& index, QString& filterText )
{
  kDebug() << "executing model";
  if( !index.isValid() ) {
    kWarning() << "Invalid index executed";
    return false;
  }
  
  QuickOpenDataPointer item = getItem( index.row() );

  if( item ) {
    return item->execute( filterText );
  }else{
    kWarning() << "Got no item for row " << index.row() << " ";
  }

  return false;
}
