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

#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <QAtomicInt>
#include <sys/time.h>
#include <unistd.h>

namespace KDevelop {

inline uint timevalToMilliSeconds(timeval v) {
  return v.tv_sec * 1000 + v.tv_usec / 1000;
}

///@param uSleep nanoseconds to sleep when waiting for the lock
template<uint uSleep, int Timeout = 0>
class SpinLock
{
    public:
        
        SpinLock(QAtomicInt& var) : m_var(var), m_locked(true) {
            if(var.testAndSetOrdered(0, 1)) {
                //Success at the first try
            }else if(Timeout) {
                //Spin with timeout
                
                //Start spinning
                timeval startTime;
                gettimeofday(&startTime, 0);
                
                while(!var.testAndSetOrdered(0, 1))
                {
                    timeval currentTime;
                    gettimeofday(&currentTime, 0);
                    timeval waited;
                    timersub(&currentTime, &startTime, &waited);
                    
                    if(Timeout && timevalToMilliSeconds(waited) > Timeout) {
                        m_locked = false;
                        break;
                    }
                    
                    if(uSleep)
                        usleep(uSleep);
                }
            }else{
                //No timeout
                while(!var.testAndSetOrdered(0, 1))
                {
                    if(uSleep)
                        usleep(uSleep);
                }
            }
        }
        
        ~SpinLock() {
            if(m_locked) {
                m_var = 0;
            }
        }
        
        //Returns whether the lock succeeded
        bool locked() const {
            return m_locked;
        }
        
    private:
        QAtomicInt& m_var;
        bool m_locked;
};

}

#endif // SPINLOCK_H
