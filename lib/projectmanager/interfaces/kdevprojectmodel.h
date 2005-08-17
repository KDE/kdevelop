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

class KDevProjectFolderItem: public KDevItemCollection
{
public:
  KDevProjectFolderItem(const QDir &dir, KDevItemGroup *parent = 0)
    : KDevItemCollection(dir.dirName(), parent), m_directory(dir) {}

  QDir directory() const { return m_directory; }

private:
  QDir m_directory;
};

class KDevProjectTargetItem: public KDevItemCollection
{
public:
  KDevProjectTargetItem(const QString &name, KDevItemGroup *parent = 0)
    : KDevItemCollection(name, parent) {}
};

class KDevProjectFileItem: public KDevItemCollection
{
public:
  KDevProjectFileItem(const QFileInfo &fileInfo, KDevItemGroup *parent = 0)
    : KDevItemCollection(fileInfo.fileName(), parent), m_fileInfo(fileInfo) {}

  QFileInfo fileInfo() const { return m_fileInfo; }

private:
  QFileInfo m_fileInfo;
};

class KDevProjectModel: public KDevItemModel
{
  Q_OBJECT
public:
  KDevProjectModel(QObject *parent = 0);
  virtual ~KDevProjectModel();

  bool isProjectFolderItem(KDevItem *item) const;
  bool isProjectFileItem(KDevItem *item) const;
  bool isProjectTargetItem(KDevItem *item) const;
};

#endif // KDEVPROJECTMODEL_H
