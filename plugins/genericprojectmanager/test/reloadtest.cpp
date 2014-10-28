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

#include <QtTest/QTest>
#include <QSignalSpy>
#include <QProcess>
#include <QTemporaryDir>
#include <QDebug>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilanguagecontroller.h>

#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>
#include <language/backgroundparser/backgroundparser.h>

#include <KIO/Global>

QTEST_MAIN(ProjectLoadTest)

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
    foreach(IProject* p, ICore::self()->projectController()->projects()) {
        ICore::self()->projectController()->closeProject(p);
    }
    QCOMPARE(ICore::self()->projectController()->projects().size(), 0);
}

struct TestProject
{
    // temp directory of project
    QTemporaryDir* dir;
    // name of the project (random)
    QString name;
    // project file (*.kdev4)
    QUrl file;
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
    ret.dir = new QTemporaryDir();
    QFileInfo dir(ret.dir->path());
    Q_ASSERT(dir.exists());
    ret.name = dir.fileName();

    QStringList projectFileContents;
    projectFileContents
    << "[Project]"
    << QString("Name=") + ret.name
    << "Manager=KDevGenericManager";

    QUrl projecturl = QUrl::fromLocalFile( dir.absoluteFilePath() + "/simpleproject.kdev4" );
    QFile projectFile(projecturl.toLocalFile());
    projectFile.open(QIODevice::WriteOnly);
    projectFile.write(projectFileContents.join("\n").toLatin1());
    projectFile.close();
    ret.file = projecturl;

    Q_ASSERT(ret.dir->isValid());
    Q_ASSERT(projecturl.adjusted(QUrl::RemoveFilename).toLocalFile() == ret.dir->path() + '/');

    return ret;
}

void ProjectLoadTest::addRemoveFiles()
{
    const TestProject p = makeProject();

    QFile f(p.dir->path()+"/sdf");
    f.open(QIODevice::WriteOnly);
    f.close();

    ICore::self()->projectController()->openProject(p.file);
    QSignalSpy spy(KDevelop::ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    QVERIFY(spy.wait(2000));
    IProject* project = ICore::self()->projectController()->projects().first();
    QCOMPARE(project->projectFile().toUrl(), p.file);

    //KDirWatch adds/removes the file automatically
    for (int i=0; i<100; ++i) {
        QFile f2(p.dir->path()+"/blub"+QString::number(i));
        f2.open(QIODevice::WriteOnly);
        f2.close();
    }
    for (int i=0; i<50; ++i) {
        QFile f2(p.dir->path()+"/blub"+QString::number(i));
        QVERIFY(f2.exists());
        f2.remove();
    }
    QTest::qWait(500);

    QUrl url = QUrl::fromLocalFile(p.dir->path()+"/blub"+QString::number(50)).adjusted(QUrl::NormalizePathSegments);
    QCOMPARE(project->filesForPath(IndexedString(url)).count(), 1);
    ProjectFileItem* file = project->filesForPath(IndexedString(url)).first();
    project->projectFileManager()->removeFilesAndFolders(QList<ProjectBaseItem*>() << file ); //message box has to be accepted manually :(
    for (int i=51; i<100; ++i) {
        QFile f2(p.dir->path()+"/blub"+QString::number(i));
        f2.remove();
    }

    QTest::qWait(2000);
    QCOMPARE(project->projectItem()->fileList().count(), 1);
}

void ProjectLoadTest::removeDirRecursive()
{
    const TestProject p = makeProject();

    {
        QFile f(p.dir->path()+"/sdf");
        f.open(QIODevice::WriteOnly);
        f.close();
    }
    {
        QDir(p.dir->path()).mkdir("blub");
        for (int i=0; i<10; ++i) {
            QFile f(p.dir->path()+"/blub/file"+QString::number(i));
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
    QSignalSpy spy(KDevelop::ICore::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)));
    QVERIFY(spy.wait(20000));
    IProject* project = ICore::self()->projectController()->projects().first();
    QCOMPARE(project->projectFile().toUrl(), p.file);

    for (int i=0; i<1; ++i) {
        QUrl url = QUrl::fromLocalFile(p.dir->path()+"/blub").adjusted(QUrl::NormalizePathSegments);
        QCOMPARE(project->foldersForPath(IndexedString(url)).count(), 1);

        ProjectFolderItem* file = project->foldersForPath(IndexedString(url)).first();
        project->projectFileManager()->removeFilesAndFolders(QList<ProjectBaseItem*>() << file );
    }

    QTest::qWait(2000);
    QCOMPARE(project->projectItem()->fileList().count(), 1);
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
//         qDebug() << "wrote path" << path;
    } else {
        createFile(path+'/'+name);
//         qDebug() << "wrote file" << path+"/"+name;
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
        QDir(project.dir->path()).mkdir(name);
        _writeRandomStructure(project.dir->path() + name, filesPerDir);
        if (wait) {
            QTest::qWait(100);
        }
    }
}

