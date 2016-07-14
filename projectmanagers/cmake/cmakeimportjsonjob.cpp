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

#include <KShell>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QRegularExpression>

using namespace KDevelop;

namespace {

CMakeJsonData importCommands(const Path& commandsFile)
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

        PathResolutionResult result = resolver.processOutput(entry[KEY_COMMAND].toString(), entry[KEY_DIRECTORY].toString());

        CMakeFile ret;
        ret.includes = result.paths;
        ret.frameworkDirectories = result.frameworkDirectories;
        ret.defines = result.defines;
        // NOTE: we use the canonical file path to prevent issues with symlinks in the path
        //       leading to lookup failures
        const auto path = Path(QFileInfo(entry[KEY_FILE].toString()).canonicalFilePath());
        data.files[path] = ret;
    }

    data.isValid = true;
    return data;
}

QVector<Test> importTestSuites(const Path &buildDir)
{
    QVector<Test> ret;
#pragma message("TODO use subdirs instead of this")
    foreach(const QFileInfo &info, QDir(buildDir.toLocalFile()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        ret += importTestSuites(Path(buildDir, info.fileName()));
    }

    QFile file(buildDir.toLocalFile()+"/CTestTestfile.cmake");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ret;
    }

    const QRegularExpression rx("add_test *\\((.+?) (.*?)\\) *$");
    Q_ASSERT(rx.isValid());
    for (; !file.atEnd();) {
        QByteArray line = file.readLine();
        line.chop(1);
        const auto match = rx.match(QString::fromLocal8Bit(line));
        if (match.hasMatch()) {
            Test test;
            QStringList args = KShell::splitArgs(match.captured(2));
            test.name = match.captured(1);
            test.executable = Path(buildDir, args.takeFirst());
            test.arguments = args;
            ret += test;
        }
    }

    return ret;
}

ImportData import(const Path& commandsFile, const Path &targetsFilePath, const QString &sourceDir, const KDevelop::Path &buildPath)
{
    return ImportData {
        importCommands(commandsFile),
        CMake::enumerateTargets(targetsFilePath, sourceDir, buildPath),
        importTestSuites(buildPath)
    };
}

}

CMakeImportJob::CMakeImportJob(IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
{
    connect(&m_futureWatcher, &QFutureWatcher<ImportData>::finished, this, &CMakeImportJob::importFinished);
}

CMakeImportJob::~CMakeImportJob()
{
}

void CMakeImportJob::start()
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

    auto future = QtConcurrent::run(import, commandsFile, targetsFilePath, sourceDir, currentBuildDir);
    m_futureWatcher.setFuture(future);
}

void CMakeImportJob::importFinished()
{
    Q_ASSERT(m_project->thread() == QThread::currentThread());
    Q_ASSERT(m_futureWatcher.isFinished());

    auto future = m_futureWatcher.future();
    auto data = future.result();
    if (!data.json.isValid) {
        qCWarning(CMAKE) << "Could not import CMake project ('compile_commands.json' invalid)";
        emitResult();
        return;
    }

    m_data = data.json;
    m_targets = data.targets;
    m_testSuites = data.testSuites;
    qCDebug(CMAKE) << "Done importing, found" << m_data.files.count() << "entries for" << project()->path();

    emitResult();
}

IProject* CMakeImportJob::project() const
{
    return m_project;
}

CMakeJsonData CMakeImportJob::jsonData() const
{
    Q_ASSERT(!m_futureWatcher.isRunning());
    return m_data;
}

#include "moc_cmakeimportjsonjob.cpp"
