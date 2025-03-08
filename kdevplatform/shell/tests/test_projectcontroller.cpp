/*
    SPDX-FileCopyrightText: 2008 Manuel Breugelmans <mbr.nxi@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_projectcontroller.h"

#include <QFile>
#include <QSignalSpy>
#include <QTest>

#include <KAboutData>

#include <tests/testhelpers.h>
#include <tests/autotestshell.h>
#include <tests/plugintesthelpers.h>
#include <tests/testcore.h>

#include <interfaces/iplugin.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>
#include <shell/core.h>
#include <shell/projectcontroller.h>
#include <shell/plugincontroller.h>
#include <shell/project.h>

using namespace KDevelop;

namespace {

class DialogProviderFake : public IProjectDialogProvider
{
Q_OBJECT
public:
    DialogProviderFake()
    {}
    ~DialogProviderFake() override {}
    bool m_reopen = true;

public Q_SLOTS:
    QUrl askProjectConfigLocation(bool /*fetch*/, const QUrl& /*startUrl*/,
                                  const QUrl& /*repoUrl*/, IPlugin* /*plugin*/) override
    { return QUrl(); }
    bool userWantsReopen() override { return m_reopen; }
};

}

/*! A Filemanager plugin that allows you to setup a file & directory structure */
class FakeFileManager : public IPlugin, public IProjectFileManager
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectFileManager)
public:
    FakeFileManager(QObject*, const QVariantList&)
        : IPlugin("FakeFileManager", Core::self(), makeTestPluginMetaData("FakeFileManager"))
    {
    }

    FakeFileManager()
        : IPlugin("FakeFileManager", Core::self(), makeTestPluginMetaData("FakeFileManager"))
    {
    }

    ~FakeFileManager() override {}

    Features features() const override
    {
        return IProjectFileManager::Files | IProjectFileManager::Folders;
    }

    QMap<Path, Path::List> m_filesInFolder; // initialize
    QMap<Path, Path::List> m_subFoldersInFolder;

    /*! Setup this manager such that @p folder contains @p file */
    void addFileToFolder(const Path& folder, const Path& file)
    {
        if (!m_filesInFolder.contains(folder)) {
            m_filesInFolder[folder] = Path::List();
        }
        m_filesInFolder[folder] << file;
    }

    /*! Setup this manager such that @p folder has @p subFolder */
    void addSubFolderTo(const Path& folder, const Path& subFolder)
    {
        if (!m_subFoldersInFolder.contains(folder)) {
            m_subFoldersInFolder[folder] = Path::List();
        }
        m_subFoldersInFolder[folder] << subFolder;
    }

    QList<ProjectFolderItem*> parse(ProjectFolderItem *dom) override
    {
        const Path::List files = m_filesInFolder[dom->path()];
        for (const Path& file : files) {
            new ProjectFileItem(dom->project(), file, dom);
        }
        const Path::List folderPaths = m_subFoldersInFolder[dom->path()];
        QList<ProjectFolderItem*> folders;
        for (const Path& folderPath : folderPaths) {
            folders << new ProjectFolderItem(dom->project(), folderPath, dom);
        }
        return folders;
    }

    ProjectFolderItem *import(IProject *project) override
    {
        auto* it = new ProjectFolderItem(project, project->path());
        return it;
    }

    ProjectFolderItem* addFolder(const Path& /*folder*/, ProjectFolderItem* /*parent*/) override { return nullptr; }
    ProjectFileItem* addFile(const Path& /*file*/, ProjectFolderItem* /*parent*/) override { return nullptr; }
    bool removeFilesAndFolders(const QList<ProjectBaseItem*> &/*items*/) override { return false; }
    bool moveFilesAndFolders(const QList< KDevelop::ProjectBaseItem* > &/*items*/, KDevelop::ProjectFolderItem* /*newParent*/) override { return false; }
    bool copyFilesAndFolders(const Path::List &/*items*/, KDevelop::ProjectFolderItem* /*newParent*/) override { return false; }
    bool renameFile(ProjectFileItem* /*file*/, const Path& /*newPath*/) override { return false; }
    bool renameFolder(ProjectFolderItem* /*oldFolder*/, const Path& /*newPath*/ ) override { return false; }
    bool reload(ProjectFolderItem* /*item*/) override { return false; }
};

