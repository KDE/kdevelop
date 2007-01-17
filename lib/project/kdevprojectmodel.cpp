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

namespace Koncrete
{

QList<ProjectFolderItem*> ProjectItem::folderList() const
{
  QList<ProjectFolderItem*> lst;
  for (int i = 0; i < rowCount(); ++i)
  {
    QStandardItem* item = child(i);
    if ( item->type() == Folder || item->type() == BuildFolder )
    {
      ProjectFolderItem *kdevitem = dynamic_cast<ProjectFolderItem*>( item );
      if ( kdevitem )
        lst.append(kdevitem);
    }
  }

  return lst;
}

QList<ProjectTargetItem*> ProjectItem::targetList() const
{
  QList<ProjectTargetItem*> lst;
  for (int i = 0; i < rowCount(); ++i)
  {
    QStandardItem* item = child(i);
    if ( item->type() == Target )
    {
      ProjectTargetItem *kdevitem = dynamic_cast<ProjectTargetItem*>( item );
      if ( kdevitem )
        lst.append(kdevitem);
    }
  }

  return lst;
}

QList<ProjectFileItem*> ProjectItem::fileList() const
{
  QList<ProjectFileItem*> lst;
  for (int i = 0; i < rowCount(); ++i)
  {
    QStandardItem* item = child(i);
    if ( item->type() == File )
    {
      ProjectFileItem *kdevitem = dynamic_cast<ProjectFileItem*>( item );
      if ( kdevitem )
        lst.append(kdevitem);
    }

  }

  return lst;
}

ProjectModel::ProjectModel(QObject *parent)
  : QStandardItemModel(parent)
{
}

ProjectModel::~ProjectModel()
{
}


ProjectItem *ProjectModel::item(const QModelIndex &index) const
{
  return reinterpret_cast<ProjectItem*>(itemFromIndex(index));
}

void ProjectModel::resetModel()
{
  reset();
}

void ProjectBuildFolderItem::setIncludeDirectories( const KUrl::List& dirList )
{
  m_includeDirs = dirList;
}

const KUrl::List& ProjectBuildFolderItem::includeDirectories() const
{
  return m_includeDirs;
}

const QHash<QString, QString>& ProjectBuildFolderItem::environment() const
{
  return m_env;
}


ProjectFolderItem::ProjectFolderItem( const KUrl & dir, QStandardItem * parent )
  : ProjectItem(dir.directory(), parent)
  , m_url(dir)
{
  setText( dir.fileName() );
  setIcon( KIO::pixmapForUrl(url(), 0, K3Icon::Small) );
}

const KUrl& ProjectFolderItem::url( ) const
{
  return m_url;
}

void ProjectFolderItem::setUrl( const KUrl& url )
{
  m_url = url;
  setText( url.fileName() );
}

ProjectFileItem::ProjectFileItem( const KUrl & file, QStandardItem * parent )
  : ProjectItem(file.fileName(), parent)
  , m_url(file)
{
  setIcon( KIO::pixmapForUrl(url(), 0, K3Icon::Small) );
}

const KUrl & ProjectFileItem::url( ) const
{
  return m_url;
}

void ProjectFileItem::setUrl( const KUrl& url )
{
  m_url = url;
}

}

#include "kdevprojectmodel.moc"
//kate: space-indent on; indent-width 2; indent-mode cstyle; replace-tabs on;

