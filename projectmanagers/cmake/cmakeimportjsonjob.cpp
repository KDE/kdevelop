/* KDevelop CMake Support
 *
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
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

#include "cmakeimportjsonjob.h"

#include "cmakeutils.h"
#include "cmakeprojectdata.h"
#include "cmakemodelitems.h"
#include "debug.h"

#include <projectmanagers/custommake/makefileresolver/makefileresolver.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <interfaces/iproject.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrentRun>
#include <QFutureWatcher>

using namespace KDevelop;

namespace {

CMakeJsonData import(const Path& commandsFile)
{
    // NOTE: to get compile_commands.json, you need -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    QFile f(commandsFile.toLocalFile());
    bool r = f.open(QFile::ReadOnly|QFile::Text);
    if(!r) {
        qCWarning(CMAKE) << "Couldn't open commands file" << commandsFile;
        return {};
    }

    qCDebug(CMAKE) << "Found commands file" << commandsFile;

    CMakeJsonData data;
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error) {
        qCWarning(CMAKE) << "Failed to parse JSON in commands file:" << error.errorString() << commandsFile;
        data.isValid = false;
        return data;
    } else if (!document.isArray()) {
        qCWarning(CMAKE) << "JSON document in commands file is not an array: " << commandsFile;
        data.isValid = false;
        return data;
    }

    MakeFileResolver resolver;
    static const QString KEY_COMMAND = QStringLiteral("command");
    static const QString KEY_DIRECTORY = QStringLiteral("directory");
    static const QString KEY_FILE = QStringLiteral("file");
    foreach(const QJsonValue& value, document.array()) {
        if (!value.isObject()) {
            qCWarning(CMAKE) << "JSON command file entry is not an object:" << value;
            continue;
        }
        const QJsonObject entry = value.toObject();
        if (!entry.contains(KEY_FILE) || !entry.contains(KEY_COMMAND) || !entry.contains(KEY_DIRECTORY)) {
            qCWarning(CMAKE) << "JSON command file entry does not contain required keys:" << entry;
            continue;
        }

        PathResolutionResult result = resolver.processOutput(entry["command"].toString(), entry["directory"].toString());

        CMakeFile ret;
        ret.includes = result.paths;
        ret.defines = result.defines;
        data.files[Path(entry["file"].toString())] = ret;
    }

    data.isValid = true;
    return data;
}

}

CMakeImportJob::CMakeImportJob(IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
    , m_futureWatcher(new QFutureWatcher<CMakeJsonData>)
{
    connect(m_futureWatcher, &QFutureWatcher<CMakeJsonData>::finished, this, &CMakeImportJob::importFinished);
}

void CMakeImportJob::start()
{
    auto commandsFile = CMake::commandsFile(project());
    if (!QFileInfo(commandsFile.toLocalFile()).exists()) {
        qCWarning(CMAKE) << "Could not import CMake project ('compile_commands.json' missing)";
        emitResult();
        return;
    }

    auto future = QtConcurrent::run(import, commandsFile);
    m_futureWatcher->setFuture(future);
}

void CMakeImportJob::importFinished()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());
    Q_ASSERT(m_futureWatcher->isFinished());

    auto future = m_futureWatcher->future();
    auto data = future.result();
    if (!data.isValid) {
        qCWarning(CMAKE) << "Could not import CMake project ('compile_commands.json' invalid)";
        emitResult();
        return;
    }

    m_data = data;

    qCDebug(CMAKE) << "Done importing, found" << m_data.files.count() << "entries for" << project()->path();
    emitResult();
}

IProject* CMakeImportJob::project() const
{
    return m_project;
}

CMakeJsonData CMakeImportJob::jsonData() const
{
    Q_ASSERT(!m_futureWatcher->isRunning());
    return m_data;
}

#include "moc_cmakeimportjsonjob.cpp"
