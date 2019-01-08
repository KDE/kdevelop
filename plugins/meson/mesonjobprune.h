/* This file is part of KDevelop
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
    explicit MesonJobPrune(Meson::BuildDir const& buildDir, QObject* parent);
    void start() override;
    bool doKill() override;

private:
    KDevelop::Path m_buildDir;
    QString m_backend;
    KJob* m_job = nullptr;
};
