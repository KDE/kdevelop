/*
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_FOREGROUNDLOCK_H
#define KDEVPLATFORM_FOREGROUNDLOCK_H

#include "utilexport.h"
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class QApplication;

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
        ForegroundLock(bool lock = true);
        ~ForegroundLock();
        void unlock();
        void relock();
        bool tryLock();

        /// Returns whether the current thread holds the foreground lock
        static bool isLockedForThread();

        bool isLocked() const;

    private:
        ForegroundLock(const ForegroundLock& rhs);
        ForegroundLock& operator=(const ForegroundLock& rhs);
        bool m_locked;
};

/**
 * Use this object if you want to temporarily release the foreground lock,
 * for example when sleeping in the foreground thread, or when waiting in the foreground
 * thread for a background thread which should get the chance to lock the foreground.
 *
 * While this object is alive, you _must not_ access any non-threadsafe resources
 * that belong to the foreground, and you must not start an event-loop.
 */
class KDEVPLATFORMUTIL_EXPORT TemporarilyReleaseForegroundLock {
public:
    TemporarilyReleaseForegroundLock();
    ~TemporarilyReleaseForegroundLock();
private:
    TemporarilyReleaseForegroundLock(const TemporarilyReleaseForegroundLock&);
    TemporarilyReleaseForegroundLock& operator=(const TemporarilyReleaseForegroundLock& rhs);
    int m_recursion;
};

#define VERIFY_FOREGROUND_LOCKED Q_ASSERT(KDevelop::ForegroundLock::isLockedForThread());

class KDEVPLATFORMUTIL_EXPORT DoInForeground : public QObject
{
Q_OBJECT
public:
    DoInForeground() ;
    virtual ~DoInForeground() ;

    void doIt() ;

private Q_SLOTS:
    void doInternalSlot();

private:
    virtual void doInternal() = 0;
    QMutex m_mutex;
    QWaitCondition m_wait;
};

}

#endif
