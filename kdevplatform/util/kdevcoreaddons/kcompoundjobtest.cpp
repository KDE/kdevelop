/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcompoundjobtest.h"

#include "ksequentialcompoundjob.h"

#include <QEventLoop>
#include <QMetaEnum>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <QTimer>

namespace
{
class TestSequentialCompoundJob : public KSequentialCompoundJob
{
public:
    using KSequentialCompoundJob::addSubjob;
    using KSequentialCompoundJob::clearSubjobs;
};

struct JobSpies {
    QSignalSpy finished;
    QSignalSpy result;
    QSignalSpy destroyed;
    explicit JobSpies(KJob *job)
        : finished(job, &KJob::finished)
        , result(job, &KJob::result)
        , destroyed(job, &QObject::destroyed)
    {
    }
};
} // namespace

TestJob::TestJob(QObject *parent)
    : KJob(parent)
{
}

void TestJob::start()
{
    QTimer::singleShot(1000, this, &TestJob::emitResult);
}

KillableTestJob::KillableTestJob(QObject *parent)
    : TestJob(parent)
{
    setCapabilities(Killable);
}

bool KillableTestJob::doKill()
{
    return true;
}

void TestCompoundJob::start()
{
    if (hasSubjobs()) {
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

void TestCompoundJob::subjobFinished(KJob *job)
{
    KCompoundJob::subjobFinished(job);

    if (error()) {
        return; // KCompoundJob::subjobFinished() must have called emitResult().
    }
    if (hasSubjobs()) {
        // start next
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

KCompoundJobTest::KCompoundJobTest()
{
}

void KCompoundJobTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

/**
 * In case a compound job is deleted during execution
 * we still want to assure that we don't crash
 *
 * see bug: https://bugs.kde.org/show_bug.cgi?id=230692
 */
template<class CompoundJob>
static void testDeletionDuringExecution()
{
    QObject *someParent = new QObject;
    KJob *job = new TestJob(someParent);

    auto *compoundJob = new CompoundJob;
    compoundJob->setAutoDelete(false);
    QVERIFY(compoundJob->addSubjob(job));

    QCOMPARE(job->parent(), compoundJob);

    QSignalSpy destroyed_spy(job, &QObject::destroyed);
    // check if job got reparented properly
    delete someParent;
    someParent = nullptr;
    // the job should still exist, because it is a child of KCompoundJob now
    QCOMPARE(destroyed_spy.size(), 0);

    // start async, the subjob takes 1 second to finish
    compoundJob->start();

    // delete the job during the execution
    delete compoundJob;
    compoundJob = nullptr;
    // at this point, the subjob should be deleted, too
    QCOMPARE(destroyed_spy.size(), 1);
}

void KCompoundJobTest::testDeletionDuringExecution_data()
{
    QTest::addColumn<bool>("useSequentialCompoundJob");
    QTest::newRow("CompoundJob") << false;
    QTest::newRow("SequentialCompoundJob") << true;
}

void KCompoundJobTest::testDeletionDuringExecution()
{
    QFETCH(const bool, useSequentialCompoundJob);
    if (useSequentialCompoundJob) {
        ::testDeletionDuringExecution<TestSequentialCompoundJob>();
    } else {
        ::testDeletionDuringExecution<TestCompoundJob>();
    }
}

template<class CompoundJob>
static void testFinishingSubjob()
{
    auto *const job = new KillableTestJob;
    auto *const compoundJob = new CompoundJob;
    QVERIFY(compoundJob->addSubjob(job));

    JobSpies jobSpies(job);
    JobSpies compoundJobSpies(compoundJob);

    compoundJob->start();

    using Action = KCompoundJobTest::Action;
    QFETCH(const Action, action);
    switch (action) {
    case Action::Finish:
        job->emitResult();
        break;
    case Action::KillVerbosely:
        QVERIFY(job->kill(KJob::EmitResult));
        break;
    case Action::KillQuietly:
        QVERIFY(job->kill(KJob::Quietly));
        break;
    case Action::Destroy:
        job->deleteLater();
        break;
    }

    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    QObject::connect(compoundJob, &QObject::destroyed, &loop, &QEventLoop::quit);
    QCOMPARE(loop.exec(), 0);

    // The following 3 comparisons verify that KJob works as expected.
    QCOMPARE(jobSpies.finished.size(), 1); // KJob::finished() is always emitted.
    // KJob::result() is not emitted when a job is killed quietly or destroyed.
    QCOMPARE(jobSpies.result.size(), action == Action::Finish || action == Action::KillVerbosely);
    // An auto-delete job is destroyed via deleteLater() when finished.
    QCOMPARE(jobSpies.destroyed.size(), 1);

    // KCompoundJob must listen to &KJob::finished signal to invoke subjobFinished()
    // no matter how a subjob is finished - normally, killed or destroyed.
    // CompoundJob calls emitResult() and is destroyed when its last subjob finishes.
    QFETCH(const bool, crashOnFailure);
    if (crashOnFailure) {
        if (compoundJobSpies.destroyed.empty()) {
            // compoundJob is still alive. This must be a bug.
            // The clearSubjobs() call will segfault if the already destroyed job
            // has not been removed from the subjob list.
            compoundJob->clearSubjobs();
            delete compoundJob;
        }
    } else {
        QCOMPARE(compoundJobSpies.finished.size(), 1);
        QCOMPARE(compoundJobSpies.result.size(), 1);
        QCOMPARE(compoundJobSpies.destroyed.size(), 1);
    }
}

void KCompoundJobTest::testFinishingSubjob_data()
{
    QTest::addColumn<bool>("useSequentialCompoundJob");
    QTest::addColumn<Action>("action");
    QTest::addColumn<bool>("crashOnFailure");

    const auto actionName = [](Action action) {
        return QMetaEnum::fromType<Action>().valueToKey(static_cast<int>(action));
    };

    for (bool useSequentialCompoundJob : {false, true}) {
        const char *const sequentialStr = useSequentialCompoundJob ? "sequential-" : "";
        for (bool crashOnFailure : {false, true}) {
            const char *const failureStr = crashOnFailure ? "segfault-on-failure" : "compound-job-destroyed";
            for (Action action : {Action::Finish, Action::KillVerbosely, Action::KillQuietly, Action::Destroy}) {
                const QByteArray dataTag = QByteArray{actionName(action)} + "-a-subjob-" + sequentialStr + failureStr;
                QTest::newRow(dataTag.constData()) << useSequentialCompoundJob << action << crashOnFailure;
            }
        }
    }
}

void KCompoundJobTest::testFinishingSubjob()
{
    QFETCH(const bool, useSequentialCompoundJob);
    if (useSequentialCompoundJob) {
        ::testFinishingSubjob<TestSequentialCompoundJob>();
    } else {
        ::testFinishingSubjob<TestCompoundJob>();
    }
}

QTEST_GUILESS_MAIN(KCompoundJobTest)
