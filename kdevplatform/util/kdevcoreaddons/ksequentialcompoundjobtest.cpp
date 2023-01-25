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

#include <memory>
#include <vector>

using namespace KDevCoreAddons;

class PublicSequentialCompoundJob : public KSequentialCompoundJob
{
public:
    using KSequentialCompoundJob::addSubjob;
    using KSequentialCompoundJob::clearSubjobs;
    using KSequentialCompoundJob::removeSubjob;
    using KSequentialCompoundJob::subjobs;
};

struct JobSpy {
    explicit JobSpy(const KJob *job)
        : infoMessage(job, &KJob::infoMessage)
        , percentChanged(job, &KJob::percentChanged)
        , finished(job, &KJob::finished)
        , result(job, &KJob::result)
    {
    }

    QSignalSpy infoMessage;
    QSignalSpy percentChanged;
    QSignalSpy finished;
    QSignalSpy result;
};

struct KillableTestJobSpy : public JobSpy {
    explicit KillableTestJobSpy(KillableTestJob *job)
        : JobSpy(job)
        , started(job, &KillableTestJob::started)
        , killed(job, &KillableTestJob::killed)
        , job(job)
    {
    }

    QSignalSpy started;
    QSignalSpy killed;
    QPointer<KillableTestJob> job;
};

std::vector<std::unique_ptr<KillableTestJobSpy>> generateKillableTestJobSpies(int subjobCount)
{
    std::vector<std::unique_ptr<KillableTestJobSpy>> jobSpies;
    for (int j = 0; j < subjobCount; ++j) {
        jobSpies.push_back(std::make_unique<KillableTestJobSpy>(new KillableTestJob));
    }
    return jobSpies;
}

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

void KSequentialCompoundJobTest::addRemoveClearSubjob_data()
{
    QTest::addColumn<bool>("shouldClearSubjobs");
    QTest::newRow("removeSubjob") << false;
    QTest::newRow("clearSubjobs") << true;
}

void KSequentialCompoundJobTest::addRemoveClearSubjob()
{
    QPointer compoundJob(new PublicSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);
    QVERIFY(!compoundJob->addSubjob(nullptr));
    QVERIFY(!compoundJob->removeSubjob(nullptr));

    QPointer subjob(new TestJob);
    JobSpy subjobSpy(subjob);
    QCOMPARE(subjob->parent(), nullptr);
    QVERIFY(!compoundJob->removeSubjob(subjob));

    subjob->emitInfoMessage("hi");
    QCOMPARE(subjobSpy.infoMessage.count(), 1);
    subjob->setPercent(10);
    QCOMPARE(subjobSpy.percentChanged.count(), 1);

    QCOMPARE(compoundJob->subjobs(), {});
    QVERIFY(compoundJob->addSubjob(subjob));
    QCOMPARE(compoundJob->subjobs(), {subjob});

    QCOMPARE(subjob->parent(), compoundJob);
    QVERIFY(!compoundJob->addSubjob(subjob));
    QCOMPARE(subjob->parent(), compoundJob);

    QCOMPARE(compoundSpy.infoMessage.count(), 0);
    QCOMPARE(compoundSpy.percentChanged.count(), 0);

    subjob->emitInfoMessage("there");
    QCOMPARE(subjobSpy.infoMessage.count(), 2);
    QCOMPARE(compoundSpy.infoMessage.count(), 1);
    QCOMPARE(compoundSpy.infoMessage.constLast().at(0).value<KJob *>(), subjob);
    QCOMPARE(compoundSpy.infoMessage.constLast().at(1), "there");

    subjob->setPercent(25);
    QCOMPARE(subjobSpy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 0); // the subjob is not started, its percent changes are ignored

    compoundJob->start();
    subjob->setPercent(45);
    QCOMPARE(subjobSpy.percentChanged.count(), 3);
    QCOMPARE(compoundSpy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 45);
    QCOMPARE(compoundJob->percent(), 45);

    QVERIFY(!compoundJob->removeSubjob(nullptr));
    QCOMPARE(compoundJob->subjobs(), {subjob});
    QCOMPARE(subjob->parent(), compoundJob);

    QFETCH(const bool, shouldClearSubjobs);
    if (shouldClearSubjobs) {
        compoundJob->clearSubjobs();
    } else {
        QVERIFY(compoundJob->removeSubjob(subjob));
    }
    QCOMPARE(compoundJob->subjobs(), {});
    QCOMPARE(subjob->parent(), nullptr);
    QVERIFY(!compoundJob->removeSubjob(subjob));

    subjob->emitInfoMessage("!");
    QCOMPARE(subjobSpy.infoMessage.count(), 3);
    subjob->setPercent(91);
    QCOMPARE(subjobSpy.percentChanged.count(), 4);

    QCOMPARE(subjobSpy.finished.count(), 0);
    QCOMPARE(subjobSpy.result.count(), 0);
    subjob->emitResult();
    QCOMPARE(subjobSpy.finished.count(), 1);
    QCOMPARE(subjobSpy.result.count(), 1);

    QVERIFY(subjob);
    QTest::qWait(1);
    QVERIFY(!subjob);

    QVERIFY(compoundJob);
    QCOMPARE(compoundSpy.finished.count(), 0);
    delete compoundJob;
    QCOMPARE(compoundSpy.finished.count(), 1);

    QCOMPARE(compoundSpy.infoMessage.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 0);
}

