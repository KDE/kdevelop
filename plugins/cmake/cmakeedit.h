/* KDevelop CMake Support
 * 
 * Copyright 2007-2013 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEEDIT_H
#define CMAKEEDIT_H

#include <QUrl>

namespace KDevelop {
    class ProjectBaseItem;
    class ApplyChangesWidget;
    class IProject;
    class ProjectTargetItem;
    class ProjectFileItem;
}

class CMakeFolderItem;

namespace CMakeEdit {
    CMakeFolderItem* nearestCMakeFolder(KDevelop::ProjectBaseItem* item);
    QList<KDevelop::ProjectBaseItem*> cmakeListedItemsAffectedByUrlChange(const KDevelop::IProject *proj, const QUrl &url, QUrl rootUrl = QUrl());
    QList<KDevelop::ProjectBaseItem*> cmakeListedItemsAffectedByItemsChanged(const QList<KDevelop::ProjectBaseItem*> &items);

    bool changesWidgetRemoveFilesFromTargets(const QList<KDevelop::ProjectFileItem*> &files, KDevelop::ApplyChangesWidget *widget);
    bool changesWidgetRemoveItems(const QSet<KDevelop::ProjectBaseItem*>& items, KDevelop::ApplyChangesWidget* widget);
    bool changesWidgetAddFolder(const QUrl &folderUrl, const CMakeFolderItem *toFolder, KDevelop::ApplyChangesWidget *widget);
    bool changesWidgetRemoveCMakeFolder(const CMakeFolderItem *folder, KDevelop::ApplyChangesWidget *widget);
    bool changesWidgetAddFolder(const QUrl &folderUrl, const CMakeFolderItem *toFolder, KDevelop::ApplyChangesWidget *widget);
    bool changesWidgetMoveTargetFile(const KDevelop::ProjectBaseItem *file, const QUrl &newUrl, KDevelop::ApplyChangesWidget *widget);
    bool changesWidgetAddFilesToTarget(const QList<KDevelop::ProjectFileItem*> &files, const KDevelop::ProjectTargetItem* target, KDevelop::ApplyChangesWidget *widget);
    bool changesWidgetRenameFolder(const CMakeFolderItem *folder, const QUrl &newUrl, KDevelop::ApplyChangesWidget *widget);

    QUrl afterMoveUrl(const QUrl &origUrl, const QUrl& movedOrigUrl, const QUrl& movedNewUrl);
}

#endif
