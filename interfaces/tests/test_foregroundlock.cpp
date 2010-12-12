/* This file is part of KDevelop
    Copyright 2010 Milian Wolff <mail@milianw.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "test_foregroundlock.h"

#include <QtTest/QtTest>

#include <QDebug>
#include <QThread>

#include <unistd.h>

#include "../foregroundlock.h"

QTEST_MAIN(KDevelop::TestForegroundLock)

using namespace KDevelop;

//BEGIN Helper Threads

class TryLockThread : public QThread {
    Q_OBJECT
public:
    virtual void run() {
        ForegroundLock lock(false);
        for(int i = 0; i < 1000; ++i) {
            if (lock.tryLock()) {
                lock.unlock();
            }
            usleep(qrand() % 20);
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
        usleep(10);
        lock.unlock();
    }
}

#include "moc_test_foregroundlock.cpp"
#include "test_foregroundlock.moc"
