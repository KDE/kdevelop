/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FOREGROUNDLOCK_H
#define KDEVPLATFORM_FOREGROUNDLOCK_H

#include "utilexport.h"
#include <QObject>

namespace KDevelop {

/**
 * A locking object that locks the resources that are associated to the main thread. When this lock is held,
 * you can call any thread-unsafe functions, because the foreground thread is locked in an event.
 *
 * The lock always becomes available when the foreground thread stops processing events.
 *
 * @warning There is one simple rule you must always follow to prevent deadlocks:
 *                  @em Never lock anything before locking the foreground mutex!!
 *                   That also means that you must not take this lock in contexts where
 *                   you don't know what other mutexes might be locked.
 *
 * @warning Objects that have QObject as base always get the thread they were created in assigned (see thread affinity, QObject::moveToThread),
 *                  which seriously affects the objects functionality regarding signals/slots.
 *                 The foreground lock does not change the thread affinity, so holding the foreground lock does not fully equal being in the foreground.
 *                 It may generally be unsafe to call foreground functions that create QObjects from within the background.
 */
class KDEVPLATFORMUTIL_EXPORT ForegroundLock
{
public:
    explicit ForegroundLock(bool lock = true);
    ~ForegroundLock();
    ForegroundLock(const ForegroundLock& rhs) = delete;
    ForegroundLock& operator=(const ForegroundLock& rhs) = delete;

    void unlock();
    void relock();
    /**
     * Try acquire the foreground lock.
     * @warning This can never succeed if none of the non-main threads are not frequently
     *          acquiring the lock the without tryLock(). Please do make loops that call tryLock() repeatedly.
     */
    [[nodiscard]] bool tryLock();

    /// Returns whether the current thread holds the foreground lock
    [[nodiscard]] static bool isLockedForThread();

    [[nodiscard]] bool isLocked() const;

private:
    bool m_locked = false;
};

/**
 * Use this object if you want to temporarily release the foreground lock,
 * for example when sleeping in the foreground thread, or when waiting in the foreground
 * thread for a background thread which should get the chance to lock the foreground.
 *
 * While this object is alive, you _must not_ access any non-threadsafe resources
 * that belong to the foreground, and you must not start an event-loop.
 * All (recursively) acquired ForegroundLock(s) by the thread will temporarily be released.
 */
class KDEVPLATFORMUTIL_EXPORT TemporarilyReleaseForegroundLock
{
public:
    TemporarilyReleaseForegroundLock();
    ~TemporarilyReleaseForegroundLock();

private:
    TemporarilyReleaseForegroundLock(const TemporarilyReleaseForegroundLock&);
    TemporarilyReleaseForegroundLock& operator=(const TemporarilyReleaseForegroundLock& rhs);
    int m_recursion;
};

#define VERIFY_FOREGROUND_LOCKED Q_ASSERT(KDevelop::ForegroundLock::isLockedForThread());

/**
 * Implementation detail, do not use.
 */
class KDEVPLATFORMUTIL_EXPORT IForegroundReleaser : public QObject
{
    Q_OBJECT
public:
    IForegroundReleaser() = default;
    ~IForegroundReleaser() override;
};

}

#endif
