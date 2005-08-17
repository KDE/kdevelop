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

#include "kdevprojectmanager.h"
#include "kdevprojectmanager_part.h"
#include "kdevprojectmodel.h"

#include <QtGui/QHeaderView>

#include <kdevcore.h>
#include <kpopupmenu.h>
#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>

#include <QtCore/qdebug.h>

KDevProjectManager::KDevProjectManager(KDevProjectManagerPart *part, QWidget *parent)
  : KDevTreeView(parent),
    m_part(part)
{
  header()->hide();

  setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(popupContextMenu(QPoint)));
  connect(this, SIGNAL(activated(QModelIndex)), this, SLOT(slotActivated(QModelIndex)));
}

KDevProjectManager::~KDevProjectManager()
{
}

KDevProjectManagerPart *KDevProjectManager::part() const
{
  return m_part;
}

KDevProjectFolderItem *KDevProjectManager::currentFolderItem() const
{
  Q_ASSERT(projectModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  while (current.isValid())
    {
      if (KDevProjectFolderItem *folderItem = dynamic_cast<KDevProjectFolderItem*>(projectModel()->item(current)))
        return folderItem;

      current = projectModel()->parent(current);
    }

  return 0;
}

KDevProjectFileItem *KDevProjectManager::currentFileItem() const
{
  Q_ASSERT(projectModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  while (current.isValid())
    {
      if (KDevProjectFileItem *fileItem = dynamic_cast<KDevProjectFileItem*>(projectModel()->item(current)))
        return fileItem;

      current = projectModel()->parent(current);
    }

  return 0;
}

KDevProjectTargetItem *KDevProjectManager::currentTargetItem() const
{
  Q_ASSERT(projectModel() != 0);

  QItemSelectionModel *selection = selectionModel();
  QModelIndex current = selection->currentIndex();

  while (current.isValid())
    {
      if (KDevProjectTargetItem *targetItem = dynamic_cast<KDevProjectTargetItem*>(projectModel()->item(current)))
        return targetItem;

      current = projectModel()->parent(current);
    }

  return 0;
}

KDevProjectModel *KDevProjectManager::projectModel() const
{
  return qobject_cast<KDevProjectModel*>(model());
}

void KDevProjectManager::slotActivated(const QModelIndex &index)
{
  KDevProjectItem *item = projectModel()->item(index);

  if (item && item->file())
    {
      KURL url;
      url.setPath(item->file()->fileInfo().absoluteFilePath());
      emit activateURL(url);
    }
}

void KDevProjectManager::popupContextMenu(const QPoint &pos)
{
  QModelIndex index = indexAt(pos);

  if (KDevProjectItem *item = projectModel()->item(index))
    {
      KPopupMenu menu(this);

      if (KDevProjectFolderItem *folder = item->folder())
        {
          menu.addTitle(i18n("Folder: %1").arg(folder->directory().dirName()));
        }
      else if (KDevProjectFileItem *file = item->file())
        {
          menu.addTitle(i18n("File: %1").arg(file->fileInfo().fileName()));
        }
      else if (KDevProjectTargetItem *target = item->target())
        {
          menu.addTitle(i18n("Target: %1").arg(target->name()));
        }

      ProjectItemContext context(item);
      part()->core()->fillContextMenu(&menu, &context);

      menu.exec(mapToGlobal(pos));
    }
}
