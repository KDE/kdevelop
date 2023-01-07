/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcompoundjobtest.h"

#include <QSignalSpy>
#include <QStandardPaths>
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

void TestCompoundJob::start()
{
    if (hasSubjobs()) {
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

bool TestCompoundJob::addSubjob(KJob *job)
{
    return KCompoundJob::addSubjob(job);
}

void TestCompoundJob::slotResult(KJob *job)
{
    KCompoundJob::slotResult(job);

    if (!error() && hasSubjobs()) {
        // start next
        subjobs().first()->start();
    } else {
        setError(job->error());
        setErrorText(job->errorText());
        emitResult();
    }
}

KCompoundJobTest::KCompoundJobTest()
    : loop(this)
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
void KCompoundJobTest::testDeletionDuringExecution()
{
    QObject *someParent = new QObject;
    KJob *job = new TestJob(someParent);

    auto *compoundJob = new TestCompoundJob;
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

QTEST_GUILESS_MAIN(KCompoundJobTest)
