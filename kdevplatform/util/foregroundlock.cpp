/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "foregroundlock.h"

#include <QAtomicPointer>
#include <QCoreApplication>
#include <QMutex>
#include <QSemaphore>
#include <QThread>

using namespace KDevelop;

namespace {

QRecursiveMutex foregroundMutex;
QSemaphore waitToLock;

const auto ForegroundEventType = static_cast<QEvent::Type>(QEvent::registerEventType());

// Lock-free stack of threads blocked on foregroundMutex
class ForegroundReleaser;
QAtomicPointer<ForegroundReleaser> foregroundWaiters;

bool handleForegroundLockWaiters();

class ForegroundReleaser : public IForegroundReleaser
{
public:
    QSemaphore waitToUnlock;
    QAtomicPointer<ForegroundReleaser> next;

    ForegroundReleaser()
    {
        Q_ASSERT(QCoreApplication::instance());
        Q_ASSERT(QCoreApplication::instance()->thread() != QThread::currentThread());
        connect(QThread::currentThread(), &QThread::finished, this, &ForegroundReleaser::deleteLater);
        moveToThread(QCoreApplication::instance()->thread());
    }

protected:
    void customEvent(QEvent* event) override
    {
        if (event->type() == ForegroundEventType) {
            VERIFY_FOREGROUND_LOCKED
            handleForegroundLockWaiters();
        }
        QObject::customEvent(event);
    }
};

bool handleForegroundLockWaiters()
{
    auto current = foregroundWaiters.fetchAndStoreOrdered(nullptr);
    if (!current) {
        return false;
    }
    // Now we release the foreground lock.
    // note: this does nothing if foregroundMutex is not held.
    TemporarilyReleaseForegroundLock release;
    // Release them all.
    int numWaiters = 0;
    while (current) {
        auto waiter = current;
        current = current->next.fetchAndStoreRelaxed(nullptr);
        waiter->waitToUnlock.release();
        ++numWaiters;
    }
    // Wait for all of the requester threads to acquire the foreground mutex.
    waitToLock.acquire(numWaiters);
    // Block on the foreground mutex until it is released.
    return true;
}

class ThreadState
{
public:
    int recursion;
    const bool isMainThread;
    ForegroundReleaser* releaser;

    ThreadState()
        : recursion(0)
        , isMainThread(QThread::isMainThread())
        , releaser(nullptr)
    {
    }

    ~ThreadState()
    {
        if (isMainThread) {
            Q_ASSERT_X(recursion == 1, Q_FUNC_INFO, "Wrong foreground mutex recursion count on static destruction!");
        } else {
            Q_ASSERT_X(recursion <= 0, Q_FUNC_INFO, "Secondary thread did not release the foreground mutex!");
        }
    }

    void acquireLock()
    {
        Q_ASSERT(recursion == 0);
        Q_ASSERT(!isMainThread);
        if (!releaser) {
            releaser = new ForegroundReleaser();
        }

        // avoid a __tls_get_addr call being emitted in the loop below.
        auto* const entry = releaser;

        bool first = false;
        Q_ASSERT(entry->next == nullptr);
        // note: because releaser is a unique value and there is only one per thread,
        // the value doubles as the "ABA token" for the lock-free push():
        auto head = reinterpret_cast<ForegroundReleaser*>(1);
        while (!foregroundWaiters.testAndSetOrdered(head, entry, head)) {
            first |= head == nullptr;
            entry->next.storeRelaxed(head);
        }
        if (first) {
            // We are the first thread, so notify the main thread to unlock the foreground mutex.
            QCoreApplication::postEvent(entry, new QEvent(ForegroundEventType), Qt::LowEventPriority);
        }
        // Wait for this thread's turn.
        entry->waitToUnlock.acquire();
        // Main thread released the foreground mutex.
        // All threads except the main thread can now compete for the lock:
        foregroundMutex.lock();
        ++recursion;
        // Let the main thread continue once it acquires the foregroundMutex after us.
        waitToLock.release();
    }

    static ThreadState* get()
    {
        // Note: this method incurs a library call to get the address of the object.
        thread_local ThreadState state;
        return &state;
    }
};

bool tryLockForegroundMutexInternal(ThreadState* local = ThreadState::get())
{
    if (foregroundMutex.tryLock()) {
        ++local->recursion;
        return true;
    } else {
        return false;
    }
}

void lockForegroundMutexInternal(ThreadState* local = ThreadState::get())
{
    if (local->isMainThread) {
        // The main thread must handle the blocked waiters, so we must spin.
        while (!tryLockForegroundMutexInternal(local)) {
            if (!handleForegroundLockWaiters()) {
                QThread::yieldCurrentThread();
            }
        }
        return;
    }
    foregroundMutex.lock();
    ++local->recursion;
}

void unlockForegroundMutexInternal(ThreadState* local = ThreadState::get(), bool duringDestruction = false)
{
    if (duringDestruction) {
        // This must be the main thread.
        Q_ASSERT(local->isMainThread);
        // The main thread shall never install a ForegroundReleaser.
        Q_ASSERT(local->releaser == nullptr);
    }

    Q_ASSERT(local->recursion > 0);
    --local->recursion;
    foregroundMutex.unlock();
}

void relockForegroundMutexInternal(ThreadState* local = ThreadState::get())
{
    if (local->isMainThread || local->recursion > 0) {
        // Already holding the foreground mutex or we are the main thread.
        lockForegroundMutexInternal(local);
        return;
    }

    if (tryLockForegroundMutexInternal(local)) {
        return;
    }

    // Slow path.
    // (This is implemented by ThreadState to make it visible in stack-traces.)
    local->acquireLock();
}
}

ForegroundLock::ForegroundLock(bool lock)
{
    if (lock)
        relock();
}

void KDevelop::ForegroundLock::relock()
{
    Q_ASSERT(!m_locked);
    relockForegroundMutexInternal();
    m_locked = true;
}

bool KDevelop::ForegroundLock::isLockedForThread()
{
    return ThreadState::get()->recursion > 0;
}

bool KDevelop::ForegroundLock::tryLock()
{
    Q_ASSERT(!m_locked);
    if (tryLockForegroundMutexInternal()) {
        m_locked = true;
        return true;
    }
    return false;
}

void KDevelop::ForegroundLock::unlock()
{
    Q_ASSERT(m_locked);
    unlockForegroundMutexInternal();
    m_locked = false;
}

TemporarilyReleaseForegroundLock::TemporarilyReleaseForegroundLock()
{
    auto local = ThreadState::get();
    Q_ASSERT(local->recursion >= 0);
    m_recursion = local->recursion;

    while (local->recursion) {
        unlockForegroundMutexInternal(local);
    }
}

TemporarilyReleaseForegroundLock::~TemporarilyReleaseForegroundLock()
{
    auto local = ThreadState::get();
    for (int a = 0; a < m_recursion; ++a) {
        relockForegroundMutexInternal(local);
    }

    Q_ASSERT(local->recursion == m_recursion);
}

KDevelop::ForegroundLock::~ForegroundLock()
{
    if (m_locked)
        unlock();
}

bool KDevelop::ForegroundLock::isLocked() const
{
    return m_locked;
}

IForegroundReleaser::~IForegroundReleaser()
{
}

// Important: The foreground lock has to be held by default, so lock it during static initialization
static struct StaticLock
{
    StaticLock()
    {
        lockForegroundMutexInternal();
    }
    ~StaticLock()
    {
        unlockForegroundMutexInternal(ThreadState::get(), true);
    }
private:
    Q_DISABLE_COPY(StaticLock)
} staticLock;

#include "moc_foregroundlock.cpp"