class FakePluginController : public PluginController
{
    Q_OBJECT
public:
    using PluginController::PluginController;

    IPlugin* pluginForExtension(const QString& extension, const QString& pluginName = {}, const QVariantMap& constraints = QVariantMap()) override
    {
        if (extension == qobject_interface_iid<IProjectFileManager*>()) {
            if (!m_fakeFileManager) {
                // Can't initialize in the constructor, because the pluginController must be setup
                //  before constructing a plugin, and this _is_ the pluginController.
                m_fakeFileManager = new FakeFileManager;
            }
            return m_fakeFileManager;
        }
        return PluginController::pluginForExtension(extension, pluginName, constraints);
    }

private:
    FakeFileManager* m_fakeFileManager = nullptr;
};

////////////////////// Fixture ///////////////////////////////////////////////

void TestProjectController::initTestCase()
{
    AutoTestShell::init({{}});
    auto* testCore = new TestCore;
    testCore->setPluginController( new FakePluginController(testCore) );
    testCore->initialize();
    qRegisterMetaType<KDevelop::IProject*>();
    m_core = Core::self();
    m_scratchDir = QDir(QDir::tempPath());
    m_scratchDir.mkdir(QStringLiteral("prjctrltest"));
    m_scratchDir.cd(QStringLiteral("prjctrltest"));

    QSignalSpy projectControllerInitializedSpy(m_core->projectControllerInternal(),
                                               &ProjectController::initialized);
    QVERIFY(projectControllerInitializedSpy.wait(100));
}

void TestProjectController::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestProjectController::init()
{
    m_projName = QStringLiteral("foo");
    m_projFilePath = writeProjectConfig(m_projName);
    m_projCtrl = m_core->projectControllerInternal();
    m_tmpConfigs << m_projFilePath;
    m_projFolder = Path(m_scratchDir.absolutePath() + '/');
}

void TestProjectController::cleanup()
{
    // also close any opened projects as we do not get a clean fixture,
    // following tests should start off clean.
    const auto projects = m_projCtrl->projects();
    for (IProject* p : projects) {
        m_projCtrl->closeProject(p);
    }
    for (const Path& cfg : std::as_const(m_tmpConfigs)) {
        QFile::remove(cfg.pathOrUrl());
    }
    qDeleteAll(m_fileManagerGarbage);
    m_fileManagerGarbage.clear();
}

////////////////////// Commands //////////////////////////////////////////////

#define WAIT_FOR_OPEN_SIGNAL \
{\
    QSignalSpy signal(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)));\
    QVERIFY2(signal.wait(30000), "Timeout while waiting for opened signal");\
} void(0)

void TestProjectController::openProject()
{
    auto spy = createOpenedSpy();
    QVERIFY(!m_projCtrl->isProjectNameUsed(m_projName));
    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(m_projCtrl->projectCount(), 1);
    auto* proj = assertProjectOpened(m_projName);
    assertSpyCaughtProject(spy.get(), proj);
    QCOMPARE(proj->projectFile(), m_projFilePath);
    QCOMPARE(proj->path(), Path(m_scratchDir.absolutePath()+'/'));
    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
}

void TestProjectController::closeProject()
{
    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    IProject* proj = m_projCtrl->findProjectByName(m_projName);
    Q_ASSERT(proj);

    auto spy1 = createClosedSpy();
    auto spy2 = createClosingSpy();
    m_projCtrl->closeProject(proj);

    QVERIFY(!m_projCtrl->isProjectNameUsed(m_projName));
    QCOMPARE(m_projCtrl->projectCount(), 0);
    assertProjectClosed(proj);
    assertSpyCaughtProject(spy1.get(), proj);
    assertSpyCaughtProject(spy2.get(), proj);
}