void KSequentialCompoundJobTest::addClearSubjobs()
{
    QPointer compoundJob(new PublicSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);

    QPointer subjob1(new TestJob);
    JobSpy subjob1Spy(subjob1);
    QSignalSpy started1Spy(subjob1, &TestJob::started);
    QPointer subjob2(new TestJob);
    JobSpy subjob2Spy(subjob2);
    QSignalSpy started2Spy(subjob2, &TestJob::started);

    QVERIFY(compoundJob->addSubjob(subjob1));
    QVERIFY(!compoundJob->removeSubjob(subjob2));
    QVERIFY(compoundJob->addSubjob(subjob2));
    const QList<KJob *> expectedSubjobList{subjob1, subjob2};
    QCOMPARE(compoundJob->subjobs(), expectedSubjobList);
    QCOMPARE(subjob1->parent(), compoundJob);
    QCOMPARE(subjob2->parent(), compoundJob);

    QVERIFY(!compoundJob->addSubjob(subjob2));
    QVERIFY(!compoundJob->addSubjob(subjob1));
    QCOMPARE(compoundJob->subjobs(), expectedSubjobList);

    subjob1->emitInfoMessage("take");
    QCOMPARE(subjob1Spy.infoMessage.count(), 1);
    QCOMPARE(compoundSpy.infoMessage.count(), 1);
    QCOMPARE(compoundSpy.infoMessage.constLast().at(0).value<KJob *>(), subjob1);
    QCOMPARE(compoundSpy.infoMessage.constLast().at(1), "take");

    subjob2->emitInfoMessage("that");
    QCOMPARE(subjob2Spy.infoMessage.count(), 1);
    QCOMPARE(compoundSpy.infoMessage.count(), 2);
    QCOMPARE(compoundSpy.infoMessage.constLast().at(0).value<KJob *>(), subjob2);
    QCOMPARE(compoundSpy.infoMessage.constLast().at(1), "that");

    subjob1->setPercent(31);
    QCOMPARE(subjob1Spy.percentChanged.count(), 1);
    subjob2->setPercent(12);
    QCOMPARE(subjob2Spy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.count(), 0); // the subjobs are not started, their percent changes are ignored

    QCOMPARE(started1Spy.count(), 0);
    compoundJob->start();
    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(started2Spy.count(), 0);

    subjob1->setPercent(52);
    QCOMPARE(subjob1Spy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 26);
    QCOMPARE(compoundJob->percent(), 26);

    subjob2->setPercent(95);
    QCOMPARE(subjob2Spy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 1); // the 2nd subjob is not started, its percent changes are ignored
    QCOMPARE(compoundJob->percent(), 26);

    subjob1->setPercent(83);
    QCOMPARE(subjob1Spy.percentChanged.count(), 3);
    QCOMPARE(compoundSpy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 41);
    QCOMPARE(compoundJob->percent(), 41);

    QVERIFY(!compoundJob->removeSubjob(nullptr));
    QCOMPARE(compoundJob->subjobs(), expectedSubjobList);
    compoundJob->clearSubjobs();
    QCOMPARE(compoundJob->subjobs(), {});
    QCOMPARE(subjob1->parent(), nullptr);
    QCOMPARE(subjob2->parent(), nullptr);
    QVERIFY(!compoundJob->removeSubjob(nullptr));
    QVERIFY(!compoundJob->removeSubjob(subjob2));
    QVERIFY(!compoundJob->removeSubjob(subjob1));
    QCOMPARE(compoundJob->subjobs(), {});

    subjob1->emitInfoMessage("lost");
    subjob2->emitInfoMessage("too");
    subjob1->setPercent(99);
    subjob2->setPercent(100);
    QCOMPARE(compoundSpy.infoMessage.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 2);
    QCOMPARE(compoundJob->percent(), 41);

    delete subjob1;
    delete subjob2;
    QTest::qWait(1);
    QVERIFY(compoundJob);

    QCOMPARE(compoundSpy.finished.count(), 0);
    QVERIFY(compoundJob->kill());
    QCOMPARE(compoundSpy.finished.count(), 1);

    QVERIFY(compoundJob);
    QTest::qWait(1);
    QVERIFY(!compoundJob);

    QCOMPARE(compoundSpy.infoMessage.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 0);
}

