/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  David Nolden <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
void forEachFiles(const ProjectBaseItem* projectItem,
                  const std::function<void(ProjectFileItem*)>& callback)
{
    if (auto* file = projectItem->file()) {
        callback(file);
        return;
    }

    const auto children = projectItem->children();
    for (const auto *child : children) {
        forEachFiles(child, callback);
    }
}

QList<ProjectFileItem*> allFiles(const ProjectBaseItem* projectItem)
{
    QList<ProjectFileItem*> files;
    forEachFiles(projectItem, [&files](ProjectFileItem *fileItem) {
        files.append(fileItem);
    });
    return files;
}

}
