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

#include "debug.h"
#include "mesonbuilder.h"
#include "mesonconfig.h"
#include "mesonintrospectjob.h"
#include "mesontargets.h"
#include "settings/mesonconfigpage.h"
#include "settings/mesonnewbuilddir.h"

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
#include <QStandardPaths>

#include <algorithm>

using namespace KDevelop;
using namespace std;

static const QString GENERATOR_NINJA = QStringLiteral("ninja");

K_PLUGIN_FACTORY_WITH_JSON(MesonSupportFactory, "kdevmesonmanager.json", registerPlugin<MesonManager>();)

// ***********************************
// * Meson specific executable class *
// ***********************************

class MesonProjectExecutableTargetItem final : public ProjectExecutableTargetItem
{
public:
    MesonProjectExecutableTargetItem(IProject* project, const QString& name, ProjectBaseItem* parent, Path buildPath,
                                     Path installPath = Path())
        : ProjectExecutableTargetItem(project, name, parent)
        , m_buildPath(buildPath)
        , m_installPath(installPath)
    {
    }

    QUrl builtUrl() const override { return m_buildPath.toUrl(); }
    QUrl installedUrl() const override { return m_installPath.toUrl(); }

private:
    Path m_buildPath;
    Path m_installPath;
};

// ***************
// * Constructor *
// ***************

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

bool MesonManager::reload(KDevelop::ProjectFolderItem* item)
{
    // "Inspired" by CMakeManager::reload

    IProject* project = item->project();
    if (!project->isReady()) {
        return false;
    }

    qCDebug(KDEV_Meson) << "reloading meson project" << project->name() << "; Path:" << item->path();

    KJob* job = createImportJob(item);
    project->setReloadJob(job);
    ICore::self()->runController()->registerJob(job);
    if (item == project->projectItem()) {
        connect(job, &KJob::finished, this, [project](KJob* job) -> void {
            if (job->error()) {
                return;
            }

            KDevelop::ICore::self()->projectController()->projectConfigurationChanged(project);
            KDevelop::ICore::self()->projectController()->reparseProject(project);
        });
    }

    return true;
}

// ***************************
// * IBuildSystemManager API *
// ***************************

void MesonManager::populateTargets(ProjectFolderItem* item, QVector<MesonTarget*> targets)
{
    // Remove all old targets
    for (ProjectTargetItem* i : item->targetList()) {
        delete i;
    }

    // Add the new targets
    auto dirPath = item->path();
    for (MesonTarget* i : targets) {
        if (!dirPath.isDirectParentOf(i->definedIn())) {
            continue;
        }

        if (i->type().contains(QStringLiteral("executable"))) {
            auto outFiles = i->filename();
            Path outFile;
            if (outFiles.size() > 0) {
                outFile = outFiles[0];
            }
            new MesonProjectExecutableTargetItem(item->project(), i->name(), item, outFile);
        } else if (i->type().contains(QStringLiteral("library"))) {
            new ProjectLibraryTargetItem(item->project(), i->name(), item);
        } else {
            new ProjectTargetItem(item->project(), i->name(), item);
        }
    }

    // Recurse
    for (ProjectFolderItem* i : item->folderList()) {
        QVector<MesonTarget*> filteredTargets;
        copy_if(begin(targets), end(targets), back_inserter(filteredTargets),
                [i](MesonTarget* t) -> bool { return i->path().isParentOf(t->definedIn()); });
        populateTargets(i, filteredTargets);
    }
}

QList<ProjectTargetItem*> MesonManager::targets(ProjectFolderItem* item) const
{
    Q_ASSERT(item);
    QList<ProjectTargetItem*> res = item->targetList();
    for (ProjectFolderItem* i : item->folderList()) {
        res << targets(i);
    }
    return res;
}

void MesonManager::onMesonInfoChanged(QString path, QString projectName)
{
    qCDebug(KDEV_Meson) << "File" << path << "changed --> reparsing project";
    IProject* foundProject = ICore::self()->projectController()->findProjectByName(projectName);
    if (!foundProject) {
        return;
    }

    KJob* job = createImportJob(foundProject->projectItem());
    foundProject->setReloadJob(job);
    ICore::self()->runController()->registerJob(job);
    connect(job, &KJob::finished, this, [foundProject](KJob* job) -> void {
        if (job->error()) {
            return;
        }

        KDevelop::ICore::self()->projectController()->projectConfigurationChanged(foundProject);
        KDevelop::ICore::self()->projectController()->reparseProject(foundProject);
    });
}

