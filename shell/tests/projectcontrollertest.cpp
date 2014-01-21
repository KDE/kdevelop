/***************************************************************************
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "projectcontrollertest.h"

#include <QFile>
#include <QSignalSpy>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include "../core.h"
#include "../projectcontroller.h"
#include "../project.h"
#include "../../interfaces/iplugin.h"
#include "../../project/interfaces/iprojectfilemanager.h"
#include "../project/projectmodel.h"

using namespace KDevelop;

using QTest::kWaitForSignal;

Q_DECLARE_METATYPE(KDevelop::IProject*)

namespace {

class DialogProviderFake : public IProjectDialogProvider
{
Q_OBJECT
public:
    DialogProviderFake() : m_reopen(true) {}
    virtual ~DialogProviderFake() {}
    bool m_reopen;

public slots:
    virtual KUrl askProjectConfigLocation(const KUrl& startUrl) { return KUrl(); }
    virtual bool userWantsReopen() { return m_reopen; }
};

}

/*! A Filemanager plugin that allows you to setup a file & directory structure */
class FakeFileManager : public IPlugin, public IProjectFileManager
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IProjectFileManager)

public:
    FakeFileManager(QObject*, const QVariantList&) : IPlugin(KGlobal::mainComponent(), Core::self()) {
        KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    }

    FakeFileManager() : IPlugin(KGlobal::mainComponent(), Core::self()) {
        KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    }

    virtual ~FakeFileManager() {}

    virtual Features features() const {
        return IProjectFileManager::Files | IProjectFileManager::Folders;
    }

    QMap<KUrl, KUrl::List> m_filesInFolder; // initialize 
    QMap<KUrl, KUrl::List> m_subFoldersInFolder;

    /*! Setup this manager such that @p folder contains @p file */
    void addFileToFolder(KUrl folder, KUrl file) {
        if (!m_filesInFolder.contains(folder)) {
            m_filesInFolder[folder] = KUrl::List();
        }
        m_filesInFolder[folder] << file;
    }

    /*! Setup this manager such that @p folder has @p subFolder */
    void addSubFolderTo(KUrl folder, KUrl subFolder) {
        if (!m_subFoldersInFolder.contains(folder)) {
            m_subFoldersInFolder[folder] = KUrl::List();
        }
        m_subFoldersInFolder[folder] << subFolder;
    }

    virtual QList<ProjectFolderItem*> parse(ProjectFolderItem *dom) {
        KUrl::List files = m_filesInFolder[dom->url()];
        foreach (const KUrl& file, files) {
            new ProjectFileItem(dom->project(), file, dom);
        }
        KUrl::List folderUrls = m_subFoldersInFolder[dom->url()];
        QList<ProjectFolderItem*> folders;
        foreach (const KUrl& folderUrl, folderUrls) {
            folders << new ProjectFolderItem(dom->project(), folderUrl, dom);
        }
        return folders;
    }

    virtual ProjectFolderItem *import(IProject *project) {
        ProjectFolderItem* it = new ProjectFolderItem(project, project->folder());
        it->setProjectRoot(true);
        return it;
    }

    virtual ProjectFolderItem* addFolder(const KUrl& folder, ProjectFolderItem *parent) { return 0; }
    virtual ProjectFileItem* addFile(const KUrl& folder, ProjectFolderItem *parent) { return 0; }
    virtual bool removeFolder(ProjectFolderItem *folder) { return false; }
    virtual bool removeFile(ProjectFileItem *file) { return false; }
    virtual bool renameFile(ProjectFileItem* oldFile,
                            const KUrl& newFile) { return false; }
    virtual bool renameFolder(ProjectFolderItem* oldFolder,
                              const KUrl& newFolder ) { return false; }
    virtual bool reload(ProjectBaseItem* item) { return false; }

};

K_PLUGIN_FACTORY(FakeFileManagerFactory, registerPlugin<FakeFileManager>(); )
K_EXPORT_PLUGIN(FakeFileManager())


////////////////////// Fixture ///////////////////////////////////////////////

void ProjectControllerTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();
    qRegisterMetaType<KDevelop::IProject*>();
    m_core = Core::self();
    m_scratchDir = QDir(QDir::tempPath());
    m_scratchDir.mkdir("prjctrltest");
    m_scratchDir.cd("prjctrltest");
}

void ProjectControllerTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void ProjectControllerTest::init()
{
    m_projName = "foo";
    m_projFileUrl = writeProjectConfig(m_projName);
    m_projCtrl = m_core->projectControllerInternal();
    m_tmpConfigs << m_projFileUrl;
    m_projFolder = KUrl(m_scratchDir.absolutePath() + '/');
}

void ProjectControllerTest::cleanup()
{
    // also close any opened projects as we do not get a clean fixture,
    // following tests should start off clean.
    foreach(IProject* p, m_projCtrl->projects()) {
        m_projCtrl->closeProject(p);
    }
    foreach(const KUrl &cfg, m_tmpConfigs) {
        QFile::remove(cfg.pathOrUrl());
    }
    qDeleteAll(m_fileManagerGarbage);
    m_fileManagerGarbage.clear();
}

////////////////////// Commands //////////////////////////////////////////////

#define WAIT_FOR_OPEN_SIGNAL \
{\
    bool gotSignal = kWaitForSignal(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)), 30000);\
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");\
} void(0)

void ProjectControllerTest::openProject()
{
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(!m_projCtrl->isProjectNameUsed(m_projName));
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(m_projCtrl->projectCount(), 1);
    IProject* proj;
    assertProjectOpened(m_projName, proj);QVERIFY(proj);
    assertSpyCaughtProject(spy, proj);
    QCOMPARE(proj->projectFileUrl(), m_projFileUrl);
    QCOMPARE(proj->folder(), KUrl(m_scratchDir.absolutePath()+'/'));
    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
}

void ProjectControllerTest::closeProject()
{
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    IProject* proj = m_projCtrl->findProjectByName(m_projName);
    Q_ASSERT(proj);

    QSignalSpy* spy1 = createClosedSpy();
    QSignalSpy* spy2 = createClosingSpy();
    m_projCtrl->closeProject(proj);

    QVERIFY(!m_projCtrl->isProjectNameUsed(m_projName));
    QCOMPARE(m_projCtrl->projectCount(), 0);
    assertProjectClosed(proj);
    assertSpyCaughtProject(spy1, proj);
    assertSpyCaughtProject(spy2, proj);
}

void ProjectControllerTest::openCloseOpen()
{
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    QVERIFY(m_projCtrl->closeProject(proj));
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
    QCOMPARE(m_projCtrl->projectCount(), 1);
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);
}

void ProjectControllerTest::reopen()
{
    m_projCtrl->setDialogProvider(new DialogProviderFake);
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(m_projCtrl->projectCount(), 1);
    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);
}

