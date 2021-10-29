/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
