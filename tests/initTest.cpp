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

#include <tests/common/kdevtest.h>
#include <QtTest/QtTest>

#include <KUrl>
#include <KDebug>
#include <kio/netaccess.h>

#include <dvcsjob.h>
#include <gitexecutor.h>

#define GITTEST_DIR1                    "kdevGit_testdir"
#define GITTEST_BASEDIR_NO_TR_SLASH     "/tmp/kdevGit_testdir"
#define GITTEST_BASEDIR                 "/tmp/kdevGit_testdir/"
#define GIT_REPO                        GITTEST_BASEDIR".git"
#define GITTEST_BASEDIR2                "/tmp/kdevGit_testdir2/"

#define GIT_SRC_DIR                     GITTEST_BASEDIR"src/"
#define GIT_TESTFILE_NAME               "testfile"
#define GIT_TESTFILE_NAME2              "foo"
#define GIT_TESTFILE_NAME3              "bar"


void GitInitTest::initTestCase()
{
    m_proxy = new GitExecutor;

    // If the basedir for this cvs test exists from a 
    // previous run; remove it...
    if ( QFileInfo(GITTEST_BASEDIR).exists() )
        KIO::NetAccess::del(KUrl(QString(GITTEST_BASEDIR)), 0);
    if ( QFileInfo(GITTEST_BASEDIR2).exists() )
        KIO::NetAccess::del(KUrl(QString(GITTEST_BASEDIR2)), 0);

    // Now create the basic directory structure
    QDir tmpdir("/tmp");
    tmpdir.mkdir(GITTEST_BASEDIR);
    tmpdir.mkdir(GIT_SRC_DIR);
    tmpdir.mkdir(GITTEST_BASEDIR2);
}

void GitInitTest::cleanupTestCase()
{
    delete m_proxy;

   if ( QFileInfo(GITTEST_BASEDIR).exists() )
       KIO::NetAccess::del(KUrl(QString(GITTEST_BASEDIR)), 0);
   if ( QFileInfo(GITTEST_BASEDIR2).exists() )
       KIO::NetAccess::del(KUrl(QString(GITTEST_BASEDIR2)), 0);
}

void GitInitTest::repoInit()
{
    kDebug() << "Trying to init repo";
    // make job that creates the local repository
    DVCSjob* j = m_proxy->init(KUrl(GITTEST_BASEDIR));
    QVERIFY( j );


    // try to start the job
    QVERIFY( j->exec() );

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(GIT_REPO)).exists() );

    //check if isValidDirectory works
    QVERIFY(m_proxy->isValidDirectory(KUrl(GITTEST_BASEDIR)));
    //and for non-git dir, I hope nobody has /tmp under git
    QVERIFY(!m_proxy->isValidDirectory(KUrl("/tmp")));
}

void GitInitTest::addFiles()
{
    kDebug() << "Adding files to the repo";

    //we start it after repoInit, so we still have empty git repo
    QFile f(GITTEST_BASEDIR""GIT_TESTFILE_NAME);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "HELLO WORLD";
    }
    f.flush();
    f.close();
    f.setFileName(GITTEST_BASEDIR""GIT_TESTFILE_NAME2);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "No, bar()!";
    }
    f.flush();
    f.close();

    // /tmp/kdevGit_testdir/ and kdevGit_testdir
    //add always should use relative path to the any directory of the repository, let's check:
    DVCSjob* j = m_proxy->add(QString(GITTEST_BASEDIR), KUrl::List(QStringList(QString(GITTEST_DIR1))));
    QVERIFY( j );

    if (j)
        QVERIFY(j->exec() );
    //Wait the job will be finished
    while(j->status() == KDevelop::VcsJob::JobRunning) 
        ;

    // /tmp/kdevGit_testdir/ and testfile
    j = m_proxy->add(QString(GITTEST_BASEDIR), KUrl::List(QStringList(QString(GIT_TESTFILE_NAME))));
    QVERIFY( j );

    if (j)
        QVERIFY(j->exec() );
    while(j->status() == KDevelop::VcsJob::JobRunning) 
        ;

    //repository path without trailing slash
    j = m_proxy->add(QString(GITTEST_BASEDIR_NO_TR_SLASH), KUrl::List(QStringList(QString(GIT_TESTFILE_NAME))));
    QVERIFY( j );

    if (j)
        QVERIFY(j->exec() );
    while(j->status() == KDevelop::VcsJob::JobRunning) 
        ;

    f.setFileName(GIT_SRC_DIR""GIT_TESTFILE_NAME3);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "No, foo()! It's bar()!";
    }
    f.flush();
    f.close();

    //repository path without trailing slash and a file in a parent directory
    // /tmp/repo  and /tmp/repo/src/bar
    j = m_proxy->add(QString(GITTEST_BASEDIR_NO_TR_SLASH), KUrl::List(QStringList(QString(GIT_SRC_DIR""GIT_TESTFILE_NAME3))));
    QVERIFY( j );

    if (j)
        QVERIFY(j->exec() );
    while(j->status() == KDevelop::VcsJob::JobRunning)
        ;

    //let's use absolute path, because it's used in ContextMenus
    j = m_proxy->add(QString(GITTEST_BASEDIR), KUrl::List(QStringList(QString(GITTEST_BASEDIR""GIT_TESTFILE_NAME2))));
    if (j)
        QVERIFY(j->exec() );
    while(j->status() == KDevelop::VcsJob::JobRunning) 
        ;
}