void KSequentialCompoundJobTest::subjobPercentChanged()
{
    QPointer compoundJob(new KSimpleSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);

    QPointer subjob1(new TestJob);
    JobSpy subjob1Spy(subjob1);
    QSignalSpy started1Spy(subjob1, &TestJob::started);
    QPointer subjob2(new TestJob);
    JobSpy subjob2Spy(subjob2);
    QSignalSpy started2Spy(subjob2, &TestJob::started);
    QPointer subjob3(new TestJob);
    JobSpy subjob3Spy(subjob3);
    QSignalSpy started3Spy(subjob3, &TestJob::started);

    QVERIFY(compoundJob->addSubjob(subjob1));
    QVERIFY(compoundJob->addSubjob(subjob2));
    QVERIFY(compoundJob->addSubjob(subjob3));

    QCOMPARE(started1Spy.count(), 0);
    compoundJob->start();
    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(started2Spy.count(), 0);

    subjob1->setPercent(33);
    QCOMPARE(subjob1Spy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 33 / 3);
    QCOMPARE(compoundJob->percent(), 11);

    subjob1->setPercent(88);
    QCOMPARE(subjob1Spy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 88 / 3);
    QCOMPARE(compoundJob->percent(), 29);

    subjob1->emitResult();
    QCOMPARE(subjob1Spy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 3);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 100 / 3);
    QCOMPARE(compoundJob->percent(), 33);

    QCOMPARE(started1Spy.count(), 1);
    QCOMPARE(subjob1Spy.finished.count(), 1);
    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(started3Spy.count(), 0);

    subjob2->setPercent(4);
    QCOMPARE(subjob2Spy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.count(), 4);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), (100 + 4) / 3);
    QCOMPARE(compoundJob->percent(), 34);

    subjob2->setPercent(5);
    QCOMPARE(subjob2Spy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.count(), 5);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), (100 + 5) / 3);
    QCOMPARE(compoundJob->percent(), 35);

    subjob2->setPercent(7);
    QCOMPARE(subjob2Spy.percentChanged.count(), 3);
    QCOMPARE(compoundSpy.percentChanged.count(), 5); // the total percent value hasn't changed
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), (100 + 7) / 3);
    QCOMPARE(compoundJob->percent(), 35);

    subjob2->setPercent(99);
    QCOMPARE(subjob2Spy.percentChanged.count(), 4);
    QCOMPARE(compoundSpy.percentChanged.count(), 6);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), (100 + 99) / 3);
    QCOMPARE(compoundJob->percent(), 66);

    subjob2->emitResult();
    QCOMPARE(subjob2Spy.percentChanged.count(), 4);
    QCOMPARE(compoundSpy.percentChanged.count(), 6); // the total percent value hasn't changed
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 2 * 100 / 3);
    QCOMPARE(compoundJob->percent(), 66);

    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(subjob2Spy.finished.count(), 1);
    QCOMPARE(started3Spy.count(), 1);

    subjob3->setPercent(50);
    QCOMPARE(subjob3Spy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.count(), 7);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), (200 + 50) / 3);
    QCOMPARE(compoundJob->percent(), 83);

    delete subjob3;
    QCOMPARE(subjob3Spy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.count(), 8);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 3 * 100 / 3);
    QCOMPARE(compoundJob->percent(), 100);

    QCOMPARE(started3Spy.count(), 1);
    QCOMPARE(subjob3Spy.finished.count(), 1);

    QTest::qWait(1);
    QVERIFY(!subjob1);
    QVERIFY(!subjob2);
    QVERIFY(!subjob3);
    QVERIFY(!compoundJob);

    QCOMPARE(compoundSpy.percentChanged.count(), 8);
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 1);
}

