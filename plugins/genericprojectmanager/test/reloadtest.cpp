/* This file is part of KDevelop
    Copyright 2010 Niko Sams <niko.sams@gmail.com>

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

#include "reloadtest.h"

#include <qtest_kde.h>
#include <KDebug>
#include <KTempDir>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilanguagecontroller.h>

#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>
#include <language/backgroundparser/backgroundparser.h>

QTEST_KDEMAIN(ProjectLoadTest, GUI)

Q_DECLARE_METATYPE(KDevelop::IProject*);

using namespace KDevelop;

///FIXME: get rid of this, use temporary dir+file classes!
void exec(const QString &cmd)
{
    QProcess proc;
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    proc.start(cmd);
    proc.waitForFinished();
    Q_ASSERT(proc.exitStatus() == QProcess::NormalExit);
    Q_ASSERT(proc.exitStatus() == 0);
}

void ProjectLoadTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();
    ICore::self()->languageController()->backgroundParser()->disableProcessing();

    qRegisterMetaType<IProject*>();

    foreach(IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }
}

void ProjectLoadTest::cleanupTestCase()
{
    TestCore::shutdown();
}

void ProjectLoadTest::init()
{
    Q_ASSERT(ICore::self()->projectController()->projects().isEmpty());
}

struct TestProject
{
    // temp directory of project
    KTempDir* dir;
    // name of the project (random)
    QString name;
    // project file (*.kdev4)
    KUrl file;
    ~TestProject() {
        IProject* p = ICore::self()->projectController()->findProjectByName(name);
        if (p) {
            ICore::self()->projectController()->closeProject(p);
        }
        delete dir;
    }
};

TestProject makeProject()
{
    TestProject ret;
    ret.dir = new KTempDir();
    QFileInfo dir(ret.dir->name().left(ret.dir->name().length() - 1));
    Q_ASSERT(dir.exists());
    ret.name = dir.fileName();

    QStringList projectFileContents;
    projectFileContents
    << "[Project]"
    << QString("Name=") + ret.name
    << "Manager=KDevGenericManager";

    KUrl projecturl( dir.absoluteFilePath() + "/simpleproject.kdev4" );
    QFile projectFile(projecturl.toLocalFile());
    projectFile.open(QIODevice::WriteOnly);
    projectFile.write(projectFileContents.join("\n").toAscii());
    projectFile.close();
    ret.file = projecturl;

    Q_ASSERT(ret.dir->exists());
    Q_ASSERT(projecturl.upUrl().toLocalFile() == ret.dir->name());

    return ret;
}

void ProjectLoadTest::addRemoveFiles()
{
    const TestProject p = makeProject();

    QFile f(p.dir->name()+"/sdf");
    f.open(QIODevice::WriteOnly);
    f.close();

    ICore::self()->projectController()->openProject(p.file);
    QVERIFY(QTest::kWaitForSignal(ICore::self()->projectController(),
                                  SIGNAL(projectOpened(KDevelop::IProject*)), 2000));
    IProject* project = ICore::self()->projectController()->projects().first();
    QCOMPARE(project->projectFileUrl(), p.file);

    //KDirWatch adds/removes the file automatically
    for (int i=0; i<100; ++i) {
        QFile f2(p.dir->name()+"/blub"+QString::number(i));
        f2.open(QIODevice::WriteOnly);
        f2.close();
    }
    for (int i=0; i<50; ++i) {
        QFile f2(p.dir->name()+"/blub"+QString::number(i));
        QVERIFY(f2.exists());
        f2.remove();
    }
    QTest::qWait(500);

    KUrl url(p.dir->name()+"/blub"+QString::number(50));
    url.cleanPath();
    QCOMPARE(project->filesForUrl(url).count(), 1);
    ProjectFileItem* file = project->filesForUrl(url).first();
    project->projectFileManager()->removeFilesAndFolders(QList<ProjectBaseItem*>() << file ); //message box has to be accepted manually :(
    for (int i=51; i<100; ++i) {
        QFile f2(p.dir->name()+"/blub"+QString::number(i));
        f2.remove();
    }

    QTest::qWait(2000);
    QCOMPARE(project->fileCount(), 1);
}

void ProjectLoadTest::removeDirRecursive()
{
    const TestProject p = makeProject();

    {
        QFile f(p.dir->name()+"/sdf");
        f.open(QIODevice::WriteOnly);
        f.close();
    }
    {
        QDir(p.dir->name()).mkdir("blub");
        for (int i=0; i<10; ++i) {
            QFile f(p.dir->name()+"/blub/file"+QString::number(i));
            f.open(QIODevice::WriteOnly);
            f.close();
        }
    }

    //close previously opened projects
    QTest::qWait(1000); //wait for projects to load
    foreach ( IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
        QTest::qWait(100);
    }
    QVERIFY(ICore::self()->projectController()->projects().isEmpty());

    ICore::self()->projectController()->openProject(p.file);
    QVERIFY(QTest::kWaitForSignal(ICore::self()->projectController(),
                                  SIGNAL(projectOpened(KDevelop::IProject*)), 20000));
    IProject* project = ICore::self()->projectController()->projects().first();
    QCOMPARE(project->projectFileUrl(), p.file);

    for (int i=0; i<1; ++i) {
        KUrl url(p.dir->name()+"/blub");
        url.cleanPath();
        QCOMPARE(project->foldersForUrl(url).count(), 1);

        ProjectFolderItem* file = project->foldersForUrl(url).first();
        project->projectFileManager()->removeFilesAndFolders(QList<ProjectBaseItem*>() << file );
    }

    QTest::qWait(2000);
    QCOMPARE(project->fileCount(), 1);
}

void createFile(const QString& path)
{
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(QByteArray::number(qrand()));
    f.write(QByteArray::number(qrand()));
    f.write(QByteArray::number(qrand()));
    f.write(QByteArray::number(qrand()));
    f.close();
}

void _writeRandomStructure(QString path, int files)
{
    QDir p(path);
    QString name = QString::number(qrand());
    if (qrand() < RAND_MAX / 5) {
        p.mkdir(name);
        path += '/' + name;
//         kDebug() << "wrote path" << path;
    } else {
        createFile(path+'/'+name);
//         kDebug() << "wrote file" << path+"/"+name;
    }
    files--;
    if (files > 0) {
        _writeRandomStructure(path, files);
    }
}

void fillProject(int filesPerDir, int dirs, const TestProject& project, bool wait)
{
    for(int i=0; i < dirs; ++i) {
        const QString name = "foox" + QString::number(i);
        QDir(project.dir->name()).mkdir(name);
        _writeRandomStructure(project.dir->name() + name, filesPerDir);
        if (wait) {
            QTest::qWait(100);
        }
    }
}

void ProjectLoadTest::addLotsOfFiles()
{
    TestProject p = makeProject();

    ICore::self()->projectController()->openProject(p.file);
    QVERIFY(QTest::kWaitForSignal(ICore::self()->projectController(),
                                  SIGNAL(projectOpened(KDevelop::IProject*)), 2000));
    QCOMPARE(ICore::self()->projectController()->projects().size(), 1);
    IProject* project = ICore::self()->projectController()->projects().first();
    QCOMPARE(project->projectFileUrl(), p.file);

    fillProject(50, 25, p, true);

    QTest::qWait(2000);
}

void ProjectLoadTest::addMultipleJobs()
{
    const TestProject p1 = makeProject();
    fillProject(10, 25, p1, false);
    const TestProject p2 = makeProject();
    fillProject(10, 25, p2, false);

    QSignalSpy spy(ICore::self()->projectController(),
                   SIGNAL(projectOpened(KDevelop::IProject*)));
    ICore::self()->projectController()->openProject(p1.file);
    ICore::self()->projectController()->openProject(p2.file);

    const int wait = 25;
    const int maxWait = 2000;
    int waited = 0;
    while(waited < maxWait && spy.count() != 2) {
        QTest::qWait(wait);
        waited += wait;
    }

    QCOMPARE(ICore::self()->projectController()->projects().size(), 2);
}

void ProjectLoadTest::raceJob()
{
    // our goal here is to try to reproduce https://bugs.kde.org/show_bug.cgi?id=260741
    // my idea is that this can be triggered by the following:
    // - list dir foo/bar containing lots of files
    // - remove dir foo while listjob is still running
    TestProject p = makeProject();
    QDir dir(p.dir->name());
    QVERIFY(dir.mkpath("test/zzzzz"));
    for(int i = 0; i < 1000; ++i) {
        createFile(QString(p.dir->name() + "/test/zzzzz/%1").arg(i));
        createFile(QString(p.dir->name() + "/test/%1").arg(i));
    }

    ICore::self()->projectController()->openProject(p.file);
    QVERIFY(QTest::kWaitForSignal(ICore::self()->projectController(),
                                  SIGNAL(projectOpened(KDevelop::IProject*)), 2000));

    QCOMPARE(ICore::self()->projectController()->projectCount(), 1);
    IProject *project = ICore::self()->projectController()->projectAt(0);
    QCOMPARE(project->projectFileUrl(), p.file);
    ProjectFolderItem* root = project->projectItem();
    QCOMPARE(root->rowCount(), 1);
    ProjectBaseItem* testItem = root->child(0);
    QVERIFY(testItem->folder());
    QCOMPARE(testItem->baseName(), QString("test"));
    QCOMPARE(testItem->rowCount(), 1001);
    ProjectBaseItem* asdfItem = testItem->children().last();
    QVERIFY(asdfItem->folder());

    // move dir
    dir.rename("test", "test2");
    // move sub dir
    dir.rename("test2/zzzzz", "test2/bla");

    QTest::qWait(500);
    QCOMPARE(root->rowCount(), 1);
    testItem = root->child(0);
    QVERIFY(testItem->folder());
    QCOMPARE(testItem->baseName(), QString("test2"));
}

void ProjectLoadTest::addDuringImport()
{
    // our goal here is to try to reproduce an issue in the optimized filesForUrl implementation
    // which requires the project to be associated to the model to function properly
    // to trigger this we create a big project, import it and then call filesForUrl during
    // the import action
    TestProject p = makeProject();
    QDir dir(p.dir->name());
    QVERIFY(dir.mkpath("test/zzzzz"));
    for(int i = 0; i < 1000; ++i) {
        createFile(QString(p.dir->name() + "/test/zzzzz/%1").arg(i));
        createFile(QString(p.dir->name() + "/test/%1").arg(i));
    }

    QSignalSpy spy(ICore::self()->projectController(),
                   SIGNAL(projectAboutToBeOpened(KDevelop::IProject*)));
    ICore::self()->projectController()->openProject(p.file);
    // not yet ready
    QCOMPARE(ICore::self()->projectController()->projectCount(), 0);
    // but about to be opened
    QCOMPARE(spy.count(), 1);
    IProject* project = spy.value(0).first().value<IProject*>();
    QVERIFY(project);
    QCOMPARE(project->folder(), p.file.upUrl());
    KUrl file(p.file, "test/zzzzz/999");
    QVERIFY(QFile::exists(file.toLocalFile()));
    // this most probably is not yet loaded
    // and this should not crash
    QCOMPARE(project->itemsForUrl(file).size(), 0);
    // now delete that file and don't crash
    QFile::remove(file.toLocalFile());
    // now create another file
    KUrl file2 = file;
    file2.setFileName("999v2");
    createFile(file2.toLocalFile());
    QVERIFY(!project->isReady());
    // now wait for finish
    QVERIFY(QTest::kWaitForSignal(ICore::self()->projectController(),
                                  SIGNAL(projectOpened(KDevelop::IProject*)), 2000));
    QVERIFY(project->isReady());
    // make sure our file removal + addition was properly tracked
    QCOMPARE(project->filesForUrl(file).size(), 0);
    QCOMPARE(project->filesForUrl(file2).size(), 1);

    //NOTE: this test is probabably incomplete, I bet there are some race conditions left,
    //      esp. when adding a file at a point where the parent folder was already imported
    //      or removing a file that was already imported
}
