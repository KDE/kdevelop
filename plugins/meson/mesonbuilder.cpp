/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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

#include "mesonbuilder.h"

#include "mesonconfig.h"
#include "mesonjob.h"
#include "mesonjobprune.h"
#include "mesonmanager.h"
#include <debug.h>

#include <executecompositejob.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <outputview/outputexecutejob.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include <KLocalizedString>

#include <QDir>
#include <QFileInfo>

using namespace KDevelop;

class ErrorJob : public OutputJob
{
    Q_OBJECT
public:
    ErrorJob(QObject* parent, const QString& error)
        : OutputJob(parent)
        , m_error(error)
    {
        setStandardToolView(IOutputView::BuildView);
    }

    void start() override
    {
        auto* output = new OutputModel(this);
        setModel(output);
        startOutput();

        output->appendLine(i18n("    *** MESON ERROR ***\n"));
        QStringList lines = m_error.split(QChar::fromLatin1('\n'));
        output->appendLines(lines);

        setError(!m_error.isEmpty());
        setErrorText(m_error);
        emitResult();
    }

private:
    QString m_error;
};

MesonBuilder::MesonBuilder(QObject* parent)
    : QObject(parent)
{
    auto p = KDevelop::ICore::self()->pluginController()->pluginForExtension(
        QStringLiteral("org.kdevelop.IProjectBuilder"), QStringLiteral("KDevNinjaBuilder"));
    if (p) {
        m_ninjaBuilder = p->extension<KDevelop::IProjectBuilder>();
        if (m_ninjaBuilder) {
            connect(p, SIGNAL(built(KDevelop::ProjectBaseItem*)), this, SIGNAL(built(KDevelop::ProjectBaseItem*)));
            connect(p, SIGNAL(installed(KDevelop::ProjectBaseItem*)), this,
                    SIGNAL(installed(KDevelop::ProjectBaseItem*)));
            connect(p, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)), this, SIGNAL(cleaned(KDevelop::ProjectBaseItem*)));
            connect(p, SIGNAL(failed(KDevelop::ProjectBaseItem*)), this, SIGNAL(failed(KDevelop::ProjectBaseItem*)));
        } else {
            m_errorString = i18n("Failed to set the internally used Ninja builder");
        }
    } else {
        m_errorString = i18n("Failed to acquire the Ninja builder plugin");
    }
}

MesonBuilder::DirectoryStatus MesonBuilder::evaluateBuildDirectory(const Path& path, QString const& backend)
{
    QString pathSTR = path.toLocalFile();
    if (pathSTR.isEmpty()) {
        return EMPTY_STRING;
    }

    QFileInfo info(pathSTR);
    if (!info.exists()) {
        return DOES_NOT_EXIST;
    }

    if (!info.isDir() || !info.isReadable() || !info.isWritable()) {
        return INVALID_BUILD_DIR;
    }

    QDir dir(path.toLocalFile());
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    if (dir.isEmpty(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllEntries)) {
        return CLEAN;
    }
#else
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllEntries);
    if (dir.count() == 0) {
        return CLEAN;
    }
#endif

    // Check if the directory is a meson directory
    const static QStringList mesonPaths = { QStringLiteral("meson-logs"), QStringLiteral("meson-private") };
    for (auto const& i : mesonPaths) {
        Path curr = path;
        curr.addPath(i);
        QFileInfo currFI(curr.toLocalFile());
        if (!currFI.exists()) {
            return DIR_NOT_EMPTY;
        }
    }

    // Also check if the meson configuration succeeded. This should be the case if the backend file exists.
    // Meson actually checks for meson-private/coredata.dat, this might change in the future.
    // see: https://github.com/mesonbuild/meson/blob/master/mesonbuild/msetup.py#L117
    QStringList configured = {};
    if (backend == QStringLiteral("ninja")) {
        configured << QStringLiteral("build.ninja");
    }

    // Check if this is a CONFIGURED meson directory
    for (auto const& i : configured) {
        Path curr = path;
        curr.addPath(i);
        QFileInfo currFI(curr.toLocalFile());
        if (!currFI.exists()) {
            return MESON_FAILED_CONFIGURATION;
        }
    }

    return MESON_CONFIGURED;
}