void KSequentialCompoundJobTest::abortOnSubjobError()
{
    QPointer compoundJob(new PublicSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);

    QPointer subjob1(new TestJob);
    QPointer subjob2(new TestJob);
    JobSpy subjob2Spy(subjob2);
    QSignalSpy started2Spy(subjob2, &TestJob::started);

    QVERIFY(compoundJob->addSubjob(subjob1));
    QVERIFY(compoundJob->addSubjob(subjob2));
    const QList<KJob *> expectedSubjobList{subjob1, subjob2};
    QCOMPARE(compoundJob->subjobs(), expectedSubjobList);

    compoundJob->start();

    QCOMPARE(started2Spy.count(), 0);
    subjob1->setError(244);
    subjob1->setErrorText("Crashed");
    subjob1->emitResult();
    QCOMPARE(compoundJob->error(), 244);
    QCOMPARE(compoundJob->errorText(), "Crashed");
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 1);

    QTest::qWait(1);
    QVERIFY(!subjob1);
    QVERIFY(!subjob2);
    QVERIFY(!compoundJob);

    QCOMPARE(started2Spy.count(), 0);
    QCOMPARE(subjob2Spy.finished.count(), 1);
    QCOMPARE(subjob2Spy.result.count(), 0);
}

void KSequentialCompoundJobTest::disableAbortOnSubjobError_data()
{
    QTest::addColumn<bool>("lastSubjobError");
    QTest::newRow("last-subjob-no-error") << false;
    QTest::newRow("last-subjob-error") << true;
}

void KSequentialCompoundJobTest::disableAbortOnSubjobError()
{
    QPointer compoundJob(new PublicSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);
    compoundJob->setAbortOnSubjobError(false);

    QPointer subjob1(new TestJob);
    QPointer subjob2(new TestJob);
    JobSpy subjob2Spy(subjob2);
    QSignalSpy started2Spy(subjob2, &TestJob::started);

    QVERIFY(compoundJob->addSubjob(subjob1));
    QVERIFY(compoundJob->addSubjob(subjob2));
    const QList<KJob *> expectedSubjobList{subjob1, subjob2};
    QCOMPARE(compoundJob->subjobs(), expectedSubjobList);

    compoundJob->start();

    QCOMPARE(started2Spy.count(), 0);
    subjob1->setError(105);
    subjob1->setErrorText("Failed");
    subjob1->emitResult();
    QCOMPARE(started2Spy.count(), 1);
    QCOMPARE(subjob1->parent(), nullptr);
    QCOMPARE(subjob2->parent(), compoundJob);
    QCOMPARE(compoundJob->subjobs(), {subjob2});

    QTest::qWait(1);
    QVERIFY(!subjob1);

    QCOMPARE(subjob2Spy.finished.count(), 0);
    QCOMPARE(subjob2Spy.result.count(), 0);
    QCOMPARE(compoundSpy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 50);
    QCOMPARE(compoundJob->percent(), 50);
    QCOMPARE(compoundJob->error(), 0);
    QCOMPARE(compoundJob->errorText(), "");
    QCOMPARE(compoundSpy.finished.count(), 0);
    QCOMPARE(compoundSpy.result.count(), 0);

    QFETCH(const bool, lastSubjobError);
    const int errorCode = lastSubjobError ? 902 : 0;
    const QString errorText = lastSubjobError ? "broken pipe" : "";
    if (lastSubjobError) {
        subjob2->setError(errorCode);
        subjob2->setErrorText(errorText);
    }
    subjob2->emitResult();
    QCOMPARE(compoundSpy.percentChanged.count(), 2);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 100);
    QCOMPARE(compoundJob->percent(), 100);
    QCOMPARE(compoundJob->error(), errorCode);
    QCOMPARE(compoundJob->errorText(), errorText);
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 1);

    QTest::qWait(1);
    QVERIFY(!subjob2);
    QVERIFY(!compoundJob);
}

void KSequentialCompoundJobTest::finishWrongSubjob_data()
{
    QTest::addColumn<bool>("abortOnSubjobError");
    QTest::newRow("abort-on-subjob-error") << true;
    QTest::newRow("no-abort-on-subjob-error") << false;
}

