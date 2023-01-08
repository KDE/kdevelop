/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

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
        : percentChanged(job, &KJob::percentChanged)
        , finished(job, &KJob::finished)
        , result(job, &KJob::result)
    {
    }

    QSignalSpy percentChanged;
    QSignalSpy finished;
    QSignalSpy result;
};

void KSequentialCompoundJobTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KSequentialCompoundJobTest::runZeroJobs()
{
    QPointer master(new KSimpleSequentialCompoundJob);
    JobSpy masterSpy(master);

    QCOMPARE(masterSpy.finished.count(), 0);
    QCOMPARE(masterSpy.result.count(), 0);
    master->start();
    QCOMPARE(masterSpy.finished.count(), 1);
    QCOMPARE(masterSpy.result.count(), 1);

    QTest::qWait(1);
    QVERIFY(!master);
}

void KSequentialCompoundJobTest::runOneJob()
{
    QPointer slave(new TestJob);

    QPointer master(new KSimpleSequentialCompoundJob);
    QVERIFY(master->addSubjob(slave));
    JobSpy masterSpy(master);

    QSignalSpy startedSpy(slave, &TestJob::started);
    JobSpy slaveSpy(slave);

    master->start();
    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(slaveSpy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    QCOMPARE(masterSpy.percentChanged.count(), 0);
    QCOMPARE(master->percent(), 0);

    slave->emitResult();

    QCOMPARE(masterSpy.percentChanged.count(), 1);
    QCOMPARE(master->percent(), 100);

    QCOMPARE(masterSpy.finished.count(), 1);
    QCOMPARE(masterSpy.result.count(), 1);

    QCOMPARE(startedSpy.count(), 1);
    QCOMPARE(slaveSpy.finished.count(), 1);
    QCOMPARE(slaveSpy.result.count(), 1);

    QTest::qWait(1);

    QVERIFY(!slave);
    QVERIFY(!master);

    QCOMPARE(slaveSpy.percentChanged.count(), 0);
    QCOMPARE(masterSpy.percentChanged.count(), 1);
}

void KSequentialCompoundJobTest::runTwoJobs()
{
    QPointer slave1(new TestJob);
    QPointer slave2(new TestJob);

    QPointer master(new KSimpleSequentialCompoundJob);
    QVERIFY(master->addSubjob(slave1));
    QVERIFY(master->addSubjob(slave2));
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

    QCOMPARE(masterSpy.percentChanged.count(), 0);
    QCOMPARE(master->percent(), 0);

    slave1->emitResult();

    QCOMPARE(masterSpy.percentChanged.count(), 1);
    QCOMPARE(master->percent(), 50);

    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(slave1Spy.finished.count(), 1);
    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(slave2Spy.finished.count(), 0);
    QCOMPARE(masterSpy.finished.count(), 0);

    slave2->emitResult();

    QCOMPARE(masterSpy.percentChanged.count(), 2);
    QCOMPARE(master->percent(), 100);

    QCOMPARE(masterSpy.finished.count(), 1);
    QCOMPARE(masterSpy.result.count(), 1);

    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(slave1Spy.finished.count(), 1);
    QCOMPARE(slave1Spy.result.count(), 1);

    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(slave2Spy.finished.count(), 1);
    QCOMPARE(slave2Spy.result.count(), 1);

    QTest::qWait(1);

    QVERIFY(!slave1);
    QVERIFY(!slave2);
    QVERIFY(!master);

    QCOMPARE(slave1Spy.percentChanged.count(), 0);
    QCOMPARE(slave2Spy.percentChanged.count(), 0);
    QCOMPARE(masterSpy.percentChanged.count(), 2);
}

QTEST_GUILESS_MAIN(KSequentialCompoundJobTest)
