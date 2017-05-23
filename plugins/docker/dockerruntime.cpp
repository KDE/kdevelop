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
    inspectImage();
}

void DockerRuntime::inspectImage()
{
    QProcess* process = new QProcess(this);
    connect(process, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, [process, this](int code, QProcess::ExitStatus status){
        process->deleteLater();
        qCDebug(DOCKER) << "inspect upper dir" << code << status;
        if (code || status) {
            qCWarning(DOCKER) << "Could not figure out the upperDir of" << m_tag;
            return;
        }
        m_upperDir = Path(QFile::decodeName(process->readAll().trimmed()));
        qCDebug(DOCKER) << "upper dir:" << m_tag << m_upperDir;
    });
    process->start("docker", {"image", "inspect", m_tag, "--format", "{{.GraphDriver.Data.UpperDir}}"});

    QProcess* processEnvs = new QProcess(this);
    connect(processEnvs, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, [processEnvs, this](int code, QProcess::ExitStatus status){
        processEnvs->deleteLater();
        qCDebug(DOCKER) << "inspect envs" << code << status;
        if (code || status) {
            qCWarning(DOCKER) << "Could not figure out the environment variables of" << m_tag;
            return;
        }

        const auto list = processEnvs->readAll();
        const auto data = list.mid(1, list.size()-3);
        const auto entries = data.split(' ');

        for (auto entry : entries) {
            const auto content = entry.split('=');
            if (content.count() != 2)
                continue;
            m_envs.insert(content[0], content[1]);
        }

        qCDebug(DOCKER) << "envs:" << m_tag << m_envs;
    });
    processEnvs->start("docker", {"image", "inspect", m_tag, "--format", "{{.Config.Env}}"});
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
        m_userUpperDir = KDevelop::Path(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/docker-" + QString(m_tag).replace('/', '_'));
        QDir().mkpath(m_userUpperDir.toLocalFile());

        const QStringList cmd = {"pkexec", "bindfs", "--map=root/"+KUser().loginName(), m_upperDir.toLocalFile(), m_userUpperDir.toLocalFile() };
        QProcess p;
        p.start(cmd.first(), cmd.mid(1));
        p.waitForFinished();
        if (p.exitCode()) {
            qCDebug(DOCKER) << "bindfs returned" << m_upperDir << m_userUpperDir << cmd << p.exitCode() << p.readAll();
        }
    } else {
        int code = QProcess::execute(QStringLiteral("pkexec"), {"umount", m_userUpperDir.toLocalFile()});
        qCDebug(DOCKER) << "umount returned" << code;
    }
}

static QString ensureEndsSlash(const QString &string)
{
    return string.endsWith('/') ? string : (string + QLatin1Char('/'));
}

static QStringList projectVolumes()
{
    QStringList ret;
    const QString dir = ensureEndsSlash(DockerRuntime::s_settings->projectsVolume());
    const QString buildDir = ensureEndsSlash(DockerRuntime::s_settings->buildDirsVolume());

    for (IProject* project: ICore::self()->projectController()->projects()) {
        const Path path = project->path();
        if (path.isLocalFile()) {
            ret << "--volume" << QStringLiteral("%1:%2").arg(path.toLocalFile(), dir + path.lastPathSegment());
        }

        const auto ibsm = project->buildSystemManager();
        if (ibsm) {
            ret << "--volume" << ibsm->buildDirectory(project->projectItem()).toLocalFile() + QLatin1Char(':') +  buildDir + path.lastPathSegment();
        }
    }
    return ret;
}

QStringList DockerRuntime::workingDirArgs(QProcess* process) const
{
    const auto wd = process->workingDirectory();
    return wd.isEmpty() ? QStringList{} : QStringList{"-w", pathInRuntime(KDevelop::Path(wd)).toLocalFile()};
}

void DockerRuntime::startProcess(QProcess* process) const
{
    auto program = process->program();
    if (program.contains('/'))
        program = pathInRuntime(Path(program)).toLocalFile();

    const QStringList args = QStringList{"run", "--rm"} << workingDirArgs(process) << KShell::splitArgs(s_settings->extraArguments()) << projectVolumes() << m_tag << program << process->arguments();
    process->setProgram("docker");
    process->setArguments(args);

    qCDebug(DOCKER) << "starting qprocess" << process->program() << process->arguments();
    process->start();
}

void DockerRuntime::startProcess(KProcess* process) const
{
    auto program = process->program();
    if (program[0].contains('/'))
        program[0] = pathInRuntime(Path(program[0])).toLocalFile();
    process->setProgram(QStringList{ "docker", "run", "--rm" } << workingDirArgs(process) << KShell::splitArgs(s_settings->extraArguments()) << projectVolumes() << m_tag << program);

    qCDebug(DOCKER) << "starting kprocess" << process->program().join(' ');
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
            ret = KDevelop::Path(m_userUpperDir, KDevelop::Path(QStringLiteral("/")).relativePath(runtimePath));
    }
    qCDebug(DOCKER) << "pathInHost" << ret << runtimePath;
    return ret;
}

KDevelop::Path DockerRuntime::pathInRuntime(const KDevelop::Path& localPath) const
{
    if (m_userUpperDir==localPath || m_userUpperDir.isParentOf(localPath)) {
        KDevelop::Path ret(KDevelop::Path("/"), m_userUpperDir.relativePath(localPath));
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
        qCWarning(DOCKER) << "only project files are available on the docker runtime" << localPath;
    }
    qCDebug(DOCKER) << "bypass..." << localPath;
    return localPath;
}

