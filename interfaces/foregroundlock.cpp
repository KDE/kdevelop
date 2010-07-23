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

#include "foregroundlock.h"
#include <QMutex>
#include <QThread>
#include <QApplication>

using namespace KDevelop;

namespace {
QMutex mutex(QMutex::Recursive);

QMutex tryLockMutex;
QMutex waitMutex;
QMutex finishMutex;
QWaitCondition condition;

Qt::HANDLE holderThread = 0;
int recursion = 0;
}

ForegroundLock::ForegroundLock(bool lock) : m_locked(false)
{
    if(lock)
        relock();
}

void KDevelop::ForegroundLock::relock()
{
    Q_ASSERT(!m_locked);
    
    if(!QApplication::instance() || QThread::currentThread() == QApplication::instance()->thread())
    {
        mutex.lock();
    }else{
        QMutexLocker lock(&tryLockMutex);
        
        while(!mutex.tryLock(10))
        {
            // In case an additional event-loop was started from within the foreground, we send
            // events to the foreground to temporarily release the lock.
            
            class ForegroundReleaser : public DoInForeground {
                public:
                virtual void doInternal() {
                    // By locking the mutex, we make sure that the requester is actually waiting for the condition
                    waitMutex.lock();
                    // Now we release the foreground lock
                    TemporarilyReleaseForegroundLock release;
                    // And signalize to the requester that we've released it
                    condition.wakeAll();
                    // Allow the requester to actually wake up, by unlocking m_waitMutex
                    waitMutex.unlock();
                    // Now wait until the requester is ready
                    QMutexLocker lock(&finishMutex);
                }
            };
            
            static ForegroundReleaser releaser;
            
            QMutexLocker lockWait(&waitMutex);
            QMutexLocker lockFinish(&finishMutex);
            
            QMetaObject::invokeMethod(&releaser, "doInternalSlot", Qt::QueuedConnection);
            condition.wait(&waitMutex);
            
            if(mutex.tryLock())
            {
                //success
                break;
            }else{
                //Probably a third thread has creeped in and
                //got the foreground lock before us. Just try again.
            }
        }
    }
    m_locked = true;
    holderThread = QThread::currentThreadId();
    ++recursion;
}

bool KDevelop::ForegroundLock::isLockedForThread()
{
    return QThread::currentThreadId() == holderThread;
}

bool KDevelop::ForegroundLock::tryLock()
{
    if(mutex.tryLock())
    {
        ++recursion;
        m_locked = true;
        holderThread = QThread::currentThreadId();
        return true;
    }
    return false;
}

void KDevelop::ForegroundLock::unlock()
{
    Q_ASSERT(holderThread == QThread::currentThreadId());
    --recursion;
    if(recursion == 0)
        holderThread = 0;
    mutex.unlock();
    m_locked = false;
}

TemporarilyReleaseForegroundLock::TemporarilyReleaseForegroundLock()
{
    Q_ASSERT(holderThread == QThread::currentThreadId());
    m_recursion = recursion;
    
    // Release all recursive locks
    recursion = 0;
    holderThread = 0;
    for(int a = 0; a < m_recursion; ++a)
        mutex.unlock();
}

TemporarilyReleaseForegroundLock::~TemporarilyReleaseForegroundLock()
{
    for(int a = 0; a < m_recursion; ++a)
        mutex.lock();
    recursion = m_recursion;
    holderThread = QThread::currentThreadId();
}

KDevelop::ForegroundLock::~ForegroundLock()
{
    if(m_locked)
        unlock();
}

bool KDevelop::ForegroundLock::isLocked() const
{
    return m_locked;
}

namespace KDevelop {
    const int __fg_dummy1 = 0, __fg_dummy2 = 0, __fg_dummy3 = 0, __fg_dummy4 = 0, __fg_dummy5 = 0, __fg_dummy6 = 0, __fg_dummy7 = 0, __fg_dummy8 = 0, __fg_dummy9 = 0;

    void DoInForeground::doIt() {
        if(QThread::currentThread() == QApplication::instance()->thread())
        {
            // We're already in the foreground, just call the handler code
            doInternal();
        }else{
            QMutexLocker lock(&m_mutex);
            QMetaObject::invokeMethod(this, "doInternalSlot", Qt::QueuedConnection);
            m_wait.wait(&m_mutex);
        }
    }

    DoInForeground::~DoInForeground() {
    }

    DoInForeground::DoInForeground() {
        moveToThread(QApplication::instance()->thread());
    }

    void DoInForeground::doInternalSlot()
    {
        VERIFY_FOREGROUND_LOCKED
        doInternal();
        QMutexLocker lock(&m_mutex);
        m_wait.wakeAll();
    }
}
