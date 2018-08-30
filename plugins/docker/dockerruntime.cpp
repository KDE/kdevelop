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

#include "dockerruntime.h"
#include "dockerpreferencessettings.h"
#include "debug_docker.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <KLocalizedString>
#include <KProcess>
#include <KActionCollection>
#include <KShell>
#include <KUser>
#include <QProcess>
#include <QDir>
#include <outputview/outputexecutejob.h>

using namespace KDevelop;

DockerPreferencesSettings* DockerRuntime::s_settings = nullptr;

DockerRuntime::DockerRuntime(const QString &tag)
    : KDevelop::IRuntime()
    , m_tag(tag)
{
    setObjectName(tag);
}

void DockerRuntime::inspectContainer()
{
    QProcess* process = new QProcess(this);
    connect(process, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, [process, this](int code, QProcess::ExitStatus status){
        process->deleteLater();
        qCDebug(DOCKER) << "inspect container" << code << status;
        if (code || status) {
            qCWarning(DOCKER) << "Could not figure out the container" << m_container;
            return;
        }
        const QJsonArray arr = QJsonDocument::fromJson(process->readAll()).array();
        const QJsonObject obj = arr.constBegin()->toObject();

        const QJsonObject objGraphDriverData = obj.value(QLatin1String("GraphDriver")).toObject().value(QLatin1String("Data")).toObject();
        m_mergedDir = Path(objGraphDriverData.value(QLatin1String("MergedDir")).toString());
        qCDebug(DOCKER) << "mergeddir:" << m_container << m_mergedDir;

        const auto& entries = obj[QLatin1String("Config")].toObject()[QLatin1String("Env")].toArray();
        for (const auto& entry : entries) {
            const auto content = entry.toString().split(QLatin1Char('='));
            if (content.count() != 2)
                continue;
            m_envs.insert(content[0].toLocal8Bit(), content[1].toLocal8Bit());
        }
        qCDebug(DOCKER) << "envs:" << m_container << m_envs;
    });
    process->start(QStringLiteral("docker"), {QStringLiteral("container"), QStringLiteral("inspect"), m_container});
    process->waitForFinished();
    qDebug() << "inspecting" << QStringList{QStringLiteral("container"), QStringLiteral("inspect"), m_container} << process->exitCode();
}

DockerRuntime::~DockerRuntime()
{
}

QByteArray DockerRuntime::getenv(const QByteArray& varname) const
{
    return m_envs.value(varname);
}