void KSequentialCompoundJobTest::finishWrongSubjob()
{
    QPointer compoundJob(new PublicSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);

    QFETCH(const bool, abortOnSubjobError);
    compoundJob->setAbortOnSubjobError(abortOnSubjobError);

    QPointer subjob1(new TestJob);
    QPointer subjob2(new TestJob);
    JobSpy subjob2Spy(subjob2);
    QSignalSpy started2Spy(subjob2, &TestJob::started);

    QVERIFY(compoundJob->addSubjob(subjob1));
    QVERIFY(compoundJob->addSubjob(subjob2));
    const QList<KJob *> expectedSubjobList{subjob1, subjob2};
    QCOMPARE(compoundJob->subjobs(), expectedSubjobList);

    compoundJob->start();

    subjob2->setError(543);
    subjob2->setErrorText("Canceled");
    subjob2->emitResult();
    QCOMPARE(subjob2->error(), 543);
    QCOMPARE(subjob2->errorText(), "Canceled");
    QCOMPARE(started2Spy.count(), 0);
    QCOMPARE(subjob2Spy.finished.count(), 1);
    QCOMPARE(subjob2Spy.result.count(), 1);
    // when an unstarted job finishes, sequential compound job simply removes it
    QCOMPARE(subjob2->parent(), nullptr);
    QCOMPARE(compoundJob->subjobs(), {subjob1});

    QTest::qWait(1);
    QVERIFY(!subjob2);

    QCOMPARE(compoundJob->error(), 0);
    QCOMPARE(compoundJob->errorText(), "");
    QCOMPARE(compoundSpy.finished.count(), 0);
    QCOMPARE(compoundSpy.result.count(), 0);

    subjob1->emitResult();
    QCOMPARE(subjob1->parent(), nullptr);
    QCOMPARE(compoundJob->subjobs(), {});
    QCOMPARE(compoundJob->error(), 0);
    QCOMPARE(compoundJob->errorText(), "");
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 1);

    // These percent checks do not test expected behavior, but demonstrate what the current behavior is.
    // compoundSpy.percentChanged.count() == 2 and compoundJob->percent() == 100 may be more correct/useful.
    QCOMPARE(compoundSpy.percentChanged.count(), 1);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(0).value<KJob *>(), compoundJob);
    QCOMPARE(compoundSpy.percentChanged.constLast().at(1), 50);
    QCOMPARE(compoundJob->percent(), 50);

    QTest::qWait(1);
    QVERIFY(!subjob1);
    QVERIFY(!compoundJob);
}

void KSequentialCompoundJobTest::killUnstartedCompoundJob_data()
{
    QTest::addColumn<int>("subjobCount");
    QTest::addColumn<KJob::KillVerbosity>("killVerbosity");
    for (int subjobCount : {0, 1, 4}) {
        QTest::addRow("kill-quietly-%d-subjobs", subjobCount) << subjobCount << KJob::Quietly;
        QTest::addRow("emit-result-%d-subjobs", subjobCount) << subjobCount << KJob::EmitResult;
    }
}

void KSequentialCompoundJobTest::killUnstartedCompoundJob()
{
    QPointer compoundJob(new KSimpleSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);
    QVERIFY(compoundJob->capabilities() & KJob::Killable);

    QFETCH(const int, subjobCount);
    auto subjobSpies = generateKillableTestJobSpies(subjobCount);
    for (const auto &spy : subjobSpies) {
        QVERIFY(compoundJob->addSubjob(spy->job));
    }

    QCOMPARE(compoundSpy.finished.count(), 0);
    QFETCH(const KJob::KillVerbosity, killVerbosity);
    QVERIFY(compoundJob->kill(killVerbosity));
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), killVerbosity == KJob::EmitResult);
    QCOMPARE(compoundJob->error(), KJob::KilledJobError);

    QVERIFY(compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(spy->job);
        // no need to kill unstarted subjobs, just destroy them
        QCOMPARE(spy->job->error(), 0);
        QCOMPARE(spy->job->errorText(), "");
    }
    QTest::qWait(1);
    QVERIFY(!compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(!spy->job);
        QCOMPARE(spy->started.count(), 0);
        QCOMPARE(spy->killed.count(), 0); // no need to kill unstarted subjobs, just destroy them
        QCOMPARE(spy->finished.count(), 1);
        QCOMPARE(spy->result.count(), 0);
    }
}

void KSequentialCompoundJobTest::killFinishedCompoundJob_data()
{
    killUnstartedCompoundJob_data();
}

