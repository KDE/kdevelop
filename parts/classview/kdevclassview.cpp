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

#include "kdevclassview.h"
#include "kdevclassview_part.h"
#include "kdevcodemodel.h"

#include <QtGui/QHeaderView>

#include <kdevcore.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <QtCore/qdebug.h>

KDevClassView::KDevClassView(KDevClassViewPart *part, QWidget *parent)
  : KDevTreeView(parent),
    m_part(part)
{
  header()->hide();
  
  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(popupContextMenu(QPoint)));
  connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(slotActivated(QModelIndex)));
}

KDevClassView::~KDevClassView()
{
}

KDevClassViewPart *KDevClassView::part() const
{
  return m_part;
}

KDevCodeItem *KDevClassView::currentItem() const
{
  Q_ASSERT(codeModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  if (KDevCodeItem *item = codeModel()->item(current))
    return item;

  return 0;
}

KDevCodeNamespaceItem *KDevClassView::currentNamespaceItem() const
{
  Q_ASSERT(codeModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  while (current.isValid())
    {
      if (KDevCodeNamespaceItem *item = codeModel()->item(current)->namespaceItem())
        return item;

      current = codeModel()->parent(current);
    }

  return 0;
}

KDevCodeClassItem *KDevClassView::currentClassItem() const
{
  Q_ASSERT(codeModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  while (current.isValid())
    {
      if (KDevCodeClassItem *item = codeModel()->item(current)->classItem())
        return item;

      current = codeModel()->parent(current);
    }

  return 0;
}

KDevCodeFunctionItem *KDevClassView::currentFunctionItem() const
{
  Q_ASSERT(codeModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  while (current.isValid())
    {
      if (KDevCodeFunctionItem *item = codeModel()->item(current)->functionItem())
        return item;

      current = codeModel()->parent(current);
    }

  return 0;
}

KDevCodeVariableItem *KDevClassView::currentVariableItem() const
{
  Q_ASSERT(codeModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  while (current.isValid())
    {
      if (KDevCodeVariableItem *item = codeModel()->item(current)->variableItem())
        return item;

      current = codeModel()->parent(current);
    }

  return 0;
}

KDevCodeModel *KDevClassView::codeModel() const
{
  return qobject_cast<KDevCodeModel*>(model());
}

void KDevClassView::slotActivated(const QModelIndex &index)
{
}

void KDevClassView::popupContextMenu(const QPoint &pos)
{
  QModelIndex index = indexAt(pos);

  if (KDevCodeItem *item = codeModel()->item(index))
    {
      KPopupMenu menu(this);

      CodeItemContext context(item);
      part()->core()->fillContextMenu(&menu, &context);

      menu.exec(mapToGlobal(pos));
    }
}
