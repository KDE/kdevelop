/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesonimportjob.h"
#include "mesonconfig.h"
#include "mesonmanager.h"
#include <util/path.h>
#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>

#include <QtConcurrentRun>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

using namespace KDevelop;

MesonImportJob::MesonImportJob(MesonManager* manager, KDevelop::IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
    , m_manager(manager)
{
    connect(&m_futureWatcher, &QFutureWatcher<QJsonObject>::finished, this, &MesonImportJob::importFinished);
}

//TODO: probably want to process the object in this function (e.g. see CMakeImportJsonJob)
QJsonObject import(const KDevelop::Path &commandsFile, const KDevelop::Path &sourcePath, const KDevelop::Path &builddir)
{
    QFile f(commandsFile.toLocalFile());
    if (!f.open(QFile::ReadOnly)) {
        return {};
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error) {
        qDebug() << "error parsing" << commandsFile << error.errorString();
        return {};
    }
    return doc.object();
}

void MesonImportJob::start()
{
    const Path currentBuildDir(Meson::buildDirectory(m_project));
    const Path commandsFile(currentBuildDir, QLatin1String("compile_commands.json"));
    const auto sourceDir = m_project->path();

    auto rt = ICore::self()->runtimeController()->currentRuntime();
    auto future = QtConcurrent::run(import, commandsFile, sourceDir, rt->pathInRuntime(currentBuildDir));
    m_futureWatcher.setFuture(future);
}

void MesonImportJob::importFinished()
{
    auto future = m_futureWatcher.future();
    auto data = future.result();
    m_manager->setProjectData(m_project, data);
}