KJob* MesonBuilder::configure(IProject* project, const Meson::BuildDir& buildDir, QStringList args,
                              DirectoryStatus status)
{
    Q_ASSERT(project);

    if (!buildDir.isValid()) {
        return new ErrorJob(this, i18n("The current build directory for %1 is invalid", project->name()));
    }

    if (status == ___UNDEFINED___) {
        status = evaluateBuildDirectory(buildDir.buildDir, buildDir.mesonBackend);
    }

    KJob* job = nullptr;

    switch (status) {
    case DOES_NOT_EXIST:
    case CLEAN:
    case MESON_FAILED_CONFIGURATION:
        job = new MesonJob(buildDir, project, MesonJob::CONFIGURE, args, this);
        connect(job, &KJob::result, this, [this, project]() { emit configured(project); });
        return job;
    case MESON_CONFIGURED:
        job = new MesonJob(buildDir, project, MesonJob::RE_CONFIGURE, args, this);
        connect(job, &KJob::result, this, [this, project]() { emit configured(project); });
        return job;
    case DIR_NOT_EMPTY:
        return new ErrorJob(
            this,
            i18n("The directory '%1' is not empty and does not seem to be an already configured build directory",
                 buildDir.buildDir.toLocalFile()));
    case INVALID_BUILD_DIR:
        return new ErrorJob(
            this,
            i18n("The directory '%1' cannot be used as a meson build directory", buildDir.buildDir.toLocalFile()));
    case EMPTY_STRING:
        return new ErrorJob(
            this, i18n("The current build configuration is broken, because the build directory is not specified"));
    default:
        // This code should NEVER be reached
        return new ErrorJob(this,
                            i18n("Congratulations: You have reached unreachable code!\n"
                                 "Please report a bug at https://bugs.kde.org/\n"
                                 "FILE: %1:%2",
                                 QStringLiteral(__FILE__), __LINE__));
    }
}

KJob* MesonBuilder::configure(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    auto buildDir = Meson::currentBuildDir(project);
    if(!buildDir.isValid()) {
        auto *bsm = project->buildSystemManager();
        MesonManager *manager = dynamic_cast<MesonManager *>(bsm);
        if(!manager) {
            return new ErrorJob(this, i18n("Internal error: The buildsystem manager is not the MesonManager"));
        }

        KJob *newBDJob = manager->newBuildDirectory(project);
        if(!newBDJob) {
            return new ErrorJob(this, i18n("Failed to create a new build directory"));
        }
        return newBDJob;
    }
    return configure(project, buildDir, {});
}

KJob* MesonBuilder::configureIfRequired(KDevelop::IProject* project, KJob* realJob)
{
    Q_ASSERT(project);
    Meson::BuildDir buildDir = Meson::currentBuildDir(project);
    DirectoryStatus status = evaluateBuildDirectory(buildDir.buildDir, buildDir.mesonBackend);

    if (status == MESON_CONFIGURED) {
        return realJob;
    }

    KJob *configureJob = nullptr;
    if(buildDir.isValid()) {
        configureJob = configure(project, buildDir, {}, status);
    } else {
        // Create a new build directory
        auto *bsm = project->buildSystemManager();
        MesonManager *manager = dynamic_cast<MesonManager *>(bsm);
        if(!manager) {
            return new ErrorJob(this, i18n("Internal error: The buildsystem manager is not the MesonManager"));
        }

        configureJob = manager->newBuildDirectory(project);
        if(!configureJob) {
            return new ErrorJob(this, i18n("Failed to create a new build directory"));
        }
    }

    QList<KJob*> jobs = {
        configure(project, buildDir, {}, status), // First configure the build directory
        realJob // If this succeeds execute the real job
    };

    return new ExecuteCompositeJob(this, jobs);
}

KJob* MesonBuilder::build(KDevelop::ProjectBaseItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_ninjaBuilder);
    return configureIfRequired(item->project(), m_ninjaBuilder->build(item));
}

KJob* MesonBuilder::clean(KDevelop::ProjectBaseItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_ninjaBuilder);
    return configureIfRequired(item->project(), m_ninjaBuilder->clean(item));
}

KJob* MesonBuilder::install(KDevelop::ProjectBaseItem* item, const QUrl& installPath)
{
    Q_ASSERT(item);
    Q_ASSERT(m_ninjaBuilder);
    return configureIfRequired(item->project(), m_ninjaBuilder->install(item, installPath));
}

KJob* MesonBuilder::prune(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    Meson::BuildDir buildDir = Meson::currentBuildDir(project);
    if (!buildDir.isValid()) {
        qCWarning(KDEV_Meson) << "The current build directory is invalid";
        return new ErrorJob(this, i18n("The current build directory for %1 is invalid", project->name()));
    }

    KJob* job = new MesonJobPrune(buildDir, this);
    connect(job, &KJob::result, this, [this, project]() { emit pruned(project); });
    return job;
}

QList<IProjectBuilder*> MesonBuilder::additionalBuilderPlugins(IProject*) const
{
    return { m_ninjaBuilder };
}

bool MesonBuilder::hasError() const
{
    return m_errorString.size() > 0;
}

QString MesonBuilder::errorDescription() const
{
    return m_errorString;
}

#include "mesonbuilder.moc"
