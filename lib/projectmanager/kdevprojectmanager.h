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

#ifndef KDEVPROJECTMANAGER_H
#define KDEVPROJECTMANAGER_H

#include "kdevtreeview.h"

class KUrl;

namespace Koncrete
{

class ProjectManagerPart;
class ProjectModel;
class ProjectFolderItem;
class ProjectFileItem;
class ProjectTargetItem;
class ProjectItem;

class KDEVPROJECTMANAGER_EXPORT ProjectManager: public TreeView
{
  Q_OBJECT
public:
  ProjectManager(ProjectManagerPart *part, QWidget *parent);
  virtual ~ProjectManager();

  ProjectManagerPart *part() const;
  ProjectModel *projectModel() const;

  ProjectFolderItem *currentFolderItem() const;
  ProjectFileItem *currentFileItem() const;
  ProjectTargetItem *currentTargetItem() const;

  virtual void reset();

signals:
  void activateURL(const KUrl &url);
  void currentChanged(ProjectItem *item);

protected slots:
  void slotActivated(const QModelIndex &index);
  void slotCurrentChanged(const QModelIndex &index);
  void popupContextMenu(const QPoint &pos);

private:
  ProjectManagerPart *m_part;
};

}
#endif // KDEVPROJECTMANAGER_H

// kate: space-indent on; indent-width 2; replace-tabs on;
