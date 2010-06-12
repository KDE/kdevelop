/*
    Copyright 2010 David Nolden

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

#ifndef KDEVELOP_FOREGROUNDLOCK_H
#define KDEVELOP_FOREGROUNDLOCK_H

#include "interfacesexport.h"

class QApplication;

namespace KDevelop {

/**
 * A locking object that locks the resources that are associated to the main thread. When this lock is held,
 * you can call any thread-unsafe functions.
 *
 * The lock always becomes available when the foreground thread stops processing events.
 *
 * @warning There is one simple rule you must always follow to prevent deadlocks:
 *      @em Never lock anything before locking the foreground mutex!!
 *
 * That also means that you must not take this lock in contexts where you don't know what other mutexes might be locked. 
 *
 * In order to make this work properly, you have to redefine the QCoreApplication::notify(..) function so that it locks the foreground mutex:
 *
 * <example>
 * static Qt::HANDLE mainThread = QThread::currentThreadId();
 *
 * bool KDevelopApplication::notify(QObject* receiver, QEvent* event)
 * {
 *    KDevelop::ForegroundLock lock(QThread::currentThreadId() == mainThread);
 *    return KApplication::notify(receiver, event);
 * }
 * </example>
 *
 * Also make sure to lock the foreground lock during initialization before the event loop starts.
 */
class KDEVPLATFORMINTERFACES_EXPORT ForegroundLock
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

#define VERIFY_FOREGROUND_LOCKED Q_ASSERT(KDevelop::ForegroundLock::isLockedForThread());

}

#endif // KDEVELOP_FOREGROUNDLOCK_H
