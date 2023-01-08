/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ksequentialcompoundjobtest.h"

#include "ksimplesequentialcompoundjob.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

using namespace KDevCoreAddons;

struct JobSpy {
    explicit JobSpy(const KJob *job)
        : finished(job, &KJob::finished)
        , result(job, &KJob::result)
    {
    }
    QSignalSpy finished;
    QSignalSpy result;
};

void KSequentialCompoundJobTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KSequentialCompoundJobTest::runOneJob()
{
    QPointer slave(new TestJob);

    QPointer master(new KSimpleSequentialCompoundJob);
    master->addSubjob(slave);
    JobSpy masterSpy(master);

    QSignalSpy startedSpy(slave, &TestJob::started);
    JobSpy slaveSpy(slave);

    master->start();
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(slaveSpy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    slave->emitResult();

    QCOMPARE(masterSpy.finished.count(), 1);
    QCOMPARE(masterSpy.result.count(), 1);

    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(slaveSpy.finished.count(), 1);
    QCOMPARE(slaveSpy.result.count(), 1);

    QTest::qWait(10);

    QVERIFY(!slave);
    QVERIFY(!master);
}

void KSequentialCompoundJobTest::runTwoJobs()
{
    QPointer slave1(new TestJob);
    QPointer slave2(new TestJob);

    QPointer master(new KSimpleSequentialCompoundJob);
    master->addSubjob(slave1);
    master->addSubjob(slave2);
    JobSpy masterSpy(master);

    QSignalSpy started1Spy(slave1, &TestJob::started);
    QSignalSpy started2Spy(slave2, &TestJob::started);
    JobSpy slave1Spy(slave1);
    JobSpy slave2Spy(slave2);

    master->start();
    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(slave1Spy.finished.count(), 0);
    QCOMPARE(started2Spy.count(), 0);
    QCOMPARE(slave2Spy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    slave1->emitResult();
    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(slave1Spy.finished.count(), 1);
    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(slave2Spy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    slave2->emitResult();

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

QTEST_GUILESS_MAIN(KSequentialCompoundJobTest)
