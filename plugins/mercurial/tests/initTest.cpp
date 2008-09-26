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

#include <qtest_kde.h>
#include <QtTest/QtTest>

#include <KUrl>
#include <kio/netaccess.h>

#include <vcs/dvcs/dvcsjob.h>
#include "../hgexecutor.h"

const QString tempDir = QDir::tempPath();
const QString hgTest_BaseDir(tempDir + "/kdevHg_testdir/");
const QString hgTest_BaseDir2(tempDir + "/kdevHg_testdir2/");
const QString hgRepo(hgTest_BaseDir + ".hg");
const QString hgTest_FileName("testfile");

// test for isValidDirectory is required!!!

void HgInitTest::initTestCase()
{
    m_proxy = new HgExecutor;
    removeTempDirs();

    // Now create the basic directory structure
    QDir tmpdir(tempDir);
    tmpdir.mkdir(hgTest_BaseDir);
    tmpdir.mkdir(hgTest_BaseDir2);
}

void HgInitTest::cleanupTestCase()
{
    delete m_proxy;
    removeTempDirs();
}

void HgInitTest::repoInit()
{
    // make job that creates the local repository
    DVCSjob* j = m_proxy->init(KUrl(hgTest_BaseDir));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(hgRepo)).exists() );
}

void HgInitTest::addFiles()
{
    //we start it after repoInit, so we still have empty hg repo
    //First let's create a file
    QFile f(hgTest_BaseDir + hgTest_FileName);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "HELLO WORLD";
    }
    f.flush();

    DVCSjob* j = m_proxy->add(QString(hgTest_BaseDir), KUrl::List(QStringList(hgTest_FileName)));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //since we added the file to the empty repository, .hg/dirstate should exist
    //TODO: maybe other method should be used
    QString testfile(hgRepo + "/dirstate");
    QVERIFY( QFileInfo(testfile).exists() );
}

void HgInitTest::commitFiles()
{
    //we start it after addFiles, so we just have to commit
    DVCSjob* j = m_proxy->commit(QString(hgTest_BaseDir), QString("KDevelop's Test commit"),
                                KUrl::List(QStringList(hgTest_FileName)));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //since we commited the file to the "pure" repository, .hg/store/data/HG_TESTFILE_NAME.i should exist
    //TODO: maybe other method should be used
    QString headRefName(hgRepo + "/store/data/" + hgTest_FileName + ".i");
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
    QFile f(hgTest_BaseDir + hgTest_FileName);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "Just another HELLO WORLD";
    }
    f.flush();

    //Since KJob uses delete later we don't care about deleting pld *j
    j = m_proxy->commit(QString(hgTest_BaseDir), QString("KDevelop's Test commit2"),KUrl::List(QStringList("-A")));
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
    DVCSjob* j = m_proxy->clone(KUrl(hgTest_BaseDir), KUrl(hgTest_BaseDir2));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the .hg directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(hgTest_BaseDir2 + "kdevHg_testdir/.hg/")).exists() );
}

void HgInitTest::testInitAndCommit()
{
    repoInit();
    addFiles();
    commitFiles();
    cloneRepository();
}

void HgInitTest::removeTempDirs()
{
    if (QFileInfo(hgTest_BaseDir).exists() )
        if (!KIO::NetAccess::del(KUrl(QString(hgTest_BaseDir)), 0) )
            qDebug() << "KIO::NetAccess::del(" << hgTest_BaseDir << ") returned false";
    if (QFileInfo(hgTest_BaseDir2).exists() )
        if (!KIO::NetAccess::del(KUrl(QString(hgTest_BaseDir2)), 0) )
            qDebug() << "KIO::NetAccess::del(" << hgTest_BaseDir2 << ") returned false";
}

QTEST_KDEMAIN(HgInitTest, GUI)

#include "initTest.moc"
