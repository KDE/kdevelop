/*
    This file was inspired by KDevelop's git plugin
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    Adapted for Perforce
    SPDX-FileCopyrightText: 2011 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_perforce.h"

#include <QTest>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>

#include <KPluginMetaData>

#include <tests/autotestshell.h>
#include <tests/plugintesthelpers.h>
#include <tests/testcore.h>

#include <vcs/vcsjob.h>
#include <vcs/vcsannotation.h>

#include <perforceplugin.h>

#define VERIFYJOB(j) \
    QVERIFY(j); QVERIFY(j->exec()); QVERIFY((j)->status() == VcsJob::JobSucceeded)

using namespace KDevelop;

PerforcePluginTest::PerforcePluginTest()
    : tempDir{QDir::tempPath()}
    , perforceTestBaseDirNoSlash{tempDir + QLatin1String("/kdevPerforce_testdir")}
    , perforceTestBaseDir{perforceTestBaseDirNoSlash + QLatin1Char('/')}
    , perforceConfigFileName{QStringLiteral("p4config.txt")}
    , perforceSrcDir{perforceTestBaseDir + QLatin1String("src/")}
    , perforceTest_FileName{QStringLiteral("testfile")}
    , perforceTest_FileName2{QStringLiteral("foo")}
    , perforceTest_FileName3{QStringLiteral("bar")}
{
}

void PerforcePluginTest::initTestCase()
{
    AutoTestShell::init({QStringLiteral("kdevperforce")});
    TestCore::initialize();

    const auto pluginMetaData = makeTestPluginMetaData("TestPerforce");
    m_plugin = new PerforcePlugin(TestCore::self(), pluginMetaData);

    /// During test we are setting the executable the plugin uses to our own stub
    QDirIterator it(QString::fromUtf8(P4_BINARY_DIR), QStringList() << QStringLiteral("*"),
                    QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QStringList pathsToSearch;
    while (it.hasNext()) {
        it.next();
        pathsToSearch << it.filePath();
    }
    QString p4stubPath = QStandardPaths::findExecutable(QStringLiteral("p4clientstub"), pathsToSearch);
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
}


void PerforcePluginTest::removeTempDirsIfAny()
{
    QDir dir(perforceTestBaseDir);
    if (dir.exists() && !dir.removeRecursively())
        qDebug() << "QDir::removeRecursively(" << perforceTestBaseDir << ") returned false";
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

#include "moc_test_perforce.cpp"
