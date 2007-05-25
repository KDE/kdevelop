/* This file is part of KDevelop
    Copyright (C) 2007 Kris Wong <kwong@fuse.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "duchainlock.h"

#ifndef NDEBUG

#include <unistd.h>
#include <QThread>

// Uncomment the following to turn on verbose locking information
//#define DUCHAIN_LOCK_VERBOSE_OUTPUT

#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
#include <kdebug.h>
#endif

class DUChainLockPrivate
{
public:
  QMutex m_mutex;
  Qt::HANDLE m_writer;
  QSet<Qt::HANDLE> m_readers;
};

class DUChainReadLockerPrivate
{
public:
  DUChainLock* m_lock;
};

class DUChainWriteLockerPrivate
{
public:
  DUChainLock* m_lock;
};

DUChainLock::DUChainLock()
  : d(new DUChainLockPrivate)
{
  d->m_writer = 0;
}

DUChainLock::~DUChainLock()
{
}

bool DUChainLock::lockForRead()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain read lock requested by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&d->m_mutex);

  int currentTime = 0;
  bool locked = false;

  // 10 second timeout...
  while (d->m_writer != 0 && currentTime < 1000) {
    lock.unlock();
    usleep(10000);
    currentTime++;
    lock.relock();
  }

  if (d->m_writer == 0) {
    d->m_readers << QThread::currentThreadId();
    locked = true;
  }

  return locked;
}

void DUChainLock::releaseReadLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain read lock released by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&d->m_mutex);
  d->m_readers.remove(QThread::currentThreadId());
}

bool DUChainLock::currentThreadHasReadLock()
{
  QMutexLocker lock(&d->m_mutex);
  return d->m_readers.contains(QThread::currentThreadId());
}

bool DUChainLock::lockForWrite()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain write lock requested by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&d->m_mutex);

  // Write locks are non-recursive.
  Q_ASSERT(d->m_writer != QThread::currentThreadId());

  int currentTime = 0;
  bool locked = false;

  // 10 second timeout...
  while (d->m_writer != 0 && !d->m_readers.empty() && currentTime < 1000) {
    lock.unlock();
    usleep(10000);
    currentTime++;
    lock.relock();
  }

  if (d->m_writer == 0 && d->m_readers.empty()) {
    d->m_writer = QThread::currentThreadId();
    locked = true;
  }

  return locked;
}

void DUChainLock::releaseWriteLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain write lock released by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&d->m_mutex);
  d->m_writer = 0;
}

bool DUChainLock::currentThreadHasWriteLock()
{
  QMutexLocker lock(&d->m_mutex);
  return d->m_writer == QThread::currentThreadId();
}

DUChainReadLocker::DUChainReadLocker(DUChainLock* duChainLock)
: d(new DUChainReadLockerPrivate)
{
  d->m_lock = duChainLock;
  lock();
}

DUChainReadLocker::~DUChainReadLocker()
{
  unlock();
  delete d;
}

bool DUChainReadLocker::lock() {
  bool l = false;
  if (d->m_lock) {
    l = d->m_lock->lockForRead();
    Q_ASSERT(l);
  };
  return l;
}

void DUChainReadLocker::unlock() {
  if (d->m_lock) {
    d->m_lock->releaseReadLock();
  }
}

DUChainWriteLocker::DUChainWriteLocker(DUChainLock* duChainLock)
  : d(new DUChainWriteLockerPrivate)
{
  d->m_lock = duChainLock;
  lock();
}
DUChainWriteLocker::~DUChainWriteLocker()
{
  unlock();
  delete d;
}

bool DUChainWriteLocker::lock() {
  bool l = false;
  if (d->m_lock) {
    l = d->m_lock->lockForWrite();
    Q_ASSERT(l);
  };
  return l;
}

void DUChainWriteLocker::unlock() {
  if (d->m_lock) {
    d->m_lock->releaseWriteLock();
  }
}

#endif // NDEBUG

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
