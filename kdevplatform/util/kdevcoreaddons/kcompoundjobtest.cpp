/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcompositejobtest.h"

#include <QSignalSpy>
#include <QTest>
#include <QTimer>

TestJob::TestJob(QObject *parent)
    : KJob(parent)
{
}

void TestJob::start()
{
    QTimer::singleShot(1000, this, &TestJob::doEmit);
}

void TestJob::doEmit()
{
    emitResult();
}

void CompositeJob::start()
{
    if (hasSubjobs()) {
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

bool CompositeJob::addSubjob(KJob *job)
{
    return KCompositeJob::addSubjob(job);
}

void CompositeJob::slotResult(KJob *job)
{
    KCompositeJob::slotResult(job);

    if (!error() && hasSubjobs()) {
        // start next
        subjobs().first()->start();
    } else {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
    }
}

KCompositeJobTest::KCompositeJobTest()
    : loop(this)
{
}

/**
 * In case a composite job is deleted during execution
 * we still want to assure that we don't crash
 *
 * see bug: https://bugs.kde.org/show_bug.cgi?id=230692
 */
void KCompositeJobTest::testDeletionDuringExecution()
{
    QObject *someParent = new QObject;
    KJob *job = new TestJob(someParent);

    CompositeJob *compositeJob = new CompositeJob;
    compositeJob->setAutoDelete(false);
    QVERIFY(compositeJob->addSubjob(job));

    QCOMPARE(job->parent(), compositeJob);

    QSignalSpy destroyed_spy(job, &QObject::destroyed);
    // check if job got reparented properly
    delete someParent;
    someParent = nullptr;
    // the job should still exist, because it is a child of KCompositeJob now
    QCOMPARE(destroyed_spy.size(), 0);

    // start async, the subjob takes 1 second to finish
    compositeJob->start();

    // delete the job during the execution
    delete compositeJob;
    compositeJob = nullptr;
    // at this point, the subjob should be deleted, too
    QCOMPARE(destroyed_spy.size(), 1);
}

QTEST_GUILESS_MAIN(KCompositeJobTest)
