/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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

static KJob* createExecuteJob(const QStringList &program, const QString &title, const QUrl &wd = {}, bool checkExitCode = true)
{
    auto* process = new OutputExecuteJob;
    process->setProperties(OutputExecuteJob::DisplayStdout | OutputExecuteJob::DisplayStderr);
    process->setExecuteOnHost(true);
    process->setJobName(title);
    process->setWorkingDirectory(wd);
    process->setCheckExitCode(checkExitCode);
    // TODO: call process->setStandardToolView(IOutputView::?); to prevent creating a new tool view for each
    // job in OutputJob::startOutput(). Such nonstandard and unshared tool views are also not configurable.
    *process << program;
    return process;
}

KJob* FlatpakRuntime::createBuildDirectory(const KDevelop::Path &buildDirectory, const KDevelop::Path &file, const QString &arch)
{
    return createExecuteJob(QStringList{QStringLiteral("flatpak-builder"), QLatin1String("--arch=") + arch,
                                        QStringLiteral("--build-only"), QStringLiteral("--force-clean"),
                                        QStringLiteral("--ccache"), buildDirectory.toLocalFile(), file.toLocalFile()},
                            i18n("Flatpak"), file.parent().toUrl());
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

    //First check if local user has flatpak runtime before checking system runtimes.
    m_sdkPath = KDevelop::Path(QDir::homePath() + QLatin1String("/.local/share/flatpak/runtime/") + usedRuntime + QLatin1String("/active/files"));
    if(!QFile::exists(m_sdkPath.toLocalFile())) {
        m_sdkPath = KDevelop::Path(QLatin1String("/var/lib/flatpak/runtime/") + usedRuntime + QLatin1String("/active/files"));
    }
    qCDebug(FLATPAK) << "flatpak runtime path..." << name() << m_sdkPath;
    Q_ASSERT(QFile::exists(m_sdkPath.toLocalFile()));

    m_finishArgs = kTransform<QStringList>(doc[QLatin1String("finish-args")].toArray(), [](const QJsonValue& val){ return val.toString(); });
}

void FlatpakRuntime::setEnabled(bool /*enable*/)
{
}

//Take any environment variables specified in process to pass through to flatpak.
static QStringList envVarsForProcess(const QProcess* process)
{
    QStringList env_args;
    const QStringList env_vars = process->processEnvironment().toStringList();
    const QStringList system = QProcessEnvironment::systemEnvironment().toStringList();
    for (const QString& env_var : env_vars) {
        if (!system.contains(
                env_var)) { // Filter out the ones inherited from the system, let flatpak decide what to do there
            env_args << QLatin1String("--env=") + env_var;
        }
    }
    return env_args;
}

void FlatpakRuntime::startProcess(QProcess* process) const
{
    const QStringList args = envVarsForProcess(process)
        << QStringList{QStringLiteral("--run"), m_buildDirectory.toLocalFile(), m_file.toLocalFile(),
                       process->program()}
        << process->arguments();
    process->setProgram(QStringLiteral("flatpak-builder"));
    process->setArguments(args);

    qCDebug(FLATPAK) << "starting qprocess" << process->program() << process->arguments();
    process->start();
}

void FlatpakRuntime::startProcess(KProcess* process) const
{
    process->setProgram(QStringList{QStringLiteral("flatpak-builder")}
                        << envVarsForProcess(process)
                        << QStringList{QStringLiteral("--run"), m_buildDirectory.toLocalFile(), m_file.toLocalFile()}
                        << process->program());

    qCDebug(FLATPAK) << "starting kprocess" << process->program().join(QLatin1Char(' '));
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

    auto* dir = new QTemporaryDir(QDir::tempPath()+QLatin1String("/flatpak-tmp-repo"));
    if (!dir->isValid() || doc.isEmpty()) {
        qCWarning(FLATPAK) << "Couldn't export:" << path << dir->isValid() << dir->path() << doc.isEmpty();
        return {};
    }

    const QString name = doc[QLatin1String("id")].toString();
    QStringList args = m_finishArgs;
    if (doc.contains(QLatin1String("command")))
        args << QLatin1String("--command=")+doc[QLatin1String("command")].toString();

    const QString title = i18n("Bundling");
    const QList<KJob*> jobs = {
        createExecuteJob(QStringList{QStringLiteral("flatpak"), QStringLiteral("build-finish"), m_buildDirectory.toLocalFile()} << args, title, {}, false),
        createExecuteJob(QStringList{QStringLiteral("flatpak"), QStringLiteral("build-export"), QLatin1String("--arch=")+m_arch, dir->path(), m_buildDirectory.toLocalFile()}, title),
        createExecuteJob(QStringList{QStringLiteral("flatpak"), QStringLiteral("build-bundle"), QLatin1String("--arch=")+m_arch, dir->path(), path, name }, title)
    };
    connect(jobs.last(), &QObject::destroyed, jobs.last(), [dir]() { delete dir; });
    return jobs;
}

