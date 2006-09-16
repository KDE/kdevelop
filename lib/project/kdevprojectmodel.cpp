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

QList<KDevProjectFolderItem*> KDevProjectItem::folderList() const
{
  QList<KDevProjectFolderItem*> lst;
  for (int i = 0; i < rowCount(); ++i)
  {
    QStandardItem* item = child(i);
    if ( item->type() == Folder || item->type() == BuildFolder )
    {
      KDevProjectFolderItem *kdevitem = dynamic_cast<KDevProjectFolderItem*>( item );
      if ( kdevitem )
        lst.append(kdevitem);
    }
  }

  return lst;
}

QList<KDevProjectTargetItem*> KDevProjectItem::targetList() const
{
  QList<KDevProjectTargetItem*> lst;
  for (int i = 0; i < rowCount(); ++i)
  {
    QStandardItem* item = child(i);
    if ( item->type() == Target )
    {
      KDevProjectTargetItem *kdevitem = dynamic_cast<KDevProjectTargetItem*>( item );
      if ( kdevitem )
        lst.append(kdevitem);
    }
  }

  return lst;
}

QList<KDevProjectFileItem*> KDevProjectItem::fileList() const
{
  QList<KDevProjectFileItem*> lst;
  for (int i = 0; i < rowCount(); ++i)
  {
    QStandardItem* item = child(i);
    if ( item->type() == File )
    {
      KDevProjectFileItem *kdevitem = dynamic_cast<KDevProjectFileItem*>( item );
      if ( kdevitem )
        lst.append(kdevitem);
    }

  }

  return lst;
}

KDevProjectModel::KDevProjectModel(QObject *parent)
  : QStandardItemModel(parent)
{
}

KDevProjectModel::~KDevProjectModel()
{
}


KDevProjectItem *KDevProjectModel::item(const QModelIndex &index) const
{
  return reinterpret_cast<KDevProjectItem*>(itemFromIndex(index));
}

void KDevProjectBuildFolderItem::setIncludeDirectories( const KUrl::List& dirList )
{
  m_includeDirs = dirList;
}

const KUrl::List& KDevProjectBuildFolderItem::includeDirectories() const
{
  return m_includeDirs;
}

const QHash<QString, QString>& KDevProjectBuildFolderItem::environment() const
{
  return m_env;
}


KDevProjectFolderItem::KDevProjectFolderItem( const KUrl & dir, QStandardItem * parent )
  : KDevProjectItem(dir.directory(), parent)
  , m_url(dir)
{
  setText( dir.fileName() );
  setIcon( KIO::pixmapForUrl(url(), 0, K3Icon::Small) );
}

const KUrl& KDevProjectFolderItem::url( ) const
{
  return m_url;
}

void KDevProjectFolderItem::setUrl( const KUrl& url )
{
  m_url = url;
  setText( url.fileName() );
}

KDevProjectFileItem::KDevProjectFileItem( const KUrl & file, QStandardItem * parent )
  : KDevProjectItem(file.fileName(), parent)
  , m_url(file)
{
  setIcon( KIO::pixmapForUrl(url(), 0, K3Icon::Small) );
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

