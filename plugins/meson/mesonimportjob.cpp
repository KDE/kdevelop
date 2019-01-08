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
#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <util/path.h>

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>
#include <debug.h>
#include <klocalizedstring.h>

using namespace KDevelop;

MesonImportJob::MesonImportJob(MesonManager* manager, KDevelop::IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
    , m_manager(manager)
{
    connect(&m_futureWatcher, &QFutureWatcher<QJsonObject>::finished, this, &MesonImportJob::importFinished);
}

// TODO: probably want to process the object in this function (e.g. see CMakeImportJsonJob)
QJsonObject import()
{
    qCDebug(KDEV_Meson) << "import(): This is a stub.";
    return QJsonObject();
}

void MesonImportJob::start()
{
    Meson::BuildDir buildDir = Meson::currentBuildDir(m_project);
    if (!buildDir.isValid()) {
        qCWarning(KDEV_Meson) << "The current build directory is invalid";
        setError(true);
        setErrorText(i18n("The current build directory for %1 is invalid").arg(m_project->name()));
        emitResult();
        return;
    }

    auto future = QtConcurrent::run(import);
    m_futureWatcher.setFuture(future);
}

void MesonImportJob::importFinished()
{
    auto future = m_futureWatcher.future();
    auto data = future.result();
    m_manager->setProjectData(m_project, data);
    emitResult();
}

bool MesonImportJob::doKill()
{
    if (m_futureWatcher.isRunning()) {
        m_futureWatcher.cancel();
    }
    return true;
}
