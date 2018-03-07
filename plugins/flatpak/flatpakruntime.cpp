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
#include "debug_flatpak.h"

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
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>

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

static KJob* createExecuteJob(const QStringList &program, const QString &title, const QUrl &wd = {})
{
    OutputExecuteJob* process = new OutputExecuteJob;
    process->setProperties(OutputExecuteJob::DisplayStdout | OutputExecuteJob::DisplayStderr);
    process->setExecuteOnHost(true);
    process->setJobName(title);
    process->setWorkingDirectory(wd);
    *process << program;
    return process;
}

KJob* FlatpakRuntime::createBuildDirectory(const KDevelop::Path &buildDirectory, const KDevelop::Path &file, const QString &arch)
{
    return createExecuteJob(QStringList{ "flatpak-builder", "--arch="+arch, "--build-only", buildDirectory.toLocalFile(), file.toLocalFile() }, i18n("Creating Flatpak %1", file.lastPathSegment()), file.parent().toUrl());
}

FlatpakRuntime::FlatpakRuntime(const KDevelop::Path &buildDirectory, const KDevelop::Path &file, const QString &arch)
    : KDevelop::IRuntime()
    , m_file(file)
    , m_buildDirectory(buildDirectory)
    , m_arch(arch)
{
    refreshJson();
}

FlatpakRuntime::~FlatpakRuntime()
{
}

void FlatpakRuntime::refreshJson()
{
    const auto doc = config();
    const QString sdkName = doc[QLatin1String("sdk")].toString();
    const QString runtimeVersion = doc.value(QLatin1String("runtime-version")).toString();
    const QString usedRuntime = sdkName + QLatin1Char('/') + m_arch + QLatin1Char('/') + runtimeVersion;

    m_sdkPath = KDevelop::Path("/var/lib/flatpak/runtime/" + usedRuntime + "/active/files");
    qCDebug(FLATPAK) << "flatpak runtime path..." << name() << m_sdkPath;
    Q_ASSERT(QFile::exists(m_sdkPath.toLocalFile()));

    m_finishArgs = kTransform<QStringList>(doc["finish-args"].toArray(), [](const QJsonValue& val){ return val.toString(); });
}

void FlatpakRuntime::setEnabled(bool /*enable*/)
{
}

void FlatpakRuntime::startProcess(QProcess* process) const
{
    const QStringList args = m_finishArgs + QStringList{"build", "--talk-name=org.freedesktop.DBus", m_buildDirectory.toLocalFile(), process->program()} << process->arguments();
    process->setProgram("flatpak");
    process->setArguments(args);

    qCDebug(FLATPAK) << "starting qprocess" << process->program() << process->arguments();
    process->start();
}

void FlatpakRuntime::startProcess(KProcess* process) const
{
    process->setProgram(QStringList{ "flatpak" } << m_finishArgs << QStringList{ "build", "--talk-name=org.freedesktop.DBus", m_buildDirectory.toLocalFile() } << process->program());

    qCDebug(FLATPAK) << "starting kprocess" << process->program().join(' ');
    process->start();
}

KJob* FlatpakRuntime::rebuild()
{
    QDir(m_buildDirectory.toLocalFile()).removeRecursively();
    auto job = createBuildDirectory(m_buildDirectory, m_file, m_arch);
    refreshJson();
    return job;
}

QList<KJob*> FlatpakRuntime::exportBundle(const QString &path) const
{
    const auto doc = config();

    QTemporaryDir* dir = new QTemporaryDir(QDir::tempPath()+"/flatpak-tmp-repo");
    if (!dir->isValid() || doc.isEmpty()) {
        qCWarning(FLATPAK) << "Couldn't export:" << path << dir->isValid() << dir->path() << doc.isEmpty();
        return {};
    }

    const QString name = doc[QLatin1String("id")].toString();
    QStringList args = m_finishArgs;
    if (doc.contains("command"))
        args << "--command="+doc["command"].toString();
    const QList<KJob*> jobs = {
        createExecuteJob(QStringList{ "flatpak", "build-finish", m_buildDirectory.toLocalFile()} << args, {}),
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

KJob * FlatpakRuntime::executeOnDevice(const QString& host, const QString &path) const
{
    const QString name = config()[QLatin1String("id")].toString();
    const QString destPath = QStringLiteral("/tmp/kdevelop-test-app.flatpak");
    const QString replicatePath = QStringLiteral("/tmp/replicate.sh");
    const QString localReplicatePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("kdevflatpak/replicate.sh"));
    const QString process;

    const QList<KJob*> jobs = exportBundle(path) << QList<KJob*> {
        createExecuteJob({ "scp", path, host+QLatin1Char(':')+destPath}, i18n("Transferring flatpak to %1", host)),
        createExecuteJob({ "scp", localReplicatePath, host+QLatin1Char(':')+replicatePath}, i18n("Transferring replicate.sh to %1", host)),
        createExecuteJob({ "ssh", host, "flatpak", "install", "--user", "--bundle", destPath}, i18n("Installing %1 to %2", name, host)),
        createExecuteJob({ "ssh", host, "bash", replicatePath, "plasmashell", "flatpak", "run", name }, i18n("Running %1 on %2", name, host)),
    };
    return new KDevelop::ExecuteCompositeJob( parent(), jobs );
}

QJsonObject FlatpakRuntime::config(const KDevelop::Path& path)
{
    QFile f(path.toLocalFile());
    if (!f.open(QIODevice::ReadOnly)) {
        qCWarning(FLATPAK) << "couldn't open" << path;
        return {};
    }

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error) {
        qCWarning(FLATPAK) << "couldn't parse" << path << error.errorString();
        return {};
    }

    return doc.object();
}

QJsonObject FlatpakRuntime::config() const
{
    return config(m_file);
}

Path FlatpakRuntime::pathInHost(const KDevelop::Path& runtimePath) const
{
    KDevelop::Path ret = runtimePath;
    if (runtimePath.isLocalFile() && runtimePath.segments().at(0) == QLatin1String("usr")) {
        const auto relpath = KDevelop::Path("/usr").relativePath(runtimePath);
        ret = Path(m_sdkPath, relpath);
    } else if (runtimePath.isLocalFile() && runtimePath.segments().at(0) == QLatin1String("app")) {
        const auto relpath = KDevelop::Path("/app").relativePath(runtimePath);
        ret = Path(m_buildDirectory, "/active/files/" + relpath);
    }

    qCDebug(FLATPAK) << "path in host" << runtimePath << ret;
    return ret;
}

Path FlatpakRuntime::pathInRuntime(const KDevelop::Path& localPath) const
{
    KDevelop::Path ret = localPath;
    if (m_sdkPath.isParentOf(localPath)) {
        const auto relpath = m_sdkPath.relativePath(localPath);
        ret = Path(Path("/usr"), relpath);
    } else {
        const Path bdfiles(m_buildDirectory, "/active/flies");
        if (bdfiles.isParentOf(localPath)) {
            const auto relpath = bdfiles.relativePath(localPath);
            ret = Path(Path("/app"), relpath);
        }
    }

    qCDebug(FLATPAK) << "path in runtime" << localPath << ret;
    return ret;
}

QByteArray FlatpakRuntime::getenv(const QByteArray& varname) const
{
    return qgetenv(varname);
}
