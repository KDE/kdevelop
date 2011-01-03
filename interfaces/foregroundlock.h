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

#ifndef KDEVELOP_FOREGROUNDLOCK_H
#define KDEVELOP_FOREGROUNDLOCK_H

#include "interfacesexport.h"
#include <QObject>
#include <QMutex>
#include <QWaitCondition>

class QApplication;

namespace KDevelop {

/**
 * A locking object that locks the resources that are associated to the main thread. When this lock is held,
 * you can call any thread-unsafe functions.
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
 *                 @note In order to circumvent this problem, better use the DO_IN_FOREGROUND macro!
 *
 * 
 * Implementation:
 * In order to make this object work properly in your application,
 * you have to redefine the QCoreApplication::notify(..) function so that it locks the foreground mutex:
 *
 * <example>
 * static QThread* mainThread = QThread::currentThread();
 *
 * bool KDevelopApplication::notify(QObject* receiver, QEvent* event)
 * {
 *    KDevelop::ForegroundLock lock(QThread::currentThread() == mainThread);
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

/**
 * Use this object if you want to temporarily release the foreground lock,
 * for example when entering a local event-loop from within the foreground thread.
 */
class KDEVPLATFORMINTERFACES_EXPORT TemporarilyReleaseForegroundLock {
public:
    TemporarilyReleaseForegroundLock();
    ~TemporarilyReleaseForegroundLock();
private:
    TemporarilyReleaseForegroundLock(const TemporarilyReleaseForegroundLock&);
    TemporarilyReleaseForegroundLock& operator=(const TemporarilyReleaseForegroundLock& rhs);
    int m_recursion;
};

#define VERIFY_FOREGROUND_LOCKED Q_ASSERT(KDevelop::ForegroundLock::isLockedForThread());

class KDEVPLATFORMINTERFACES_EXPORT DoInForeground : public QObject
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

// Dummy variables used to fill in unused parameters
extern const int __fg_dummy1, __fg_dummy2, __fg_dummy3, __fg_dummy4, __fg_dummy5, __fg_dummy6, __fg_dummy7, __fg_dummy8, __fg_dummy9;

/**
 * In the following a macro is defined that allows you transparently to synchronously execute code in the foreground thread, without breaking the code flow.
 * Note that for this macro, the same deadlock dangers exist as for the foreground lock. Make sure that you never hold any mutex when using the macro!
 * 
 * The macros allows you to execute a piece of code in the foreground, and to specify a set of variable references that will be available in the code.
 * 
 * Example:
 * 
 * void myFunctionInBackground() {
 *     int someVariable = 3;
 *     int anotherVariable = 5;
 *     
 *     DO_IN_FOREGROUND(someVariable, anotherVariable,
 *        someVariable += someGlobalNotThreadsafeFunction(anotherVariable);
 *     )
 * }
 * 
 * In the code which is given through the laster parameter of DO_IN_FOREGROUND, you can reference the
 * given variables using their real names. Other nonstatic variables of the current context are not available though.
 * You can give up to 9 variables. If you need more, create a container object and reach that one on.
 * You can only reference variables directly by their name. If you want to reference scoped variables like "object.variable",
 * you should create a local reference and hand that one to the macro.
 * */

#define DO_IN_FOREGROUND(__v1, __v2, __v3, __v4, __v5, __v6, __v7, __v8, __v9, __Code)   \
template<class __V1, __V2, __V3, __V4, __v5, __V6, __V7, __V8, __V9>   \
class DoInForeground_ ## __LINE__ : public KDevelop::DoInForeground   \
{ \
    __V1& __v1; \
    __V2& __v2; \
    __V3& __v3; \
    __V4& __v4; \
    __V5& __v5; \
    __V6& __v6; \
    __V7& __v7; \
    __V8& __v8; \
    __V9& __v9; \
    DoInForeground_ ## __LINE__(__V1& v1, __V2& v2, __V3& v3, __V4& v4, __V5& v5, __V6& v6, __V7& v7, __V8& v8, __V9& v9) : __v1(v1), __v2(v2), __v3(v3), __v4(v4), __v5(v5), __v6(v6), __v7(v7), __v8(v8), __v9(v9) {    \
    }    \
    virtual void doInternal() { \
      __Code    \
    } \
};   \
\
struct DoInForeground_ ## __LINE__ ## _creator \
{ \
   template<class __V1, __V2, __V3, __V4, __V5, __V6, __V7, __V8, __V9> \
   static doIt(__V1& v1, __V2& v2, __V3& v3, __V4& v4, __V5& v5, __V6& v6, __V7& v7, __V8& v8, __V9& v9) { DoInForeground_ ## __LINE__<__V1, __V2, __V3, __V4, __V5, __V6, __V7, __V8, __V9> doer(v1, v2, v3, v4, v5, v6, v7, v8, v9); doer.doIt(); } \
} \
DoInForeground_ ## __LINE__ ## _creator::doIt(__v1, __v2, __v3, __v4, __v5, __v6, __v7, __v8, __v9);

#define DO_IN_FOREGROUND_8(__v1, __v2, __v3, __v4, __v5, __v6, __v7, __v8, __Code)   \
DO_IN_FOREGROUND(__v1, __v2, __v3, __v4, __v5, __v6, __v7, __v8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_7(__v1, __v2, __v3, __v4, __v5, __v6, __v7, __Code)   \
DO_IN_FOREGROUND(__v1, __v2, __v3, __v4, __v5, __v6, __v7, __fg_dummy8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_6(__v1, __v2, __v3, __v4, __v5, __v6, __Code)   \
DO_IN_FOREGROUND(__v1, __v2, __v3, __v4, __v5, __v6, __fg_dummy7, __fg_dummy8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_5(__v1, __v2, __v3, __v4, __v5, __Code)   \
DO_IN_FOREGROUND(__v1, __v2, __v3, __v4, __v5, __fg_dummy6, __fg_dummy7, __fg_dummy8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_4(__v1, __v2, __v3, __v4, __Code)   \
DO_IN_FOREGROUND(__v1, __v2, __v3, __v4, __fg_dummy5, __fg_dummy6, __fg_dummy7, __fg_dummy8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_3(__v1, __v2, __v3, __Code)   \
DO_IN_FOREGROUND(__v1, __v2, __v3, __fg_dummy4, __fg_dummy5, __fg_dummy6, __fg_dummy7, __fg_dummy8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_2(__v1, __v2, __Code)   \
DO_IN_FOREGROUND(__v1, __v2, __fg_dummy3, __fg_dummy4, __fg_dummy5, __fg_dummy6, __fg_dummy7, __fg_dummy8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_1(__v1, __Code)   \
DO_IN_FOREGROUND(__v1, __fg_dummy2, __fg_dummy3, __fg_dummy4, __fg_dummy5, __fg_dummy6, __fg_dummy7, __fg_dummy8, __fg_dummy9, __Code)

#define DO_IN_FOREGROUND_0(__Code)   \
DO_IN_FOREGROUND(__fg_dummy1, __fg_dummy2, __fg_dummy3, __fg_dummy4, __fg_dummy5, __fg_dummy6, __fg_dummy7, __fg_dummy8, __fg_dummy9, __Code)

}

#endif
