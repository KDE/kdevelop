/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Adapted for Bazaar                                                    *
 *   Copyright 2014 Maciej Poleski                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "bazaartest.h"

#include <qtest_kde.h>
#include <QtTest/QtTest>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <KUrl>
#include <KDebug>
#include <kio/netaccess.h>

#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include "../src/bazaarplugin.h"

#define VERIFYJOB(j) \
do { QVERIFY(j); QVERIFY(j->exec()); QVERIFY((j)->status() == KDevelop::VcsJob::JobSucceeded); } while(0)

const QString tempDir = QDir::tempPath();
const QString bazaarTest_BaseDir(tempDir + "/kdevBazaar_testdir/");
const QString bazaarTest_BaseDir2(tempDir + "/kdevBazaar_testdir2/");
const QString bazaarRepo(bazaarTest_BaseDir + ".bzr");
const QString bazaarSrcDir(bazaarTest_BaseDir + "src/");
const QString bazaarTest_FileName("testfile");
const QString bazaarTest_FileName2("foo");
const QString bazaarTest_FileName3("bar");

using namespace KDevelop;

void BazaarTest::init()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
    m_plugin = new BazaarPlugin(TestCore::self());
    removeTempDirs();

    // Now create the basic directory structure
    QDir tmpdir(tempDir);
    tmpdir.mkdir(bazaarTest_BaseDir);
    tmpdir.mkdir(bazaarSrcDir);
    tmpdir.mkdir(bazaarTest_BaseDir2);
}

void BazaarTest::cleanup()
{
    delete m_plugin;
    TestCore::shutdown();
    if (QFileInfo(bazaarTest_BaseDir).exists())
        KIO::NetAccess::del(KUrl(bazaarTest_BaseDir), 0);

    if (QFileInfo(bazaarTest_BaseDir2).exists())
        KIO::NetAccess::del(KUrl(bazaarTest_BaseDir2), 0);
}


void BazaarTest::repoInit()
{
    kDebug() << "Trying to init repo";
    // make job that creates the local repository
    VcsJob* j = m_plugin->init(KUrl(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //check if the .bzr directory in the new local repository exists now
    QVERIFY(QFileInfo(bazaarRepo).exists());
}

void BazaarTest::addFiles()
{
    kDebug() << "Adding files to the repo";

    //we start it after repoInit, so we still have empty bazaar repo
    QFile f(bazaarTest_BaseDir + bazaarTest_FileName);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "HELLO WORLD";
    }

    f.close();
    f.setFileName(bazaarTest_BaseDir + bazaarTest_FileName2);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "No, bar()!";
    }

    f.close();

    //test bzr-status exitCode (see DVcsJob::setExitCode).
    VcsJob* j = m_plugin->status(KUrl::List(bazaarTest_BaseDir));
    VERIFYJOB(j);

    // /tmp/kdevBazaar_testdir/ and testfile
    j = m_plugin->add(KUrl::List(QString(bazaarTest_BaseDir + bazaarTest_FileName)));
    VERIFYJOB(j);

    f.setFileName(bazaarSrcDir + bazaarTest_FileName3);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "No, foo()! It's bar()!";
    }

    f.close();

    //test bzr-status exitCode again
    j = m_plugin->status(KUrl::List(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //repository path without trailing slash and a file in a parent directory
    // /tmp/repo  and /tmp/repo/src/bar
    j = m_plugin->add(KUrl::List(QStringList(bazaarSrcDir + bazaarTest_FileName3)));
    VERIFYJOB(j);

    //let's use absolute path, because it's used in ContextMenus
    j = m_plugin->add(KUrl::List(QStringList(bazaarTest_BaseDir + bazaarTest_FileName2)));
    VERIFYJOB(j);

    //Now let's create several files and try "bzr add file1 file2 file3"
    QStringList files = QStringList() << "file1" << "file2" << "la la";
    KUrl::List multipleFiles;
    foreach(const QString& file, files) {
        QFile f(bazaarTest_BaseDir + file);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QTextStream input(&f);
        input << file;
        f.close();
        multipleFiles << KUrl::fromPath(bazaarTest_BaseDir + file);
    }
    j = m_plugin->add(multipleFiles);
    VERIFYJOB(j);
}

void BazaarTest::commitFiles()
{
    kDebug() << "Committing...";
    //we start it after addFiles, so we just have to commit
    VcsJob* j = m_plugin->commit(QString("Test commit"), KUrl::List(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //test bzr-status exitCode one more time.
    j = m_plugin->status(KUrl::List(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //since we committed the file to the "pure" repository, .bzr/repository/indices should exist
    //TODO: maybe other method should be used
    QString headRefName(bazaarRepo + "/repository/indices");
    QVERIFY(QFileInfo(headRefName).exists());

    //Test the results of the "bzr add"
    DVcsJob* jobLs = new DVcsJob(bazaarTest_BaseDir, m_plugin);
    *jobLs << "bzr" << "ls" << "-R";

    if (jobLs->exec() && jobLs->status() == KDevelop::VcsJob::JobSucceeded) {
        QStringList files = jobLs->output().split('\n');
        QVERIFY(files.contains(bazaarTest_FileName));
        QVERIFY(files.contains(bazaarTest_FileName2));
        QVERIFY(files.contains("src/" + bazaarTest_FileName3));
    }

    kDebug() << "Committing one more time";
    //let's try to change the file and test "bzr commit"
    QFile f(bazaarTest_BaseDir + bazaarTest_FileName);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "Just another HELLO WORLD\n";
    }

    f.close();

    //add changes
    j = m_plugin->add(KUrl::List(QStringList(bazaarTest_BaseDir + bazaarTest_FileName)));
    VERIFYJOB(j);

    j = m_plugin->commit(QString("KDevelop's Test commit2"), KUrl::List(bazaarTest_BaseDir));
    VERIFYJOB(j);
}

void BazaarTest::testInit()
{
    repoInit();
}

void BazaarTest::testAdd()
{
    repoInit();
    addFiles();
}

void BazaarTest::testCommit()
{
    repoInit();
    addFiles();
    commitFiles();
}

void BazaarTest::testAnnotation()
{
    repoInit();
    addFiles();
    commitFiles();

    // called after commitFiles
    QFile f(bazaarTest_BaseDir + bazaarTest_FileName);
    QVERIFY(f.open(QIODevice::Append));
    QTextStream input(&f);
    input << "An appended line";
    f.close();

    VcsJob* j = m_plugin->commit(QString("KDevelop's Test commit3"), KUrl::List(bazaarTest_BaseDir));
    VERIFYJOB(j);

    j = m_plugin->annotate(KUrl(bazaarTest_BaseDir + bazaarTest_FileName), VcsRevision::createSpecialRevision(VcsRevision::Head));
    VERIFYJOB(j);

    QList<QVariant> results = j->fetchResults().toList();
    QCOMPARE(results.size(), 2);
    QVERIFY(results.at(0).canConvert<VcsAnnotationLine>());
    VcsAnnotationLine annotation = results.at(0).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 0);
    QCOMPARE(annotation.commitMessage(), QString("KDevelop's Test commit2"));

    QVERIFY(results.at(1).canConvert<VcsAnnotationLine>());
    annotation = results.at(1).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 1);
    QCOMPARE(annotation.commitMessage(), QString("KDevelop's Test commit3"));
}

void BazaarTest::testRemoveEmptyFolder()
{
    repoInit();

    QDir d(bazaarTest_BaseDir);
    d.mkdir("emptydir");

    VcsJob* j = m_plugin->remove(KUrl::List(KUrl::fromLocalFile(bazaarTest_BaseDir+"emptydir/")));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists("emptydir"));
}

void BazaarTest::testRemoveEmptyFolderInFolder()
{
    repoInit();

    QDir d(bazaarTest_BaseDir);
    d.mkdir("dir");

    QDir d2(bazaarTest_BaseDir+"dir");
    d2.mkdir("emptydir");

    VcsJob* j = m_plugin->remove(KUrl::List(KUrl::fromLocalFile(bazaarTest_BaseDir+"dir/")));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists("dir"));
}