void ProjectControllerTest::reopenWhileLoading()
{
    // Open the same project again while the first is still
    // loading. The second open request should be blocked.
    m_projCtrl->setDialogProvider(new DialogProviderFake);
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    QVERIFY(!m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    // wait a bit for a second signal, this should timeout
    QVERIFY2(!kWaitForSignal(m_projCtrl,
        SIGNAL(projectOpened(KDevelop::IProject*)), 100),
        "Received 2 projectOpened signals.");
    QCOMPARE(m_projCtrl->projectCount(), 1);
    IProject* proj;
    assertProjectOpened(m_projName, proj);
    assertSpyCaughtProject(spy, proj);
}

void ProjectControllerTest::openMultiple()
{
    QString secondProj("bar");
    KUrl secondCfgUrl = writeProjectConfig(secondProj);
    QSignalSpy* spy = createOpenedSpy();
    QVERIFY(m_projCtrl->openProject(m_projFileUrl));
    WAIT_FOR_OPEN_SIGNAL;
    QVERIFY(m_projCtrl->openProject(secondCfgUrl));
    WAIT_FOR_OPEN_SIGNAL;

    QCOMPARE(m_projCtrl->projectCount(), 2);
    IProject *proj1, *proj2;
    assertProjectOpened(m_projName, proj1);
    assertProjectOpened(secondProj, proj2); 

    QVERIFY(m_projCtrl->isProjectNameUsed(m_projName));
    QVERIFY(m_projCtrl->isProjectNameUsed("bar"));

    QCOMPARE(spy->size(), 2);
    IProject* emittedProj1 = (*spy)[0][0].value<IProject*>();
    IProject* emittedProj2 = (*spy)[1][0].value<IProject*>();
    QCOMPARE(emittedProj1, proj1);
    QCOMPARE(emittedProj2, proj2);

    m_tmpConfigs << secondCfgUrl;
}

/*! Verify that the projectmodel contains a single project. Put this project's 
 *  ProjectFolderItem in the output parameter @p RootItem */
#define ASSERT_SINGLE_PROJECT_IN_MODEL(rootItem) \
{\
    QCOMPARE(1,m_projCtrl->projectModel()->rowCount()); \
    QModelIndex projIndex = m_projCtrl->projectModel()->index(0,0); \
    QVERIFY(projIndex.isValid()); \
    ProjectBaseItem* i = m_projCtrl->projectModel()->item( projIndex ); \
    QVERIFY(i); \
    QVERIFY(i->folder()); \
    rootItem = i->folder();\
} void(0)

/*! Verify that the projectitem @p item has a single child item
 *  named @p name with url @p url. @p subFolder is an output parameter
 *  that contains the sub-folder projectitem. */
#define ASSERT_SINGLE_SUBFOLDER_IN(item, name, url__, subFolder) \
{\
    QCOMPARE(1,item->rowCount());\
    QCOMPARE(1, item->folderList().size());\
    ProjectFolderItem* fo = item->folderList()[0];\
    QVERIFY(fo);\
    QCOMPARE(url__, fo->url());\
    QCOMPARE(QString(name), fo->data(Qt::DisplayRole).toString());\
    subFolder = fo;\
} void(0)

#define ASSERT_SINGLE_FILE_IN(rootFolder, name, url__, fileItem)\
{\
    QCOMPARE(1,rootFolder->rowCount());\
    QCOMPARE(1, rootFolder->fileList().size());\
    ProjectFileItem* fi__ = rootFolder->fileList()[0];\
    QVERIFY(fi__);\
    QCOMPARE(url__, fi__->url());\
    QCOMPARE(QString(name), fi__->data(Qt::DisplayRole).toString());\
    fileItem = fi__;\
} void(0)

// command
void ProjectControllerTest::emptyProject()
{
    // verify that the project model contains a single top-level folder after loading
    // an empty project

    assertEmptyProjectModel();
    Project* proj = new Project();
    FakeFileManager* fileMng = createFileManager();
    Q_ASSERT(fileMng);

    proj->setManagerPlugin(fileMng);
    proj->open(m_projFileUrl);
    WAIT_FOR_OPEN_SIGNAL;
    ProjectFolderItem* rootFolder;
    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);

    // check that the project is empty
    QCOMPARE(0,rootFolder->rowCount());
    QCOMPARE(m_projName, rootFolder->data(Qt::DisplayRole).toString());
    QCOMPARE(m_projFolder, rootFolder->url());
}

// command
void ProjectControllerTest::singleFile()
{
    // verify that the project model contains a single file in the
    // top folder. First setup a FakeFileManager with this file

    Project* proj = new Project();
    FakeFileManager* fileMng = createFileManager();
    proj->setManagerPlugin(fileMng);

    KUrl fileUrl = KUrl(m_projFolder, "foobar");
    fileMng->addFileToFolder(m_projFolder, fileUrl);

    proj->open(m_projFileUrl);
    WAIT_FOR_OPEN_SIGNAL;
    ProjectFolderItem* rootFolder;
    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ProjectFileItem* fi;
    ASSERT_SINGLE_FILE_IN(rootFolder, "foobar", fileUrl, fi);
    QCOMPARE(0,fi->rowCount());

    proj->reloadModel();
    QTest::qWait(100); // NO signals for reload ...

    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ASSERT_SINGLE_FILE_IN(rootFolder, "foobar", fileUrl, fi);    
}

// command
void ProjectControllerTest::singleDirectory()
{
    // verify that the project model contains a single folder in the
    // top folder. First setup a FakeFileManager with this folder

    Project* proj = new Project();
    KUrl folderUrl = KUrl(m_projFolder, "foobar/");
    FakeFileManager* fileMng = createFileManager();
    fileMng->addSubFolderTo(m_projFolder, folderUrl);
 
    proj->setManagerPlugin(fileMng);
    proj->open(m_projFileUrl);
    WAIT_FOR_OPEN_SIGNAL;
    ProjectFolderItem* rootFolder;
    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);

    // check that the project contains a single subfolder
    ProjectFolderItem* sub;
    ASSERT_SINGLE_SUBFOLDER_IN(rootFolder, "foobar", folderUrl, sub);
    QCOMPARE(0,sub->rowCount());
}

