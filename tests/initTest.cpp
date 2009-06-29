/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
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

#include "initTest.h"

#include <qtest_kde.h>
#include <QtTest/QtTest>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <KUrl>
#include <KDebug>
#include <kio/netaccess.h>

#include <vcs/dvcs/dvcsjob.h>
#include "../gitplugin.h"

const QString tempDir = QDir::tempPath();
const QString gitTest_BaseDir(tempDir + "/kdevGit_testdir/");
const QString gitTest_BaseDir2(tempDir + "/kdevGit_testdir2/");
const QString gitRepo(gitTest_BaseDir + ".git");
const QString gitSrcDir(gitTest_BaseDir + "src/");
const QString gitTest_FileName("testfile");
const QString gitTest_FileName2("foo");
const QString gitTest_FileName3("bar");

using namespace KDevelop;

void GitInitTest::initTestCase()
{
    AutoTestShell::init();
    m_testCore = new KDevelop::TestCore();
    m_testCore->initialize(KDevelop::Core::NoUi);
    m_plugin = new GitPlugin(m_testCore);
    removeTempDirs();

    // Now create the basic directory structure
    QDir tmpdir(tempDir);
    tmpdir.mkdir(gitTest_BaseDir);
    tmpdir.mkdir(gitSrcDir);
    tmpdir.mkdir(gitTest_BaseDir2);
}

void GitInitTest::cleanupTestCase()
{
    delete m_plugin;
    m_testCore->cleanup();
    delete m_testCore;
    if (QFileInfo(gitTest_BaseDir).exists())
        KIO::NetAccess::del(KUrl(gitTest_BaseDir), 0);

    if (QFileInfo(gitTest_BaseDir2).exists())
        KIO::NetAccess::del(KUrl(gitTest_BaseDir2), 0);
}

void GitInitTest::repoInit()
{
    kDebug() << "Trying to init repo";
    // make job that creates the local repository
    VcsJob* j = m_plugin->init(KUrl(gitTest_BaseDir));
    QVERIFY(j);


    // try to start the job
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY(QFileInfo(gitRepo).exists());

    //check if isValidDirectory works
    QVERIFY(m_plugin->isValidDirectory(KUrl(gitTest_BaseDir)));
    //and for non-git dir, I hope nobody has /tmp under git
    QVERIFY(!m_plugin->isValidDirectory(KUrl("/tmp")));

    //we have nothing, so ouput should be empty
    DVcsJob * j2 = m_plugin->gitRevParse(gitRepo, QStringList(QString("--branches")));
    QVERIFY(j2);
    QVERIFY(j2->exec());
    QString out = j2->output();
    QVERIFY(j2->output().isEmpty());
}

void GitInitTest::addFiles()
{
    kDebug() << "Adding files to the repo";

    //we start it after repoInit, so we still have empty git repo
    QFile f(gitTest_BaseDir + gitTest_FileName);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "HELLO WORLD";
    }

    f.flush();

    f.close();
    f.setFileName(gitTest_BaseDir + gitTest_FileName2);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "No, bar()!";
    }

    f.flush();

    f.close();

    //test git-status exitCode (see DVcsJob::setExitCode).
    VcsJob* j = m_plugin->status(KUrl::List(gitTest_BaseDir));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    // /tmp/kdevGit_testdir/ and testfile
    j = m_plugin->add(KUrl::List(gitTest_BaseDir + gitTest_FileName));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    f.setFileName(gitSrcDir + gitTest_FileName3);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "No, foo()! It's bar()!";
    }

    f.flush();

    f.close();

    //test git-status exitCode again
    j = m_plugin->status(KUrl::List(gitTest_BaseDir));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    //repository path without trailing slash and a file in a parent directory
    // /tmp/repo  and /tmp/repo/src/bar
    j = m_plugin->add(KUrl::List(QStringList(gitSrcDir + gitTest_FileName3)));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    //let's use absolute path, because it's used in ContextMenus
    j = m_plugin->add(KUrl::List(QStringList(gitTest_BaseDir + gitTest_FileName2)));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    //Now let's create several files and try "git add file1 file2 file3"
    f.setFileName(gitTest_BaseDir + "file1");

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "file1";
    }

    f.flush();

    f.close();
    f.setFileName(gitTest_BaseDir + "file2");

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "file2";
    }

    f.flush();

    f.close();
    QStringList multipleFiles;
    multipleFiles << (gitTest_BaseDir + "file1");
    multipleFiles << (gitTest_BaseDir + "file2");
    j = m_plugin->add(KUrl::List(multipleFiles));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);
}

