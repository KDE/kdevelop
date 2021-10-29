/*
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <outputview/outputjob.h>
#include <util/path.h>

namespace Meson
{
struct BuildDir;
}

// "Inspired" by the CMake prune job
class MesonJobPrune : public KDevelop::OutputJob
{
    Q_OBJECT
public:
    explicit MesonJobPrune(const Meson::BuildDir& buildDir, QObject* parent);
    void start() override;
    bool doKill() override;

private:
    KDevelop::Path m_buildDir;
    QString m_backend;
    KJob* m_job = nullptr;
};
