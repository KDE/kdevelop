/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_SPINLOCK_H
#define KDEVPLATFORM_SPINLOCK_H

#include <QAtomicInt>
#include <QMutex>
#include <QWaitCondition>
#include <sys/time.h>
#include <unistd.h>

namespace KDevelop {

inline uint timevalToMilliSeconds(timeval v) {
  return v.tv_sec * 1000 + v.tv_usec / 1000;
}

struct SpinLockData {
    SpinLockData() : needWake(false) {
        lock = 0;
    }
    
    QAtomicInt lock; //The variable that manages the lock
    
    volatile bool needWake;
    QMutex waitingMutex;
    QWaitCondition waitingCondition;
};

///This spin-lock uses a QWaitCondition together with a QMutex as fallback in case the lock is already taken.
///That means that this spin-lock is only more efficient than a QMutex when there is a low amount of lock-contention.
///When there is a high amount of lock-contention, QMutex should be more efficient.
///@note This implementation does not guarantee an optimal behavior as QMutex does: There is no guarantee
///      that a waiting thread is really woken up in exactly the moment the lock becomes available.
///      In _most_ cases it should work, but in worst-case the thread sleeps @p mSleep milliseconds.
///@param mSleep maximum number of milliseconds to sleep when waiting for the lock
///@param Timeout the timeount value
template<uint mSleep = 10, int mTimeout = 0>
class SpinLock
{
    public:
        
        SpinLock(SpinLockData& data) : m_data(data), m_locked(true) {
            if(m_data.lock.testAndSetOrdered(0, 1)) {
                //Success at the first try
            }else if(mTimeout) {
                //Spin with timeout
                
                //Start spinning
                timeval startTime;
                gettimeofday(&startTime, 0);
                
                while(!m_data.lock.testAndSetOrdered(0, 1))
                {
                    timeval currentTime;
                    gettimeofday(&currentTime, 0);
                    timeval waited;
                    timersub(&currentTime, &startTime, &waited);
                    
                    if(mTimeout && timevalToMilliSeconds(waited) > mTimeout) {
                        m_locked = false;
                        break;
                    }
                    
                    //We need to wait now
                    m_data.waitingMutex.lock();
                    m_data.needWake = true;
                    m_data.waitingCondition.wait(&m_data.waitingMutex, mSleep);
                    m_data.waitingMutex.unlock();
                }
            }else{
                //No timeout
                while(!m_data.lock.testAndSetOrdered(0, 1))
                {
                    //We need to wait now
                    m_data.waitingMutex.lock();
                    m_data.needWake = true;
                    m_data.waitingCondition.wait(&m_data.waitingMutex, mSleep);
                    m_data.waitingMutex.unlock();
                }
            }
        }
        
        ~SpinLock() {
            if(m_locked) {
                bool needWake = m_data.needWake;
                
                m_data.needWake = false;
                m_data.lock = 0;
                
                if(needWake)
                    m_data.waitingCondition.wakeOne();
            }
        }
        
        //Returns whether the lock succeeded
        bool locked() const {
            return m_locked;
        }
        
    private:
        SpinLockData& m_data;
        bool m_locked;
};

}

#endif // KDEVPLATFORM_SPINLOCK_H
