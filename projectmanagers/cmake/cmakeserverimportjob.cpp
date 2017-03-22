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
#include <projectmanagers/custommake/makefileresolver/makefileresolver.h>

#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QFileInfo>

#include "debug.h"


template <typename T, typename Q, typename W>
static T kTransform(const Q& list, W func)
{
    T ret;
    ret.reserve(list.size());
    for (auto it = list.constBegin(), itEnd = list.constEnd(); it!=itEnd; ++it)
        ret += func(*it);
    return ret;
}

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

static void processFileData(const QJsonObject &response, CMakeProjectData &data)
{
    const auto configs = response.value(QLatin1String("configurations")).toArray();
    qCDebug(CMAKE) << "process response" << response;
    for (const auto &config: configs) {
        const auto projects = config.toObject().value(QLatin1String("projects")).toArray();
        for (const auto &project: projects) {
            const auto targets = project.toObject().value(QLatin1String("targets")).toArray();
            for (const auto &targetObject: targets) {
                const auto target = targetObject.toObject();
                const KDevelop::Path targetDir(target.value(QLatin1String("sourceDirectory")).toString());

                data.targets[targetDir] += target.value(QLatin1String("name")).toString();

                const auto fileGroups = target.value(QLatin1String("fileGroups")).toArray();
                for (const auto &fileGroupValue: fileGroups) {
                    const auto fileGroup = fileGroupValue.toObject();
                    CMakeFile file;
                    file.includes = kTransform<KDevelop::Path::List>(fileGroup.value(QLatin1String("includePath")).toArray(), [](const QJsonValue& val) { return KDevelop::Path(val.toObject().value(QLatin1String("path")).toString()); });
                    file.defines = processDefines(fileGroup.value(QLatin1String("compileFlags")).toString(), fileGroup.value(QLatin1String("defines")).toArray());

                    const auto sourcesArray = fileGroup.value(QLatin1String("sources")).toArray();
                    const KDevelop::Path::List sources = kTransform<KDevelop::Path::List>(sourcesArray, [targetDir](const QJsonValue& val) { return KDevelop::Path(targetDir, val.toString()); });
                    for (const auto& source: sources) {
                        // NOTE: we use the canonical file path to prevent issues with symlinks in the path
                        //       leading to lookup failures
                        const auto localFile = source.toLocalFile();
                        const auto canonicalFile = QFileInfo(source.toLocalFile()).canonicalFilePath();
                        const auto sourcePath = localFile == canonicalFile ? source : KDevelop::Path(canonicalFile);
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
    const auto responseType = response.value(QLatin1String("type"));
    if (responseType == QLatin1String("reply")) {
        const auto inReplyTo = response.value(QLatin1String("inReplyTo"));
        qCDebug(CMAKE) << "replying..." << inReplyTo;
        if (inReplyTo == QLatin1String("handshake")) {
            m_server->configure({});
        } else if (inReplyTo == QLatin1String("configure")) {
            m_server->compute();
        } else if (inReplyTo == QLatin1String("compute")) {
            m_server->codemodel();
        } else if(inReplyTo == QLatin1String("codemodel")) {
            processFileData(response, m_data);
            emitResult();
        } else {
            qWarning() << "unhandled reply" << response;
        }
    } else if(responseType == QLatin1String("error")) {
        setError(ErrorResponse);
        setErrorText(response.value(QLatin1String("errorMessage")).toString());
        qWarning() << "error!!" << response;
        emitResult();
    } else {
        qWarning() << "unhandled message" << response;
    }
}
