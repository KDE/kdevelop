/* This file is part of KDevelop
    Copyright 2007 Kris Wong <kris.p.wong@gmail.com>
    Copyright 2007 Hamish Rodda <rodda@kde.org>

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
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QHash>
#include <QTime>

// Uncomment the following to turn on verbose locking information
//#define DUCHAIN_LOCK_VERBOSE_OUTPUT

#include <kdebug.h>

namespace KDevelop
{
class DUChainLockPrivate
{
public:
  DUChainLockPrivate() {
    m_writer = 0;
    m_writerRecursion = 0;
    m_totalReaderRecursion = 0;
    
  }

  /**
   * Returns true if ther is no reader that is not this thread.
   * */
  bool haveOtherReaders() const {
    ///Since m_totalReaderRecursion is the sum of all reader-recursions, it will be same if either there is no reader at all, or if this thread is the only reader.
    return m_totalReaderRecursion != ownReaderRecursion();
  }

  int ownReaderRecursion() const {
    int ownReaderRecursion = 0;

    DUChainLockPrivate::ReaderMap::const_iterator it = m_readers.find( QThread::currentThreadId() );
    if( it != m_readers.end() )
      ownReaderRecursion = *it;
    return ownReaderRecursion;
  }
  
  void auditTime() const {
    int ms = m_lockTime.elapsed();
    if (ms > 100)
      kWarning(9007) << "Long lock time:" << ms << "miliseconds.";
  }

  QMutex m_mutex;
  Qt::HANDLE m_writer;

  int m_writerRecursion; ///How often is the chain write-locked by the writer?
  int m_totalReaderRecursion; ///How often is the chain read-locked recursively by all readers? Should be sum of all m_reader values

  ///Map readers to the count of recursive read-locks they hold(0 if they do not hold a lock)
  typedef QHash<Qt::HANDLE,int> ReaderMap;
  ReaderMap m_readers;
  
  QTime m_lockTime;
};

class DUChainReadLockerPrivate
{
public:
  DUChainReadLockerPrivate() : m_locked(false) {
  }
  DUChainLock* m_lock;
  bool m_locked;
};

class DUChainWriteLockerPrivate
{
public:
  DUChainWriteLockerPrivate() : m_locked(false) {
  }
  DUChainLock* m_lock;
  bool m_locked;
};


DUChainLock::DUChainLock()
  : d(new DUChainLockPrivate)
{
}

DUChainLock::~DUChainLock()
{
  delete d;
}

bool DUChainLock::lockForRead(unsigned int timeout)
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain read lock requested by thread:" << QThread::currentThreadId();
#endif

  QMutexLocker lock(&d->m_mutex);

  int currentTime = 0;
  bool locked = false;

  unsigned int sleepTime = 10000;
  unsigned int cycles = (timeout*1000)/sleepTime;
  // 10 second timeout...
  while ( (d->m_writer && d->m_writer != QThread::currentThreadId()) && currentTime <= cycles) {
    lock.unlock();
    usleep(sleepTime);
    currentTime++;
    lock.relock();
  }

  if (d->m_writer == 0 || d->m_writer == QThread::currentThreadId()) {
    DUChainLockPrivate::ReaderMap::iterator it = d->m_readers.find( QThread::currentThreadId() );
    if( it != d->m_readers.end() )
      ++(*it);
    else
      d->m_readers.insert(QThread::currentThreadId(), 1);
    locked = true;
  }

  if(locked) {
    ++d->m_totalReaderRecursion;
    d->m_lockTime.start();
  }

  lock.unlock();
  
  return locked;
}

bool DUChainLock::lockForRead() {
  bool ret = lockForRead(10000);
  Q_ASSERT(currentThreadHasReadLock());
  return ret;
}

void DUChainLock::releaseReadLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain read lock released by thread:" << QThread::currentThreadId();
#endif

  QMutexLocker lock(&d->m_mutex);
  DUChainLockPrivate::ReaderMap::iterator it = d->m_readers.find( QThread::currentThreadId() );
  Q_ASSERT(it != d->m_readers.end());
  --(*it);
  Q_ASSERT((*it)>=0);
  --d->m_totalReaderRecursion;

  ///@todo Remove the all readers that do not exist any more at some point(leave other readers there with recursion 0
  ///      because it is very probable that they will lock again, and not having to re-allocate the bucket might speed up the locking.

/*  if( *it == 0 )
    d->m_readers.erase(it); //Maybe it would even be wise simply leaving it there*/

  d->auditTime();
}

bool DUChainLock::currentThreadHasReadLock()
{
  QMutexLocker lock(&d->m_mutex);
  DUChainLockPrivate::ReaderMap::iterator it = d->m_readers.find( QThread::currentThreadId() );
  if( it != d->m_readers.end() )
    return (*it) != 0;
  else
    return false;
}

bool DUChainLock::lockForWrite()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain write lock requested by thread:" << QThread::currentThreadId();
#endif

  //It is not allowed to acquire a write-lock while holding read-lock 
  Q_ASSERT(d->ownReaderRecursion() == 0);

  QMutexLocker lock(&d->m_mutex);

  int currentTime = 0;
  bool locked = false;

  ///@todo use some wake-up queue instead of sleeping
  // 10 second timeout...
  
  while ( ( (d->m_writer && d->m_writer != QThread::currentThreadId()) || d->haveOtherReaders()) && currentTime < 10000) {
    lock.unlock();
    usleep(10000);
    currentTime++;
    lock.relock();
  }

  if ( (d->m_writer == 0 || d->m_writer == QThread::currentThreadId()) && !d->haveOtherReaders()) {
    d->m_writer = QThread::currentThreadId();
    ++d->m_writerRecursion;
    locked = true;
    d->m_lockTime.start();
  }

  return locked;
}

void DUChainLock::releaseWriteLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain write lock released by thread:" << QThread::currentThreadId();
#endif

  Q_ASSERT(currentThreadHasWriteLock());
  
  QMutexLocker lock(&d->m_mutex);
  
  --d->m_writerRecursion;
  
  if( !d->m_writerRecursion )
    d->m_writer = 0;

  d->auditTime();
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

bool DUChainReadLocker::lock()
{
  if( d->m_locked )
    return true;
  
  bool l = false;
  if (d->m_lock) {
    l = d->m_lock->lockForRead();
    Q_ASSERT(l);
  };

  d->m_locked = l;
  
  return l;
}

void DUChainReadLocker::unlock()
{
  if (d->m_locked && d->m_lock) {
    d->m_lock->releaseReadLock();
    d->m_locked = false;
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

bool DUChainWriteLocker::lock()
{
  if( d->m_locked )
    return true;
  
  bool l = false;
  if (d->m_lock) {
    l = d->m_lock->lockForWrite();
    Q_ASSERT(l);
  };

  d->m_locked = l;
  
  return l;
}

void DUChainWriteLocker::unlock()
{
  if (d->m_locked && d->m_lock) {
    d->m_lock->releaseWriteLock();
    d->m_locked = false;
  }
}
}

#endif // NDEBUG

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
