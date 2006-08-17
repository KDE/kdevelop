/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdevprojectmodel.h"

#include <kmimetype.h>
#include <kiconloader.h>
#include <kio/global.h>

QIcon KDevProjectFolderItem::icon() const
{
    return KIO::pixmapForURL(url(), 0, K3Icon::Small);
}

QIcon KDevProjectFileItem::icon() const
{
    return KIO::pixmapForURL(url(), 0, K3Icon::Small);
}

QList<KDevProjectFolderItem*> KDevProjectItem::folderList() const
{
  QList<KDevProjectFolderItem*> lst;
  for (int i=0; i<itemCount(); ++i)
  {
    KDevProjectFolderItem *item = itemAt(i)->folder();
    if ( item )
        lst.append(item);
  }

  return lst;
}

QList<KDevProjectTargetItem*> KDevProjectItem::targetList() const
{
  QList<KDevProjectTargetItem*> lst;
  for (int i=0; i<itemCount(); ++i)
  {
    KDevProjectTargetItem *item = itemAt( i )->target();
    if ( item )
      lst.append(item);
  }

  return lst;
}

QList<KDevProjectFileItem*> KDevProjectItem::fileList() const
{
  QList<KDevProjectFileItem*> lst;
  for (int i=0; i<itemCount(); ++i)
  {
    KDevProjectFileItem* item = itemAt( i )->file();
    if ( item )
      lst.append(item);
  }

  return lst;
}

KDevProjectItem *KDevProjectItem::itemAt(int index) const
{
  return dynamic_cast<KDevProjectItem*>(KDevItemCollection::itemAt(index)); // ### no dynamic_cast
}

KDevProjectModel::KDevProjectModel(QObject *parent)
  : KDevItemModel(parent)
{
}

KDevProjectModel::~KDevProjectModel()
{
}

KDevProjectItem *KDevProjectModel::item(const QModelIndex &index) const
{
  return reinterpret_cast<KDevProjectItem*>(index.internalPointer());
}

KDevProjectFolderItem::KDevProjectFolderItem( const KUrl & dir, KDevItemGroup * parent )
  : KDevProjectItem(dir.directory(), parent)
  , m_url(dir)
{
  setName( dir.fileName() );
}

const KUrl& KDevProjectFolderItem::url( ) const
{
  return m_url;
}

void KDevProjectFolderItem::setUrl( const KUrl& url )
{
  m_url = url;
  setName( url.fileName() );
}

KDevProjectFileItem::KDevProjectFileItem( const KUrl & file, KDevItemGroup * parent )
  : KDevProjectItem(file.fileName(), parent)
  , m_url(file)
{
}

const KUrl & KDevProjectFileItem::url( ) const
{
  return m_url;
}

void KDevProjectFileItem::setUrl( const KUrl& url )
{
  m_url = url;
}

#include "kdevprojectmodel.moc"
//kate: space-indent on; indent-width 2; indent-mode cstyle; replace-tabs on;

