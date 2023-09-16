/*
    SPDX-FileCopyrightText: 2020 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>
#include <QFutureWatcher>
#include "cmakeprojectdata.h"

namespace KDevelop
{
class IProject;
}

namespace CMake {
namespace FileApi {
class ImportJob : public KJob
{
    Q_OBJECT
public:
    enum {
        // Add a "random" number to KJob::UserDefinedError and hopefully avoid
        // clashes with OutputJob's, OutputExecuteJob's, etc. error codes.
        InvalidProjectDataError = UserDefinedError + 172
    };

    explicit ImportJob(KDevelop::IProject* project, QObject* parent = nullptr);
    ~ImportJob();

    /// Call this function if outdated data is going to be discarded to let this job return earlier.
    /// After this is called, outdated data is considered invalid. Thus when the data is outdated, the job
    /// finishes with InvalidProjectDataError, or emits invalid data if setEmitInvalidData() has been called.
    void setInvalidateOutdatedData();

    /// If this function is called, the job finishes without error and
    /// dataAvailable() signal is emitted when project data is invalid.
    void setEmitInvalidData();

    void start() override;

Q_SIGNALS:
    void dataAvailable(const CMakeProjectData& data);

private:
    KDevelop::IProject* m_project = nullptr;
    QFutureWatcher<CMakeProjectData> m_futureWatcher;
    bool m_invalidateOutdatedData = false;
    bool m_emitInvalidData = false;
};
}
}
