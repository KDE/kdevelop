/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_foregroundlock.h"

#include <QTest>
#include <QStandardPaths>
#include <QThread>
#include <QRandomGenerator>
#include <QEventLoop>
#include <QAtomicInt>

#include <memory>
#include <vector>

#include "../foregroundlock.h"

QTEST_MAIN(KDevelop::TestForegroundLock)

using namespace KDevelop;

//BEGIN Helper Threads

class TryLockThread : public QThread
{
    Q_OBJECT

public:
    QAtomicInt counter;

    void run() override
    {
        ForegroundLock lock(false);
        auto* randomGenerator = QRandomGenerator::global();
        int localCounter = 0;
        for (int i = 0; i < 1000; ++i) {
            if (lock.tryLock()) {
                ++localCounter;
                lock.unlock();
            }
            QThread::usleep(randomGenerator->bounded(20));
        }
        counter = localCounter;
    }
};

class QueuedLockThread : public QThread
{
    Q_OBJECT

public:
    QAtomicInt counter;

    void run() override
    {
        ForegroundLock lock(false);
        auto* randomGenerator = QRandomGenerator::global();
        int localCounter = 0;
        for (int i = 0; i < 1000; ++i) {
            lock.relock();
            ++localCounter;
            {
                ForegroundLock nested;
                TemporarilyReleaseForegroundLock unlocker;
                // Multiple threads can sleep here.
                QThread::usleep(randomGenerator->bounded(20));
            }
            lock.unlock();
        }
        counter = localCounter;
    }
};

void TestForegroundLock::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestForegroundLock::testMainThreadLock()
{
    QCOMPARE(ForegroundLock::isLockedForThread(), true);
    ForegroundLock lock;
    QCOMPARE(lock.isLocked(), true);
    QCOMPARE(ForegroundLock::isLockedForThread(), true);
    lock.unlock();
    QCOMPARE(lock.isLocked(), false);
    QCOMPARE(ForegroundLock::isLockedForThread(), true);
    {
        TemporarilyReleaseForegroundLock unlocker;
        QCOMPARE(ForegroundLock::isLockedForThread(), false);
    }
    QCOMPARE(ForegroundLock::isLockedForThread(), true);
}

void TestForegroundLock::testQueuedLock_data()
{
    QTest::addColumn<int>("numThreads");
    for (int i = 1; i <= 10; ++i) {
        QTest::newRow(qPrintable(QString::number(i))) << i;
    }
}

void TestForegroundLock::testQueuedLock()
{
    QFETCH(int, numThreads);
    std::vector<std::unique_ptr<QueuedLockThread>> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::make_unique<QueuedLockThread>());
    }

    for (auto& thread : threads) {
        thread->start();
    }

    // Process events until all of the threads stop running.
    // (Otherwise we would dead-lock.)
    QEventLoop events;
    while (true) {
        const bool running = std::any_of(threads.cbegin(), threads.cend(), [](const auto& thread) {
            return thread->isRunning();
        });
        if (!running) {
            break;
        }
        ForegroundLock lock;
        events.processEvents();
    }

    for (auto& thread : threads) {
        QCOMPARE(thread->counter.loadAcquire(), 1000);
    }
}

void TestForegroundLock::testTryLock_data()
{
    QTest::addColumn<int>("numThreads");
    for (int i = 1; i <= 10; ++i) {
        QTest::newRow(qPrintable(QString::number(i))) << i;
    }
}

void TestForegroundLock::testTryLock()
{
    QFETCH(int, numThreads);
    std::vector<std::unique_ptr<TryLockThread>> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::make_unique<TryLockThread>());
    }

    ForegroundLock lock(true);

    for (auto& thread : threads) {
        thread->start();
    }

    lock.unlock();
    QCOMPARE(lock.isLocked(), false);
    QCOMPARE(ForegroundLock::isLockedForThread(), true);

    while (true) {
        const bool running
            = std::any_of(threads.cbegin(), threads.cend(), [](const auto& thread) { return thread->isRunning(); });

        if (!running) {
            break;
        }
        lock.relock();
        QThread::usleep(10);
        lock.unlock();
    }

    for (auto& thread : threads) {
        // Main thread was never asked to release the foreground mutex, so all of the tryLock() attempts failed.
        QCOMPARE(thread->counter.loadAcquire(), 0);
    }

    // TEST: unlocker drives the unlocking while the threads try loan the lock from it.
    auto unlocker = std::make_unique<QueuedLockThread>();
    unlocker->start();
    for (auto& thread : threads) {
        thread->start();
    }

    // Process events until all of the threads stop running.
    // (Otherwise we would dead-lock.)
    QEventLoop events;
    while (true) {
        const bool running = std::any_of(threads.cbegin(), threads.cend(), [](const auto& thread) {
            return thread->isRunning();
        });
        if (!(running || unlocker->isRunning())) {
            break;
        }
        events.processEvents();
    }

    QCOMPARE(unlocker->counter.loadAcquire(), 1000);

    int tryLockSuccesses = 0;
    for (auto& thread : threads) {
        tryLockSuccesses += thread->counter.loadAcquire();
    }
    // This sum can validly be zero due to "racing to finish". However, if tryLockSuccesses is always
    // consistently zero over multiple test runs the ForegroundLock is not working as intended.
    if (tryLockSuccesses == 0) {
        qCritical() << "No threads succeeded with ForegroundLock::tryLock()?";
    } else {
        qInfo() << "Measured" << tryLockSuccesses << "ForegroundLock::tryLock() acquires.";
    }
}

#include "moc_test_foregroundlock.cpp"
#include "test_foregroundlock.moc"
