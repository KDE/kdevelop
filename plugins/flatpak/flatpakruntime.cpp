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
#include "flatpakplugin.h"

#include <util/executecompositejob.h>
#include <outputview/outputexecutejob.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/icore.h>

#include <KLocalizedString>
#include <KProcess>
#include <KActionCollection>
#include <QProcess>
#include <QTemporaryDir>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

using namespace KDevelop;

template <typename T, typename Q, typename W>
static T kTransform(const Q& list, W func)
{
    T ret;
    ret.reserve(list.size());
    for (auto it = list.constBegin(), itEnd = list.constEnd(); it!=itEnd; ++it)
        ret += func(*it);
    return ret;
}

static KJob* createExecuteJob(const QStringList &program, const QString &title)
{
    OutputExecuteJob* process = new OutputExecuteJob;
    process->setExecuteOnHost(true);
    process->setJobName(title);
    *process << program;
    return process;
}

KJob* FlatpakRuntime::createBuildDirectory(const KDevelop::Path &buildDirectory, const KDevelop::Path &file, const QString &arch)
{
    return createExecuteJob(QStringList{ "flatpak-builder", "--arch="+arch, "--build-only", buildDirectory.toLocalFile(), file.toLocalFile() }, i18n("Creating Flatpak %1", file.lastPathSegment()));
}

FlatpakRuntime::FlatpakRuntime(const KDevelop::Path &buildDirectory, const KDevelop::Path &file, const QString &arch)
    : KDevelop::IRuntime()
    , m_file(file)
    , m_buildDirectory(buildDirectory)
    , m_arch(arch)
{
}

FlatpakRuntime::~FlatpakRuntime()
{
    QDir(m_buildDirectory.toLocalFile()).removeRecursively();
}

void FlatpakRuntime::setEnabled(bool /*enable*/)
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

KJob* FlatpakRuntime::rebuild()
{
    QDir(m_buildDirectory.toLocalFile()).removeRecursively();
    auto job = createBuildDirectory(m_buildDirectory, m_file, m_arch);
    return job;
}

QList<KJob*> FlatpakRuntime::exportBundle(const QString &path)
{
    const auto doc = config();

    QTemporaryDir* dir = new QTemporaryDir(QDir::tempPath()+"/flatpak-tmp-repo");
    if (!dir->isValid() || doc.isEmpty()) {
        qWarning() << "Couldn't export:" << path << dir->isValid() << dir->path() << doc.isEmpty();
        return {};
    }

    const QString name = doc[QLatin1String("id")].toString();
    const QList<KJob*> jobs = {
        createExecuteJob(QStringList{ "flatpak", "build-finish", m_buildDirectory.toLocalFile()} << kTransform<QStringList>(doc["finish-args"].toArray(), [](const QJsonValue& val){ return val.toString(); }), {}),
        createExecuteJob(QStringList{ "flatpak", "build-export", "--arch="+m_arch, dir->path(), m_buildDirectory.toLocalFile()}, {}),
        createExecuteJob(QStringList{ "flatpak", "build-bundle", "--arch="+m_arch, dir->path(), path, name }, i18n("Exporting %1", path))
    };
    connect(jobs.last(), &QObject::destroyed, jobs.last(), [dir]() { delete dir; });
    return jobs;
}

QString FlatpakRuntime::name() const
{
    return m_file.lastPathSegment() + QLatin1Char(':') + m_arch;
}

KJob * FlatpakRuntime::executeOnDevice(const QString& host, const QString &path)
{
    const QString name = config()[QLatin1String("id")].toString();
    const QString destPath = QStringLiteral("/tmp/kdevelop-test-app.flatpak");

//     const QString copyPlasmashellEnvironment = QStringLiteral("`export $(xargs --null --max-args=1 < /proc/$(pidof plasmashell)/environ)`");

    const QList<KJob*> jobs = exportBundle(path) << QList<KJob*> {
        createExecuteJob({ "scp", path, host+QLatin1Char(':')+destPath}, i18n("Transferring to %1", host)),
        createExecuteJob({ "ssh", host, "flatpak", "install", "--user", "--bundle", destPath}, i18n("Installing %1 to %2", name, host)),
        createExecuteJob({ "ssh", host, "flatpak", "run", name }, i18n("Running %1 on %2", name, host)),
    };
    return new KDevelop::ExecuteCompositeJob( this, jobs );
}

QJsonObject FlatpakRuntime::config() const
{
    QFile f(m_file.toLocalFile());
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "couldn't open" << m_file;
        return {};
    }

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error) {
        qDebug() << "couldn't parse" << m_file << error.errorString();
        return {};
    }

    return doc.object();
}