// command
void ProjectControllerTest::fileInSubdirectory()
{
    // verify that the project model contains a single file in a subfolder
    // First setup a FakeFileManager with this folder + file

    Project* proj = new Project();
    KUrl folderUrl = KUrl(m_projFolder, "foobar/");
    FakeFileManager* fileMng = createFileManager();
    fileMng->addSubFolderTo(m_projFolder, folderUrl);
    KUrl fileUrl = KUrl(folderUrl, "zoo");
    fileMng->addFileToFolder(folderUrl, fileUrl);
 
    proj->setManagerPlugin(fileMng);
    proj->open(m_projFileUrl);
    WAIT_FOR_OPEN_SIGNAL;
    ProjectFolderItem* rootFolder;
    ProjectFolderItem* sub;
    ProjectFileItem* file;

    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ASSERT_SINGLE_SUBFOLDER_IN(rootFolder, "foobar", folderUrl, sub);
    ASSERT_SINGLE_FILE_IN(sub,"zoo",fileUrl,file);

    proj->reloadModel();
    QTest::qWait(100); // NO signals for reload ...

    ASSERT_SINGLE_PROJECT_IN_MODEL(rootFolder);
    ASSERT_SINGLE_SUBFOLDER_IN(rootFolder, "foobar", folderUrl, sub);
    ASSERT_SINGLE_FILE_IN(sub,"zoo",fileUrl,file);
}

////////////////////// Helpers ///////////////////////////////////////////////

KUrl ProjectControllerTest::writeProjectConfig(const QString& name)
{
    KUrl configUrl = KUrl(m_scratchDir.absolutePath() + '/' + name + ".kdev4");
    QFile f(configUrl.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    str << "[Project]\n"
        << "Name=" << name << "\n";
    f.close();
    return configUrl;
}

////////////////// Custom assertions /////////////////////////////////////////

void ProjectControllerTest::assertProjectOpened(const QString& name, IProject*& proj)
{
    QVERIFY(proj = m_projCtrl->findProjectByName(name));
    QVERIFY(m_projCtrl->projects().contains(proj));
}

void ProjectControllerTest::assertSpyCaughtProject(QSignalSpy* spy, IProject* proj)
{
    QCOMPARE(spy->size(), 1);
    IProject* emittedProj = (*spy)[0][0].value<IProject*>();
    QCOMPARE(proj, emittedProj);
}

void ProjectControllerTest::assertProjectClosed(IProject* proj)
{
    IProject* p = m_projCtrl->findProjectByName(proj->name());
    QVERIFY(p == 0);
    QVERIFY(!m_projCtrl->projects().contains(proj));
}

void ProjectControllerTest::assertEmptyProjectModel()
{
    ProjectModel* m = m_projCtrl->projectModel();
    Q_ASSERT(m);
    QCOMPARE(m->rowCount(), 0);
}

///////////////////// Creation stuff /////////////////////////////////////////

QSignalSpy* ProjectControllerTest::createOpenedSpy()
{
    return new QSignalSpy(m_projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)));
}

QSignalSpy* ProjectControllerTest::createClosedSpy()
{
    return new QSignalSpy(m_projCtrl, SIGNAL(projectClosed(KDevelop::IProject*)));
}

QSignalSpy* ProjectControllerTest::createClosingSpy()
{
    return new QSignalSpy(m_projCtrl, SIGNAL(projectClosing(KDevelop::IProject*)));
}

FakeFileManager* ProjectControllerTest::createFileManager()
{
    FakeFileManagerFactory* f = new FakeFileManagerFactory();
    FakeFileManager* fileMng = qobject_cast<FakeFileManager*>(f->create());
    m_fileManagerGarbage << fileMng;
    return fileMng;
}

QTEST_MAIN( ProjectControllerTest)
#include "moc_projectcontrollertest.cpp"
#include "projectcontrollertest.moc"
