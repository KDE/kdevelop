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

void ImportJob::setInvalidateOutdatedData()
{
    m_invalidateOutdatedData = true;
}

void ImportJob::setEmitInvalidData()
{
    m_emitInvalidData = true;
}

void ImportJob::start()
{
    const auto* bsm = m_project->buildSystemManager();
    const auto sourceDirectory = m_project->path();
    const auto buildDirectory = bsm->buildDirectory(m_project->projectItem());

    auto future = QtConcurrent::run(
        [sourceDirectory, buildDirectory, invalidateOutdatedData = m_invalidateOutdatedData]() -> CMakeProjectData {
            const auto replyIndex = findReplyIndexFile(buildDirectory.toLocalFile());
            if (!replyIndex.isValid() || (invalidateOutdatedData && replyIndex.isOutdated())) {
                return {};
            }

            auto ret = parseReplyIndexFile(replyIndex, sourceDirectory, buildDirectory);
            if (invalidateOutdatedData && ret.isOutdated) {
                ret.compilationData.isValid = false;
            }
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

#include "moc_cmakefileapiimportjob.cpp"
