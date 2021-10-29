/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_executecompositejob.h"

#include <QTest>
#include <QSignalSpy>
#include <QStandardPaths>

#include <util/executecompositejob.h>

QTEST_MAIN(TestExecuteCompositeJob)

using namespace KDevelop;

struct JobSpy
{
    explicit JobSpy(KJob* job)
        : finished(job, &KJob::finished)
        , result(job, &KJob::result)
    {}
    QSignalSpy finished;
    QSignalSpy result;
};

void TestExecuteCompositeJob::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestExecuteCompositeJob::runOneJob()
{
    QPointer<TestJob> slave(new TestJob);

    QPointer<ExecuteCompositeJob> master(new ExecuteCompositeJob(nullptr, {slave.data()}));
    JobSpy masterSpy(master.data());

    QSignalSpy startedSpy(slave.data(), SIGNAL(started(KJob*)));
    JobSpy slaveSpy(slave.data());

    master->start();
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(slaveSpy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    slave->callEmitResult();

    QCOMPARE(masterSpy.finished.count(), 1);
    QCOMPARE(masterSpy.result.count(), 1);

    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(slaveSpy.finished.count(), 1);
    QCOMPARE(slaveSpy.result.count(), 1);

    QTest::qWait(10);

    QVERIFY(!slave);
    QVERIFY(!master);
}

void TestExecuteCompositeJob::runTwoJobs()
{
    QPointer<TestJob> slave1(new TestJob);
    QPointer<TestJob> slave2(new TestJob);

    QPointer<ExecuteCompositeJob> master(new ExecuteCompositeJob(nullptr, {slave1.data(), slave2.data()}));
    JobSpy masterSpy(master.data());

    QSignalSpy started1Spy(slave1.data(), SIGNAL(started(KJob*)));
    QSignalSpy started2Spy(slave2.data(), SIGNAL(started(KJob*)));
    JobSpy slave1Spy(slave1.data());
    JobSpy slave2Spy(slave2.data());

    master->start();
    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(slave1Spy.finished.count(), 0);
    QCOMPARE(started2Spy.count(), 0);
    QCOMPARE(slave2Spy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    slave1->callEmitResult();
    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(slave1Spy.finished.count(), 1);
    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(slave2Spy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    slave2->callEmitResult();

    QCOMPARE(masterSpy.finished.count(), 1);
    QCOMPARE(masterSpy.result.count(), 1);

    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(slave1Spy.finished.count(), 1);
    QCOMPARE(slave1Spy.result.count(), 1);

    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(slave2Spy.finished.count(), 1);
    QCOMPARE(slave2Spy.result.count(), 1);

    QTest::qWait(10);

    QVERIFY(!slave1);
    QVERIFY(!slave2);
    QVERIFY(!master);
}