void KSequentialCompoundJobTest::killFinishedCompoundJob()
{
    QPointer compoundJob(new KSimpleSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);
    QVERIFY(compoundJob->capabilities() & KJob::Killable);

    QFETCH(const int, subjobCount);
    auto subjobSpies = generateKillableTestJobSpies(subjobCount);
    for (const auto &spy : subjobSpies) {
        QVERIFY(compoundJob->addSubjob(spy->job));
    }

    compoundJob->start();
    for (std::size_t i = 0; i < subjobSpies.size(); ++i) {
        QCOMPARE(compoundSpy.finished.count(), 0);
        auto &spy = subjobSpies[i];
        QCOMPARE(spy->started.count(), 1);
        QCOMPARE(spy->finished.count(), 0);
        QCOMPARE(spy->result.count(), 0);
        spy->job->emitResult();
        QCOMPARE(spy->started.count(), 1);
        QCOMPARE(spy->finished.count(), 1);
        QCOMPARE(spy->result.count(), 1);
    }
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 1);

    QFETCH(const KJob::KillVerbosity, killVerbosity);
    QVERIFY(compoundJob->kill(killVerbosity));
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 1);
    QCOMPARE(compoundJob->error(), 0);
    QCOMPARE(compoundJob->errorText(), "");

    QVERIFY(compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(spy->job);
        QCOMPARE(spy->job->error(), 0);
        QCOMPARE(spy->job->errorText(), "");
    }
    QTest::qWait(1);
    QVERIFY(!compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(!spy->job);
        QCOMPARE(spy->started.count(), 1);
        QCOMPARE(spy->killed.count(), 0);
        QCOMPARE(spy->finished.count(), 1);
        QCOMPARE(spy->result.count(), 1);
    }
}

void KSequentialCompoundJobTest::killRunningCompoundJob()
{
    QPointer compoundJob(new KSimpleSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);
    QVERIFY(compoundJob->capabilities() & KJob::Killable);

    auto subjobSpies = generateKillableTestJobSpies(3);
    QCOMPARE(subjobSpies.size(), 3);
    for (const auto &spy : subjobSpies) {
        QVERIFY(compoundJob->addSubjob(spy->job));
    }
    const auto &firstSubjobSpy = *subjobSpies.front();

    compoundJob->start();
    QCOMPARE(firstSubjobSpy.started.count(), 1);
    QCOMPARE(firstSubjobSpy.finished.count(), 0);
    QCOMPARE(compoundSpy.finished.count(), 0);

    QVERIFY(compoundJob->kill());
    QCOMPARE(firstSubjobSpy.started.count(), 1);
    QCOMPARE(firstSubjobSpy.killed.count(), 1);
    QCOMPARE(firstSubjobSpy.killed.constLast().constFirst(), true);
    QCOMPARE(firstSubjobSpy.finished.count(), 1);
    QCOMPARE(firstSubjobSpy.result.count(), 0); // kill the running subjob quietly
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 0);

    QCOMPARE(firstSubjobSpy.job->error(), KJob::KilledJobError);
    QCOMPARE(compoundJob->error(), KJob::KilledJobError);
    QCOMPARE(compoundJob->errorText(), firstSubjobSpy.job->errorText());

    QVERIFY(compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(spy->job);
        if (spy.get() != &firstSubjobSpy) {
            QCOMPARE(spy->job->error(), 0);
            QCOMPARE(spy->job->errorText(), "");
        }
    }
    QTest::qWait(1);
    QVERIFY(!compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(!spy->job);
        const bool isFirstSubjob = spy.get() == &firstSubjobSpy;
        QCOMPARE(spy->started.count(), isFirstSubjob); // don't start subjobs once killed
        QCOMPARE(spy->killed.count(), isFirstSubjob); // no need to kill unstarted subjobs, just destroy them
        QCOMPARE(spy->finished.count(), 1);
        QCOMPARE(spy->result.count(), 0);
    }
}

