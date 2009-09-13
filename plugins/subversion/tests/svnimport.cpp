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
#include <qtest_kde.h>
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
        qDebug() << "ooops, no exec";
        kDebug() << j->errorString();
        // On error, wait for key in order to allow manual state inspection
    }

    QCOMPARE(j->status(), status);
}

void SvnImport::initTestCase()
{
    AutoTestShell::init();
    core = new TestCore();
    core->initialize(Core::Default);
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
    core->cleanup();
    core->deleteLater();
}

void SvnImport::testBasic()
{
    KTempDir reposDir;
    KProcess cmd;
    cmd.setWorkingDirectory(reposDir.name());
    cmd << "svnadmin" << "create" << reposDir.name();
    QCOMPARE(cmd.execute(10000), 0);
    
    VcsLocation reposLoc;
    reposLoc.setRepositoryServer("file://" + reposDir.name() );
    
    KTempDir projectDir;
    QFile sampleFile( projectDir.name() + "/sample.file" );
    sampleFile.open( QIODevice::WriteOnly );
    QString origcontent = "This is a Test";
    sampleFile.write( origcontent.toUtf8() );
    sampleFile.close();

    VcsJob* job = vcs->import( "import test", KUrl( projectDir.name() ), reposLoc );
    validatingExecJob(job);

    KTempDir checkoutDir;
    checkoutDir.setAutoRemove(false);
    reposLoc.setRepositoryServer( reposLoc.repositoryServer() + "/" + QFileInfo( projectDir.name() ).fileName() );
    job = vcs->createWorkingCopy( reposLoc, checkoutDir.name() );
    validatingExecJob(job);

    QFile newfile(checkoutDir.name() + "/sample.file" );
    QVERIFY(newfile.exists());
    QVERIFY(newfile.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8( newfile.readAll() ), origcontent);
    core->cleanup();
}

QTEST_KDEMAIN(SvnImport, GUI)
