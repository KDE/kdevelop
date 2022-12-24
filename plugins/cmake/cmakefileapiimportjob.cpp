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

#include <KLocalizedString>

#include <QtConcurrentRun>
#include <QFile>
#include <QJsonObject>

#include <utility>

namespace CMake {
namespace FileApi {

ImportJob::ImportJob(KDevelop::IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
{
    connect(&m_futureWatcher, &QFutureWatcher<CMakeProjectData>::finished, this, [this]() {
        auto data = m_futureWatcher.result();
        if (m_emitInvalidData || data.compilationData.isValid) {
            emit dataAvailable(std::move(data));
        } else {
            setError(InvalidProjectDataError);
            setErrorText(i18nc("error message", "invalid CMake file API project data"));
        }
        emitResult();
    });
}

ImportJob::~ImportJob() = default;

void ImportJob::setEmitInvalidData()
{
    m_emitInvalidData = true;
}

void ImportJob::start()
{
    const auto* bsm = m_project->buildSystemManager();
    const auto sourceDirectory = m_project->path();
    const auto buildDirectory = bsm->buildDirectory(m_project->projectItem());

    auto future = QtConcurrent::run([sourceDirectory, buildDirectory]() -> CMakeProjectData {
        // don't import data when no suitable CMakeCache file exists, which could happen
        // because our prune job didn't use to delete the .cmake folder
        if (!QFile::exists(Path(buildDirectory, QStringLiteral("CMakeCache.txt")).toLocalFile())) {
            return {};
        }

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
