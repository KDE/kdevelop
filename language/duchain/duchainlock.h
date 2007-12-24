/* This file is part of KDevelop
    Copyright 2007 Kris Wong <kris.p.wong@gmail.com>

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

#ifndef DUCHAINLOCK_H
#define DUCHAINLOCK_H

#include "../languageexport.h"


/**
 * In a DEBUG build, keeps track of additional locking information.
 * In a non-DEBUG build, is actually a QReadWriteLock.
 */
#if 0

#include <QtCore/QReadWriteLock>

namespace KDevelop
{

#define ENSURE_CHAIN_READ_LOCKED
#define ENSURE_CHAIN_WRITE_LOCKED


class KDEVPLATFORMLANGUAGE_EXPORT DUChainLock : public QReadWriteLock //krazy:exclude=dpointer
{
public:
  DUChainLock() : QReadWriteLock() {}
  ~DUChainLock() {}
};

class KDEVPLATFORMLANGUAGE_EXPORT DUChainReadLocker : public QReadLocker //krazy:exclude=dpointer
{
public:
  DUChainReadLocker(DUChainLock* duChainLock) : QReadLocker(duChainLock) {}
  ~DUChainReadLocker() {}
};

class KDEVPLATFORMLANGUAGE_EXPORT DUChainWriteLocker : public QWriteLocker //krazy:exclude=dpointer
{
public:
  DUChainWriteLocker(DUChainLock* duChainLock) : QWriteLocker(duChainLock) {}
  ~DUChainWriteLocker() {}
};

#else

namespace KDevelop
{

/**
 * Macros for ensuring the DUChain is locked properly.
 *
 * These should be used in every method that accesses or modifies a
 * member on the DUChain or one of its contexts, if ENSURE_CAN_WRITE and ENSURE_CAN_READ do not apply.
 * From within a Declaration or DUContext, ENSURE_CAN_WRITE and ENSURE_CAN_READ should be used instead of these.
 */
#define ENSURE_CHAIN_READ_LOCKED Q_ASSERT(DUChain::lock()->currentThreadHasReadLock() || DUChain::lock()->currentThreadHasWriteLock());
#define ENSURE_CHAIN_WRITE_LOCKED Q_ASSERT(DUChain::lock()->currentThreadHasWriteLock());

class KDEVPLATFORMLANGUAGE_EXPORT DUChainLock
{
public:
  DUChainLock();
  ~DUChainLock();

  /**
   * Acquires a read lock. Will not return until the lock is acquired
   * or timeout
   *
   * Any number of read locks can be acquired at once, but not while
   * there is a write lock.  Read locks are recursive.
   * That means that a thread can acquire a read-lock when it already
   * has an arbitrary count of read- and write-locks acquired.
   * @param timeout A locking timeout in milliseconds. If it is reached, and the lock could not be acquired, false is returned.
   */
  bool lockForRead(unsigned int timeout);

  /**
   * Acquires a read lock. Will not return until the lock is acquired
   * or timeout of 10 seconds is reached.
   *
   * Any number of read locks can be acquired at once, but not while
   * there is a write lock.  Read locks are recursive.
   * That means that a thread can acquire a read-lock when it already
   * has an arbitrary count of read- and write-locks acquired.
   */
  bool lockForRead();

  /**
   * Releases a previously acquired read lock.
   */
  void releaseReadLock();

  /**
   * Determines if the current thread has a read lock.
   */
  bool currentThreadHasReadLock();

  /**
   * Acquires a write lock. Will not return until the lock is acquired
   * or timeout is reached (10 seconds).
   *
   * Write locks are recursive. That means that they can by acquired by threads
   * that already have an arbitrary count of write-locks acquired.
   *
   * WARNING: Write-locks can NOT be acquired by threads that already have a read-lock.
   */
  bool lockForWrite();

  /**
   * Releases a previously acquired write lock.
   */
  void releaseWriteLock();

  /**
   * Determines if the current thread has a write lock.
   */
  bool currentThreadHasWriteLock();

private:
  class DUChainLockPrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT DUChainReadLocker
{
public:
  DUChainReadLocker(DUChainLock* duChainLock);
  ~DUChainReadLocker();

  bool lock();
  void unlock();

private:
  class DUChainReadLockerPrivate* const d;
};

class KDEVPLATFORMLANGUAGE_EXPORT DUChainWriteLocker
{
public:
  DUChainWriteLocker(DUChainLock* duChainLock);
  ~DUChainWriteLocker();

  bool lock();
  void unlock();

private:
  class DUChainWriteLockerPrivate* const d;
};

#endif // NDEBUG

/**
 * Like the ENSURE_CHAIN_WRITE_LOCKED and .._READ_LOCKED, except that this should be used in items that can be detached from the du-chain, like DOContext's and Declarations.
 * Those items must implement an inDUChain() function that returns whether the item is in the du-chain.
 * Examples for such detachable items are DUContext's and Declarations, they can be written as long as they are not in the DUChain.
 * */
#define ENSURE_CAN_WRITE {if( inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ {if( inDUChain()) { ENSURE_CHAIN_READ_LOCKED }}

}

#endif // DUCHAINLOCK_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