void GitInitTest::commitFiles()
{
    kDebug() << "\nListing variables with KProcess\n";
    DVCSjob* j_var = m_proxy->var(QString(GITTEST_BASEDIR));
    QVERIFY(j_var->exec() );

    kDebug() << "Committing...";
    //we start it after addFiles, so we just have to commit
    ///TODO: if "" is ok?
    DVCSjob* j = m_proxy->commit(QString(GITTEST_BASEDIR), QString("Test commit"));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //since we commited the file to the "pure" repository, .git/refs/heads/master should exist
    //TODO: maybe other method should be used
    QString headRefName(GIT_REPO"/refs/heads/master");
    QVERIFY( QFileInfo(headRefName).exists() );

    //Test the results of the "git add"
    DVCSjob* jobLs = new DVCSjob(0);
    jobLs->clear();
    jobLs->setDirectory(QString(GITTEST_BASEDIR));
    *jobLs<<"git-ls-tree"<<"--name-only"<<"-r"<<"HEAD";
    if (jobLs) {
        QVERIFY(jobLs->exec() );
        while(jobLs->status() == KDevelop::VcsJob::JobRunning) ;
        QStringList files = jobLs->output().split("\n");
        QVERIFY(files.contains(QString(GIT_TESTFILE_NAME)));
        QVERIFY(files.contains(QString(GIT_TESTFILE_NAME2)));
        QVERIFY(files.contains(QString("src/"GIT_TESTFILE_NAME3)));
    }

    QString firstCommit;
    QFile headRef(headRefName);
    if(headRef.open(QIODevice::ReadOnly)) {
        QTextStream output( &headRef );
        output>>firstCommit;
    }
    headRef.flush();
    headRef.close();

    QVERIFY(firstCommit!="");

    kDebug() << "Committing one more time";
    //let's try to change the file and test "git commit -a"
    QFile f(GITTEST_BASEDIR""GIT_TESTFILE_NAME);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "Just another HELLO WORLD";
    }
    f.flush();

    //add changes
    j = m_proxy->add(QString(GITTEST_BASEDIR), KUrl::List(QStringList(GIT_TESTFILE_NAME)));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    j = m_proxy->commit(QString(GITTEST_BASEDIR), QString("KDevelop's Test commit2"));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    QString secondCommit;
    if(headRef.open(QIODevice::ReadOnly)) {
        QTextStream output( &headRef );
        output>>secondCommit;
    }
    headRef.flush();
    headRef.close();

    QVERIFY(secondCommit!="");
    QVERIFY(firstCommit != secondCommit);
}

void GitInitTest::cloneRepository()
{
    kDebug() << "Do not clone people, clone Git repos!";
    // make job that clones the local repository, created in the previous test
    DVCSjob* j = m_proxy->clone(KUrl(GITTEST_BASEDIR), KUrl(GITTEST_BASEDIR2));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the .git directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(GITTEST_BASEDIR2"kdevGit_testdir/.git/")).exists() );
}

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

KDEVTEST_MAIN(GitInitTest)


// #include "gittest.moc"
