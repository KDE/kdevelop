/***************************************************************************
*   This file is part of KDevelop                                         *
*   Copyright 2009 Fabian Wiesel <fabian.wiesel@googlemail.com>           *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License as       *
*   published by the Free Software Foundation; either version 2 of the    *
*   License, or (at your option) any later version.                       *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
***************************************************************************/

#include "svnimport.h"
#include <QtTest/QtTest>
#include <KTempDir>
#include <KProcess>
#include <kdebug.h>
#include <kparts/part.h>
#include <kio/netaccess.h>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcsjob.h>

#define VERBOSE
#if defined(VERBOSE)
#define TRACE(X) kDebug() << X
#else
#define TRACE(X) { line = line; }
#endif

using namespace KDevelop;

void validatingExecJob(VcsJob* j, VcsJob::JobStatus status = VcsJob::JobSucceeded)
{
    QVERIFY(j);

    if (!j->exec()) {
        kDebug() << j->errorString();
        // On error, wait for key in order to allow manual state inspection
    }
    QCOMPARE(j->status(), status);
}

void setupLocalRepository( const QString& name, VcsLocation & reposLoc )
{
    KProcess cmd;
    cmd.setWorkingDirectory(name);
    cmd << "svnadmin" << "create" << name;
    QCOMPARE(cmd.execute(10000), 0);

    reposLoc.setRepositoryServer("file://" + name );
}

void setupSampleProject( const QString& name, const QString& content )
{
    QFile sampleFile( name + "/sample.file" );
    sampleFile.open( QIODevice::WriteOnly );
    sampleFile.write( content.toUtf8() );
    sampleFile.close();
}

void SvnImport::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize();

    QList<IPlugin*> plugins = Core::self()->pluginController()->allPluginsForExtension("org.kdevelop.IBasicVersionControl");
    foreach(IPlugin* p,  plugins) {
        qDebug() << "checking plugin" << p;
        ICentralizedVersionControl* icentr = p->extension<ICentralizedVersionControl>();
        if (!icentr)
            continue;
        if (icentr->name() == "Subversion") {
            vcs = icentr;
            break;
        }
    }
    qDebug() << "ok, got vcs" << vcs;
    QVERIFY(vcs);
}

void SvnImport::cleanupTestCase()
{
    TestCore::shutdown();
}

void SvnImport::testBasic()
{
    KTempDir reposDir;
    VcsLocation reposLoc;
    setupLocalRepository( reposDir.name(), reposLoc );
    
    KTempDir projectDir;
    QString origcontent = "This is a Test";
    setupSampleProject( projectDir.name(), origcontent );

    VcsJob* job = vcs->import( "import test", QUrl::fromLocalFile( projectDir.name() ), reposLoc );
    validatingExecJob(job);

    KTempDir checkoutDir;
    validateImport( reposLoc.repositoryServer(), checkoutDir, origcontent );
}

void SvnImport::testImportWithMissingDirs()
{
    KTempDir reposDir;
    VcsLocation reposLoc;
    setupLocalRepository( reposDir.name(), reposLoc );

    KTempDir projectDir;
    QString origcontent = "This is a Test";
    setupSampleProject( projectDir.name(), origcontent );

    reposLoc.setRepositoryServer( reposLoc.repositoryServer() + "/foobar/" + QDir( projectDir.name() ).dirName() );
    VcsJob* job = vcs->import( "import test", QUrl::fromLocalFile( projectDir.name() ), reposLoc );
    validatingExecJob(job);

    KTempDir checkoutDir;
    validateImport( reposLoc.repositoryServer(), checkoutDir, origcontent );
}

void SvnImport::testImportIntoDir()
{
    KTempDir reposDir;
    VcsLocation reposLoc;
    setupLocalRepository( reposDir.name(), reposLoc );

    KTempDir projectDir;
    QString origcontent = "This is a Test";
    setupSampleProject( projectDir.name(), origcontent );

    reposLoc.setRepositoryServer( reposLoc.repositoryServer() + '/' + QDir( projectDir.name() ).dirName() );
    VcsJob* job = vcs->import( "import test", QUrl::fromLocalFile( projectDir.name() ), reposLoc );
    validatingExecJob(job);

    KTempDir checkoutDir;
    validateImport( reposLoc.repositoryServer(), checkoutDir, origcontent );
}

void SvnImport::validateImport( const QString& repourl, KTempDir& checkoutdir, const QString& origcontent )
{
    VcsLocation reposLoc;
    reposLoc.setRepositoryServer( repourl );
    VcsJob* job = vcs->createWorkingCopy( reposLoc, checkoutdir.name() );
    validatingExecJob(job);

    QFile newfile( checkoutdir.name() + "/sample.file" );
    QVERIFY(newfile.exists());
    QVERIFY(newfile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8( newfile.readAll() ), origcontent);
}

QTEST_MAIN(SvnImport)
