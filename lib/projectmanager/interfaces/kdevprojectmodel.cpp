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

#include "kdevprojectmodel.h"

QList<KDevProjectFolderItem*> KDevProjectItem::folderList() const
{
  QList<KDevProjectFolderItem*> lst;
  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevProjectFolderItem *item = itemAt(i)->folder())
        lst.append(item);
    }

  return lst;
}

QList<KDevProjectTargetItem*> KDevProjectItem::targetList() const
{
  QList<KDevProjectTargetItem*> lst;
  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevProjectTargetItem *item = itemAt(i)->target())
        lst.append(item);
    }

  return lst;
}

QList<KDevProjectFileItem*> KDevProjectItem::fileList() const
{
  QList<KDevProjectFileItem*> lst;
  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevProjectFileItem *item = itemAt(i)->file())
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

bool KDevProjectModel::isProjectFolderItem(KDevItem *item) const
{
  return dynamic_cast<KDevProjectFolderItem*>(item) != 0; // ### hmm, dynamic_cast
}

bool KDevProjectModel::isProjectFileItem(KDevItem *item) const
{
  return dynamic_cast<KDevProjectFileItem*>(item) != 0; // ### hmm, dynamic_cast
}

bool KDevProjectModel::isProjectTargetItem(KDevItem *item) const
{
  return dynamic_cast<KDevProjectTargetItem*>(item) != 0; // ### hmm, dynamic_cast
}

#include "kdevprojectmodel.moc"
