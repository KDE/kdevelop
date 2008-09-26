/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Bazaar                                                    *
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
#include <KDebug>
#include <kio/netaccess.h>

#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>

#include <vcs/interfaces/ibasicversioncontrol.h>

#include <vcs/dvcs/dvcsjob.h>
#include "../bzrexecutor.h"

const QString tempDir = QDir::tempPath();
const QString bzrTest_BaseDir(tempDir + "/kdevBazaar_testdir/");
const QString bzrTest_BaseDir2(tempDir + "/kdevBazaar_testdir2/");
const QString bzrRepo(bzrTest_BaseDir + ".bzr");
const QString bzrTest_FileName("testfile");


//TODO: bugs, this test didn't notice:
//* git commit home/... wasn't notice
// test for isValidDirectory is required!!!

void BzrInitTest::initTestCase()
{
    m_proxy = new BzrExecutor;

    // If the basedir for this cvs test exists from a 
    // previous run; remove it...
    if ( QFileInfo(bzrTest_BaseDir).exists() )
        KIO::NetAccess::del(KUrl(QString(bzrTest_BaseDir)), 0);
    if ( QFileInfo(bzrTest_BaseDir2).exists() )
        KIO::NetAccess::del(KUrl(QString(bzrTest_BaseDir2)), 0);

    // Now create the basic directory structure
    QDir tmpdir(tempDir);
    tmpdir.mkdir(bzrTest_BaseDir);
    tmpdir.mkdir(bzrTest_BaseDir2);
}

void BzrInitTest::cleanupTestCase()
{
    delete m_proxy;

   if ( QFileInfo(bzrTest_BaseDir).exists() )
       KIO::NetAccess::del(KUrl(QString(bzrTest_BaseDir)), 0);
   if ( QFileInfo(bzrTest_BaseDir2).exists() )
       KIO::NetAccess::del(KUrl(QString(bzrTest_BaseDir2)), 0);
}

void BzrInitTest::repoInit()
{
    // make job that creates the local repository
    DVCSjob* j = m_proxy->init(KUrl(bzrTest_BaseDir));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(bzrRepo)).exists() );
}

void BzrInitTest::addFiles()
{
    //we start it after repoInit, so we still have empty bazaar repo
    //First let's create a file
    QFile f(bzrTest_BaseDir + bzrTest_FileName);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "HELLO WORLD";
    }
    f.flush();

    DVCSjob* j = m_proxy->add(QString(bzrTest_BaseDir), KUrl::List(QStringList(bzrTest_FileName)));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

//TODO: Bazaar just change checkout/dirstate created by bzr init
//     //TODO: maybe other method should be used
//     QString testfile(BZR_REPO"/dirstate");
//     QVERIFY( QFileInfo(testfile).exists() );
}

void BzrInitTest::commitFiles()
{
    //we start it after addFiles, so we just have to commit
    DVCSjob* j = m_proxy->commit(QString(bzrTest_BaseDir), QString("KDevelop's Test commit"),
                                KUrl::List(QStringList(bzrTest_FileName)));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //since we commited the file to the "pure" repository, .bzr/store/data/BZR_TESTFILE_NAME.i should exist
    //TODO: maybe other method should be used
    QString headRefName(bzrRepo + "/repository/indices/");
    QVERIFY( QFileInfo(headRefName).exists() );

    QString firstCommit;
    QFile headRef(headRefName);
    if(headRef.open(QIODevice::ReadOnly)) {
        QTextStream output( &headRef );
        output>>firstCommit;
    }
    headRef.flush();
    headRef.close();

//     QVERIFY(firstCommit!="");

    //let's try to change the file and test "bazaar commit -A"
    QFile f(bzrTest_BaseDir + bzrTest_FileName);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "Just another HELLO WORLD";
    }
    f.flush();

    //Since KJob uses delete later we don't care about deleting pld *j
    j = m_proxy->commit(QString(bzrTest_BaseDir), QString("KDevelop's Test commit2"));
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

void BzrInitTest::cloneRepository()
{
    // make job that clones the local repository, created in the previous test
    DVCSjob* j = m_proxy->clone(KUrl(bzrTest_BaseDir), KUrl(bzrTest_BaseDir2));
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the .bzr directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(bzrTest_BaseDir2 + "kdevBazaar_testdir/.bzr/")).exists() );
}

void BzrInitTest::testInitAndCommit()
{
    repoInit();
    addFiles();
    commitFiles();
    cloneRepository();
}

using namespace KDevelop;

void BzrInitTest::testPlugin()
{
//     Core::initialize();
//     Core *core;
//     core->initialize();
//     IPluginController* controller = core->pluginController();

//     IPlugin* bzrPlugin = controller->loadPlugin(QString("kdevbzr"));
//     QVERIFY(bzrPlugin);
// 
//     KPluginInfo bzrInfo = controller->pluginInfo(bzrPlugin);
//     QVERIFY(bzrInfo.isValid());
// 
//     //TODO: compare name() etc
//     kDebug()<<"name: "<<bzrInfo.name()<<" pluginName: "<<bzrInfo.pluginName();
// 
//     IBasicVersionControl* iface = bzrPlugin->extension<IBasicVersionControl>();
//     QVERIFY(iface);
}

void BzrInitTest::removeTempDirs()
{
    if (QFileInfo(bzrTest_BaseDir).exists() )
        if (!KIO::NetAccess::del(KUrl(QString(bzrTest_BaseDir)), 0) )
            qDebug() << "KIO::NetAccess::del(" << bzrTest_BaseDir << ") returned false";
    if (QFileInfo(bzrTest_BaseDir2).exists() )
        if (!KIO::NetAccess::del(KUrl(QString(bzrTest_BaseDir2)), 0) )
            qDebug() << "KIO::NetAccess::del(" << bzrTest_BaseDir2 << ") returned false";
}

QTEST_KDEMAIN(BzrInitTest, GUI)


#include "initTest.moc"