void TestProjectController::openCloseOpen()
{
    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    auto* proj = assertProjectOpened(m_projName);
    m_projCtrl->closeProject(proj);
    auto spy = createOpenedSpy();
    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
    QCOMPARE(m_projCtrl->projectCount(), 1);
    proj = assertProjectOpened(m_projName);
    assertSpyCaughtProject(spy.get(), proj);
}

void TestProjectController::reopen()
{
    m_projCtrl->setDialogProvider(new DialogProviderFake);
    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    auto spy = createOpenedSpy();
    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(m_projCtrl->projectCount(), 1);
    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
    auto* proj = assertProjectOpened(m_projName);
    assertSpyCaughtProject(spy.get(), proj);
}

void TestProjectController::reopenWhileLoading()
{
    // Open the same project again while the first is still
    // loading. The second open request should be blocked.
    m_projCtrl->setDialogProvider(new DialogProviderFake);
    auto spy = createOpenedSpy();
    m_projCtrl->openProject(m_projFilePath.toUrl());
    //m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    // wait a bit for a second signal, this should timeout
    QSignalSpy signal(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)));
    QVERIFY2(!signal.wait(100), "Received 2 projectOpened signals.");
    QCOMPARE(m_projCtrl->projectCount(), 1);
    auto* proj = assertProjectOpened(m_projName);
    assertSpyCaughtProject(spy.get(), proj);
}

void TestProjectController::openMultiple()
{
    QString secondProj(QStringLiteral("bar"));
    Path secondCfgUrl = writeProjectConfig(secondProj);
    auto spy = createOpenedSpy();
    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    m_projCtrl->openProject(secondCfgUrl.toUrl());
    WAIT_FOR_OPEN_SIGNAL;

    QCOMPARE(m_projCtrl->projectCount(), 2);
    auto* proj1 = assertProjectOpened(m_projName);
    auto* proj2 = assertProjectOpened(secondProj);

    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
    QVERIFY(m_projCtrl->isProjectNameUsed(QStringLiteral("bar")));

    QCOMPARE(spy->size(), 2);
    auto* emittedProj1 = (*spy)[0][0].value<IProject*>();
    auto* emittedProj2 = (*spy)[1][0].value<IProject*>();
    QCOMPARE(emittedProj1, proj1);
    QCOMPARE(emittedProj2, proj2);

    m_tmpConfigs << secondCfgUrl;
}

/*! Verify that the projectmodel contains a single project. Put this project's
 *  ProjectFolderItem in the output parameter @p RootItem */
#define ASSERT_SINGLE_PROJECT_IN_MODEL(rootItem) \
{\
    QCOMPARE(m_projCtrl->projectModel()->rowCount(), 1); \
    QModelIndex projIndex = m_projCtrl->projectModel()->index(0,0); \
    QVERIFY(projIndex.isValid()); \
    ProjectBaseItem* i = m_projCtrl->projectModel()->itemFromIndex( projIndex ); \
    QVERIFY(i); \
    QVERIFY(i->folder()); \
    rootItem = i->folder();\
} void(0)

/*! Verify that the projectitem @p item has a single child item
 *  named @p name with url @p url. @p subFolder is an output parameter
 *  that contains the sub-folder projectitem. */
#define ASSERT_SINGLE_SUBFOLDER_IN(item, name, path__, subFolder) \
{\
    QCOMPARE(item->rowCount(), 1);\
    QCOMPARE(item->folderList().size(), 1);\
    ProjectFolderItem* fo = item->folderList().at(0);\
    QVERIFY(fo);\
    QCOMPARE(fo->path(), path__);\
    QCOMPARE(fo->folderName(), QStringLiteral(name));\
    subFolder = fo;\
} void(0)

#define ASSERT_SINGLE_FILE_IN(rootFolder, name, path__, fileItem)\
{\
    QCOMPARE(rootFolder->rowCount(), 1);\
    QCOMPARE(rootFolder->fileList().size(), 1);\
    fileItem = rootFolder->fileList().at(0);\
    QVERIFY(fileItem);\
    QCOMPARE(fileItem->path(), path__);\
    QCOMPARE(fileItem->fileName(), QStringLiteral(name));\
} void(0)

