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

#include "kdevcodemodel.h"

namespace
{

struct _CodeModel
{
};

} // namespace

KDevCodeItem::KDevCodeItem(const QString &name, KDevItemGroup *parent)
  : KDevItemCollection(name, parent)
{
}

KDevCodeItem::~KDevCodeItem()
{
}

KDevCodeItem *KDevCodeItem::itemAt(int index) const
{
  return static_cast<KDevCodeItem*>(KDevItemCollection::itemAt(index));
}

KDevCodeNamespaceItem::KDevCodeNamespaceItem(const QString &name, KDevItemGroup *parent)
  : KDevCodeItem(name, parent)
{
}

KDevCodeNamespaceItem::~KDevCodeNamespaceItem()
{
}

QList<KDevCodeNamespaceItem*> KDevCodeNamespaceItem::namespaceList() const
{
  QList<KDevCodeNamespaceItem*> lst;

  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevCodeNamespaceItem *item = itemAt(i)->namespaceItem())
        lst.append(item);
    }

  return lst;
}

QList<KDevCodeClassItem*> KDevCodeNamespaceItem::classList() const
{
  QList<KDevCodeClassItem*> lst;

  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevCodeClassItem *item = itemAt(i)->classItem())
        lst.append(item);
    }

  return lst;
}

QList<KDevCodeFunctionItem*> KDevCodeNamespaceItem::functionList() const
{
  QList<KDevCodeFunctionItem*> lst;

  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevCodeFunctionItem *item = itemAt(i)->functionItem())
        lst.append(item);
    }

  return lst;
}

QList<KDevCodeVariableItem*> KDevCodeNamespaceItem::variableList() const
{
  QList<KDevCodeVariableItem*> lst;

  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevCodeVariableItem *item = itemAt(i)->variableItem())
        lst.append(item);
    }

  return lst;
}

KDevCodeClassItem::KDevCodeClassItem(const QString &name, KDevItemGroup *parent)
  : KDevCodeItem(name, parent)
{
}

KDevCodeClassItem::~KDevCodeClassItem()
{
}

QList<KDevCodeClassItem*> KDevCodeClassItem::classList() const
{
  QList<KDevCodeClassItem*> lst;

  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevCodeClassItem *item = itemAt(i)->classItem())
        lst.append(item);
    }

  return lst;
}

QList<KDevCodeFunctionItem*> KDevCodeClassItem::functionList() const
{
  QList<KDevCodeFunctionItem*> lst;

  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevCodeFunctionItem *item = itemAt(i)->functionItem())
        lst.append(item);
    }

  return lst;
}

QList<KDevCodeVariableItem*> KDevCodeClassItem::variableList() const
{
  QList<KDevCodeVariableItem*> lst;

  for (int i=0; i<itemCount(); ++i)
    {
      if (KDevCodeVariableItem *item = itemAt(i)->variableItem())
        lst.append(item);
    }

  return lst;
}

KDevCodeFunctionItem::KDevCodeFunctionItem(const QString &name, KDevItemGroup *parent)
  : KDevCodeItem(name, parent)
{
}

KDevCodeFunctionItem::~KDevCodeFunctionItem()
{
}

KDevCodeVariableItem::KDevCodeVariableItem(const QString &name, KDevItemGroup *parent)
  : KDevCodeItem(name, parent)
{
}

KDevCodeVariableItem::~KDevCodeVariableItem()
{
}




