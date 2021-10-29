/*
    SPDX-FileCopyrightText: 2011 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PROJECTUTILS_H
#define KDEVPLATFORM_PROJECTUTILS_H

#include "projectexport.h"

#include <interfaces/context.h>

#include <QList>

#include <functional>

class QMenu;

namespace KDevelop {

class ProjectBaseItem;
class ProjectFileItem;

// TODO: Move to own header? Rename to ProjectItemContext and remove original from context.h?
class KDEVPLATFORMPROJECT_EXPORT ProjectItemContextImpl : public ProjectItemContext
{
public:
    explicit ProjectItemContextImpl(const QList<ProjectBaseItem*>& items);

    QList<QUrl> urls() const override;

private:
    Q_DISABLE_COPY(ProjectItemContextImpl)
};

/**
 * Runs the @p callback on all files that have @p projectItem as ancestor
 */
KDEVPLATFORMPROJECT_EXPORT void forEachFile(const ProjectBaseItem* projectItem,
                                            const std::function<void(ProjectFileItem*)>& callback);

/**
 * Returns all the files that have @p projectItem as ancestor
 */
KDEVPLATFORMPROJECT_EXPORT QList<ProjectFileItem*> allFiles(const ProjectBaseItem* projectItem);
}

#endif // KDEVPLATFORM_PROJECTUTILS_H
