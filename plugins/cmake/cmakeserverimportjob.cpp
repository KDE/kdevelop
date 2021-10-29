/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakeserverimportjob.h"
#include "cmakeutils.h"
#include "cmakeserver.h"

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <makefileresolver/makefileresolver.h>

#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QFileInfo>

#include "debug.h"

void CMakeServerImportJob::processCodeModel(const QJsonObject &response, CMakeProjectData &data)
{
    const auto configs = response.value(QStringLiteral("configurations")).toArray();
    qCDebug(CMAKE) << "process response" << response;

    data.targets.clear();
    data.compilationData.files.clear();

    StringInterner stringInterner;

    const auto rt = KDevelop::ICore::self()->runtimeController()->currentRuntime();
    for (const auto &config: configs) {
        const auto projects = config.toObject().value(QStringLiteral("projects")).toArray();
        for (const auto &project: projects) {
            const auto targets = project.toObject().value(QStringLiteral("targets")).toArray();
            for (const auto &targetObject: targets) {
                const auto target = targetObject.toObject();
                const KDevelop::Path targetDir = rt->pathInHost(KDevelop::Path(target.value(QStringLiteral("sourceDirectory")).toString()));

                KDevelop::Path::List targetSources;
                const auto fileGroups = target.value(QStringLiteral("fileGroups")).toArray();
                for (const auto &fileGroupValue: fileGroups) {
                    const auto fileGroup = fileGroupValue.toObject();
                    CMakeFile file;
                    file.includes = kTransform<KDevelop::Path::List>(fileGroup.value(QStringLiteral("includePath")).toArray(), [](const QJsonValue& val) { return KDevelop::Path(val.toObject().value(QStringLiteral("path")).toString()); });

                    file.language = fileGroup.value(QStringLiteral("language")).toString(),
                    file.compileFlags = fileGroup.value(QStringLiteral("compileFlags")).toString();
                    for (const auto& jsonDefine : fileGroup.value(QStringLiteral("defines")).toArray()) {
                        file.addDefine(jsonDefine.toString());
                    }
                    file.defines = MakeFileResolver::extractDefinesFromCompileFlags(file.compileFlags, stringInterner, file.defines);

                    // apparently some file groups do not contain build system information
                    // skip these, as they would produce bogus results for us and break the fallback
                    // implemented in CMakeManager::fileInformation
                    if (file.isEmpty()) {
                        continue;
                    }

                    const auto sourcesArray = fileGroup.value(QStringLiteral("sources")).toArray();
                    const KDevelop::Path::List sources = kTransform<KDevelop::Path::List>(sourcesArray, [targetDir](const QJsonValue& val) { return KDevelop::Path(targetDir, val.toString()); });
                    targetSources.reserve(targetSources.size() + sources.size());
                    for (const auto& source: sources) {
                        // NOTE: we use the canonical file path to prevent issues with symlinks in the path
                        //       leading to lookup failures
                        const auto localFile = rt->pathInHost(source);
                        const auto canonicalFile = QFileInfo(source.toLocalFile()).canonicalFilePath();
                        const auto sourcePath = (canonicalFile.isEmpty() || localFile.toLocalFile() == canonicalFile)
                                              ? localFile : KDevelop::Path(canonicalFile);
                        data.compilationData.files[sourcePath] = file;
                        targetSources << sourcePath;
                    }
                    qCDebug(CMAKE) << "registering..." << sources << file;
                }

                CMakeTarget cmakeTarget{
                    CMakeTarget::typeToEnum(target.value(QLatin1String("type")).toString()),
                    target.value(QStringLiteral("name")).toString(),
                    kTransform<KDevelop::Path::List>(target[QLatin1String("artifacts")].toArray(), [](const QJsonValue& val) { return KDevelop::Path(val.toString()); }),
                    targetSources,
                    QString()
                };

                // ensure we don't add the same target multiple times, for different projects
                // cf.: https://bugs.kde.org/show_bug.cgi?id=387095
                auto& dirTargets = data.targets[targetDir];
                if (dirTargets.contains(cmakeTarget))
                    continue;
                dirTargets += cmakeTarget;

                qCDebug(CMAKE) << "adding target" << cmakeTarget.name << "with sources" << cmakeTarget.sources;
            }
        }
    }
}

CMakeServerImportJob::CMakeServerImportJob(KDevelop::IProject* project, const QSharedPointer<CMakeServer> &server, QObject* parent)
    : KJob(parent)
    , m_server(server)
    , m_project(project)
{
    connect(m_server.data(), &CMakeServer::disconnected, this, [this]() {
        setError(UnexpectedDisconnect);
        emitResult();
    });
}

void CMakeServerImportJob::start()
{
    if (m_server->isServerAvailable())
        doStart();
    else
        connect(m_server.data(), &CMakeServer::connected, this, &CMakeServerImportJob::doStart);
}

void CMakeServerImportJob::doStart()
{
    connect(m_server.data(), &CMakeServer::response, this, &CMakeServerImportJob::processResponse);

    m_server->handshake(m_project->path(), CMake::currentBuildDir(m_project));
}

void CMakeServerImportJob::processResponse(const QJsonObject& response)
{
    const auto responseType = response.value(QStringLiteral("type"));
    if (responseType == QLatin1String("reply")) {
        const auto inReplyTo = response.value(QStringLiteral("inReplyTo"));
        qCDebug(CMAKE) << "replying..." << inReplyTo;
        if (inReplyTo == QLatin1String("handshake")) {
            m_server->configure({});
        } else if (inReplyTo == QLatin1String("configure")) {
            m_server->compute();
        } else if (inReplyTo == QLatin1String("compute")) {
            m_server->codemodel();
        } else if(inReplyTo == QLatin1String("codemodel")) {
            processCodeModel(response, m_data);
            m_data.testSuites = CMake::importTestSuites(CMake::currentBuildDir(m_project));
            m_data.compilationData.rebuildFileForFolderMapping();
            emitResult();
        } else {
            qCDebug(CMAKE) << "unhandled reply" << response;
        }
    } else if(responseType == QLatin1String("error")) {
        setError(ErrorResponse);
        setErrorText(response.value(QStringLiteral("errorMessage")).toString());
        qCWarning(CMAKE) << "error!!" << response;
        emitResult();
    } else if (responseType == QLatin1String("progress")) {
        int progress = response.value(QStringLiteral("progressCurrent")).toInt();
        int total = response.value(QStringLiteral("progressMaximum")).toInt();
        if (progress >= 0 && total > 0) {
            setPercent(100.0 * progress / total);
        }
    } else if (responseType == QLatin1String("message") || responseType == QLatin1String("hello")) {
        // Known, but not used for anything currently.
    } else {
        qCDebug(CMAKE) << "unhandled message" << response;
    }
}
