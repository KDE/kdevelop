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

#include <projectmanagers/custommake/makefileresolver/makefileresolver.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <interfaces/iproject.h>

#include <QJsonDocument>
#include <QtConcurrentRun>
#include <QFutureWatcher>

using namespace KDevelop;

namespace {

QList<QUrl> fromLocalPaths(const QStringList &urls)
{
    QList<QUrl> lst;
    lst.reserve(urls.size());
    foreach (const QString &str, urls) {
        lst.append(QUrl::fromLocalFile(str));
    }
    return lst;
}

CMakeFile dataFromJson(const QVariantMap& entry)
{
    MakeFileResolver resolver;
    PathResolutionResult result = resolver.processOutput(entry["command"].toString(), entry["directory"].toString());

    CMakeFile ret;
    ret.includes = KDevelop::toPathList(fromLocalPaths(result.paths));
    return ret;
}

CMakeJsonData import(const KUrl& commandsFile)
{
    // NOTE: to get compile_commands.json, you need -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    QFile f(commandsFile.toLocalFile());
    bool r = f.open(QFile::ReadOnly|QFile::Text);
    if(!r) {
        kDebug() << "Couldn't open commands file" << commandsFile;
        return {};
    }

    kDebug() << "Found commands file" << commandsFile;

    CMakeJsonData data;
    QJsonDocument parser;
    QJsonParseError error;
    QVariantList values = parser.fromJson(f.readAll(), &error).toVariant().toList();
    Q_ASSERT(error.error == QJsonParseError::NoError);

    foreach(const QVariant& v, values) {
        QVariantMap entry = v.toMap();
        data.files[Path(entry["file"].toString())] = dataFromJson(entry);
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
    connect(m_futureWatcher, SIGNAL(finished()), SLOT(importFinished()));
}

void CMakeImportJob::start()
{
    auto commandsFile = CMake::commandsFile(project());
    if (!QFileInfo(commandsFile.toLocalFile()).exists()) {
        setError(FileMissingError);
        setErrorText(tr("Could not import CMake project ('compile_commands.json' missing)"));
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
        setError(ReadError);
        setErrorText(tr("Could not import CMake project ('compile_commands.json' invalid)"));
        emitResult();
        return;
    }

    m_data = data;

    kDebug() << "Done importing, found" << m_data.files.count() << "entries for" << project()->path();
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
