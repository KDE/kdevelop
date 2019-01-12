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

#include "mesonmanager.h"
#include "mesonbuilder.h"
#include "mesonconfig.h"
#include "mesonimportjob.h"
#include "settings/mesonconfigpage.h"
#include "settings/mesonnewbuilddir.h"
#include <interfaces/iproject.h>
#include <project/projectconfigpage.h>
#include <project/projectmodel.h>
#include <util/executecompositejob.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QFileDialog>
#include <QStandardPaths>

#include "debug.h"

using namespace KDevelop;

static const QString GENERATOR_NINJA = QStringLiteral("ninja");

K_PLUGIN_FACTORY_WITH_JSON(MesonSupportFactory, "kdevmesonmanager.json", registerPlugin<MesonManager>();)

MesonManager::MesonManager(QObject* parent, const QVariantList& args)
    : AbstractFileManagerPlugin(QStringLiteral("KDevMesonManager"), parent, args)
    , m_builder(new MesonBuilder(this))
{
    if (m_builder->hasError()) {
        setErrorDescription(i18n("Meson builder error: %1", m_builder->errorDescription()));
    }
}

MesonManager::~MesonManager()
{
    delete m_builder;
}

// *********************************
// * AbstractFileManagerPlugin API *
// *********************************

IProjectFileManager::Features MesonManager::features() const
{
    return IProjectFileManager::Files | IProjectFileManager::Folders | IProjectFileManager::Targets;
}

ProjectFolderItem* MesonManager::createFolderItem(IProject* project, const Path& path, ProjectBaseItem* parent)
{
    // TODO: Maybe use meson targets instead
    if (QFile::exists(path.toLocalFile() + QStringLiteral("/meson.build")))
        return new ProjectBuildFolderItem(project, path, parent);
    else
        return AbstractFileManagerPlugin::createFolderItem(project, path, parent);
}

// ***************************
// * IBuildSystemManager API *
// ***************************

KJob* MesonManager::createImportJob(ProjectFolderItem* item)
{
    auto project = item->project();
    auto job = new MesonImportJob(this, project, this);

    connect(job, &KJob::result, this, [this, job, project]() {
        if (job->error() != 0) {
            qCWarning(KDEV_Meson) << "couldn't load project successfully" << project->name();
            m_projects.remove(project);
        }
    });

    const QList<KJob*> jobs = {
        builder()->configure(project), // Make sure the project is configured
        job, // Import the compile_commands.json file
        AbstractFileManagerPlugin::createImportJob(item) // generate the file system listing
    };

    Q_ASSERT(!jobs.contains(nullptr));
    auto composite = new ExecuteCompositeJob(this, jobs);
    composite->setAbortOnError(false);
    return composite;
}

Path MesonManager::buildDirectory(ProjectBaseItem* item) const
{
    Q_ASSERT(item);
    Meson::BuildDir buildDir = Meson::currentBuildDir(item->project());
    return buildDir.buildDir;
}

IProjectBuilder* MesonManager::builder() const
{
    return m_builder;
}

KJob *MesonManager::newBuildDirectory(IProject* project)
{
    Q_ASSERT(project);
    MesonNewBuildDir newBD(project);

    if (!newBD.exec() || !newBD.isConfigValid()) {
        qCWarning(KDEV_Meson) << "Failed to create new build directory for project " << project->name();
        return nullptr;
    }

    Meson::BuildDir buildDir = newBD.currentConfig();
    Meson::MesonConfig mesonCfg = Meson::getMesonConfig(project);
    buildDir.canonicalizePaths();
    mesonCfg.currentIndex = mesonCfg.addBuildDir(buildDir);
    Meson::writeMesonConfig(project, mesonCfg);

    return m_builder->configure(project, buildDir, newBD.mesonArgs());
}

QStringList MesonManager::supportedMesonBackends() const
{
    // Maybe add support for other generators
    return { GENERATOR_NINJA };
}

QString MesonManager::defaultMesonBackend() const
{
    return GENERATOR_NINJA;
}

Path MesonManager::findMeson() const
{
    QString mesonPath;

    const static QStringList mesonExecutables = { QStringLiteral("meson"), QStringLiteral("meson.py") };
    const static QStringList mesonPaths
        = { QStringLiteral("%1/.local/bin").arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]) };

    for (auto const& i : mesonExecutables) {
        mesonPath = QStandardPaths::findExecutable(i);
        if (!mesonPath.isEmpty()) {
            break;
        }

        mesonPath = QStandardPaths::findExecutable(i, mesonPaths);
        if (!mesonPath.isEmpty()) {
            break;
        }
    }

    return Path(mesonPath);
}

void MesonManager::setProjectData(IProject* project, const QJsonObject& data)
{
    m_projects[project] = data;
}

ConfigPage* MesonManager::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number == 0) {
        return new MesonConfigPage(this, options.project, parent);
    }
    return nullptr;
}

int MesonManager::perProjectConfigPages() const
{
    return 1;
}

#include "mesonmanager.moc"
