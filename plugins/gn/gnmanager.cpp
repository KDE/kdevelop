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

#include "gnmanager.h"

#include "debug.h"
#include "gnbuilder.h"
#include "gnconfig.h"
#include "gnimportprojectjob.h"
#include "settings/gnconfigpage.h"
#include "settings/gnnewbuilddir.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/itestcontroller.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>
#include <util/executecompositejob.h>

#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

#include <algorithm>

using namespace KDevelop;
using namespace std;

K_PLUGIN_FACTORY_WITH_JSON(GNSupportFactory, "kdevgnmanager.json", registerPlugin<GNManager>();)

// ********************************
// * Error job for failed imports *
// ********************************

namespace mmanager_internal
{

class ErrorJob : public KJob
{
    Q_OBJECT
public:
    ErrorJob(QObject* parent, const QString& error)
        : KJob(parent)
        , m_error(error)
    {
    }

    void start() override
    {
        QMessageBox::critical(nullptr, i18nc("@title:window", "Project Import Failed"), m_error);

        setError(KJob::UserDefinedError + 1); // Indicate that there was an error
        setErrorText(m_error);
        emitResult();
    }

private:
    QString m_error;
};

}

using namespace mmanager_internal;

// ***************
// * Constructor *
// ***************

GNManager::GNManager(QObject* parent, const QVariantList& args)
    : AbstractFileManagerPlugin(QStringLiteral("KDevGNManager"), parent, args)
    , m_builder(new GNBuilder(this))
{
    if (m_builder->hasError()) {
        setErrorDescription(i18n("GN builder error: %1", m_builder->errorDescription()));
    }
}

GNManager::~GNManager()
{
    delete m_builder;
}

// *********************************
// * AbstractFileManagerPlugin API *
// *********************************

IProjectFileManager::Features GNManager::features() const
{
    return IProjectFileManager::None;
}

ProjectFolderItem* GNManager::createFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    // TODO: Maybe use gn targets instead
    if (QFile::exists(path.toLocalFile() + QStringLiteral("/gn.build")))
        return new ProjectBuildFolderItem(project, path, parent);
    else
        return AbstractFileManagerPlugin::createFolderItem(project, path, parent);
}

bool GNManager::reload(KDevelop::ProjectFolderItem* item)
{
    // "Inspired" by CMakeManager::reload

    IProject* project = item->project();
    if (!project->isReady()) {
        return false;
    }

    qCDebug(KDEV_GN) << "reloading gn project" << project->name() << "; Path:" << item->path();

    KJob* job = createImportJob(item);
    project->setReloadJob(job);
    ICore::self()->runController()->registerJob(job);
    if (item == project->projectItem()) {
        connect(job, &KJob::finished, this, [project](KJob* job) -> void {
            if (job->error()) {
                return;
            }

            emit KDevelop::ICore::self()->projectController()->projectConfigurationChanged(project);
            KDevelop::ICore::self()->projectController()->reparseProject(project);
        });
    }

    return true;
}


QList<ProjectTargetItem*> GNManager::targets(ProjectFolderItem* item) const
{
    Q_ASSERT(item);
    QList<ProjectTargetItem*> res = item->targetList();
    for (ProjectFolderItem* i : item->folderList()) {
        res << targets(i);
    }
    return res;
}

void GNManager::onGNInfoChanged(QString path, QString projectName)
{
    qCDebug(KDEV_GN) << "File" << path << "changed --> reparsing project";
    IProject* foundProject = ICore::self()->projectController()->findProjectByName(projectName);
    if (!foundProject) {
        return;
    }

    QList<KJob*> jobs {builder()->configure(foundProject), createImportJob(foundProject->projectItem())};
    Q_ASSERT(!jobs.contains(nullptr));
    auto composite = new ExecuteCompositeJob(this, jobs);
    composite->setAbortOnError(false);
    foundProject->setReloadJob(composite);
    ICore::self()->runController()->registerJob(composite);
    connect(composite, &KJob::finished, this, [foundProject](KJob* job) -> void {
        if (job->error()) {
            return;
        }

        emit KDevelop::ICore::self()->projectController()->projectConfigurationChanged(foundProject);
        KDevelop::ICore::self()->projectController()->reparseProject(foundProject);
    });
}

