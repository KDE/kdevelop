/*
    SPDX-FileCopyrightText: 2007 Kris Wong <kris.p.wong@gmail.com>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchainlock.h"
#include "duchain.h"

#include <QThread>
#include <QThreadStorage>
#include <QElapsedTimer>

///@todo Always prefer exactly that lock that is requested by the thread that has the foreground mutex,
///           to reduce the amount of UI blocking.

//Microseconds to sleep when waiting for a lock
const uint uSleepTime = 500;

namespace KDevelop {
class DUChainLockPrivate
{
public:
    DUChainLockPrivate()
        : m_writer(nullptr)
        , m_writerRecursion(0)
        , m_totalReaderRecursion(0)
    { }

    int ownReaderRecursion() const
    {
        return m_readerRecursion.localData();
    }

    void changeOwnReaderRecursion(int difference)
    {
        m_readerRecursion.localData() += difference;
        Q_ASSERT(m_readerRecursion.localData() >= 0);
        m_totalReaderRecursion.fetchAndAddOrdered(difference);
    }

    ///Holds the writer that currently has the write-lock, or zero. Is protected by m_writerRecursion.
    QAtomicPointer<QThread> m_writer;

    ///How often is the chain write-locked by the writer? This value protects m_writer,
    ///m_writer may only be changed by the thread that successfully increases this value from 0 to 1
    QAtomicInt m_writerRecursion;
    ///How often is the chain read-locked recursively by all readers? Should be sum of all m_readerRecursion values
    QAtomicInt m_totalReaderRecursion;

    QThreadStorage<int> m_readerRecursion;
};

DUChainLock::DUChainLock()
    : d_ptr(new DUChainLockPrivate)
{
}

DUChainLock::~DUChainLock() = default;

bool DUChainLock::lockForRead(unsigned int timeout)
{
    Q_D(DUChainLock);

    ///Step 1: Increase the own reader-recursion. This will make sure no further write-locks will succeed
    d->changeOwnReaderRecursion(1);

    QThread* w = d->m_writer.loadAcquire();
    if (w == nullptr || w == QThread::currentThread()) {
        //Successful lock: Either there is no writer, or we hold the write-lock by ourselves
    } else {
        ///Step 2: Start spinning until there is no writer any more

        QElapsedTimer t;
        if (timeout) {
            t.start();
        }

        while (d->m_writer.loadAcquire()) {
            if (!timeout || t.elapsed() < timeout) {
                QThread::usleep(uSleepTime);
            } else {
                //Fail!
                d->changeOwnReaderRecursion(-1);
                return false;
            }
        }
    }

    return true;
}

void DUChainLock::releaseReadLock()
{
    Q_D(DUChainLock);

    d->changeOwnReaderRecursion(-1);
}

bool DUChainLock::currentThreadHasReadLock()
{
    Q_D(DUChainLock);

    return ( bool )d->ownReaderRecursion();
}

bool DUChainLock::lockForWrite(uint timeout)
{
    Q_D(DUChainLock);

    //It is not allowed to acquire a write-lock while holding read-lock

    Q_ASSERT(d->ownReaderRecursion() == 0);

    if (d->m_writer.loadRelaxed() == QThread::currentThread()) {
        //We already hold the write lock, just increase the recursion count and return
        d->m_writerRecursion.fetchAndAddRelaxed(1);
        return true;
    }

    QElapsedTimer t;
    if (timeout) {
        t.start();
    }

    while (1) {
        //Try acquiring the write-lcok
        if (d->m_totalReaderRecursion.loadRelaxed() == 0 && d->m_writerRecursion.testAndSetOrdered(0, 1)) {
            //Now we can be sure that there is no other writer, as we have increased m_writerRecursion from 0 to 1
            d->m_writer = QThread::currentThread();
            if (d->m_totalReaderRecursion.loadRelaxed() == 0) {
                //There is still no readers, we have successfully acquired a write-lock
                return true;
            } else {
                //There may be readers.. we have to continue spinning
                d->m_writer = nullptr;
                d->m_writerRecursion = 0;
            }
        }

        if (!timeout || t.elapsed() < timeout) {
            QThread::usleep(uSleepTime);
        } else {
            //Fail!
            return false;
        }
    }

    return false;
}

void DUChainLock::releaseWriteLock()
{
    Q_D(DUChainLock);

    Q_ASSERT(currentThreadHasWriteLock());

    //The order is important here, m_writerRecursion protects m_writer

    //TODO: could testAndSet here
    if (d->m_writerRecursion.loadRelaxed() == 1) {
        d->m_writer = nullptr;
        d->m_writerRecursion = 0;
    } else {
        d->m_writerRecursion.fetchAndAddOrdered(-1);
    }
}

bool DUChainLock::currentThreadHasWriteLock() const
{
    Q_D(const DUChainLock);

    return d->m_writer.loadRelaxed() == QThread::currentThread();
}

DUChainReadLocker::DUChainReadLocker(DUChainLock* duChainLock, uint timeout)
    : m_lock(duChainLock ? duChainLock : DUChain::lock())
    , m_locked(false)
    , m_timeout(timeout)
{
    lock();
}

DUChainReadLocker::~DUChainReadLocker()
{
    unlock();
}

bool DUChainReadLocker::locked() const
{
    return m_locked;
}

bool DUChainReadLocker::lock()
{
    if (m_locked) {
        return true;
    }

    bool l = false;
    if (m_lock) {
        l = m_lock->lockForRead(m_timeout);
        Q_ASSERT(m_timeout || l);
    }
    ;

    m_locked = l;

    return l;
}

void DUChainReadLocker::unlock()
{
    if (m_locked && m_lock) {
        m_lock->releaseReadLock();
        m_locked = false;
    }
}

DUChainWriteLocker::DUChainWriteLocker(DUChainLock* duChainLock, uint timeout)
    : m_lock(duChainLock ? duChainLock : DUChain::lock())
    , m_locked(false)
    , m_timeout(timeout)
{
    lock();
}

DUChainWriteLocker::~DUChainWriteLocker()
{
    unlock();
}

bool DUChainWriteLocker::lock()
{
    if (m_locked) {
        return true;
    }

    bool l = false;
    if (m_lock) {
        l = m_lock->lockForWrite(m_timeout);
        Q_ASSERT(m_timeout || l);
    }
    ;

    m_locked = l;

    return l;
}

bool DUChainWriteLocker::locked() const
{
    return m_locked;
}

void DUChainWriteLocker::unlock()
{
    if (m_locked && m_lock) {
        m_lock->releaseWriteLock();
        m_locked = false;
    }
}
}
