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

#include <ktexteditor/codecompletionmodel.h>
#include <kdebug.h>

using namespace KDevelop;

QuickOpenModel::QuickOpenModel( QWidget* parent ) : ExpandingWidgetModel( parent )
{
}

void QuickOpenModel::registerProvider( const QString& scope, const QString& type, KDevelop::QuickOpenDataProviderBase* provider )
{
  m_providers.insert( type, provider );
  connect( provider, SIGNAL( destroyed(QObject*) ), this, SLOT( destroyed( QObject* ) ) );
}

bool QuickOpenModel::removeProvider( KDevelop::QuickOpenDataProviderBase* provider )
{
  for( ProviderMap::iterator it = m_providers.begin(); it != m_providers.end(); ++it ) {
    if( *it == provider ) {
      m_providers.erase( it );
      disconnect( provider, SIGNAL( destroyed(QObject*) ), this, SLOT( destroyed( QObject* ) ) );
      return true;
    }
  }
  return false;
}

void QuickOpenModel::textChanged( const QString& str )
{
  foreach( KDevelop::QuickOpenDataProviderBase* provider, m_providers )
    provider->setFilterText( str );
  
  m_cachedData.clear();
  clearExpanding();
  reset();
}

void QuickOpenModel::restart()
{
  foreach( KDevelop::QuickOpenDataProviderBase* provider, m_providers )
    provider->reset();
  
  m_cachedData.clear();
  clearExpanding();

  reset();
}

void QuickOpenModel::destroyed( QObject* obj )
{
  removeProvider( dynamic_cast<KDevelop::QuickOpenDataProviderBase*>(obj) );
}

QModelIndex QuickOpenModel::index( int row, int column, const QModelIndex& parent) const
{
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
  foreach( KDevelop::QuickOpenDataProviderBase* provider, m_providers )
    count += provider->itemCount();

  return count;
}

int QuickOpenModel::columnCount( const QModelIndex& ) const
{
  return 1;
}

QVariant QuickOpenModel::data( const QModelIndex& index, int role ) const
{
  QuickOpenDataPointer d = getItem( index.row() );

  if( !d )
    return QVariant();

  switch( role ) {
    case Qt::DisplayRole:
      return d->text();
    case KTextEditor::CodeCompletionModel::ItemSelected:
      return d->htmlDescription();
  }
  
  return ExpandingWidgetModel::data( index, role );
}

QuickOpenDataPointer QuickOpenModel::getItem( int row ) const {
  ///@todo mix all the models alphabetically here. For now, they are simply ordered.

  foreach( KDevelop::QuickOpenDataProviderBase* provider, m_providers ) {
    if( row < provider->itemCount() )
    {
      QList<QuickOpenDataPointer> items = provider->data( row, row+1 );
      
      if( items.isEmpty() )
      {
        kWarning() << "Provider returned no item";
        return QuickOpenDataPointer();
      } else {
        return items.first();
      }
    } else {
      row -= provider->itemCount();
    }
  }

  kWarning() << "No item for row " <<  row;

  return QuickOpenDataPointer();
}

QTreeView* QuickOpenModel::treeView() const {
  return m_treeView;
}

bool QuickOpenModel::indexIsItem(const QModelIndex& index) const {
  return true;
}

void QuickOpenModel::setTreeView( QTreeView* view ) {
  m_treeView = view;
}

int QuickOpenModel::contextMatchQuality(const QModelIndex & index) const {
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
