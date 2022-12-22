/*
    SPDX-FileCopyrightText: 2020 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KJob>
#include <QFutureWatcher>

namespace KDevelop
{
class IProject;
}

struct CMakeProjectData;

namespace CMake {
namespace FileApi {
class ImportJob : public KJob
{
    Q_OBJECT
public:
    explicit ImportJob(KDevelop::IProject* project, QObject* parent = nullptr);
    ~ImportJob();

    void start() override;

Q_SIGNALS:
    void dataAvailable(const CMakeProjectData& data);

private:
    KDevelop::IProject* m_project = nullptr;
    QFutureWatcher<CMakeProjectData> m_futureWatcher;
};
}
}
