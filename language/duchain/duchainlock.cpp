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
#include <QtCore/QStack>
#include <QTime>
#include <ext/hash_map>

// Uncomment the following to turn on verbose locking information
//#define DUCHAIN_LOCK_VERBOSE_OUTPUT

// Uncomment this to enable checking for long lock times. It adds significant performance cost though.
//#define DEBUG_LOG_TIMING

// When uncommented, a backtrace will be printed whenever a too long lock-time is discovered
//#define DEBUG_LOG_BACKTRACE

//If DEBUG_LOG_TIMING is uncommented, and the duchain is locked for more then this count of milliseconds, a message is printed
#define LOCK_LOG_MILLISECONDS 1000

//If this is uncommented, backtraces are produced whenever the duchain is read-locked, and shows it when the same thread tries to get a write-lock, triggering an assertion.
//Very expensive!
//#define DEBUG_ASSERTION_BACKTRACES

//Uncomment this to search for Deadlocks. DEBUG_LOG_TIMING must be enabled too, and DEBUG_LOG_BACKTRACE is recommended
//Warning: this may result in a total spamming of the command-line.
//#define SEARCH_DEADLOCKS

#include <kdebug.h>

namespace std {
  using namespace __gnu_cxx;
}

namespace KDevelop
{
class DUChainLockPrivate
{
public:
  DUChainLockPrivate() {
    m_writer = 0;
    m_writerRecursion = 0;
    m_totalReaderRecursion = 0;
    m_readersEnd = m_readers.end(); //This is used to speedup currentThreadHasReadLock()
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

    DUChainLockPrivate::DUChainLockPrivate::ReaderMap::const_iterator it = m_readers.find( QThread::currentThreadId() );
    if( it != m_readersEnd )
      ownReaderRecursion = (*it).second;
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
  typedef std::hash_map<Qt::HANDLE,int> ReaderMap; //Faster than QHash
  ReaderMap m_readers;
  DUChainLockPrivate::ReaderMap::const_iterator m_readersEnd; //Must always be updated when a new reader was added

#ifdef DEBUG_LOG_TIMING
  QTime m_lockTime;
#ifdef DEBUG_LOG_BACKTRACE
  QString m_lockBacktrace;
#endif
#endif
  
#ifdef DEBUG_ASSERTION_BACKTRACES
  QHash<Qt::HANDLE, QStack<QString> > m_readerBacktraces;
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
    timeout = 40000;
  
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
    DUChainLockPrivate::DUChainLockPrivate::ReaderMap::iterator it = d->m_readers.find( QThread::currentThreadId() );
    if( it != d->m_readers.end() )
      ++(*it).second;
    else {
      d->m_readers.insert( DUChainLockPrivate::ReaderMap::value_type(QThread::currentThreadId(), 1) );
      d->m_readersEnd = d->m_readers.end();
    }
    locked = true;
  }

  if(locked) {
    ++d->m_totalReaderRecursion;
#ifdef DEBUG_LOCK_TIMING
    d->startLockTiming();
#endif
    
#ifdef DEBUG_ASSERTION_BACKTRACES
    d->m_readerBacktraces[QThread::currentThreadId()].push(kBacktrace());
    Q_ASSERT(d->m_readerBacktraces[QThread::currentThreadId()].size() == d->m_readers[QThread::currentThreadId()]);
#endif
  }

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
  DUChainLockPrivate::DUChainLockPrivate::ReaderMap::iterator it = d->m_readers.find( QThread::currentThreadId() );
  Q_ASSERT(it != d->m_readers.end());
  --(*it).second;
  Q_ASSERT((*it).second>=0);
  --d->m_totalReaderRecursion;

  ///@todo Remove the all readers that do not exist any more at some point(leave other readers there with recursion 0
  ///      because it is very probable that they will lock again, and not having to re-allocate the bucket might speed up the locking.

/*  if( *it == 0 )
    d->m_readers.erase(it); //Maybe it would even be wise simply leaving it there*/

#ifdef DEBUG_ASSERTION_BACKTRACES
  d->m_readerBacktraces[QThread::currentThreadId()].pop();
#endif

#ifdef DEBUG_LOCK_TIMING
  d->auditTime();
#endif
}

bool DUChainLock::currentThreadHasReadLock()
{
  d->m_mutex.lock();
  
  DUChainLockPrivate::DUChainLockPrivate::ReaderMap::const_iterator it = d->m_readers.find( QThread::currentThreadId() );
  bool ret = false;
  if( it != d->m_readersEnd )
    ret = ((*it).second != 0);

  d->m_mutex.unlock();
  return ret;
}

bool DUChainLock::lockForWrite(uint timeout)
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9505) << "DUChain write lock requested by thread:" << QThread::currentThreadId();
  kDebug(9505) << "Current backtrace:" << kBacktrace();
#endif

  if(timeout == 0)
    timeout = 10000;
  
  QMutexLocker lock(&d->m_mutex);
  //It is not allowed to acquire a write-lock while holding read-lock

#ifdef DEBUG_ASSERTION_BACKTRACES
  if(d->ownReaderRecursion())
    kWarning(9505) << "Tried to lock the duchain for writing, but it was already locked for reading here:\n" << d->m_readerBacktraces[QThread::currentThreadId()].top();
#endif

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
  d->m_mutex.lock();
  bool ret = d->m_writer == QThread::currentThreadId();
  d->m_mutex.unlock();
  return ret;
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
