/* This file is part of KDevelop

    Copyright 2020 Milian Wolff <mail@milianw.de>

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
    ImportJob(KDevelop::IProject* project, QObject* parent = nullptr);
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
