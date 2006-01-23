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

#ifndef KDEVPROJECTMODEL_H
#define KDEVPROJECTMODEL_H

#include "kdevitemmodel.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

class KDevProjectFolderItem;
class KDevProjectFileItem;
class KDevProjectTargetItem;

/**
 * Interface that allows a developer to implement the three basic types of
 * items you would see in a project
 * \li Folder
 * \li Build Target
 * \li File
 */
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

/**
 * Implementation of the KDevProjectItem interface that is specific to a 
 * folder
 */
class KDevProjectFolderItem: public KDevProjectItem
{
public:
  KDevProjectFolderItem(const QDir &dir, KDevItemGroup *parent = 0)
    : KDevProjectItem(dir.dirName(), parent), m_directory(dir) {}

  virtual KDevProjectFolderItem *folder() const
  { return const_cast<KDevProjectFolderItem*>(this); }

  /** Get the QDir representation of this folder */
  QDir directory() const { return m_directory; }
  virtual QIcon icon() const;

private:
  QDir m_directory;
};

/**
 * Implementation of the KDevProjectItem interface that is specific to a
 * build target
 */
class KDevProjectTargetItem: public KDevProjectItem
{
public:
  KDevProjectTargetItem(const QString &name, KDevItemGroup *parent = 0)
    : KDevProjectItem(name, parent) {}

  virtual KDevProjectTargetItem *target() const
  { return const_cast<KDevProjectTargetItem*>(this); }
};

/**
 * Implementation of the KDevProjectItem interface that is specific to a
 * file
 */
class KDevProjectFileItem: public KDevProjectItem
{
public:
  KDevProjectFileItem(const QFileInfo &fileInfo, KDevItemGroup *parent = 0)
    : KDevProjectItem(fileInfo.fileName(), parent), m_fileInfo(fileInfo) {}

  /** Get the QFileInfo representation of this item */
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