void KSequentialCompoundJobTest::killingSubjobFails()
{
    QPointer compoundJob(new PublicSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);
    QVERIFY(compoundJob->capabilities() & KJob::Killable);

    auto subjobSpies = generateKillableTestJobSpies(7);
    QCOMPARE(subjobSpies.size(), 7);
    for (const auto &spy : subjobSpies) {
        QVERIFY(compoundJob->addSubjob(spy->job));
    }
    const auto &firstSubjobSpy = *subjobSpies.front();
    firstSubjobSpy.job->setKillingSucceeds(false);

    compoundJob->start();
    QCOMPARE(firstSubjobSpy.started.count(), 1);
    QCOMPARE(firstSubjobSpy.finished.count(), 0);
    QCOMPARE(compoundSpy.finished.count(), 0);

    QVERIFY(!compoundJob->kill());
    QCOMPARE(firstSubjobSpy.started.count(), 1);
    QCOMPARE(firstSubjobSpy.killed.count(), 1);
    QCOMPARE(firstSubjobSpy.killed.constLast().constFirst(), false);
    QCOMPARE(firstSubjobSpy.finished.count(), 0);
    QCOMPARE(firstSubjobSpy.result.count(), 0);
    QCOMPARE(compoundSpy.finished.count(), 0);
    QCOMPARE(compoundJob->subjobs().count(), subjobSpies.size());

    QCOMPARE(firstSubjobSpy.job->error(), 0);
    QCOMPARE(firstSubjobSpy.job->errorText(), "");
    QCOMPARE(compoundJob->error(), 0);
    QCOMPARE(compoundJob->errorText(), "");

    // unstarted subjobs are simply removed when finished, their errors are ignored

    const auto &finishedSubjobSpy = *subjobSpies[3];
    finishedSubjobSpy.job->emitResult();
    QCOMPARE(finishedSubjobSpy.started.count(), 0);
    QCOMPARE(finishedSubjobSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.finished.count(), 0);
    QCOMPARE(compoundJob->subjobs().count(), subjobSpies.size() - 1);
    QVERIFY(!compoundJob->subjobs().contains(finishedSubjobSpy.job));

    const auto &errorSubjobSpy = *subjobSpies[1];
    errorSubjobSpy.job->setError(142);
    errorSubjobSpy.job->emitResult();
    QCOMPARE(errorSubjobSpy.started.count(), 0);
    QCOMPARE(errorSubjobSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.finished.count(), 0);
    QCOMPARE(compoundJob->subjobs().count(), subjobSpies.size() - 2);
    QVERIFY(!compoundJob->subjobs().contains(errorSubjobSpy.job));

    const auto &killedSubjobSpy = *subjobSpies[4];
    QVERIFY(killedSubjobSpy.job->kill());
    QCOMPARE(killedSubjobSpy.started.count(), 0);
    QCOMPARE(killedSubjobSpy.finished.count(), 1);
    QCOMPARE(killedSubjobSpy.result.count(), 0);
    QCOMPARE(compoundSpy.finished.count(), 0);
    QCOMPARE(compoundJob->subjobs().count(), subjobSpies.size() - 3);
    QVERIFY(!compoundJob->subjobs().contains(killedSubjobSpy.job));

    QCOMPARE(compoundJob->error(), 0);
    QCOMPARE(compoundJob->errorText(), "");

    firstSubjobSpy.job->emitResult();
    QCOMPARE(firstSubjobSpy.started.count(), 1);
    QCOMPARE(firstSubjobSpy.killed.count(), 1);
    QCOMPARE(firstSubjobSpy.finished.count(), 1);
    QCOMPARE(firstSubjobSpy.result.count(), 1);
    QCOMPARE(compoundSpy.finished.count(), 1);
    QCOMPARE(compoundSpy.result.count(), 1);

    QCOMPARE(firstSubjobSpy.job->error(), 0);
    // the compound job was killed, so it should report KilledJobError, even if the killed subjob does not
    QCOMPARE(compoundJob->error(), KJob::KilledJobError);
    QCOMPARE(compoundJob->errorText(), firstSubjobSpy.job->errorText());

    QVERIFY(compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(spy->job);
    }
    QTest::qWait(1);
    QVERIFY(!compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(!spy->job);
        const bool isFirstSubjob = spy.get() == &firstSubjobSpy;
        QCOMPARE(spy->started.count(), isFirstSubjob); // don't start subjobs once killed
        QCOMPARE(spy->killed.count(), isFirstSubjob || spy.get() == &killedSubjobSpy);
        QCOMPARE(spy->finished.count(), 1);
        QCOMPARE(spy->result.count(), isFirstSubjob || spy.get() == &finishedSubjobSpy || spy.get() == &errorSubjobSpy);
    }
}

