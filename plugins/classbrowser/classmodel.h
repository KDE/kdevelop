/*
 * KDevelop Class Browser
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CLASSMODEL_H
#define CLASSMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>

#include <ktexteditor/cursor.h>
#include <kurl.h>

#include "duchainbase.h"
#include "duchainobserver.h"

class ClassBrowserPart;

namespace KDevelop {
 class TopDUContext;
 class IDocument;
 class ParseJob;
}

class ClassModel : public QAbstractItemModel, public KDevelop::DUChainObserver
{
  Q_OBJECT

public:
  ClassModel(ClassBrowserPart* parent);
  virtual ~ClassModel();

public:
  virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex & index) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

  // Definition use chain observer implementation
  virtual void contextChanged(KDevelop::DUContext* context, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);
  virtual void declarationChanged(KDevelop::Declaration* declaration, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);
  virtual void definitionChanged(KDevelop::Definition* definition, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);
  virtual void useChanged(KDevelop::Use* use, Modification change, Relationship relationship, KDevelop::DUChainBase* relatedObject = 0);

private:
Q_SIGNALS:
  void scheduleAddition(KDevelop::DUContext* context);
  void scheduleRemoval(KDevelop::DUContext* context);

private Q_SLOTS:
  void slotScheduleAddition(KDevelop::DUContext* context);
  void slotScheduleRemoval(KDevelop::DUContext* context);

private:
  ClassBrowserPart* part() const;

  KDevelop::DUChainBase* objectForIndex(const QModelIndex& index) const;

  mutable QMutex m_mutex;
  mutable QList<KDevelop::DUChainBase*> m_topObjects;
};

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
