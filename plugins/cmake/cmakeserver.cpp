/* KDevelop CMake Support
 *
 * Copyright 2017 Aleix Pol <aleixpol@kde.org>
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

#include "cmakeserver.h"
#include "cmakeprojectdata.h"
#include "cmakeutils.h"

#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QTemporaryFile>
#include "debug.h"

CMakeServer::CMakeServer(KDevelop::IProject* project)
    : QObject()
    , m_localSocket(new QLocalSocket(this))
{
    QString path;
    {
        const auto cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir::temp().mkpath(cacheLocation);

        QTemporaryFile file(cacheLocation + QLatin1String("/kdevelopcmake"));
        file.open();
        file.close();
        path = file.fileName();
        Q_ASSERT(!path.isEmpty());
    }

    m_process.setProcessChannelMode(QProcess::ForwardedChannels);

    connect(&m_process, &QProcess::errorOccurred,
            this, [this, path](QProcess::ProcessError error) {
        qCWarning(CMAKE) << "cmake server error:" << error << path << m_process.readAllStandardError() << m_process.readAllStandardOutput();
    });
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [](int code){
        qCDebug(CMAKE) << "cmake server finished with code" << code;
    });
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CMakeServer::finished);

    connect(m_localSocket, &QIODevice::readyRead, this, &CMakeServer::processOutput);
    connect(m_localSocket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error), this, [this, path](QLocalSocket::LocalSocketError socketError) {
        qCWarning(CMAKE) << "cmake server socket error:" << socketError << path;
        setConnected(false);
    });
    connect(m_localSocket, &QLocalSocket::connected, this, [this]() { setConnected(true); });

    connect(&m_process, &QProcess::started, this, [this, path](){
        //Once the process has started, wait for the file to be created, then connect to it
        QTimer::singleShot(1000, this, [this, path]() {
            m_localSocket->connectToServer(path, QIODevice::ReadWrite);
        });
    });
    // we're called with the importing project as our parent, so we can fetch configured
    // cmake executable (project-specific or kdevelop-wide) rather than the system version.
    m_process.setProgram(CMake::currentCMakeExecutable(project).toLocalFile());
    m_process.setArguments({QStringLiteral("-E"), QStringLiteral("server"), QStringLiteral("--experimental"), QLatin1String("--pipe=") + path});
    KDevelop::ICore::self()->runtimeController()->currentRuntime()->startProcess(&m_process);
}

CMakeServer::~CMakeServer()
{
    m_process.disconnect();
    m_process.kill();
    m_process.waitForFinished();
}

void CMakeServer::setConnected(bool conn)
{
    if (conn == m_connected)
        return;

    m_connected = conn;
    if (m_connected)
        Q_EMIT connected();
    else
        Q_EMIT disconnected();
}

bool CMakeServer::isServerAvailable()
{
    return m_localSocket->isOpen();
}

static QByteArray openTag() { return QByteArrayLiteral("\n[== \"CMake Server\" ==[\n"); }
static QByteArray closeTag() { return QByteArrayLiteral("\n]== \"CMake Server\" ==]\n"); }

void CMakeServer::sendCommand(const QJsonObject& object)
{
    Q_ASSERT(isServerAvailable());

    const QByteArray data = openTag() + QJsonDocument(object).toJson(QJsonDocument::Compact) + closeTag();
    auto len = m_localSocket->write(data);
//     qCDebug(CMAKE) << "writing...\n" << QJsonDocument(object).toJson();
    Q_ASSERT(len > 0);
}

void CMakeServer::processOutput()
{
    Q_ASSERT(m_localSocket);

    const auto openTag = ::openTag();
    const auto closeTag = ::closeTag();

    m_buffer += m_localSocket->readAll();
    for(; m_buffer.size() > openTag.size(); ) {

        Q_ASSERT(m_buffer.startsWith(openTag));
        const int idx = m_buffer.indexOf(closeTag, openTag.size());
        if (idx >= 0) {
            emitResponse(m_buffer.mid(openTag.size(), idx - openTag.size()));
            m_buffer.remove(0, idx + closeTag.size());
        } else {
            break;
        }
    }
}

void CMakeServer::emitResponse(const QByteArray& data)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(data, &error);
    if (error.error) {
        qCWarning(CMAKE) << "error processing" << error.errorString() << data;
    }
    Q_ASSERT(doc.isObject());
    Q_EMIT response(doc.object());
}

void CMakeServer::handshake(const KDevelop::Path& source, const KDevelop::Path& build)
{
    Q_ASSERT(!source.isEmpty());

    const QString generatorVariable = QStringLiteral("CMAKE_GENERATOR");
    const QString homeDirectoryVariable = QStringLiteral("CMAKE_HOME_DIRECTORY");
    const QString cacheFileDirectoryVariable = QStringLiteral("CMAKE_CACHEFILE_DIR");
    const auto cacheValues = CMake::readCacheValues(KDevelop::Path(build, QStringLiteral("CMakeCache.txt")),
                                                    {generatorVariable, homeDirectoryVariable, cacheFileDirectoryVariable});

    QString generator = cacheValues.value(generatorVariable);
    if (generator.isEmpty()) {
        generator = CMake::defaultGenerator();
    }

    // prefer pre-existing source directory, see also: https://gitlab.kitware.com/cmake/cmake/issues/16736
    QString sourceDirectory = cacheValues.value(homeDirectoryVariable);
    if (sourceDirectory.isEmpty()) {
        sourceDirectory = source.toLocalFile();
    } else if (QFileInfo(sourceDirectory).canonicalFilePath() != QFileInfo(source.toLocalFile()).canonicalFilePath()) {
        qCWarning(CMAKE) << "Build directory is configured for another source directory:"
                   << homeDirectoryVariable << sourceDirectory
                   << "wanted to open" << source << "in" << build;
    }

    // prefer to reuse the exact same build dir path to prevent useless recompilation
    // when we open a symlinked project path
    QString buildDirectory = cacheValues.value(cacheFileDirectoryVariable);
    if (buildDirectory.isEmpty()) {
        buildDirectory = build.toLocalFile();
    } else if (QFileInfo(buildDirectory).canonicalFilePath() != QFileInfo(build.toLocalFile()).canonicalFilePath()) {
        qCWarning(CMAKE) << "Build directory mismatch:"
                   << cacheFileDirectoryVariable << buildDirectory
                   << "wanted to open" << build;
        buildDirectory = build.toLocalFile();
    }

    qCDebug(CMAKE) << "Using generator" << generator << "for project"
                   << sourceDirectory << "aka" << source
                   << "in" << buildDirectory << "aka" << build;

    sendCommand({
        {QStringLiteral("cookie"), {}},
        {QStringLiteral("type"), QStringLiteral("handshake")},
        {QStringLiteral("major"), 1},
        {QStringLiteral("protocolVersion"), QJsonObject{{QStringLiteral("major"), 1}} },
        {QStringLiteral("sourceDirectory"), sourceDirectory},
        {QStringLiteral("buildDirectory"), buildDirectory},
        {QStringLiteral("generator"), generator}
    });
}

void CMakeServer::configure(const QStringList& args)
{
    sendCommand({
        {QStringLiteral("type"), QStringLiteral("configure")},
        {QStringLiteral("cacheArguments"), QJsonArray::fromStringList(args)}
    });
}

void CMakeServer::compute()
{
    sendCommand({ {QStringLiteral("type"), QStringLiteral("compute")} });
}

void CMakeServer::codemodel()
{
    sendCommand({ {QStringLiteral("type"), QStringLiteral("codemodel")} });
}
