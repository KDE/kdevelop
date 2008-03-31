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


#include <unistd.h>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QHash>
#include <QTime>

// Uncomment the following to turn on verbose locking information
//#define DUCHAIN_LOCK_VERBOSE_OUTPUT

// Uncomment this to enable checking for long lock times. It adds significant performance cost though.
//#define DEBUG_LOG_TIMING

// When uncommented, a backtrace will be printed whenever a too long lock-time is discovered
//#define DEBUG_LOG_BACKTRACE

//If DEBUG_LOG_TIMING is uncommented, and the duchain is locked for more then this count of milliseconds, a message is printed
#define LOCK_LOG_MILLISECONDS 1000

//Uncomment this to search for Deadlocks. DEBUG_LOG_TIMING must be enabled too, and DEBUG_LOG_BACKTRACE is recommended
//Warning: this may result in a total spamming of the command-line.
//#define SEARCH_DEADLOCKS

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

#ifdef DEBUG_LOG_TIMING
  inline void auditTime() const {
    int ms = m_lockTime.elapsed();
    if (ms > LOCK_LOG_MILLISECONDS) {
#ifndef DEBUG_LOG_BACKTRACE
      kWarning(9512) << "Long lock time:" << ms << "milliseconds." ;
#else
      kWarning(9512) << "Long lock time:" << ms << "milliseconds, locked from:\n" << m_lockBacktrace ;
#endif
    }
  }
  inline void startLockTiming() {
    m_lockTime.start();
#ifdef DEBUG_LOG_BACKTRACE
    m_lockBacktrace = kBacktrace();
#endif
  }
#endif

  QMutex m_mutex;
  Qt::HANDLE m_writer;

  int m_writerRecursion; ///How often is the chain write-locked by the writer?
  int m_totalReaderRecursion; ///How often is the chain read-locked recursively by all readers? Should be sum of all m_reader values

  ///Map readers to the count of recursive read-locks they hold(0 if they do not hold a lock)
  typedef QHash<Qt::HANDLE,int> ReaderMap;
  ReaderMap m_readers;

#ifdef DEBUG_LOG_TIMING
  QTime m_lockTime;
#ifdef DEBUG_LOG_BACKTRACE
  QString m_lockBacktrace;
#endif
#endif
};

class DUChainWriteLockerPrivate
{
public:
  DUChainWriteLockerPrivate() : m_locked(false) {
  }
  DUChainLock* m_lock;
  bool m_locked;
  int m_timeout;
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
  kDebug(9505) << "DUChain read lock requested by thread:" << QThread::currentThreadId();
#endif

  if(timeout == 0)
    timeout = 10000;
  
  QMutexLocker lock(&d->m_mutex);

  unsigned int currentTime = 0;
  bool locked = false;

  unsigned int sleepTime = 10000;
  unsigned int cycles = (timeout*1000)/sleepTime;
  // 10 second timeout...
  while ( (d->m_writer && d->m_writer != QThread::currentThreadId()) && currentTime <= cycles) {
    lock.unlock();
    usleep(sleepTime);
    currentTime++;
    lock.relock();
#ifdef DEBUG_LOG_BACKTRACE
    d->auditTime(); //Search for deadlocks
#endif
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
#ifdef DEBUG_LOCK_TIMING
    d->startLockTiming();
#endif
  }

  lock.unlock();
  
  return locked;
}

bool DUChainLock::lockForRead() {
  bool ret = lockForRead(100000);
  Q_ASSERT(currentThreadHasReadLock());
  return ret;
}

void DUChainLock::releaseReadLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9505) << "DUChain read lock released by thread:" << QThread::currentThreadId();
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

#ifdef DEBUG_LOCK_TIMING
  d->auditTime();
#endif
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

bool DUChainLock::lockForWrite(uint timeout)
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9505) << "DUChain write lock requested by thread:" << QThread::currentThreadId();
#endif

  if(timeout == 0)
    timeout = 10000;
  
  QMutexLocker lock(&d->m_mutex);
  //It is not allowed to acquire a write-lock while holding read-lock 
  Q_ASSERT(d->ownReaderRecursion() == 0);

  unsigned int currentTime = 0;
  bool locked = false;

  ///@todo use some wake-up queue instead of sleeping
  // 10 second timeout...
  
  while ( ( (d->m_writer && d->m_writer != QThread::currentThreadId()) || d->haveOtherReaders()) && currentTime < timeout) {
    lock.unlock();
    usleep(10000);
    currentTime++;
    lock.relock();
#ifdef DEBUG_LOG_BACKTRACE
    d->auditTime(); //Search for deadlocks
#endif
  }

  if ( (d->m_writer == 0 || d->m_writer == QThread::currentThreadId()) && !d->haveOtherReaders()) {
    d->m_writer = QThread::currentThreadId();
    ++d->m_writerRecursion;
    locked = true;
#ifdef DEBUG_LOCK_TIMING
    d->startLockTiming();
#endif
  }

  return locked;
}

void DUChainLock::releaseWriteLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9505) << "DUChain write lock released by thread:" << QThread::currentThreadId();
#endif

  Q_ASSERT(currentThreadHasWriteLock());
  QMutexLocker lock(&d->m_mutex);
  
  --d->m_writerRecursion;
  
  if( !d->m_writerRecursion )
    d->m_writer = 0;

#ifdef DEBUG_LOCK_TIMING
  d->auditTime();
#endif
}

bool DUChainLock::currentThreadHasWriteLock()
{
  QMutexLocker lock(&d->m_mutex);
  return d->m_writer == QThread::currentThreadId();
}


DUChainReadLocker::DUChainReadLocker(DUChainLock* duChainLock, uint timeout) : m_locked(false), m_timeout(timeout)
{
  m_lock = duChainLock;
  m_timeout = timeout;
  lock();
}

DUChainReadLocker::~DUChainReadLocker()
{
  unlock();
}

bool DUChainReadLocker::locked() const {
  return m_locked;
}

bool DUChainReadLocker::lock()
{
  if( m_locked )
    return true;
  
  bool l = false;
  if (m_lock) {
    l = m_lock->lockForRead(m_timeout);
    Q_ASSERT(m_timeout || l);
  };

  m_locked = l;
  
  return l;
}

void DUChainReadLocker::unlock()
{
  if (m_locked && m_lock) {
    m_lock->releaseReadLock();
    m_locked = false;
  }
}


DUChainWriteLocker::DUChainWriteLocker(DUChainLock* duChainLock, uint timeout)
  : d(new DUChainWriteLockerPrivate)
{
  d->m_timeout = timeout;
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
    l = d->m_lock->lockForWrite(d->m_timeout);
    Q_ASSERT(d->m_timeout || l);
  };

  d->m_locked = l;
  
  return l;
}

bool DUChainWriteLocker::locked() const {
  return d->m_locked;
}

void DUChainWriteLocker::unlock()
{
  if (d->m_locked && d->m_lock) {
    d->m_lock->releaseWriteLock();
    d->m_locked = false;
  }
}
}


// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
