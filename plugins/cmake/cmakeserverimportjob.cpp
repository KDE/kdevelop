/* KDevelop CMake Support
 *
 * Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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

static QString unescape(const QStringRef& input)
{
  QString output;
  output.reserve(input.length());
  bool isEscaped = false;
  for (auto it = input.data(), end = it + input.length(); it != end; ++it) {
    QChar c = *it;
    if (!isEscaped && c == '\\') {
      isEscaped = true;
    } else {
      output.append(c);
      isEscaped = false;
    }
  }
  return output;
}

static QHash<QString, QString> processDefines(const QString &compileFlags, const QJsonArray &defines)
{
    QHash<QString, QString> ret;
    const auto& defineRx = MakeFileResolver::defineRegularExpression();
    auto it = defineRx.globalMatch(compileFlags);
    while (it.hasNext()) {
      const auto match = it.next();
      QString value;
      if (match.lastCapturedIndex() > 1) {
        value = unescape(match.capturedRef(match.lastCapturedIndex()));
      }
      ret[match.captured(1)] = value;
    }

    for (const QJsonValue& defineValue: defines) {
        const QString define = defineValue.toString();
        const int eqIdx = define.indexOf(QLatin1Char('='));
        if (eqIdx<0) {
            ret[define] = QString();
        } else {
            ret[define.left(eqIdx)] = define.mid(eqIdx+1);
        }
    }
    return ret;
}

CMakeTarget::Type typeToEnum(const QJsonObject& target)
{
    static const QHash<QString, CMakeTarget::Type> s_types = {
        {QStringLiteral("EXECUTABLE"), CMakeTarget::Executable},
        {QStringLiteral("STATIC_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("MODULE_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("SHARED_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("OBJECT_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("INTERFACE_LIBRARY"), CMakeTarget::Library}
    };
    const auto value = target.value(QLatin1String("type")).toString();
    return s_types.value(value, CMakeTarget::Custom);
}

void CMakeServerImportJob::processCodeModel(const QJsonObject &response, CMakeProjectData &data)
{
    const auto configs = response.value(QStringLiteral("configurations")).toArray();
    qCDebug(CMAKE) << "process response" << response;

    data.targets.clear();
    data.compilationData.files.clear();
    const auto rt = KDevelop::ICore::self()->runtimeController()->currentRuntime();
    for (const auto &config: configs) {
        const auto projects = config.toObject().value(QStringLiteral("projects")).toArray();
        for (const auto &project: projects) {
            const auto targets = project.toObject().value(QStringLiteral("targets")).toArray();
            for (const auto &targetObject: targets) {
                const auto target = targetObject.toObject();
                const KDevelop::Path targetDir = rt->pathInHost(KDevelop::Path(target.value(QStringLiteral("sourceDirectory")).toString()));

                CMakeTarget cmakeTarget{
                    typeToEnum(target),
                    target.value(QStringLiteral("name")).toString(),
                    kTransform<KDevelop::Path::List>(target[QLatin1String("artifacts")].toArray(), [](const QJsonValue& val) { return KDevelop::Path(val.toString()); })
                };

                // ensure we don't add the same target multiple times, for different projects
                // cf.: https://bugs.kde.org/show_bug.cgi?id=387095
                auto& dirTargets = data.targets[targetDir];
                if (dirTargets.contains(cmakeTarget))
                    continue;
                dirTargets += cmakeTarget;

                const auto fileGroups = target.value(QStringLiteral("fileGroups")).toArray();
                for (const auto &fileGroupValue: fileGroups) {
                    const auto fileGroup = fileGroupValue.toObject();
                    CMakeFile file;
                    file.includes = kTransform<KDevelop::Path::List>(fileGroup.value(QStringLiteral("includePath")).toArray(), [](const QJsonValue& val) { return KDevelop::Path(val.toObject().value(QStringLiteral("path")).toString()); });

                    file.compileFlags = fileGroup.value(QStringLiteral("compileFlags")).toString();
                    file.defines = processDefines(file.compileFlags, fileGroup.value(QStringLiteral("defines")).toArray());

                    const auto sourcesArray = fileGroup.value(QStringLiteral("sources")).toArray();
                    const KDevelop::Path::List sources = kTransform<KDevelop::Path::List>(sourcesArray, [targetDir](const QJsonValue& val) { return KDevelop::Path(targetDir, val.toString()); });
                    for (const auto& source: sources) {
                        // NOTE: we use the canonical file path to prevent issues with symlinks in the path
                        //       leading to lookup failures
                        const auto localFile = rt->pathInHost(source);
                        const auto canonicalFile = QFileInfo(source.toLocalFile()).canonicalFilePath();
                        const auto sourcePath = localFile.toLocalFile() == canonicalFile ? localFile : KDevelop::Path(canonicalFile);
                        data.compilationData.files[sourcePath] = file;
                    }
                    qCDebug(CMAKE) << "registering..." << sources << file;
                }
            }
        }
    }
}

CMakeServerImportJob::CMakeServerImportJob(KDevelop::IProject* project, CMakeServer* server, QObject* parent)
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
            m_data.m_testSuites = CMake::importTestSuites(CMake::currentBuildDir(m_project));
            m_data.m_server = m_server;
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