void ProjectLoadTest::addLotsOfFiles()
{
    TestProject p = makeProject();

    ICore::self()->projectController()->openProject(p.file);
    QVERIFY(QSignalSpy(KDevelop::ICore::self()->projectController(),
                       SIGNAL(projectOpened(KDevelop::IProject*))).wait(2000));
    QCOMPARE(ICore::self()->projectController()->projects().size(), 1);
    IProject* project = ICore::self()->projectController()->projects().first();
    QCOMPARE(project->projectFile().toUrl(), p.file);

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
    QDir dir(p.dir->path());
    QVERIFY(dir.mkpath("test/zzzzz"));
    for(int i = 0; i < 1000; ++i) {
        createFile(QString(p.dir->path() + "/test/zzzzz/%1").arg(i));
        createFile(QString(p.dir->path() + "/test/%1").arg(i));
    }

    ICore::self()->projectController()->openProject(p.file);
    QVERIFY(QSignalSpy(KDevelop::ICore::self()->projectController(),
                       SIGNAL(projectOpened(KDevelop::IProject*))).wait(2000));

    QCOMPARE(ICore::self()->projectController()->projectCount(), 1);
    IProject *project = ICore::self()->projectController()->projectAt(0);
    QCOMPARE(project->projectFile().toUrl(), p.file);
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
    // our goal here is to try to reproduce an issue in the optimized filesForPath implementation
    // which requires the project to be associated to the model to function properly
    // to trigger this we create a big project, import it and then call filesForPath during
    // the import action
    TestProject p = makeProject();
    QDir dir(p.dir->path());
    QVERIFY(dir.mkpath("test/zzzzz"));
    for(int i = 0; i < 1000; ++i) {
        createFile(QString(p.dir->path() + "/test/zzzzz/%1").arg(i));
        createFile(QString(p.dir->path() + "/test/%1").arg(i));
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
    QCOMPARE(project->path(), Path(KIO::upUrl(p.file)));
    QUrl file = p.file.resolved(QUrl("test/zzzzz/999"));
    QVERIFY(QFile::exists(file.toLocalFile()));
    // this most probably is not yet loaded
    // and this should not crash
    QCOMPARE(project->itemsForPath(IndexedString(file)).size(), 0);
    // now delete that file and don't crash
    QFile::remove(file.toLocalFile());
    // now create another file
    QUrl file2 = file.adjusted(QUrl::RemoveFilename);
    file2.setPath(file2.path() + "999v2");
    createFile(file2.toLocalFile());
    QVERIFY(!project->isReady());
    // now wait for finish
    QVERIFY(QSignalSpy(KDevelop::ICore::self()->projectController(),\
                       SIGNAL(projectOpened(KDevelop::IProject*))).wait(2000));
    QVERIFY(project->isReady());
    // make sure our file removal + addition was properly tracked
    QCOMPARE(project->filesForPath(IndexedString(file)).size(), 0);
    QCOMPARE(project->filesForPath(IndexedString(file2)).size(), 1);

    //NOTE: this test is probabably incomplete, I bet there are some race conditions left,
    //      esp. when adding a file at a point where the parent folder was already imported
    //      or removing a file that was already imported
}
