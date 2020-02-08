/* KDevelop CMake Support
 *
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
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

#ifndef CMAKEIMPORTJSONJOB_H
#define CMAKEIMPORTJSONJOB_H

#include "cmakeprojectdata.h"
#include <util/path.h>

#include <KJob>

#include <QFutureWatcher>

class CMakeFolderItem;

struct ImportData {
    CMakeFilesCompilationData compilationData;
    QHash<KDevelop::Path, QVector<CMakeTarget>> targets;
    QVector<CMakeTest> testSuites;
};

namespace KDevelop
{
class IProject;
class ReferencedTopDUContext;
}

class CMakeImportJsonJob : public KJob
{
    Q_OBJECT

public:
    enum Error {
        FileMissingError = UserDefinedError, ///< JSON file was not found
        ReadError ///< Failed to read the JSON file
    };

    CMakeImportJsonJob(KDevelop::IProject* project, QObject* parent);
    ~CMakeImportJsonJob() override;

    void start() override;

    KDevelop::IProject* project() const;

    CMakeProjectData projectData() const;

private Q_SLOTS:
    void importCompileCommandsJsonFinished();

private:
    KDevelop::IProject* m_project;
    QFutureWatcher<ImportData> m_futureWatcher;

    CMakeProjectData m_data;
};

#endif // CMAKEIMPORTJSONJOB_H
