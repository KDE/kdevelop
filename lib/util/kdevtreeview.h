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

#ifndef KDEVTREEVIEW_H
#define KDEVTREEVIEW_H

#include <QtGui/QTreeView>
#include "kdevexport.h"

class KDevItem;
class KDevItemGroup;
class KDevItemCollection;
class KDevItemModel;

class KDEVUTIL_EXPORT KDevTreeView: public QTreeView
{
  Q_OBJECT
public:
  KDevTreeView(QWidget *parent = 0);
  virtual ~KDevTreeView();

  KDevItemModel *itemModel() const;

  KDevItem *currentItem() const;
  KDevItemGroup *currentGroup() const;
  KDevItemCollection *currentCollection() const;
};

#endif // KDEVTREEVIEW_H
