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

#include <makefileresolver/makefileresolver.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>

#include <KShell>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QRegularExpression>

using namespace KDevelop;

namespace {

CMakeFilesCompilationData importCommands(const Path& commandsFile)
{
    // NOTE: to get compile_commands.json, you need -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    QFile f(commandsFile.toLocalFile());
    bool r = f.open(QFile::ReadOnly|QFile::Text);
    if(!r) {
        qCWarning(CMAKE) << "Couldn't open commands file" << commandsFile;
        return {};
    }

    qCDebug(CMAKE) << "Found commands file" << commandsFile;

    CMakeFilesCompilationData data;
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
    auto rt = ICore::self()->runtimeController()->currentRuntime();
    const auto values = document.array();
    for (const QJsonValue& value : values) {
        if (!value.isObject()) {
            qCWarning(CMAKE) << "JSON command file entry is not an object:" << value;
            continue;
        }
        const QJsonObject entry = value.toObject();
        if (!entry.contains(KEY_FILE) || !entry.contains(KEY_COMMAND) || !entry.contains(KEY_DIRECTORY)) {
            qCWarning(CMAKE) << "JSON command file entry does not contain required keys:" << entry;
            continue;
        }

        PathResolutionResult result = resolver.processOutput(entry[KEY_COMMAND].toString(), entry[KEY_DIRECTORY].toString());

        auto convert = [rt](const Path &path) { return rt->pathInHost(path); };

        CMakeFile ret;
        ret.includes = kTransform<Path::List>(result.paths, convert);
        ret.frameworkDirectories = kTransform<Path::List>(result.frameworkDirectories, convert);
        ret.defines = result.defines;
        const Path path(rt->pathInHost(Path(entry[KEY_FILE].toString())));
        qCDebug(CMAKE) << "entering..." << path << entry[KEY_FILE];
        data.files[path] = ret;
    }

    data.isValid = true;
    return data;
}

ImportData import(const Path& commandsFile, const Path &targetsFilePath, const QString &sourceDir, const KDevelop::Path &buildPath)
{
    QHash<KDevelop::Path, QVector<CMakeTarget>> cmakeTargets;

    //we don't have target type information in json, so we just announce all of them as exes
    const auto targets = CMake::enumerateTargets(targetsFilePath, sourceDir, buildPath);
    for(auto it = targets.constBegin(), itEnd = targets.constEnd(); it!=itEnd; ++it) {
        cmakeTargets[it.key()] = kTransform<QVector<CMakeTarget>>(*it, [](const QString &targetName) { return CMakeTarget{CMakeTarget::Executable, targetName}; });
    }

    return ImportData {
        importCommands(commandsFile),
        cmakeTargets,
        CMake::importTestSuites(buildPath)
    };
}

}

CMakeImportJsonJob::CMakeImportJsonJob(IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
    , m_data({})
{
    connect(&m_futureWatcher, &QFutureWatcher<ImportData>::finished, this, &CMakeImportJsonJob::importCompileCommandsJsonFinished);
}

CMakeImportJsonJob::~CMakeImportJsonJob()
{}

void CMakeImportJsonJob::start()
{
    auto commandsFile = CMake::commandsFile(project());
    if (!QFileInfo::exists(commandsFile.toLocalFile())) {
        qCWarning(CMAKE) << "Could not import CMake project" << project()->path() << "('compile_commands.json' missing)";
        emitResult();
        return;
    }

    const Path currentBuildDir = CMake::currentBuildDir(m_project);
    Q_ASSERT (!currentBuildDir.isEmpty());

    const Path targetsFilePath = CMake::targetDirectoriesFile(m_project);
    const QString sourceDir = m_project->path().toLocalFile();
    auto rt = ICore::self()->runtimeController()->currentRuntime();

    auto future = QtConcurrent::run(import, commandsFile, targetsFilePath, sourceDir, rt->pathInRuntime(currentBuildDir));
    m_futureWatcher.setFuture(future);
}

void CMakeImportJsonJob::importCompileCommandsJsonFinished()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());
    Q_ASSERT(m_futureWatcher.isFinished());

    auto future = m_futureWatcher.future();
    auto data = future.result();
    if (!data.compilationData.isValid) {
        qCWarning(CMAKE) << "Could not import CMake project ('compile_commands.json' invalid)";
        emitResult();
        return;
    }

    m_data = CMakeProjectData {data.targets, data.compilationData, data.testSuites};
    qCDebug(CMAKE) << "Done importing, found" << data.compilationData.files.count() << "entries for" << project()->path();

    emitResult();
}

IProject* CMakeImportJsonJob::project() const
{
    return m_project;
}

CMakeProjectData CMakeImportJsonJob::projectData() const
{
    Q_ASSERT(!m_futureWatcher.isRunning());
    return m_data;
}

#include "moc_cmakeimportjsonjob.cpp"