KJob* MesonManager::createImportJob(ProjectFolderItem* item)
{
    IProject* project = item->project();
    Q_ASSERT(project);

    auto buildDir = Meson::currentBuildDir(project);
    auto introJob
        = new MesonIntrospectJob(project, buildDir, { MesonIntrospectJob::TARGETS, MesonIntrospectJob::TESTS },
                                 MesonIntrospectJob::BUILD_DIR, this);

    KDirWatchPtr watcher = m_projectWatchers[project];
    if (!watcher) {
        // Create a new watcher
        watcher = m_projectWatchers[project] = make_shared<KDirWatch>(nullptr);
        QString projectName = project->name();

        connect(watcher.get(), &KDirWatch::dirty, this, [=](QString p) { onMesonInfoChanged(p, projectName); });
        connect(watcher.get(), &KDirWatch::created, this, [=](QString p) { onMesonInfoChanged(p, projectName); });
    }

    Path watchFile = buildDir.buildDir;
    watchFile.addPath(QStringLiteral("meson-info"));
    watchFile.addPath(QStringLiteral("meson-info.json"));
    if (!watcher->contains(watchFile.path())) {
        qCDebug(KDEV_Meson) << "Start watching file" << watchFile;
        watcher->addFile(watchFile.path());
    }

    connect(introJob, &KJob::result, this, [this, introJob, item, project]() {
        auto targets = introJob->targets();
        auto tests = introJob->tests();
        if (!targets || !tests) {
            return;
        }

        // Remove old test suites before deleting them
        if (m_projectTestSuites[project]) {
            for (auto i : m_projectTestSuites[project]->testSuites()) {
                ICore::self()->testController()->removeTestSuite(i.get());
            }
        }

        m_projectTargets[project] = targets;
        m_projectTestSuites[project] = tests;
        auto tgtList = targets->targets();
        QVector<MesonTarget*> tgtCopy;
        tgtCopy.reserve(tgtList.size());
        transform(begin(tgtList), end(tgtList), back_inserter(tgtCopy), [](auto const& a) { return a.get(); });

        populateTargets(item, tgtCopy);

        // Add test suites
        for (auto& i : tests->testSuites()) {
            ICore::self()->testController()->addTestSuite(i.get());
        }
    });

    QList<KJob*> jobs;

    // Configure the project if necessary
    if (m_builder->evaluateBuildDirectory(buildDir.buildDir, buildDir.mesonBackend) != MesonBuilder::MESON_CONFIGURED) {
        jobs << builder()->configure(project);
    }

    jobs << AbstractFileManagerPlugin::createImportJob(item); // generate the file system listing
    jobs << introJob;

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

MesonSourcePtr MesonManager::sourceFromItem(KDevelop::ProjectBaseItem* item) const
{
    Q_ASSERT(item);
    auto it = m_projectTargets.find(item->project());
    if (it == end(m_projectTargets)) {
        qCDebug(KDEV_Meson) << item->path().toLocalFile() << "not found";
        return {};
    }

    auto targets = *it;
    return targets->fileSource(item->path());
}

KDevelop::Path::List MesonManager::includeDirectories(KDevelop::ProjectBaseItem* item) const
{
    auto src = sourceFromItem(item);
    if (!src) {
        return {};
    }
    return src->includeDirs();
}

KDevelop::Path::List MesonManager::frameworkDirectories(KDevelop::ProjectBaseItem*) const
{
    return {};
}

QHash<QString, QString> MesonManager::defines(KDevelop::ProjectBaseItem* item) const
{
    auto src = sourceFromItem(item);
    if (!src) {
        return {};
    }
    return src->defines();
}

QString MesonManager::extraArguments(KDevelop::ProjectBaseItem* item) const
{
    auto src = sourceFromItem(item);
    if (!src) {
        return {};
    }
    return src->extraArgs().join(QChar::fromLatin1(' '));
}

bool MesonManager::hasBuildInfo(KDevelop::ProjectBaseItem* item) const
{
    auto src = sourceFromItem(item);
    if (!src) {
        return false;
    }
    return true;
}

KDevelop::Path MesonManager::compiler(KDevelop::ProjectTargetItem* item) const
{
    const auto source = sourceFromItem(item);
    return source && !source->compiler().isEmpty() ? KDevelop::Path(source->compiler().constFirst()) : KDevelop::Path();
}

// ********************
// * Custom functions *
// ********************

KJob* MesonManager::newBuildDirectory(IProject* project)
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

// ***********
// * IPlugin *
// ***********

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
