/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2018 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonmanager.h"

#include "debug.h"
#include "mesonbuilder.h"
#include "mesonconfig.h"
#include "mintro/mesonintrospectjob.h"
#include "mintro/mesontargets.h"
#include "settings/mesonconfigpage.h"
#include "settings/mesonnewbuilddir.h"
#include "settings/mesonrewriterpage.h"

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

#include <QCryptographicHash>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

#include <algorithm>
#include <utility>

using namespace KDevelop;
using namespace std;

static const QString GENERATOR_NINJA = QStringLiteral("ninja");

K_PLUGIN_FACTORY_WITH_JSON(MesonSupportFactory, "kdevmesonmanager.json", registerPlugin<MesonManager>();)

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

MesonManager::MesonManager(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : AbstractFileManagerPlugin(QStringLiteral("KDevMesonManager"), parent, metaData, args)
    , m_builder(new MesonBuilder(this))
{
    if (m_builder->hasError()) {
        setErrorDescription(i18n("Meson builder error: %1", m_builder->errorDescription()));
    }
    connect(ICore::self()->projectController(), &IProjectController::projectClosing, this,
            &MesonManager::projectClosing);
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

            emit KDevelop::ICore::self()->projectController()->projectConfigurationChanged(project);
            KDevelop::ICore::self()->projectController()->reparseProject(project);
        });
    }

    return true;
}

static void cleanupTestSuites(const QHash<QString, MesonTestSuitePtr>& testSuites)
{
    auto* const testController = ICore::self()->testController();
    for (const auto& suite : testSuites) {
        testController->removeTestSuite(suite.get());
    }
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

static QByteArray computeHashSum(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    QCryptographicHash hash(QCryptographicHash::Algorithm::Sha1);
    hash.addData(&file);
    return hash.result();
}

void MesonManager::onMesonInfoChanged(QString path, QString projectName)
{
    qCDebug(KDEV_Meson) << "File" << path << "changed --> reparsing project";
    IProject* foundProject = ICore::self()->projectController()->findProjectByName(projectName);
    if (!foundProject) {
        return;
    }

    // The import job can modify the meson-info file so only reparse
    // if the file contents changed to avoid a potential loop
    auto newHash = computeHashSum(path);
    auto& storedHash = m_projectMesonInfoHashes[foundProject];
    if (newHash == storedHash) {
        qCDebug(KDEV_Meson) << "File" << path << "hash unchanged --> not reparsing";
        return;
    }
    storedHash = std::move(newHash);

    KJob* job = createImportJob(foundProject->projectItem());
    foundProject->setReloadJob(job);
    ICore::self()->runController()->registerJob(job);
    connect(job, &KJob::finished, this, [foundProject](KJob* job) -> void {
        if (job->error()) {
            return;
        }

        emit KDevelop::ICore::self()->projectController()->projectConfigurationChanged(foundProject);
        KDevelop::ICore::self()->projectController()->reparseProject(foundProject);
    });
}

void MesonManager::projectClosing(IProject* project)
{
    const auto it = m_projectTestSuites.constFind(project);
    if (it != m_projectTestSuites.cend()) {
        cleanupTestSuites(it.value()->testSuites());
        m_projectTestSuites.erase(it);
    }
}

KJob* MesonManager::createImportJob(ProjectFolderItem* item)
{
    IProject* project = item->project();
    Q_ASSERT(project);

    qCDebug(KDEV_Meson) << "Importing project" << project->name();

    auto buildDir = Meson::currentBuildDir(project);

    KJob* configureJob = nullptr;
    if (!buildDir.isValid()) {
        configureJob = newBuildDirectory(project, &buildDir);
        if (!configureJob) {
            QString error = i18n("Importing %1 failed because no build directory could be created.", project->name());
            qCDebug(KDEV_Meson) << error;
            return new ErrorJob(this, error);
        }
    }

    auto introJob = new MesonIntrospectJob(
        project, buildDir, { MesonIntrospectJob::TARGETS, MesonIntrospectJob::TESTS, MesonIntrospectJob::PROJECTINFO },
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
        if (const auto& suites = m_projectTestSuites[project]) {
            cleanupTestSuites(suites->testSuites());
        }

        m_projectTargets[project] = targets;
        m_projectTestSuites[project] = tests;
        auto tgtList = targets->targets();
        QVector<MesonTarget*> tgtCopy;
        tgtCopy.reserve(tgtList.size());
        transform(begin(tgtList), end(tgtList), back_inserter(tgtCopy), [](const auto& a) { return a.get(); });

        populateTargets(item, tgtCopy);

        // Add test suites
        for (auto& i : tests->testSuites()) {
            ICore::self()->testController()->addTestSuite(i.get());
        }
    });

    QList<KJob*> jobs;

    // Configure the project if necessary
    if (!configureJob
        && m_builder->evaluateBuildDirectory(buildDir.buildDir, buildDir.mesonBackend)
            != MesonBuilder::MESON_CONFIGURED) {
        configureJob = builder()->configure(project);
    }

    if (configureJob) {
        jobs << configureJob;
    }

    jobs << AbstractFileManagerPlugin::createImportJob(item); // generate the file system listing
    jobs << introJob;

    Q_ASSERT(!jobs.contains(nullptr));
    auto composite = new ExecuteCompositeJob(this, jobs);
    composite->setAbortOnSubjobError(false);
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
    return src->extraArgs().join(QLatin1Char(' '));
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

KJob* MesonManager::newBuildDirectory(IProject* project, Meson::BuildDir* outBuildDir)
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

    if (outBuildDir) {
        *outBuildDir = buildDir;
    }

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

    for (const auto& i : mesonExecutables) {
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
    switch (number) {
    case 0:
        return new MesonConfigPage(this, options.project, parent);
    case 1:
        return new MesonRewriterPage(this, options.project, parent);
    }
    return nullptr;
}

int MesonManager::perProjectConfigPages() const
{
    return 2;
}

#include "mesonmanager.moc"
#include "moc_mesonmanager.cpp"
