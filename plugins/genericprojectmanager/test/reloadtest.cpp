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

#include <interfaces/icore.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <interfaces/iprojectcontroller.h>
#include <KDebug>
#include <interfaces/isession.h>
#include <kconfiggroup.h>
#include <interfaces/iproject.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <language/duchain/indexedstring.h>
#include <project/projectmodel.h>
#include <interfaces/iuicontroller.h>
#include <KParts/MainWindow>

QTEST_KDEMAIN(ProjectLoadTest, GUI)

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
    KDevelop::AutoTestShell::init();
    KDevelop::TestCore::initialize();
}

void ProjectLoadTest::init()
{
    exec("bash -c \"rm -r testproject*\"");

    foreach (KDevelop::IProject *p, KDevelop::ICore::self()->projectController()->projects()) {
        KDevelop::ICore::self()->projectController()->closeProject(p);
    }
    Q_ASSERT(KDevelop::ICore::self()->projectController()->projects().isEmpty());
}

QPair<QString, KUrl> makeProject()
{
    QString path;
    QString projectName;
    do {
        projectName = QString("testproject%1").arg(qrand());
        path = QDir::currentPath().append("/").append(projectName);
    } while(QFile::exists(path));
    QDir::current().mkdir(projectName);

    QStringList projectFileContents;
    projectFileContents
    << "[Project]"
    << QString("Name=") + projectName
    << "Manager=KDevGenericManager";

    KUrl projecturl( path + "/simpleproject.kdev4" );
    QFile projectFile(projecturl.toLocalFile());
    projectFile.open(QIODevice::WriteOnly);
    projectFile.write(projectFileContents.join("\n").toAscii());
    projectFile.close();

    return qMakePair(path, projecturl);
}

void ProjectLoadTest::addRemoveFiles()
{
    const QPair<QString, KUrl> p = makeProject();

    QFile f(p.first+"/sdf");
    f.open(QIODevice::WriteOnly);
    f.close();

    KDevelop::ICore::self()->projectController()->openProject(p.second);
    QTest::qWait(500);

    KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->projects().first();
    Q_ASSERT(project->projectFileUrl() == p.second);

    //KDirWatch adds/removes the file automatically
    for (int i=0; i<100; ++i) {
        QFile f2(p.first+"/blub"+QString::number(i));
        f2.open(QIODevice::WriteOnly);
        f2.close();
    }
    for (int i=0; i<50; ++i) {
        QFile f2(p.first+"/blub"+QString::number(i));
        f2.remove();
    }
    QTest::qWait(500);

    QCOMPARE(project->filesForUrl(QString(p.first+"/blub"+QString::number(50))).count(), 1);
    KDevelop::ProjectFileItem* file = project->filesForUrl(QString(p.first+"/blub"+QString::number(50))).first();
    project->projectFileManager()->removeFilesAndFolders(QList<KDevelop::ProjectBaseItem*>() << file ); //message box has to be accepted manually :(
    for (int i=51; i<100; ++i) {
        QFile f2(p.first+"/blub"+QString::number(i));
        f2.remove();
    }

    QTest::qWait(2000);
    QCOMPARE(project->fileCount(), 1);

    foreach (KDevelop::IProject *p, KDevelop::ICore::self()->projectController()->projects()) {
        KDevelop::ICore::self()->projectController()->closeProject(p);
    }
    QTest::qWait(500);
    exec("rm -r "+p.first);
}

void _writeRandomStructure(QString path, int files)
{
    QDir p(path);
    QString name = QString::number(qrand());
    if (qrand() < RAND_MAX / 5) {
        p.mkdir(name);
        path += "/" + name;
        kDebug() << "wrote path" << path;
    } else {
        QFile f(path+"/"+name);
        f.open(QIODevice::WriteOnly);
        f.write(QByteArray::number(qrand()));
        f.write(QByteArray::number(qrand()));
        f.write(QByteArray::number(qrand()));
        f.write(QByteArray::number(qrand()));
        f.close();
        kDebug() << "wrote file" << path+"/"+name;
    }
    files--;
    if (files > 0) {
        _writeRandomStructure(path, files);
    }
}

void fillProject(int /*filesPerDir*/, int /*Dirs*/, const QPair<QString, KUrl> project, bool wait)
{
    QDir(project.first).mkdir("foou");
    _writeRandomStructure(project.first+"/foou", 50);
    for(int i=0; i < 100; ++i) {
        exec("bash -c \"cp -r foou foox"+QString::number(i)+" > /dev/null 2>&1 & \"");
        if (wait) {
            QTest::qWait(100);
        }
    }
}

void ProjectLoadTest::addLotsOfFiles()
{
    QPair<QString, KUrl> p = makeProject();

    KDevelop::ICore::self()->projectController()->openProject(p.second);
    QTest::qWait(2000);

    KDevelop::IProject* project = KDevelop::ICore::self()->projectController()->projects().first();
    Q_ASSERT(project->projectFileUrl() == p.second);

    fillProject(50, 100, p, true);

    foreach (KDevelop::IProject *p, KDevelop::ICore::self()->projectController()->projects()) {
        KDevelop::ICore::self()->projectController()->closeProject(p);
    }

    QTest::qWait(500);
    exec("rm -r "+p.first);
}

void ProjectLoadTest::addMultipleJobs()
{
    QPair<QString, KUrl> p1 = makeProject();
    fillProject(10, 25, p1, false);
    QPair<QString, KUrl> p2 = makeProject();
    fillProject(10, 25, p2, false);

    KDevelop::ICore::self()->projectController()->openProject(p1.second);
    KDevelop::ICore::self()->projectController()->openProject(p2.second);

    QTest::qWait(2000);

    foreach (KDevelop::IProject *p, KDevelop::ICore::self()->projectController()->projects()) {
        KDevelop::ICore::self()->projectController()->closeProject(p);
    }

    QTest::qWait(500);
    exec("rm -r "+p1.first);
    exec("rm -r "+p2.first);
}
