/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IMPORTPROJECTJOB_H
#define KDEVPLATFORM_IMPORTPROJECTJOB_H

#include <KJob>

#include "projectexport.h"

namespace KDevelop
{
class ProjectFolderItem;
class IProjectFileManager;

class KDEVPLATFORMPROJECT_EXPORT ImportProjectJob: public KJob
{
    Q_OBJECT
public:
    ImportProjectJob(ProjectFolderItem *folder, IProjectFileManager *importer);
    ~ImportProjectJob() override;

public:
    void start() override;
    bool doKill() override;

private Q_SLOTS:
    void importDone();
    void importCanceled();
    void aboutToShutdown();

private:
    class ImportProjectJobPrivate* const d;
    friend class ImportProjectJobPrivate;
};

}
#endif // KDEVPLATFORM_IMPORTPROJECTJOB_H


