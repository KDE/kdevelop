/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "flatpakruntime.h"

#include <KLocalizedString>
#include <KProcess>
#include <KActionCollection>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <outputview/outputexecutejob.h>

using namespace KDevelop;

KJob* FlatpakRuntime::createBuildDirectory(const KDevelop::Path &buildDirectory, const KDevelop::Path &file)
{
    OutputExecuteJob* process = new OutputExecuteJob;
    process->setExecuteOnHost(true);
    process->setJobName(i18n("Creating Flatpak %1", file.lastPathSegment()));
    *process << QStringList{ "flatpak-builder", "--build-only", buildDirectory.toLocalFile(), file.toLocalFile() };
    return process;
}

FlatpakRuntime::FlatpakRuntime(const KDevelop::Path &buildDirectory, const KDevelop::Path &file)
    : KDevelop::IRuntime()
    , m_file(file)
    , m_buildDirectory(buildDirectory)
{
}

FlatpakRuntime::~FlatpakRuntime()
{
    QDir(m_buildDirectory.toLocalFile()).removeRecursively();
}

void FlatpakRuntime::setEnabled(bool enable)
{
}

void FlatpakRuntime::startProcess(QProcess* process)
{
    const QStringList args = QStringList{"build", "--socket=x11", m_buildDirectory.toLocalFile(), process->program()} << process->arguments();
    process->setProgram("flatpak");
    process->setArguments(args);
    process->start();
}

void FlatpakRuntime::startProcess(KProcess* process)
{
    process->setProgram(QStringList{ "flatpak", "--socket=x11", "build", m_buildDirectory.toLocalFile() } << process->program());

    qDebug() << "yokai!" << process << process->program().join(' ');
    process->start();
}

void FlatpakRuntime::rebuild()
{
    QDir(m_buildDirectory.toLocalFile()).removeRecursively();
    createBuildDirectory(m_buildDirectory, m_file)->start();
}

void FlatpakRuntime::exportBundle(const QString &path)
{
    const QString develName = name().remove(QLatin1String(".json"));

    OutputExecuteJob* process = new OutputExecuteJob;
    process->setExecuteOnHost(true);
    process->setJobName(i18n("Exporting %1", path));
    *process << QStringList{ "flatpak", "build-bundle", m_buildDirectory.toLocalFile(), path, develName };
}
