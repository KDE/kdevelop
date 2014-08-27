/*
 * KDevelop Class viewer
 *
 * Copyright (c) 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
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

#ifndef KDEVPLATFORM_PLUGIN_CLASSTREE_H
#define KDEVPLATFORM_PLUGIN_CLASSTREE_H

#include <QTreeView>
#include <QtCore/QPointer>

#include <language/duchain/identifier.h>

#include "language/util/navigationtooltip.h"

class ClassBrowserPlugin;
class ClassModel;

class ClassTree : public QTreeView
{
  Q_OBJECT

public:
  ClassTree(QWidget* parent, ClassBrowserPlugin* plugin);
  virtual ~ClassTree();

public:
  /// Find the given a_id in the tree and highlight it.
  void highlightIdentifier(KDevelop::IndexedQualifiedIdentifier a_id);

  static bool populatingClassBrowserContextMenu();
  
protected:
  virtual void contextMenuEvent(QContextMenuEvent* e);
  ClassModel* model();
  virtual bool event(QEvent* event);

private Q_SLOTS:
  void itemActivated(const QModelIndex& index);

private:
  ClassBrowserPlugin* m_plugin;
  QPointer<KDevelop::NavigationToolTip> m_tooltip;
};

#endif

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