void BazaarTest::testRemoveUnindexedFile()
{
    repoInit();

    QFile f(bazaarTest_BaseDir + bazaarTest_FileName);
    QVERIFY(f.open(QIODevice::Append));
    QTextStream input(&f);
    input << "An appended line";
    f.close();

    VcsJob* j = m_plugin->remove(KUrl::List(KUrl::fromLocalFile(bazaarTest_BaseDir + bazaarTest_FileName)));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(bazaarTest_BaseDir + bazaarTest_FileName));
}

void BazaarTest::testRemoveFolderContainingUnversionedFiles()
{
    repoInit();

    QDir d(bazaarTest_BaseDir);
    d.mkdir("dir");

    {
        QFile f(bazaarTest_BaseDir + "dir/foo");
        QVERIFY(f.open(QIODevice::Append));
        QTextStream input(&f);
        input << "An appended line";
        f.close();
    }
    VcsJob* j = m_plugin->add(KUrl::List(KUrl::fromLocalFile(bazaarTest_BaseDir+"dir")),IBasicVersionControl::NonRecursive);
    VERIFYJOB(j);
    j = m_plugin->commit("initial commit", KUrl::List(KUrl::fromLocalFile(bazaarTest_BaseDir)));
    VERIFYJOB(j);

    {
        QFile f(bazaarTest_BaseDir + "dir/bar");
        QVERIFY(f.open(QIODevice::Append));
        QTextStream input(&f);
        input << "An appended line";
        f.close();
    }

    j = m_plugin->remove(KUrl::List(KUrl::fromLocalFile(bazaarTest_BaseDir + "dir")));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(bazaarTest_BaseDir + "dir"));

}


void BazaarTest::removeTempDirs()
{
    if (QFileInfo(bazaarTest_BaseDir).exists())
        if (!KIO::NetAccess::del(KUrl(bazaarTest_BaseDir), 0))
            qDebug() << "KIO::NetAccess::del(" << bazaarTest_BaseDir << ") returned false";

    if (QFileInfo(bazaarTest_BaseDir2).exists())
        if (!KIO::NetAccess::del(KUrl(bazaarTest_BaseDir2), 0))
            qDebug() << "KIO::NetAccess::del(" << bazaarTest_BaseDir2 << ") returned false";
}

QTEST_KDEMAIN(BazaarTest, GUI)

#include "bazaartest.moc"
