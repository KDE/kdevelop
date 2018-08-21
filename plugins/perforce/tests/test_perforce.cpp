/***************************************************************************
 *   This file was inspired by KDevelop's git plugin                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   Adapted for Perforce                                                  *
 *   Copyright 2011  Morten Danielsen Volden <mvolden2@gmail.com>          *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "test_perforce.h"

#include <QTest>
#include <QDirIterator>
#include <QStandardPaths>

#include <tests/autotestshell.h>
#include <tests/testcore.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsannotation.h>

#include <perforceplugin.h>

#define VERIFYJOB(j) \
    QVERIFY(j); QVERIFY(j->exec()); QVERIFY((j)->status() == VcsJob::JobSucceeded)

const QString tempDir = QDir::tempPath();
const QString perforceTestBaseDirNoSlash(tempDir + "/kdevPerforce_testdir");
const QString perforceTestBaseDir(tempDir + "/kdevPerforce_testdir/");
const QString perforceTestBaseDir2(tempDir + "/kdevPerforce_testdir2/");
const QString perforceConfigFileName(QStringLiteral("p4config.txt"));

const QString perforceSrcDir(perforceTestBaseDir + "src/");
const QString perforceTest_FileName(QStringLiteral("testfile"));
const QString perforceTest_FileName2(QStringLiteral("foo"));
const QString perforceTest_FileName3(QStringLiteral("bar"));

using namespace KDevelop;

void PerforcePluginTest::initTestCase()
{
    AutoTestShell::init({QStringLiteral("kdevperforce")});
    TestCore::initialize();
    m_plugin = new PerforcePlugin(TestCore::self());

    /// During test we are setting the executable the plugin uses to our own stub
    QDirIterator it(P4_BINARY_DIR , QStringList() << QStringLiteral("*"), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QStringList pathsToSearch;
    while (it.hasNext()) {
        it.next();
        pathsToSearch << it.filePath();
    }
    QString p4stubPath = QStandardPaths::findExecutable("p4clientstub", pathsToSearch);
    qDebug() << "found p4stub executable :" << p4stubPath;
    QVERIFY(!p4stubPath.isEmpty());

    m_plugin->m_perforceExecutable = p4stubPath;
}

void PerforcePluginTest::cleanupTestCase()
{
    delete m_plugin;
    TestCore::shutdown();
}


void PerforcePluginTest::init()
{
    removeTempDirsIfAny();
    createNewTempDirs();
}

void PerforcePluginTest::createNewTempDirs()
{
    // Now create the basic directory structure
    QDir tmpdir(tempDir);
    tmpdir.mkdir(perforceTestBaseDir);
    //we start it after repoInit, so we still have empty repo
    QFile f(perforceTestBaseDir + perforceConfigFileName);
    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "P4PORT=127.0.0.1:1666\n";
        input << "P4USER=mvo\n";
        input << "P4CLIENT=testbed\n";
    }
    f.close();

    //Put a file here because the annotate and update function will check for that
    QFile g(perforceTestBaseDir + perforceTest_FileName);
    if (g.open(QIODevice::WriteOnly)) {
        QTextStream input(&g);
        input << "HELLO WORLD";
    }
    g.close();


    tmpdir.mkdir(perforceSrcDir);
    tmpdir.mkdir(perforceTestBaseDir2);
}


void PerforcePluginTest::removeTempDirsIfAny()
{
    QDir dir(perforceTestBaseDir);
    if (dir.exists() && !dir.removeRecursively())
        qDebug() << "QDir::removeRecursively(" << perforceTestBaseDir << ") returned false";

    QDir dir2(perforceTestBaseDir);
    if (dir2.exists() && !dir2.removeRecursively())
        qDebug() << "QDir::removeRecursively(" << perforceTestBaseDir2 << ") returned false";
}


void PerforcePluginTest::cleanup()
{
    removeTempDirsIfAny();
}

void PerforcePluginTest::testAdd()
{
    VcsJob* j = m_plugin->add(QList<QUrl>({ QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName) } ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testEdit()
{
    VcsJob* j = m_plugin->edit(QList<QUrl>( { QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName) } ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testEditMultipleFiles()
{
    QList<QUrl> filesForEdit;
    filesForEdit.push_back(QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName));
    filesForEdit.push_back(QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName2));
    filesForEdit.push_back(QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName3));
    VcsJob* j = m_plugin->edit(filesForEdit);
    VERIFYJOB(j);
}


void PerforcePluginTest::testStatus()
{
    VcsJob* j = m_plugin->status(QList<QUrl>( { QUrl::fromLocalFile(perforceTestBaseDirNoSlash) } ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testAnnotate()
{
    VcsJob* j = m_plugin->annotate(QUrl( QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName) ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testHistory()
{
    VcsJob* j = m_plugin->log(QUrl( QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName) ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testRevert()
{
    VcsJob* j = m_plugin->revert(QList<QUrl>( { QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName) } ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testUpdateFile()
{
    VcsJob* j = m_plugin->update(QList<QUrl>( { QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName) } ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testUpdateDir()
{
    VcsJob* j = m_plugin->update(QList<QUrl>( { QUrl::fromLocalFile(perforceTestBaseDirNoSlash) } ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testCommit()
{
    QString commitMsg(QStringLiteral("this is the commit message"));
    VcsJob* j = m_plugin->commit(commitMsg, QList<QUrl>( { QUrl::fromLocalFile(perforceTestBaseDirNoSlash) }  ));
    VERIFYJOB(j);
}

void PerforcePluginTest::testDiff()
{
    VcsRevision srcRevision;
    srcRevision.setRevisionValue(QVariant(1), VcsRevision::GlobalNumber);
    VcsRevision dstRevision;
    dstRevision.setRevisionValue(QVariant(2), VcsRevision::GlobalNumber);

    VcsJob* j = m_plugin->diff( QUrl::fromLocalFile(perforceTestBaseDir + perforceTest_FileName), srcRevision, dstRevision);
    VERIFYJOB(j);
}


QTEST_MAIN(PerforcePluginTest)