void KSequentialCompoundJobTest::killRunningCompoundJobRepeatedly_data()
{
    QTest::addColumn<int>("killAttempts");
    QTest::addColumn<int>("successfulKillAttempts");
    QTest::addColumn<bool>("destroyKilledSubjob");
    for (int killAttempts = 1; killAttempts <= 3; ++killAttempts) {
        for (int successfulKillAttempts = 0; successfulKillAttempts <= killAttempts; ++successfulKillAttempts) {
            QTest::addRow("attempts-%d-successful-%d", killAttempts, successfulKillAttempts) << killAttempts << successfulKillAttempts << false;
            if (successfulKillAttempts == 0) {
                QTest::addRow("attempts-%d-successful-%d-destroy-killed-subjob", killAttempts, successfulKillAttempts)
                    << killAttempts << successfulKillAttempts << true;
            }
        }
    }
}

void KSequentialCompoundJobTest::killRunningCompoundJobRepeatedly()
{
    QPointer compoundJob(new KSimpleSequentialCompoundJob);
    JobSpy compoundSpy(compoundJob);
    QVERIFY(compoundJob->capabilities() & KJob::Killable);

    // The number of subjobs is independent from and unrelated to the number of kill attempts.
    auto subjobSpies = generateKillableTestJobSpies(3);
    QCOMPARE(subjobSpies.size(), 3);
    for (const auto &spy : subjobSpies) {
        QVERIFY(compoundJob->addSubjob(spy->job));
    }
    const auto &firstSubjobSpy = *subjobSpies.front();

    compoundJob->start();
    QCOMPARE(firstSubjobSpy.started.count(), 1);
    QCOMPARE(firstSubjobSpy.finished.count(), 0);
    QCOMPARE(compoundSpy.finished.count(), 0);

    QFETCH(const int, killAttempts);
    QFETCH(const int, successfulKillAttempts);
    for (int attempt = 0; attempt < killAttempts; ++attempt) {
        const bool killingSucceeds = attempt >= killAttempts - successfulKillAttempts;
        firstSubjobSpy.job->setKillingSucceeds(killingSucceeds);
        QCOMPARE(compoundJob->kill(), killingSucceeds);
        // KJob::kill() calls KSequentialCompoundJob::doKill() only if the job has not finished,
        // i.e. if the previous killing failed.
        const int subjobKillCount = std::min(attempt, killAttempts - successfulKillAttempts) + 1;

        QCOMPARE(firstSubjobSpy.started.count(), 1);
        QCOMPARE(firstSubjobSpy.killed.count(), subjobKillCount);
        if (attempt < subjobKillCount) {
            QCOMPARE(firstSubjobSpy.killed.at(attempt).constFirst(), killingSucceeds);
        }
        QCOMPARE(firstSubjobSpy.finished.count(), killingSucceeds);
        QCOMPARE(firstSubjobSpy.result.count(), 0); // kill the running subjob quietly
        QCOMPARE(compoundSpy.finished.count(), killingSucceeds);
        QCOMPARE(compoundSpy.result.count(), 0);

        const auto error = killingSucceeds ? KJob::KilledJobError : KJob::NoError;
        QCOMPARE(firstSubjobSpy.job->error(), error);
        QCOMPARE(compoundJob->error(), error);
    }

    QFETCH(const bool, destroyKilledSubjob);
    if (destroyKilledSubjob) {
        delete firstSubjobSpy.job;
        QVERIFY(compoundJob);
        QCOMPARE(compoundSpy.result.count(), 1);
    } else if (successfulKillAttempts == 0) {
        delete compoundJob;
        QVERIFY(!compoundJob);
        QCOMPARE(compoundSpy.result.count(), 0);
    } else {
        QVERIFY(compoundJob);
        QCOMPARE(compoundSpy.result.count(), 0); // killed quietly in the loop above
    }

    QCOMPARE(compoundSpy.finished.count(), 1);
    QTest::qWait(1);
    QVERIFY(!compoundJob);
    for (const auto &spy : subjobSpies) {
        QVERIFY(!spy->job);
        const bool isFirstSubjob = spy.get() == &firstSubjobSpy;
        QCOMPARE(spy->started.count(), isFirstSubjob); // don't start subjobs once killed
        if (isFirstSubjob) {
            QVERIFY(spy->killed.count() <= killAttempts);
        } else {
            QCOMPARE(spy->killed.count(), 0);
        }
        QCOMPARE(spy->finished.count(), 1);
        QCOMPARE(spy->result.count(), 0);
    }
}

QTEST_GUILESS_MAIN(KSequentialCompoundJobTest)
