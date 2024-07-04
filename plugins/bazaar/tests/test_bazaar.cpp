/*
    This file was partly taken from KDevelop's cvs plugin
    SPDX-FileCopyrightText: 2007 Robert Gruber <rgruber@users.sourceforge.net>

    Adapted for Git
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    Adapted for Bazaar
    SPDX-FileCopyrightText: 2014 Maciej Poleski

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_bazaar.h"

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <tests/plugintesthelpers.h>

#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

#include <KIO/DeleteJob>
#include <KPluginMetaData>

#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include "../bazaarplugin.h"

#include <QTest>

#define VERIFYJOB(j) \
do { QVERIFY(j); QVERIFY(j->exec()); QVERIFY((j)->status() == KDevelop::VcsJob::JobSucceeded); } while(0)

using namespace KDevelop;

TestBazaar::TestBazaar()
    : tempDir{QDir::tempPath()}
    , bazaarTest_BaseDir{tempDir + QLatin1String("/kdevBazaar_testdir/")}
    , bazaarRepo{bazaarTest_BaseDir + QLatin1String(".bzr")}
    , bazaarSrcDir{bazaarTest_BaseDir + QLatin1String("src/")}
    , bazaarTest_FileName{QStringLiteral("testfile")}
    , bazaarTest_FileName2{QStringLiteral("foo")}
    , bazaarTest_FileName3{QStringLiteral("bar")}
{
}

void TestBazaar::initTestCase()
{
    AutoTestShell::init({QStringLiteral("kdevbazaar")});
    TestCore::initialize(Core::NoUi);

    const auto pluginMetaData = makeTestPluginMetaData("TestBazaar");
    m_plugin = new BazaarPlugin(TestCore::self(), pluginMetaData);
}

void TestBazaar::cleanupTestCase()
{
    delete m_plugin;

    TestCore::shutdown();
}

void TestBazaar::init()
{
    // Now create the basic directory structure
    QDir tmpdir(tempDir);
    tmpdir.mkdir(bazaarTest_BaseDir);
    tmpdir.mkdir(bazaarSrcDir);
}

void TestBazaar::cleanup()
{
    removeTempDirs();
}

void TestBazaar::repoInit()
{
    qDebug() << "Trying to init repo";
    // make job that creates the local repository
    VcsJob* j = m_plugin->init(QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //check if the .bzr directory in the new local repository exists now
    QVERIFY(QFileInfo::exists(bazaarRepo));
}

void TestBazaar::addFiles()
{
    qDebug() << "Adding files to the repo";

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
    VcsJob* j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);

    // /tmp/kdevBazaar_testdir/ and testfile
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir + bazaarTest_FileName));
    VERIFYJOB(j);

    f.setFileName(bazaarSrcDir + bazaarTest_FileName3);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "No, foo()! It's bar()!";
    }

    f.close();

    //test bzr-status exitCode again
    j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //repository path without trailing slash and a file in a parent directory
    // /tmp/repo  and /tmp/repo/src/bar
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(bazaarSrcDir + bazaarTest_FileName3));
    VERIFYJOB(j);

    //let's use absolute path, because it's used in ContextMenus
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir + bazaarTest_FileName2));
    VERIFYJOB(j);

    //Now let's create several files and try "bzr add file1 file2 file3"
    const QStringList files{QStringLiteral("file1"), QStringLiteral("file2"), QStringLiteral("la la")};
    QList<QUrl> multipleFiles;
    for (const QString& file : files) {
        QFile f(bazaarTest_BaseDir + file);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QTextStream input(&f);
        input << file;
        f.close();
        multipleFiles << QUrl::fromLocalFile(bazaarTest_BaseDir + file);
    }
    j = m_plugin->add(multipleFiles);
    VERIFYJOB(j);
}

void TestBazaar::prepareWhoamiInformations()
{
    auto* whoamiJob = new DVcsJob(bazaarTest_BaseDir, m_plugin);
    *whoamiJob<<"bzr"<<"whoami"<<"--branch"<<"kdevbazaar-test identity <>";
    VERIFYJOB(whoamiJob);
}

void TestBazaar::commitFiles()
{
    prepareWhoamiInformations();
    qDebug() << "Committing...";
    //we start it after addFiles, so we just have to commit
    VcsJob* j = m_plugin->commit(QStringLiteral("Test commit"), QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //test bzr-status exitCode one more time.
    j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);

    //since we committed the file to the "pure" repository, .bzr/repository/indices should exist
    //TODO: maybe other method should be used
    QString headRefName(bazaarRepo + "/repository/indices");
    QVERIFY(QFileInfo::exists(headRefName));

    //Test the results of the "bzr add"
    auto* jobLs = new DVcsJob(bazaarTest_BaseDir, m_plugin);
    *jobLs << "bzr" << "ls" << "-R";

    if (jobLs->exec() && jobLs->status() == KDevelop::VcsJob::JobSucceeded) {
        QStringList files = jobLs->output().split('\n');
        QVERIFY(files.contains(bazaarTest_FileName));
        QVERIFY(files.contains(bazaarTest_FileName2));
        QVERIFY(files.contains("src/" + bazaarTest_FileName3));
    }

    qDebug() << "Committing one more time";
    //let's try to change the file and test "bzr commit"
    QFile f(bazaarTest_BaseDir + bazaarTest_FileName);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "Just another HELLO WORLD\n";
    }

    f.close();

    //add changes
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir + bazaarTest_FileName));
    VERIFYJOB(j);

    j = m_plugin->commit(QStringLiteral("KDevelop's Test commit2"), QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);
}

void TestBazaar::testInit()
{
    repoInit();
}

void TestBazaar::testAdd()
{
    repoInit();
    addFiles();
}

void TestBazaar::testCommit()
{
    repoInit();
    addFiles();
    commitFiles();
}

void TestBazaar::testAnnotation()
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

    VcsJob* j = m_plugin->commit(QStringLiteral("KDevelop's Test commit3"), QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);

    j = m_plugin->annotate(QUrl::fromLocalFile(bazaarTest_BaseDir + bazaarTest_FileName), VcsRevision::createSpecialRevision(VcsRevision::Head));
    VERIFYJOB(j);

    QList<QVariant> results = j->fetchResults().toList();
    QCOMPARE(results.size(), 2);
    QVERIFY(results.at(0).canConvert<VcsAnnotationLine>());
    VcsAnnotationLine annotation = results.at(0).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 0);
    QCOMPARE(annotation.commitMessage(), QStringLiteral("KDevelop's Test commit2"));

    QVERIFY(results.at(1).canConvert<VcsAnnotationLine>());
    annotation = results.at(1).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 1);
    QCOMPARE(annotation.commitMessage(), QStringLiteral("KDevelop's Test commit3"));
}

void TestBazaar::testRemoveEmptyFolder()
{
    repoInit();

    QDir d(bazaarTest_BaseDir);
    d.mkdir(QStringLiteral("emptydir"));

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir+"emptydir/"));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists("emptydir"));
}

void TestBazaar::testRemoveEmptyFolderInFolder()
{
    repoInit();

    QDir d(bazaarTest_BaseDir);
    d.mkdir(QStringLiteral("dir"));

    QDir d2(bazaarTest_BaseDir+"dir");
    d2.mkdir(QStringLiteral("emptydir"));

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir+"dir/"));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists("dir"));
}

void TestBazaar::testRemoveUnindexedFile()
{
    repoInit();

    QFile f(bazaarTest_BaseDir + bazaarTest_FileName);
    QVERIFY(f.open(QIODevice::Append));
    QTextStream input(&f);
    input << "An appended line";
    f.close();

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir + bazaarTest_FileName));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(bazaarTest_BaseDir + bazaarTest_FileName));
}

void TestBazaar::testRemoveFolderContainingUnversionedFiles()
{
    repoInit();

    QDir d(bazaarTest_BaseDir);
    d.mkdir(QStringLiteral("dir"));

    {
        QFile f(bazaarTest_BaseDir + "dir/foo");
        QVERIFY(f.open(QIODevice::Append));
        QTextStream input(&f);
        input << "An appended line";
        f.close();
    }
    VcsJob* j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir+"dir"), IBasicVersionControl::NonRecursive);
    VERIFYJOB(j);
    prepareWhoamiInformations();
    j = m_plugin->commit(QStringLiteral("initial commit"), QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir));
    VERIFYJOB(j);

    {
        QFile f(bazaarTest_BaseDir + "dir/bar");
        QVERIFY(f.open(QIODevice::Append));
        QTextStream input(&f);
        input << "An appended line";
        f.close();
    }

    j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(bazaarTest_BaseDir + "dir"));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(bazaarTest_BaseDir + "dir"));

}


void TestBazaar::removeTempDirs()
{
    if (QFileInfo::exists(bazaarTest_BaseDir))
        if (!(KIO::del(QUrl::fromLocalFile(bazaarTest_BaseDir))->exec()))
            qDebug() << "KIO::del(" << bazaarTest_BaseDir << ") returned false";
}

QTEST_MAIN(TestBazaar)

#include "moc_test_bazaar.cpp"