// command
void TestProjectController::emptyProject()
{
    // verify that the project model contains a single top-level folder after loading
    // an empty project

    assertEmptyProjectModel();

    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    auto* proj = assertProjectOpened(m_projName);

    FakeFileManager* fileMng = createFileManager();
    Q_ASSERT(fileMng);

    proj->setManagerPlugin(fileMng);
    proj->reloadModel();
    QTest::qWait(100);

    ProjectFolderItem* rootFolder;
    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);

    // check that the project is empty
    QCOMPARE(rootFolder->rowCount(), 0);
    QCOMPARE(rootFolder->project()->name(), m_projName);
    QCOMPARE(rootFolder->path(), m_projFolder);
}

// command
void TestProjectController::singleFile()
{
    // verify that the project model contains a single file in the
    // top folder. First setup a FakeFileManager with this file

    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    auto* proj = assertProjectOpened(m_projName);

    FakeFileManager* fileMng = createFileManager();
    proj->setManagerPlugin(fileMng);

    Path filePath = Path(m_projFolder, QStringLiteral("foobar"));
    fileMng->addFileToFolder(m_projFolder, filePath);

    proj->reloadModel();
    QTest::qWait(100); // NO signals for reload ...

    ProjectFolderItem* rootFolder;
    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ProjectFileItem* fi;
    ASSERT_SINGLE_FILE_IN(rootFolder, "foobar", filePath, fi);
    QCOMPARE(fi->rowCount(), 0);

    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ASSERT_SINGLE_FILE_IN(rootFolder, "foobar", filePath, fi);
}

// command
void TestProjectController::singleDirectory()
{
    // verify that the project model contains a single folder in the
    // top folder. First setup a FakeFileManager with this folder

    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    auto* proj = assertProjectOpened(m_projName);

    Path folderPath = Path(m_projFolder, QStringLiteral("foobar/"));
    FakeFileManager* fileMng = createFileManager();
    fileMng->addSubFolderTo(m_projFolder, folderPath);

    proj->setManagerPlugin(fileMng);
    proj->reloadModel();
    QTest::qWait(100);

    ProjectFolderItem* rootFolder;
    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);

    // check that the project contains a single subfolder
    ProjectFolderItem* sub;
    ASSERT_SINGLE_SUBFOLDER_IN(rootFolder, "foobar", folderPath, sub);
    QCOMPARE(sub->rowCount(), 0);
}

// command
void TestProjectController::fileInSubdirectory()
{
    // verify that the project model contains a single file in a subfolder
    // First setup a FakeFileManager with this folder + file

    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    auto* proj = assertProjectOpened(m_projName);

    Path folderPath = Path(m_projFolder, QStringLiteral("foobar/"));
    FakeFileManager* fileMng = createFileManager();
    fileMng->addSubFolderTo(m_projFolder, folderPath);
    Path filePath = Path(folderPath, QStringLiteral("zoo"));
    fileMng->addFileToFolder(folderPath, filePath);

    proj->setManagerPlugin(fileMng);
    ProjectFolderItem* rootFolder = nullptr;
    ProjectFolderItem* sub = nullptr;
    ProjectFileItem* file = nullptr;

    proj->reloadModel();
    QTest::qWait(100);

    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ASSERT_SINGLE_SUBFOLDER_IN(rootFolder, "foobar", folderPath, sub);
    ASSERT_SINGLE_FILE_IN(sub,"zoo",filePath,file);

    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ASSERT_SINGLE_SUBFOLDER_IN(rootFolder, "foobar", folderPath, sub);
    ASSERT_SINGLE_FILE_IN(sub,"zoo",filePath,file);
}

void TestProjectController::prettyFileName_data()
{
    QTest::addColumn<QString>("relativeFilePath");

    QTest::newRow("basic")
        << "foobar.txt";
    QTest::newRow("subfolder")
        << "sub/foobar.txt";
}

