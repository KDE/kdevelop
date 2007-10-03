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
#include "iquickopendataprovider.h"

void QuickOpenModel::registerProvider( const QString& name, KDevelop::IQuickOpenDataProvider* provider )
{
  m_providers.insert( name, provider );
  connect( provider, SIGNAL( destroyed(QObject*) ), this, SLOT( destroyed( QObject* ) ) );
}

bool QuickOpenModel::removeProvider( KDevelop::IQuickOpenDataProvider* provider )
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

void QuickOpenModel::restart()
{
  foreach( KDevelop::IQuickOpenDataProvider* provider, m_providers )
    provider->clearFilterText();
  
  reset();
  m_cachedData.clear();
}

void QuickOpenModel::destroyed( QObject* obj )
{
  removeProvider( dynamic_cast<KDevelop::IQuickOpenDataProvider*>(obj) );
}

QModelIndex QuickOpenModel::index( int row, int column, const QModelIndex& parent) const
{
  return createIndex( row, column );
}

QModelIndex QuickOpenModel::parent( const QModelIndex& ) const
{
  return QModelIndex();
}

int QuickOpenModel::rowCount( const QModelIndex& ) const
{
  int count = 0;
  foreach( KDevelop::IQuickOpenDataProvider* provider, m_providers )
    count += provider->itemCount();

  return count;
}

int QuickOpenModel::columnCount( const QModelIndex& ) const
{
  return 1;
}

QVariant QuickOpenModel::data( const QModelIndex& index, int role ) const
{
  return QVariant();
}