void DockerRuntime::setEnabled(bool enable)
{
    if (enable) {
        m_userMergedDir = KDevelop::Path(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/docker-") + QString(m_tag).replace(QLatin1Char('/'), QLatin1Char('_')));
        QDir().mkpath(m_userMergedDir.toLocalFile());

        QProcess pCreateContainer;
        pCreateContainer.start(QStringLiteral("docker"), {QStringLiteral("run"), QStringLiteral("-d"), m_tag, QStringLiteral("tail"), QStringLiteral("-f"), QStringLiteral("/dev/null")});
        pCreateContainer.waitForFinished();
        if (pCreateContainer.exitCode()) {
            qCWarning(DOCKER) << "could not create the container" << pCreateContainer.readAll();
        }
        m_container = QString::fromUtf8(pCreateContainer.readAll().trimmed());

        inspectContainer();

        const QStringList cmd = {QStringLiteral("pkexec"), QStringLiteral("bindfs"), QLatin1String("--map=root/")+KUser().loginName(), m_mergedDir.toLocalFile(), m_userMergedDir.toLocalFile() };
        QProcess p;
        p.start(cmd.first(), cmd.mid(1));
        p.waitForFinished();
        if (p.exitCode()) {
            qCDebug(DOCKER) << "bindfs returned" << cmd << p.exitCode() << p.readAll();
        }
    } else {
        int codeContainer = QProcess::execute(QStringLiteral("docker"), {QStringLiteral("kill"), m_container});
        qCDebug(DOCKER) << "docker kill returned" << codeContainer;

        int code = QProcess::execute(QStringLiteral("pkexec"), {QStringLiteral("umount"), m_userMergedDir.toLocalFile()});
        qCDebug(DOCKER) << "umount returned" << code;

        m_container.clear();
    }
}

static QString ensureEndsSlash(const QString &string)
{
    return string.endsWith(QLatin1Char('/')) ? string : (string + QLatin1Char('/'));
}

static QStringList projectVolumes()
{
    QStringList ret;
    const QString dir = ensureEndsSlash(DockerRuntime::s_settings->projectsVolume());
    const QString buildDir = ensureEndsSlash(DockerRuntime::s_settings->buildDirsVolume());

    const auto& projects = ICore::self()->projectController()->projects();
    for (IProject* project : projects) {
        const Path path = project->path();
        if (path.isLocalFile()) {
            ret << QStringLiteral("--volume") << QStringLiteral("%1:%2").arg(path.toLocalFile(), dir + project->name());
        }

        const auto ibsm = project->buildSystemManager();
        if (ibsm) {
            ret << QStringLiteral("--volume") << ibsm->buildDirectory(project->projectItem()).toLocalFile() + QLatin1Char(':') +  buildDir + project->name();
        }
    }
    return ret;
}

QStringList DockerRuntime::workingDirArgs(QProcess* process) const
{
    const auto wd = process->workingDirectory();
    return wd.isEmpty() ? QStringList{} : QStringList{QStringLiteral("-w"), pathInRuntime(KDevelop::Path(wd)).toLocalFile()};
}

void DockerRuntime::startProcess(QProcess* process) const
{
    auto program = process->program();
    if (program.contains(QLatin1Char('/')))
        program = pathInRuntime(Path(program)).toLocalFile();

    const QStringList args = QStringList{QStringLiteral("run"), QStringLiteral("--rm")} << workingDirArgs(process) << KShell::splitArgs(s_settings->extraArguments()) << projectVolumes() << m_tag << program << process->arguments();
    process->setProgram(QStringLiteral("docker"));
    process->setArguments(args);

    qCDebug(DOCKER) << "starting qprocess" << process->program() << process->arguments();
    process->start();
}

void DockerRuntime::startProcess(KProcess* process) const
{
    auto program = process->program();
    if (program[0].contains(QLatin1Char('/')))
        program[0] = pathInRuntime(Path(program[0])).toLocalFile();
    process->setProgram(QStringList{QStringLiteral("docker"), QStringLiteral("run"), QStringLiteral("--rm")} << workingDirArgs(process) << KShell::splitArgs(s_settings->extraArguments()) << projectVolumes() << m_tag << program);

    qCDebug(DOCKER) << "starting kprocess" << process->program().join(QLatin1Char(' '));
    process->start();
}

static Path projectRelPath(const KDevelop::Path & projectsDir, const KDevelop::Path& runtimePath, bool sourceDir)
{
    const auto relPath = projectsDir.relativePath(runtimePath);
    const int index = relPath.indexOf(QLatin1Char('/'));
    auto project = ICore::self()->projectController()->findProjectByName(relPath.left(index));

    if (!project) {
        qCWarning(DOCKER) << "No project for" << relPath;
    } else {
        const auto repPathProject = index < 0 ? QString() : relPath.mid(index+1);
        const auto rootPath = sourceDir ? project->path() : project->buildSystemManager()->buildDirectory(project->projectItem());
        return Path(rootPath, repPathProject);
    }
    return {};
}

KDevelop::Path DockerRuntime::pathInHost(const KDevelop::Path& runtimePath) const
{
    Path ret;
    const Path projectsDir(DockerRuntime::s_settings->projectsVolume());
    if (runtimePath==projectsDir || projectsDir.isParentOf(runtimePath)) {
        ret = projectRelPath(projectsDir, runtimePath, true);
    } else {
        const Path buildDirs(DockerRuntime::s_settings->buildDirsVolume());
        if (runtimePath==buildDirs || buildDirs.isParentOf(runtimePath)) {
            ret = projectRelPath(buildDirs, runtimePath, false);
        } else
            ret = KDevelop::Path(m_userMergedDir, KDevelop::Path(QStringLiteral("/")).relativePath(runtimePath));
    }
    qCDebug(DOCKER) << "pathInHost" << ret << runtimePath;
    return ret;
}

KDevelop::Path DockerRuntime::pathInRuntime(const KDevelop::Path& localPath) const
{
    if (m_userMergedDir==localPath || m_userMergedDir.isParentOf(localPath)) {
        KDevelop::Path ret(KDevelop::Path(QStringLiteral("/")), m_userMergedDir.relativePath(localPath));
        qCDebug(DOCKER) << "docker runtime pathInRuntime..." << ret << localPath;
        return ret;
    } else if (auto project = ICore::self()->projectController()->findProjectForUrl(localPath.toUrl())) {
        const Path projectsDir(DockerRuntime::s_settings->projectsVolume());
        const QString relpath = project->path().relativePath(localPath);
        const KDevelop::Path ret(projectsDir, project->name() + QLatin1Char('/') + relpath);
        qCDebug(DOCKER) << "docker user pathInRuntime..." << ret << localPath;
        return ret;
    } else {
        const auto projects = ICore::self()->projectController()->projects();
        for (auto project : projects) {
            auto ibsm = project->buildSystemManager();
            if (ibsm) {
                const auto builddir = ibsm->buildDirectory(project->projectItem());
                if (builddir != localPath && !builddir.isParentOf(localPath))
                    continue;

                const Path builddirs(DockerRuntime::s_settings->buildDirsVolume());
                const QString relpath = builddir.relativePath(localPath);
                const KDevelop::Path ret(builddirs, project->name() + QLatin1Char('/') + relpath);
                qCDebug(DOCKER) << "docker build pathInRuntime..." << ret << localPath;
                return ret;
            }
        }
        qCWarning(DOCKER) << "only project files are accessible on the docker runtime" << localPath;
    }
    qCDebug(DOCKER) << "bypass..." << localPath;
    return localPath;
}

