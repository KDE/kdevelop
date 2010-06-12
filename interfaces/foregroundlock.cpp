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

#include "foregroundlock.h"
#include <QMutex>
#include <QThread>
#include <QApplication>

using namespace KDevelop;

namespace {
QMutex mutex(QMutex::Recursive);
Qt::HANDLE holderThread = 0;
}

ForegroundLock::ForegroundLock(bool lock) : m_locked(false)
{
    if(lock)
        relock();
}

void KDevelop::ForegroundLock::relock()
{
    Q_ASSERT(!m_locked);
    mutex.lock();
    m_locked = true;
    holderThread = QThread::currentThreadId();
}

bool KDevelop::ForegroundLock::isLockedForThread()
{
    return QThread::currentThreadId() == holderThread;
}

bool KDevelop::ForegroundLock::tryLock()
{
    if(mutex.tryLock())
    {
        m_locked = true;
        holderThread = QThread::currentThreadId();
        return true;
    }
    return false;
}

void KDevelop::ForegroundLock::unlock()
{
    Q_ASSERT(holderThread == QThread::currentThreadId());
    mutex.unlock();
    m_locked = false;
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

