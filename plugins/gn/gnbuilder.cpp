/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>
    Copyright 2021 BogDan Vatra <bogdan@kde.org>

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

#include "gnbuilder.h"

#include "gnconfig.h"
#include "gnjob.h"
#include "gnjobprune.h"
#include "gnmanager.h"
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

        output->appendLine(i18n("    *** GN ERROR ***\n"));
        QStringList lines = m_error.split(QLatin1Char('\n'));
        output->appendLines(lines);

        setError(!m_error.isEmpty());
        setErrorText(m_error);
        emitResult();
    }

private:
    QString m_error;
};

GNBuilder::GNBuilder(QObject* parent)
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

GNBuilder::DirectoryStatus GNBuilder::evaluateBuildDirectory(const Path& path)
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

    QStringList configured = {QStringLiteral("build.ninja"), QStringLiteral("project.json") };

    // Check if this is a CONFIGURED gn directory
    for (const auto& i : configured) {
        Path curr = path;
        curr.addPath(i);
        QFileInfo currFI(curr.toLocalFile());
        if (!currFI.exists()) {
            return GN_FAILED_CONFIGURATION;
        }
    }

    return GN_CONFIGURED;
}

KJob* GNBuilder::configure(IProject* project, const GN::BuildDir& buildDir,
                              DirectoryStatus status)
{
    Q_ASSERT(project);

    if (!buildDir.isValid()) {
        return new ErrorJob(this, i18n("The current build directory for %1 is invalid", project->name()));
    }

    if (status == ___UNDEFINED___) {
        status = evaluateBuildDirectory(buildDir.buildDir);
    }

    KJob* job = nullptr;

    switch (status) {
    case DOES_NOT_EXIST:
    case CLEAN:
    case GN_FAILED_CONFIGURATION:
    case GN_CONFIGURED:
        job = new GNJob(buildDir, project, this);
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
            i18n("The directory '%1' cannot be used as a gn build directory", buildDir.buildDir.toLocalFile()));
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

KJob* GNBuilder::configure(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    auto buildDir = GN::currentBuildDir(project);
    if (!buildDir.isValid()) {
        auto* bsm = project->buildSystemManager();
        auto* manager = dynamic_cast<GNManager*>(bsm);
        if (!manager) {
            return new ErrorJob(this, i18n("Internal error: The buildsystem manager is not the GNManager"));
        }

        KJob* newBDJob = manager->newBuildDirectory(project);
        if (!newBDJob) {
            return new ErrorJob(this, i18n("Failed to create a new build directory"));
        }
        return newBDJob;
    }
    return configure(project, buildDir, {});
}

KJob* GNBuilder::configureIfRequired(KDevelop::IProject* project, KJob* realJob)
{
    Q_ASSERT(project);
    GN::BuildDir buildDir = GN::currentBuildDir(project);
    DirectoryStatus status = evaluateBuildDirectory(buildDir.buildDir);
    KJob* configureJob = nullptr;
    if (buildDir.isValid()) {
        configureJob = configure(project, buildDir, status);
    } else {
        // Create a new build directory
        auto* bsm = project->buildSystemManager();
        auto* manager = dynamic_cast<GNManager*>(bsm);
        if (!manager) {
            return new ErrorJob(this, i18n("Internal error: The buildsystem manager is not the GNManager"));
        }

        configureJob = manager->newBuildDirectory(project);
        if (!configureJob) {
            return new ErrorJob(this, i18n("Failed to create a new build directory"));
        }
    }

    QList<KJob*> jobs = {
        configure(project, buildDir, status), // First configure the build directory
        realJob // If this succeeds execute the real job
    };

    return new ExecuteCompositeJob(this, jobs);
}

KJob* GNBuilder::build(KDevelop::ProjectBaseItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_ninjaBuilder);
    return configureIfRequired(item->project(), m_ninjaBuilder->build(item));
}

KJob* GNBuilder::clean(KDevelop::ProjectBaseItem* item)
{
    Q_ASSERT(item);
    Q_ASSERT(m_ninjaBuilder);
    return configureIfRequired(item->project(), m_ninjaBuilder->clean(item));
}

KJob* GNBuilder::install(KDevelop::ProjectBaseItem* item, const QUrl& installPath)
{
    Q_ASSERT(item);
    Q_ASSERT(m_ninjaBuilder);
    return configureIfRequired(item->project(), m_ninjaBuilder->install(item, installPath));
}

KJob* GNBuilder::prune(KDevelop::IProject* project)
{
    Q_ASSERT(project);
    GN::BuildDir buildDir = GN::currentBuildDir(project);
    if (!buildDir.isValid()) {
        qCWarning(KDEV_GN) << "The current build directory is invalid";
        return new ErrorJob(this, i18n("The current build directory for %1 is invalid", project->name()));
    }

    KJob* job = new GNJobPrune(buildDir, this);
    connect(job, &KJob::result, this, [this, project]() { emit pruned(project); });
    return job;
}

QList<IProjectBuilder*> GNBuilder::additionalBuilderPlugins(IProject*) const
{
    return { m_ninjaBuilder };
}

bool GNBuilder::hasError() const
{
    return !m_errorString.isEmpty();
}

QString GNBuilder::errorDescription() const
{
    return m_errorString;
}

#include "gnbuilder.moc"
