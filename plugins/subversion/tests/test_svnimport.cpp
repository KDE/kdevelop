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

#include "test_svnimport.h"

#include <QTest>
#include <QDebug>
#include <QLoggingCategory>

#include <QTemporaryDir>
#include <KProcess>
#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcsjob.h>
#include <vcs/vcslocation.h>

#define VERBOSE
#if defined(VERBOSE)
#define TRACE(X) qDebug() << X
#else
#define TRACE(X) { line = line; }
#endif

using namespace KDevelop;

void validatingExecJob(VcsJob* j, VcsJob::JobStatus status = VcsJob::JobSucceeded)
{
    QVERIFY(j);

    if (!j->exec()) {
        qDebug() << j->errorString();
        // On error, wait for key in order to allow manual state inspection
    }
    QCOMPARE(j->status(), status);
}

void setupLocalRepository( const QString& name, VcsLocation & reposLoc )
{
    KProcess cmd;
    cmd.setWorkingDirectory(name);
    cmd << QStringLiteral("svnadmin") << QStringLiteral("create") << name;
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

void TestSvnImport::initTestCase()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\ndefault.debug=true\nkdevelop.plugins.svn.debug=true\n"));
    AutoTestShell::init({QStringLiteral("kdevsubversion"), QStringLiteral("KDevStandardOutputView")});
    TestCore::initialize();

    const QList<IPlugin*> plugins = Core::self()->pluginController()->allPluginsForExtension(QStringLiteral("org.kdevelop.IBasicVersionControl"));
    for (IPlugin* p : plugins) {
        qDebug() << "checking plugin" << p;
        ICentralizedVersionControl* icentr = p->extension<ICentralizedVersionControl>();
        if (!icentr)
            continue;
        if (icentr->name() == QLatin1String("Subversion")) {
            vcs = icentr;
            break;
        }
    }
    qDebug() << "ok, got vcs" << vcs;
    QVERIFY(vcs);
}

void TestSvnImport::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestSvnImport::testBasic()
{
    QTemporaryDir reposDir;
    VcsLocation reposLoc;
    setupLocalRepository( reposDir.path(), reposLoc );

    QTemporaryDir projectDir;
    QString origcontent = QStringLiteral("This is a Test");
    setupSampleProject( projectDir.path(), origcontent );

    VcsJob* job = vcs->import( QStringLiteral("import test"), QUrl::fromLocalFile( projectDir.path() ), reposLoc );
    validatingExecJob(job);

    QTemporaryDir checkoutDir;
    validateImport( reposLoc.repositoryServer(), checkoutDir, origcontent );
}

void TestSvnImport::testImportWithMissingDirs()
{
    QTemporaryDir reposDir;
    VcsLocation reposLoc;
    setupLocalRepository( reposDir.path(), reposLoc );

    QTemporaryDir projectDir;
    QString origcontent = QStringLiteral("This is a Test");
    setupSampleProject( projectDir.path(), origcontent );

    reposLoc.setRepositoryServer( reposLoc.repositoryServer() + "/foobar/" + QDir( projectDir.path() ).dirName() );
    VcsJob* job = vcs->import( QStringLiteral("import test"), QUrl::fromLocalFile( projectDir.path() ), reposLoc );
    validatingExecJob(job);

    QTemporaryDir checkoutDir;
    validateImport( reposLoc.repositoryServer(), checkoutDir, origcontent );
}

void TestSvnImport::testImportIntoDir()
{
    QTemporaryDir reposDir;
    VcsLocation reposLoc;
    setupLocalRepository( reposDir.path(), reposLoc );

    QTemporaryDir projectDir;
    QString origcontent = QStringLiteral("This is a Test");
    setupSampleProject( projectDir.path(), origcontent );

    reposLoc.setRepositoryServer( reposLoc.repositoryServer() + '/' + QDir( projectDir.path() ).dirName() );
    VcsJob* job = vcs->import( QStringLiteral("import test"), QUrl::fromLocalFile( projectDir.path() ), reposLoc );
    validatingExecJob(job);

    QTemporaryDir checkoutDir;
    validateImport( reposLoc.repositoryServer(), checkoutDir, origcontent );
}

void TestSvnImport::validateImport( const QString& repourl, QTemporaryDir& checkoutdir, const QString& origcontent )
{
    VcsLocation reposLoc;
    reposLoc.setRepositoryServer( repourl );
    VcsJob* job = vcs->createWorkingCopy( reposLoc, QUrl::fromLocalFile(checkoutdir.path()) );
    validatingExecJob(job);

    QFile newfile( checkoutdir.path() + "/sample.file" );
    QVERIFY(newfile.exists());
    QVERIFY(newfile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8( newfile.readAll() ), origcontent);
}

QTEST_MAIN(TestSvnImport)
