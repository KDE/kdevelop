/*
    SPDX-FileCopyrightText: 2011 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "projectutils.h"
#include <project/projectmodel.h>
#include "path.h"


namespace KDevelop {

ProjectItemContextImpl::ProjectItemContextImpl(const QList<ProjectBaseItem*>& items)
    : ProjectItemContext(items)
{
}

QList<QUrl> ProjectItemContextImpl::urls() const
{
    QList<QUrl> urls;
    const auto items = this->items();
    for (const auto& item : items) {
        const auto url = item->path().toUrl();
        if (url.isValid()) {
            urls << url;
        }
    }
    return urls;
}

/**
 * Runs the @p callback on all files that have @p projectItem as ancestor
 */
void forEachFile(const ProjectBaseItem* projectItem,
                  const std::function<void(ProjectFileItem*)>& callback)
{
    if (auto* file = projectItem->file()) {
        callback(file);
        return;
    }

    const auto children = projectItem->children();
    for (const auto *child : children) {
        forEachFile(child, callback);
    }
}

QList<ProjectFileItem*> allFiles(const ProjectBaseItem* projectItem)
{
    QList<ProjectFileItem*> files;
    forEachFile(projectItem, [&files](ProjectFileItem *fileItem) {
        files.append(fileItem);
    });
    return files;
}

}