QString FlatpakRuntime::name() const
{
    return QStringLiteral("%1 - %2").arg(m_arch, config()[u"id"].toString());
}

KJob * FlatpakRuntime::executeOnDevice(const QString& host, const QString &path) const
{
    const QString name = config()[QLatin1String("id")].toString();
    const QString destPath = QStringLiteral("/tmp/kdevelop-test-app.flatpak");
    const QString replicatePath = QStringLiteral("/tmp/replicate.sh");
    const QString localReplicatePath = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("kdevflatpak/replicate.sh"));

    const QString title = i18n("Run on Device");
    const QList<KJob*> jobs = exportBundle(path) << QList<KJob*> {
        createExecuteJob({QStringLiteral("scp"), path, host+QLatin1Char(':')+destPath}, title),
        createExecuteJob({QStringLiteral("scp"), localReplicatePath, host+QLatin1Char(':')+replicatePath}, title),
        createExecuteJob({QStringLiteral("ssh"), host, QStringLiteral("flatpak"), QStringLiteral("install"), QStringLiteral("--user"), QStringLiteral("--bundle"), QStringLiteral("-y"), destPath}, title),
        createExecuteJob({QStringLiteral("ssh"), host, QStringLiteral("bash"), replicatePath, QStringLiteral("plasmashell"), QStringLiteral("flatpak"), QStringLiteral("run"), name }, title),
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
    if (!runtimePath.isLocalFile()) {
        return ret;
    }

    const auto prefix = runtimePath.segments().at(0);
    if (prefix == QLatin1String("usr")) {
        const auto relpath = KDevelop::Path(QStringLiteral("/usr")).relativePath(runtimePath);
        ret = Path(m_sdkPath, relpath);
    } else if (prefix == QLatin1String("app")) {
        const auto relpath = KDevelop::Path(QStringLiteral("/app")).relativePath(runtimePath);
        ret = Path(m_buildDirectory, QLatin1String("/active/files/") + relpath);
    }

    qCDebug(FLATPAK) << "path in host" << runtimePath << ret;
    return ret;
}

Path FlatpakRuntime::pathInRuntime(const KDevelop::Path& localPath) const
{
    KDevelop::Path ret = localPath;
    if (m_sdkPath.isParentOf(localPath)) {
        const auto relpath = m_sdkPath.relativePath(localPath);
        ret = Path(Path(QStringLiteral("/usr")), relpath);
    } else {
        const Path bdfiles(m_buildDirectory, QStringLiteral("/active/files"));
        if (bdfiles.isParentOf(localPath)) {
            const auto relpath = bdfiles.relativePath(localPath);
            ret = Path(Path(QStringLiteral("/app")), relpath);
        }
    }

    qCDebug(FLATPAK) << "path in runtime" << localPath << ret;
    return ret;
}

QString FlatpakRuntime::findExecutable(const QString& executableName) const
{
    QStringList rtPaths;

    auto envPaths = getenv(QByteArrayLiteral("PATH")).split(':');
    std::transform(envPaths.begin(), envPaths.end(), std::back_inserter(rtPaths),
                    [this](QByteArray p) {
                        return pathInHost(Path(QString::fromLocal8Bit(p))).toLocalFile();
                    });

    return QStandardPaths::findExecutable(executableName, rtPaths);
}

QByteArray FlatpakRuntime::getenv(const QByteArray& varname) const
{
    if (varname == "KDEV_DEFAULT_INSTALL_PREFIX")
        return "/app";
    return qgetenv(varname.constData());
}

KDevelop::Path FlatpakRuntime::buildPath() const
{
    auto file = m_file;
    file.setLastPathSegment(QStringLiteral(".flatpak-builder"));
    file.addPath(QStringLiteral("kdevelop"));
    return file;
}

#include "moc_flatpakruntime.cpp"
