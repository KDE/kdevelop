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

#include "test_svnrecursiveadd.h"

#include <QDebug>
#include <QTemporaryDir>
#include <QTest>

#include <KProcess>

#include <interfaces/iplugincontroller.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcsjob.h>
#include <vcs/vcslocation.h>

#define PATHETIC    // A little motivator to make things work right :)
#if defined(PATHETIC)
inline QString vcsTestDir0() { return QStringLiteral("testdir0"); }
inline QString vcsTestDir1() { return QStringLiteral("testdir1"); }
inline QString vcsTest_FileName0() { return QStringLiteral("foo"); }
inline QString vcsTest_FileName1() { return QStringLiteral("bar"); }
inline QString keywordText() { return QStringLiteral("text"); }
#else
inline QString vcsTestDir0() { return QStringLiteral("dvcs\t testdir"); }   // Directory containing whitespaces
inline QString vcsTestDir1() { return QStringLiteral("--help"); }           // Starting with hyphen for command-line tools
inline QString vcsTest_FileName0() { return QStringLiteral("foo\t bar"); }
inline QString vcsTest_FileName1() { return QStringLiteral("--help"); }
inline QString keywordText() { return QStringLiteral("Author:\nDate:\nCommit:\n------------------------------------------------------------------------\nr999999 | ehrman | 1989-11-09 18:53:00 +0100 (Thu, 09 Nov 1989) | 1 lines\nthe line\n"); }  // Text containing keywords of the various vcs-programs
#endif

inline QString simpleText() { return QStringLiteral("It's foo!\n"); }
inline QString simpleAltText() { return QStringLiteral("No, foo()! It's bar()!\n"); }

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
    // Print the commands in full, for easier bug location
#if 0
    if (QLatin1String(j->metaObject()->className()) == "DVcsJob") {
        qDebug() << "Command: \"" << ((DVcsJob*)j)->getChildproc()->program() << ((DVcsJob*)j)->getChildproc()->workingDirectory();
        qDebug() << "Output: \"" << ((DVcsJob*)j)->output();
    }
#endif

    if (!j->exec()) {
        qDebug() << "ooops, no exec";
        qDebug() << j->errorString();
        // On error, wait for key in order to allow manual state inspection
#if 0
        char c;
        std::cin.read(&c, 1);
#endif
    }

    QCOMPARE(j->status(), status);
}

void verifiedWrite(QUrl const & url, QString const & contents)
{
    QFile f(url.path());
    QVERIFY(f.open(QIODevice::WriteOnly));
    QTextStream filecontents(&f);
    filecontents << contents;
    filecontents.flush();
    f.flush();
}


void fillWorkingDirectory(QString const & dirname)
{
    QDir dir(dirname);
    //we start it after repoInit, so we still have empty dvcs repo
    QVERIFY(dir.mkdir(vcsTestDir0()));
    QVERIFY(dir.cd(vcsTestDir0()));
    QUrl file0 = QUrl::fromLocalFile(dir.absoluteFilePath(vcsTest_FileName0()));
    QVERIFY(dir.mkdir(vcsTestDir1()));
    QVERIFY(dir.cd(vcsTestDir1()));
    QUrl file1 = QUrl::fromLocalFile(dir.absoluteFilePath(vcsTest_FileName1()));
    verifiedWrite(file0, simpleText());
    verifiedWrite(file1, keywordText());
}

void TestSvnRecursiveAdd::initTestCase()
{
    AutoTestShell::init({"kdevsubversion", "KDevStandardOutputView"});
    TestCore::initialize();
}

void TestSvnRecursiveAdd::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestSvnRecursiveAdd::test()
{
    QTemporaryDir reposDir;
    KProcess cmd;
    cmd.setWorkingDirectory(reposDir.path());
    cmd << QStringLiteral("svnadmin") << QStringLiteral("create") << reposDir.path();
    QCOMPARE(cmd.execute(10000), 0);
    const QList<IPlugin*> plugins = Core::self()->pluginController()->allPluginsForExtension(QStringLiteral("org.kdevelop.IBasicVersionControl"));
    IBasicVersionControl* vcs = nullptr;
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
    VcsLocation reposLoc;
    reposLoc.setRepositoryServer("file://" + reposDir.path());
    QTemporaryDir checkoutDir;
    QUrl checkoutLoc = QUrl::fromLocalFile(checkoutDir.path());
    qDebug() << "Checking out from " << reposLoc.repositoryServer() << " to " << checkoutLoc;
    qDebug() << "creating job";
    VcsJob* job = vcs->createWorkingCopy( reposLoc, checkoutLoc );
    validatingExecJob(job);
    qDebug() << "filling wc";
    fillWorkingDirectory(checkoutDir.path());
    QUrl addUrl = QUrl::fromLocalFile( checkoutDir.path() + '/' + vcsTestDir0() );
    qDebug() << "Recursively adding files at " << addUrl;
    validatingExecJob(vcs->add({addUrl}, IBasicVersionControl::Recursive));
    qDebug() << "Recursively reverting changes at " << addUrl;
    validatingExecJob(vcs->revert({addUrl}, IBasicVersionControl::Recursive));
}

QTEST_MAIN(TestSvnRecursiveAdd)
