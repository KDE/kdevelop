/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_executecompositejob.h"

#include <QTest>
#include <QSignalSpy>

#include <util/executecompositejob.h>

QTEST_MAIN(TestExecuteCompositeJob);

using namespace KDevelop;

struct JobSpy
{
    JobSpy(KJob* job)
        : finished(job, SIGNAL(finished(KJob*)))
        , result(job, SIGNAL(result(KJob*)))
    {}
    QSignalSpy finished;
    QSignalSpy result;
};

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
