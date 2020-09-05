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

#include "cmakefileapiimportjob.h"

using namespace KDevelop;

#include "cmakeprojectdata.h"
#include "cmakefileapi.h"
#include "cmakeutils.h"

#include <interfaces/iproject.h>
#include <interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <QtConcurrentRun>
#include <QJsonObject>

namespace CMake {
namespace FileApi {

ImportJob::ImportJob(KDevelop::IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
{
    setCapabilities(Killable);

    connect(&m_futureWatcher, &QFutureWatcher<CMakeProjectData>::finished, this, [this]() {
        if (Q_LIKELY(!m_futureWatcher.isCanceled())) {
            emit dataAvailable(m_futureWatcher.result());
            emitResult();
        }
    });
}

ImportJob::~ImportJob() = default;

void ImportJob::start()
{
    const auto* bsm = m_project->buildSystemManager();
    const auto sourceDirectory = m_project->path();
    const auto buildDirectory = bsm->buildDirectory(m_project->projectItem());
    auto future = QtConcurrent::run([sourceDirectory, buildDirectory]() -> CMakeProjectData {
        const auto replyIndex = findReplyIndexFile(buildDirectory.toLocalFile());
        if (replyIndex.isEmpty()) {
            return {};
        }
        auto ret = parseReplyIndexFile(replyIndex, sourceDirectory, buildDirectory);
        if (!ret.compilationData.isValid) {
            return ret;
        }
        ret.testSuites = CMake::importTestSuites(buildDirectory);
        return ret;
    });
    m_futureWatcher.setFuture(future);
}

bool ImportJob::doKill()
{
    m_futureWatcher.cancel();
    return true;
}
}
}