KJob* GNManager::createImportJob(ProjectFolderItem* item)
{
    IProject* project = item->project();
    Q_ASSERT(project);

    qCDebug(KDEV_GN) << "Importing project" << project->name();

    auto buildDir = GN::currentBuildDir(project);

    KJob* configureJob = nullptr;
    if (!buildDir.isValid()) {
        configureJob = newBuildDirectory(project, &buildDir);
        if (!configureJob) {
            QString error = i18n("Importing %1 failed because no build directory could be created.", project->name());
            qCDebug(KDEV_GN) << error;
            return new ErrorJob(this, error);
        }
    }

    auto importJob = new GNImportProjectJob(project, buildDir, this);

    KDirWatchPtr watcher = m_projectWatchers[project] = make_shared<KDirWatch>(nullptr);
    QString projectName = project->name();
    connect(watcher.get(), &KDirWatch::dirty, this, [=](QString p) { onGNInfoChanged(p, projectName); });
    connect(watcher.get(), &KDirWatch::created, this, [=](QString p) { onGNInfoChanged(p, projectName); });

    connect(importJob, &KJob::result, this, [watcher, importJob]() {
        importJob->importProject(watcher);
    });

    QList<KJob*> jobs;

    // Configure the project if necessary
    if (!configureJob
        && m_builder->evaluateBuildDirectory(buildDir.buildDir)
            != GNBuilder::GN_CONFIGURED) {
        configureJob = builder()->configure(project);
    }

    if (configureJob) {
        jobs << configureJob;
    }

    jobs << importJob;

    Q_ASSERT(!jobs.contains(nullptr));
    auto composite = new ExecuteCompositeJob(this, jobs);
    composite->setAbortOnError(false);
    return composite;
}

Path GNManager::buildDirectory(ProjectBaseItem* item) const
{
    Q_ASSERT(item);
    GN::BuildDir buildDir = GN::currentBuildDir(item->project());
    return buildDir.buildDir;
}

IProjectBuilder* GNManager::builder() const
{
    return m_builder;
}


KDevelop::Path::List GNManager::includeDirectories(KDevelop::ProjectBaseItem* item) const
{
    if (auto data = dynamic_cast<GNTargetData*>(item)) {
        return data->includeDirectories();
    }
    return {};
}

KDevelop::Path::List GNManager::frameworkDirectories(KDevelop::ProjectBaseItem* item) const
{
    if (auto data = dynamic_cast<GNTargetData*>(item)) {
        return data->frameworkDirectories();
    }
    return {};

}

QHash<QString, QString> GNManager::defines(KDevelop::ProjectBaseItem* item) const
{
    if (auto data = dynamic_cast<GNTargetData*>(item)) {
        return data->defines();
    }
    return {};
}

QString GNManager::extraArguments(KDevelop::ProjectBaseItem* item) const
{
    if (auto data = dynamic_cast<GNTargetData*>(item)) {
        return data->extraArguments();
    }
    return {};
}

bool GNManager::hasBuildInfo(KDevelop::ProjectBaseItem* item) const
{
    return dynamic_cast<GNTargetData*>(item);
}

KDevelop::Path GNManager::compiler(KDevelop::ProjectTargetItem* item) const
{
    if (auto data = dynamic_cast<GNTargetData*>(item)) {
        return data->compiler();
    }
    return {};
}

// ********************
// * Custom functions *
// ********************

KJob* GNManager::newBuildDirectory(IProject* project, GN::BuildDir* outBuildDir)
{
    Q_ASSERT(project);
    GNNewBuildDir newBD(project);

    if (!newBD.exec() || !newBD.isConfigValid()) {
        qCWarning(KDEV_GN) << "Failed to create new build directory for project " << project->name();
        return nullptr;
    }

    GN::BuildDir buildDir = newBD.currentConfig();
    GN::GNConfig gnCfg = GN::getGNConfig(project);
    buildDir.canonicalizePaths();
    gnCfg.currentIndex = gnCfg.addBuildDir(buildDir);
    GN::writeGNConfig(project, gnCfg);

    if (outBuildDir) {
        *outBuildDir = buildDir;
    }

    // FIXME: ?!?!
//    return m_builder->configure(project, buildDir, newBD.gnArgs());
    return m_builder->configure(project, buildDir);
}

Path GNManager::findGN() const
{
    QString gnPath;

    const static QStringList gnExecutables = { QStringLiteral("gn") };
    const static QStringList gnPaths
        = { QStringLiteral("%1/.local/bin").arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]) };

    for (const auto& i : gnExecutables) {
        gnPath = QStandardPaths::findExecutable(i);
        if (!gnPath.isEmpty()) {
            break;
        }

        gnPath = QStandardPaths::findExecutable(i, gnPaths);
        if (!gnPath.isEmpty()) {
            break;
        }
    }

    return Path(gnPath);
}

// ***********
// * IPlugin *
// ***********

ConfigPage* GNManager::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    switch (number) {
    case 0:
        return new GNConfigPage(this, options.project, parent);
    }
    return nullptr;
}

int GNManager::perProjectConfigPages() const
{
    return 1;
}

#include "gnmanager.moc"
