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

#ifndef KDEVCODEMODEL_H
#define KDEVCODEMODEL_H

#include <kdevitemmodel.h>

class KDevCodeItem;
class KDevCodeNamespaceItem;
class KDevCodeClassItem;
class KDevCodeFunctionItem;
class KDevCodeVariableItem;

class KDevCodeItem: public KDevItemCollection
{
public:
  KDevCodeItem(const QString &name, KDevItemGroup *parent = 0);
  virtual ~KDevCodeItem();

  virtual KDevCodeItem *itemAt(int index) const;

  virtual KDevCodeNamespaceItem *namespaceItem() const { return 0; }
  virtual KDevCodeClassItem *classItem() const { return 0; }
  virtual KDevCodeFunctionItem *functionItem() const { return 0; }
  virtual KDevCodeVariableItem *variableItem() const { return 0; }
};

class KDevCodeNamespaceItem: public KDevCodeItem
{
public:
  KDevCodeNamespaceItem(const QString &name, KDevItemGroup *parent = 0);
  virtual ~KDevCodeNamespaceItem();

  virtual KDevCodeNamespaceItem *namespaceItem() const { return const_cast<KDevCodeNamespaceItem*>(this); }

  QList<KDevCodeNamespaceItem*> namespaceList() const;
  QList<KDevCodeClassItem*> classList() const;
  QList<KDevCodeFunctionItem*> functionList() const;
  QList<KDevCodeVariableItem*> variableList() const;
};

class KDevCodeClassItem: public KDevCodeItem
{
public:
  KDevCodeClassItem(const QString &name, KDevItemGroup *parent = 0);
  virtual ~KDevCodeClassItem();

  virtual KDevCodeClassItem *classItem() const { return const_cast<KDevCodeClassItem*>(this); }

  QList<KDevCodeClassItem*> classList() const;
  QList<KDevCodeFunctionItem*> functionList() const;
  QList<KDevCodeVariableItem*> variableList() const;
};

class KDevCodeFunctionItem: public KDevCodeItem
{
public:
  KDevCodeFunctionItem(const QString &name, KDevItemGroup *parent = 0);
  virtual ~KDevCodeFunctionItem();

  virtual KDevCodeFunctionItem *functionItem() const { return const_cast<KDevCodeFunctionItem*>(this); }
};

class KDevCodeVariableItem: public KDevCodeItem
{
public:
  KDevCodeVariableItem(const QString &name, KDevItemGroup *parent = 0);
  virtual ~KDevCodeVariableItem();

  virtual KDevCodeVariableItem *variableItem() const { return const_cast<KDevCodeVariableItem*>(this); }
};

class KDevCodeModel: public KDevItemModel
{
  Q_OBJECT
public:
  KDevCodeModel(QObject *parent = 0);
  virtual ~KDevCodeModel();

  virtual KDevCodeItem *item(const QModelIndex &index) const;
};

#endif // KDEVCODEMODEL_H
