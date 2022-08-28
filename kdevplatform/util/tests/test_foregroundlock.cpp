/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_foregroundlock.h"

#include <QTest>
#include <QStandardPaths>
#include <QThread>
#include <QRandomGenerator>

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
    void run() override
    {
        ForegroundLock lock(false);
        auto* randomGenerator = QRandomGenerator::global();
        for (int i = 0; i < 1000; ++i) {
            if (lock.tryLock()) {
                lock.unlock();
            }
            QThread::usleep(randomGenerator->bounded(20));
        }
    }
};

void TestForegroundLock::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
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
}

#include "moc_test_foregroundlock.cpp"
#include "test_foregroundlock.moc"
