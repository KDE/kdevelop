/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "test_cvs.h"

#include <QtTest/QtTest>
#include <QUrl>

#include <KIO/DeleteJob>

#include <cvsjob.h>
#include <cvsproxy.h>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#define CVSTEST_BASEDIR         "/tmp/kdevcvs_testdir/"
#define CVS_REPO                CVSTEST_BASEDIR "repo/"
#define CVS_IMPORT              CVSTEST_BASEDIR "import/"
#define CVS_TESTFILE_NAME       "testfile"
#define CVS_CHECKOUT            CVSTEST_BASEDIR "working/"

// we need to add this since it is declared in cvsplugin.cpp which we don't compile here
Q_LOGGING_CATEGORY(PLUGIN_CVS, "kdevplatform.plugins.cvs")

void TestCvs::initTestCase()
{
    KDevelop::AutoTestShell::init();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    m_proxy = new CvsProxy;

    // If the basedir for this cvs test exists from a
    // previous run; remove it...
    cleanup();
}

void TestCvs::cleanupTestCase()
{
    KDevelop::TestCore::shutdown();

    delete m_proxy;
}

void TestCvs::init()
{
    // Now create the basic directory structure
    QDir tmpdir("/tmp");
    tmpdir.mkdir(CVSTEST_BASEDIR);
    tmpdir.mkdir(CVS_REPO);
    tmpdir.mkdir(CVS_IMPORT);
}

void TestCvs::cleanup()
{
    if ( QFileInfo(CVSTEST_BASEDIR).exists() )
        KIO::del(QUrl::fromLocalFile(QString(CVSTEST_BASEDIR)))->exec();
}

void TestCvs::repoInit()
{
    // make job that creates the local repository
    CvsJob* j = new CvsJob(0);
    QVERIFY( j );
    j->setDirectory(CVSTEST_BASEDIR);
    *j << "cvs" << "-d" << CVS_REPO << "init";

    // try to start the job
    QVERIFY( j->exec() );

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY( QFileInfo(QString(CVS_REPO "/CVSROOT")).exists() );
}

void TestCvs::importTestData()
{
    // create a file so we don't import an empty dir
    QFile f(CVS_IMPORT "" CVS_TESTFILE_NAME);
    if(f.open(QIODevice::WriteOnly)) {
        QTextStream input( &f );
        input << "HELLO WORLD";
    }
    f.flush();


    CvsJob* j = m_proxy->import(QUrl::fromLocalFile(CVS_IMPORT), CVS_REPO,
                        "test", "vendor", "release",
                        "test import message");
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the directory has been added to the repository
    QString testdir(CVS_REPO "/test");
    QVERIFY( QFileInfo(testdir).exists() );

    //check if the file has been added to the repository
    QString testfile(CVS_REPO "/test/" CVS_TESTFILE_NAME ",v");
    QVERIFY( QFileInfo(testfile).exists() );
}


void TestCvs::checkoutTestData()
{
    CvsJob* j = m_proxy->checkout(QUrl::fromLocalFile(CVS_CHECKOUT), CVS_REPO, "test");
    QVERIFY( j );

    // try to start the job
    QVERIFY( j->exec() );

    //check if the directory is there
    QString testdir(CVS_CHECKOUT);
    QVERIFY( QFileInfo(testdir).exists() );

    //check if the file is there
    QString testfile(CVS_CHECKOUT "" CVS_TESTFILE_NAME);
    QVERIFY( QFileInfo(testfile).exists() );
}


void TestCvs::testInitAndImport()
{
    repoInit();
    importTestData();
    checkoutTestData();
}

void TestCvs::testLogFolder()
{
    repoInit();
    importTestData();
    checkoutTestData();
    QString testdir(CVS_CHECKOUT);
    KDevelop::VcsRevision rev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Head);
    CvsJob* job = m_proxy->log(QUrl::fromLocalFile(testdir), rev);
    QVERIFY(job);
}

QTEST_MAIN(TestCvs)
