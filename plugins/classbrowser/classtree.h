/*
 * KDevelop Class viewer
 *
 * Copyright (c) 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef DUCHAINTREE_H
#define DUCHAINTREE_H

#include <QtGui/QTreeView>

#include "duchainbase.h"
#include "duchainobserver.h"

class ClassBrowserPlugin;

namespace KDevelop {
 class TopDUContext;
 class IDocument;
}

class ClassWidget : public QWidget
{
  Q_OBJECT

public:
  ClassWidget(QWidget* parent, ClassBrowserPlugin* plugin);
  virtual ~ClassWidget();

  class ClassModel* model();

  enum Modes {
    ModeCurrentDocument,
    ModeProject,
    ModeAll
  };

private Q_SLOTS:
  void setMode(QAction* action);

private:
  ClassBrowserPlugin* m_plugin;

  class ClassTree* m_tree;
  Modes m_currentMode;
};

class ClassTree : public QTreeView
{
  Q_OBJECT

public:
  ClassTree(QWidget* parent, ClassBrowserPlugin* plugin);
  virtual ~ClassTree();

  ClassModel* model();

protected:
  virtual void contextMenuEvent(QContextMenuEvent* e);

private Q_SLOTS:
  void itemActivated(const QModelIndex& index);
  void openDeclaration();
  void openDefinition();

private:
  ClassBrowserPlugin* m_plugin;
};

#endif

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
