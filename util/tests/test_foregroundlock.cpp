/*
    Copyright 2010 Milian Wolff <mail@milianw.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "test_foregroundlock.h"

#include <QtTest/QtTest>

#include <QThread>

#include "../foregroundlock.h"

QTEST_MAIN(KDevelop::TestForegroundLock)

using namespace KDevelop;

//BEGIN Helper Threads

class TryLockThread : public QThread {
    Q_OBJECT
public:
    virtual void run() override {
        ForegroundLock lock(false);
        for(int i = 0; i < 1000; ++i) {
            if (lock.tryLock()) {
                lock.unlock();
            }
            QThread::usleep(qrand() % 20);
        }
    }
};

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
    QList<TryLockThread*> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads << new TryLockThread;
    }

    ForegroundLock lock(true);

    foreach(TryLockThread* thread, threads) {
        thread->start();
    }

    lock.unlock();

    while(true) {
        bool running = false;
        foreach(TryLockThread* thread, threads) {
            if (thread->isRunning()) {
                running = true;
                break;
            }
        }
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
