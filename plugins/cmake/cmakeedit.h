/*
    SPDX-FileCopyrightText: 2007-2013 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
