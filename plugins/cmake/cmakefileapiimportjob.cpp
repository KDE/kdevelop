/*
    SPDX-FileCopyrightText: 2020 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    connect(&m_futureWatcher, &QFutureWatcher<CMakeProjectData>::finished, this, [this]() {
        emit dataAvailable(m_futureWatcher.result());
        emitResult();
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
        if (replyIndex.data.isEmpty()) {
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
}
}
