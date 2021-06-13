/*
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_dvcsjob.h"

#include <QTest>

#include <vcs/dvcs/dvcsjob.h>
#include <tests/testcore.h>
#include <tests/autotestshell.h>

using namespace KDevelop;

void TestDVcsJob::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);
}

void TestDVcsJob::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestDVcsJob::testJob()
{
    auto* job = new KDevelop::DVcsJob(QDir::temp());
    QVERIFY(job);
    QVERIFY(job->status() == KDevelop::VcsJob::JobNotStarted);

    //try the command like "echo -n test"
    //should fail, because command and arg are in one string. We can change operator<<(QString) to split,
    //but it will be a wrong style to work with jobs.
    const QString echoCommand(QStringLiteral("echo -n test"));
    *job << echoCommand;
    QVERIFY(!job->exec());
    QVERIFY(job->status() == KDevelop::VcsJob::JobFailed);
    QCOMPARE(job->dvcsCommand().join(QStringLiteral(";;")), echoCommand);
}


QTEST_MAIN(TestDVcsJob)