void GitInitTest::commitFiles()
{
    kDebug() << "\nListing variables with KProcess\n";
    DVcsJob* j_var = m_plugin->var(gitTest_BaseDir);
    QVERIFY(j_var->exec());
    QVERIFY(j_var->status() == KDevelop::VcsJob::JobSucceeded);

    kDebug() << "Committing...";
    //we start it after addFiles, so we just have to commit
    ///TODO: if "" is ok?
    VcsJob* j = m_plugin->commit(QString("Test commit"), KUrl::List(gitTest_BaseDir));
    QVERIFY(j);

    // try to start the job
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    //test git-status exitCode one more time.
    j = m_plugin->status(KUrl::List(gitTest_BaseDir));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    //since we commited the file to the "pure" repository, .git/refs/heads/master should exist
    //TODO: maybe other method should be used
    QString headRefName(gitRepo + "/refs/heads/master");
    QVERIFY(QFileInfo(headRefName).exists());

    //Test the results of the "git add"
    DVcsJob* jobLs = new DVcsJob(0);
    jobLs->clear();
    jobLs->setDirectory(gitTest_BaseDir);
    *jobLs << "git" << "ls-tree" << "--name-only" << "-r" << "HEAD";

    if (jobLs) {
        QVERIFY(jobLs->exec());
        QVERIFY(jobLs->status() == KDevelop::VcsJob::JobSucceeded);

        QStringList files = jobLs->output().split("\n");
        QVERIFY(files.contains(gitTest_FileName));
        QVERIFY(files.contains(gitTest_FileName2));
        QVERIFY(files.contains("src/" + gitTest_FileName3));
    }

    QString firstCommit;

    QFile headRef(headRefName);

    if (headRef.open(QIODevice::ReadOnly)) {
        QTextStream output(&headRef);
        output >> firstCommit;
    }

    headRef.flush();

    headRef.close();

    QVERIFY(firstCommit != "");

    kDebug() << "Committing one more time";
    //let's try to change the file and test "git commit -a"
    QFile f(gitTest_BaseDir + gitTest_FileName);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "Just another HELLO WORLD";
    }

    f.flush();

    //add changes
    j = m_plugin->add(KUrl::List(QStringList(gitTest_BaseDir + gitTest_FileName)));
    QVERIFY(j);

    // try to start the job
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    j = m_plugin->commit(QString("KDevelop's Test commit2"), KUrl::List(gitTest_BaseDir));
    QVERIFY(j);

    // try to start the job
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    QString secondCommit;

    if (headRef.open(QIODevice::ReadOnly)) {
        QTextStream output(&headRef);
        output >> secondCommit;
    }

    headRef.flush();

    headRef.close();

    QVERIFY(secondCommit != "");
    QVERIFY(firstCommit != secondCommit);

}

// void GitInitTest::cloneRepository()
// {
//     kDebug() << "Do not clone people, clone Git repos!";
//     // make job that clones the local repository, created in the previous test
//     DVcsJob* j = m_proxy->createWorkingCopy(KUrl(gitTest_BaseDir), KUrl(gitTest_BaseDir2));
//     QVERIFY( j );
//
//     // try to start the job
//     QVERIFY( j->exec() );
//
//     //check if the .git directory in the new local repository exists now
//     QVERIFY( QFileInfo(QString(gitTest_BaseDir2"kdevGit_testdir/.git/")).exists() );
// }

void GitInitTest::testInit()
{
    repoInit();
}

void GitInitTest::testAdd()
{
    addFiles();
}

void GitInitTest::testCommit()
{
    commitFiles();
}

void GitInitTest::testBranching()
{
    DVcsJob* j = m_plugin->branch(gitTest_BaseDir);
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);

    QString curBranch = m_plugin->curBranch(gitTest_BaseDir);
    QCOMPARE(curBranch, QString("master"));

    QString newBranch("new");
    j = m_plugin->branch(gitTest_BaseDir, QString("master"), newBranch);
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);
    QVERIFY(m_plugin->branches(gitTest_BaseDir).contains(newBranch));

    j = m_plugin->switchBranch(gitTest_BaseDir, newBranch);
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);
    QCOMPARE(m_plugin->curBranch(gitTest_BaseDir), newBranch);

    j = m_plugin->branch(gitTest_BaseDir, QString("master"), QString(), QStringList("-D"));
    QVERIFY(j);
    QVERIFY(j->exec());
    QVERIFY(j->status() == KDevelop::VcsJob::JobSucceeded);
    QVERIFY(!m_plugin->branches(gitTest_BaseDir).contains(QString("master")));
}

void GitInitTest::revHistory()
{
    QList<DVcsEvent> commits = m_plugin->getAllCommits(gitTest_BaseDir);
    QVERIFY(!commits.isEmpty());
    QStringList logMessages;

    for (int i = 0; i < commits.count(); ++i)
        logMessages << commits[i].getLog();

    QCOMPARE(commits.count(), 2);

    QCOMPARE(logMessages[0], QString("KDevelop's Test commit2"));  //0 is later than 1!

    QCOMPARE(logMessages[1], QString("Test commit"));

    QVERIFY(commits[1].getParents().isEmpty());  //0 is later than 1!

    QVERIFY(!commits[0].getParents().isEmpty()); //initial commit is on the top

    QVERIFY(commits[1].getCommit().contains(QRegExp("^\\w{,40}$")));

    QVERIFY(commits[0].getCommit().contains(QRegExp("^\\w{,40}$")));

    QVERIFY(commits[0].getParents()[0].contains(QRegExp("^\\w{,40}$")));
}

void GitInitTest::removeTempDirs()
{
    if (QFileInfo(gitTest_BaseDir).exists())
        if (!KIO::NetAccess::del(KUrl(gitTest_BaseDir), 0))
            qDebug() << "KIO::NetAccess::del(" << gitTest_BaseDir << ") returned false";

    if (QFileInfo(gitTest_BaseDir2).exists())
        if (!KIO::NetAccess::del(KUrl(gitTest_BaseDir2), 0))
            qDebug() << "KIO::NetAccess::del(" << gitTest_BaseDir2 << ") returned false";
}

QTEST_KDEMAIN(GitInitTest, GUI)

// #include "gittest.moc"
