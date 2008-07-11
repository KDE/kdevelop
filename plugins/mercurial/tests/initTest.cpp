/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Hg                                                       *
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

#include <QtTest/QtTest>
#include <KUrl>

#include <dvcsjob.h>
#include <hgexecutor.h>

#define HGTEST_DIR1            "kdevHg_testdir"
#define HGTEST_BASEDIR         "/tmp/kdevHg_testdir/"
#define HG_REPO                HGTEST_BASEDIR".hg"
#define HGTEST_BASEDIR2        "/tmp/kdevHg_testdir2/"
#define HG_TESTFILE_NAME       "testfile"

//TODO: bugs, this test didn't notice:
//* git commit home/... wasn't notice
// test for isValidDirectory is required!!!

void HgInitTest::initTestCase()
{
    m_proxy = new HgExecutor;

    // If the basedir for this cvs test exists from a 
    // previous run; remove it...
    if ( QFileInfo(HGTEST_BASEDIR).exists() ) {
        system("rm -rf "HGTEST_BASEDIR"/*");
    }
    if ( QFileInfo(HGTEST_BASEDIR2).exists() ) {
        system("rm -rf "HGTEST_BASEDIR2"/*");
    }

    // Now create the basic directory structure
    QDir tmpdir("/tmp");
    tmpdir.mkdir(HGTEST_BASEDIR);
    tmpdir.mkdir(HGTEST_BASEDIR2);
}

void HgInitTest::cleanupTestCase()
{
    delete m_proxy;

   if ( QFileInfo(HGTEST_BASEDIR).exists() ) {
       system("rm -rf "HGTEST_BASEDIR);
   }
   if ( QFileInfo(HGTEST_BASEDIR2).exists() ) {
       system("rm -rf "HGTEST_BASEDIR2);
   }
}

void HgInitTest::repoInit()
{
    // make job that creates the local repository
    DVCSjob* j = m_proxy->init(KUrl(HGTEST_BASEDIR));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(HG_REPO)).exists() );
}

void HgInitTest::addFiles()
{
    //we start it after repoInit, so we still have empty hg repo
    //First let's create a file
    QFile f(HGTEST_BASEDIR""HG_TESTFILE_NAME);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "HELLO WORLD";
    }
    f.flush();

    DVCSjob* j = m_proxy->add(QString(HGTEST_BASEDIR), KUrl::List(QStringList(HG_TESTFILE_NAME)));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //since we added the file to the empty repository, .hg/dirstate should exist
    //TODO: maybe other method should be used
    QString testfile(HG_REPO"/dirstate");
    QVERIFY( QFileInfo(testfile).exists() );
}

void HgInitTest::commitFiles()
{
    //we start it after addFiles, so we just have to commit
    DVCSjob* j = m_proxy->commit(QString(HGTEST_BASEDIR), QString("KDevelop's Test commit"),
                                KUrl::List(QStringList(HG_TESTFILE_NAME)));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //since we commited the file to the "pure" repository, .hg/store/data/HG_TESTFILE_NAME.i should exist
    //TODO: maybe other method should be used
    QString headRefName(HG_REPO"/store/data/"HG_TESTFILE_NAME".i");
    QVERIFY( QFileInfo(headRefName).exists() );

    QString firstCommit;
    QFile headRef(headRefName);
    if(headRef.open(QIODevice::ReadOnly)) {
        QTextStream output( &headRef );
        output>>firstCommit;
    }
    headRef.flush();
    headRef.close();

    QVERIFY(firstCommit!="");

    //let's try to change the file and test "hg commit -A"
    QFile f(HGTEST_BASEDIR""HG_TESTFILE_NAME);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "Just another HELLO WORLD";
    }
    f.flush();

    //Since KJob uses delete later we don't care about deleting pld *j
    j = m_proxy->commit(QString(HGTEST_BASEDIR), QString("KDevelop's Test commit2"),KUrl::List(QStringList("-A")));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

//     QString secondCommit;
//     if(headRef.open(QIODevice::ReadOnly)) {
//         QTextStream output( &headRef );
//         output>>secondCommit;
//     }
//     headRef.flush();
//     headRef.close();
// 
//     QVERIFY(secondCommit!="");
//     QVERIFY(firstCommit != secondCommit);
}

void HgInitTest::cloneRepository()
{
    // make job that clones the local repository, created in the previous test
    DVCSjob* j = m_proxy->clone(KUrl(HGTEST_BASEDIR), KUrl(HGTEST_BASEDIR2));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the .hg directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(HGTEST_BASEDIR2"kdevHg_testdir/.hg/")).exists() );
}

void HgInitTest::testInitAndCommit()
{
    repoInit();
    addFiles();
    commitFiles();
    cloneRepository();
}


QTEST_MAIN(HgInitTest)


// #include "gittest.moc"