void TestProjectController::prettyFileName()
{
    QFETCH(QString, relativeFilePath);

    m_projCtrl->openProject(m_projFilePath.toUrl());
    WAIT_FOR_OPEN_SIGNAL;
    auto* proj = assertProjectOpened(m_projName);

    FakeFileManager* fileMng = createFileManager();
    proj->setManagerPlugin(fileMng);

    Path filePath = Path(m_projFolder, relativeFilePath);
    fileMng->addFileToFolder(m_projFolder, filePath);

    QCOMPARE(m_projCtrl->prettyFileName(filePath.toUrl(), ProjectController::FormattingOptions::FormatPlain), QString(m_projName + ':' + relativeFilePath));
}

void TestProjectController::changesModel()
{
    QCOMPARE_EQ(m_projCtrl->changesModel(), nullptr); // the changes model is created only on demand
    QCOMPARE_EQ(m_projCtrl->changesModel(), nullptr); // calling changesModel() does not create the model

    auto model = m_projCtrl->makeChangesModel();
    QVERIFY(model); // makeChangesModel() creates and returns the model
    QCOMPARE_EQ(m_projCtrl->makeChangesModel(), model); // makeChangesModel() returns the existent model if possible
    QCOMPARE_EQ(m_projCtrl->changesModel(), model); // changesModel() returns the existent model if available

    model.reset();
    QCOMPARE_EQ(m_projCtrl->changesModel(), nullptr); // resetting the shared pointer destroys the model
}

////////////////////// Helpers ///////////////////////////////////////////////

Path TestProjectController::writeProjectConfig(const QString& name)
{
    Path configPath = Path(m_scratchDir.absolutePath() + '/' + name + ".kdev4");
    QFile f(configPath.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    str << "[Project]\n"
        << "Name=" << name << "\n";
    f.close();
    return configPath;
}

////////////////// Custom assertions /////////////////////////////////////////

KDevelop::Project* TestProjectController::assertProjectOpened(const QString& name)
{
    auto* projRaw = m_projCtrl->findProjectByName(name);
    QVERIFY_RETURN(projRaw, nullptr);
    QVERIFY_RETURN(m_projCtrl->projects().contains(projRaw), nullptr);

    auto proj = dynamic_cast<KDevelop::Project*>(projRaw);
    QVERIFY_RETURN(projRaw, nullptr);
    return proj;
}

void TestProjectController::assertSpyCaughtProject(QSignalSpy* spy, IProject* proj)
{
    QCOMPARE(spy->size(), 1);
    auto* emittedProj = (*spy)[0][0].value<IProject*>();
    QCOMPARE(proj, emittedProj);
}

void TestProjectController::assertProjectClosed(IProject* proj)
{
    IProject* p = m_projCtrl->findProjectByName(proj->name());
    QVERIFY(p == nullptr);
    QVERIFY(!m_projCtrl->projects().contains(proj));
}

void TestProjectController::assertEmptyProjectModel()
{
    ProjectModel* m = m_projCtrl->projectModel();
    Q_ASSERT(m);
    QCOMPARE(m->rowCount(), 0);
}

///////////////////// Creation stuff /////////////////////////////////////////

std::unique_ptr<QSignalSpy> TestProjectController::createOpenedSpy()
{
    return std::make_unique<QSignalSpy>(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)));
}

std::unique_ptr<QSignalSpy> TestProjectController::createClosedSpy()
{
    return std::make_unique<QSignalSpy>(m_projCtrl, SIGNAL(projectClosed(KDevelop::IProject*)));
}

std::unique_ptr<QSignalSpy> TestProjectController::createClosingSpy()
{
    return std::make_unique<QSignalSpy>(m_projCtrl, SIGNAL(projectClosing(KDevelop::IProject*)));
}

FakeFileManager* TestProjectController::createFileManager()
{
    auto* fileMng = new FakeFileManager;
    m_fileManagerGarbage << fileMng;
    return fileMng;
}

QTEST_MAIN(TestProjectController)
#include "moc_test_projectcontroller.cpp"
#include "test_projectcontroller.moc"
