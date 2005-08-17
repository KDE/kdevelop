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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KDEVPROJECTMODEL_H
#define KDEVPROJECTMODEL_H

#include "kdevitemmodel.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

class KDevProjectFolderItem;
class KDevProjectFileItem;
class KDevProjectTargetItem;

class KDevProjectItem: public KDevItemCollection
{
public:
  KDevProjectItem(const QString &name, KDevItemGroup *parent = 0)
    : KDevItemCollection(name, parent) {}

  virtual KDevProjectItem *itemAt(int index) const;

  virtual KDevProjectFolderItem *folder() const { return 0; }
  virtual KDevProjectTargetItem *target() const { return 0; }
  virtual KDevProjectFileItem *file() const { return 0; }

  QList<KDevProjectFolderItem*> folderList() const;
  QList<KDevProjectTargetItem*> targetList() const;
  QList<KDevProjectFileItem*> fileList() const;
};

class KDevProjectFolderItem: public KDevProjectItem
{
public:
  KDevProjectFolderItem(const QDir &dir, KDevItemGroup *parent = 0)
    : KDevProjectItem(dir.dirName(), parent), m_directory(dir) {}

  virtual KDevProjectFolderItem *folder() const
  { return const_cast<KDevProjectFolderItem*>(this); }

  QDir directory() const { return m_directory; }
  virtual QIcon icon() const;

private:
  QDir m_directory;
};

class KDevProjectTargetItem: public KDevProjectItem
{
public:
  KDevProjectTargetItem(const QString &name, KDevItemGroup *parent = 0)
    : KDevProjectItem(name, parent) {}

  virtual KDevProjectTargetItem *target() const
  { return const_cast<KDevProjectTargetItem*>(this); }
};

class KDevProjectFileItem: public KDevProjectItem
{
public:
  KDevProjectFileItem(const QFileInfo &fileInfo, KDevItemGroup *parent = 0)
    : KDevProjectItem(fileInfo.fileName(), parent), m_fileInfo(fileInfo) {}

  QFileInfo fileInfo() const { return m_fileInfo; }

  virtual KDevProjectFileItem *file() const
  { return const_cast<KDevProjectFileItem*>(this); }

  virtual QIcon icon() const;

private:
  QFileInfo m_fileInfo;
};

class KDevProjectModel: public KDevItemModel
{
  Q_OBJECT
public:
  KDevProjectModel(QObject *parent = 0);
  virtual ~KDevProjectModel();

  virtual KDevProjectItem *item(const QModelIndex &index) const;
};

#endif // KDEVPROJECTMODEL_H
