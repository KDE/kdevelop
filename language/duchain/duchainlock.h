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

#ifndef KDEVPLATFORM_DUCHAINLOCK_H
#define KDEVPLATFORM_DUCHAINLOCK_H

#include <language/languageexport.h>

namespace KDevelop
{

// #define NO_DUCHAIN_LOCK_TESTING

/**
 * Macros for ensuring the DUChain is locked properly.
 *
 * These should be used in every method that accesses or modifies a
 * member on the DUChain or one of its contexts, if ENSURE_CAN_WRITE and ENSURE_CAN_READ do not apply.
 * From within a Declaration or DUContext, ENSURE_CAN_WRITE and ENSURE_CAN_READ should be used instead of these.
 */
#if !defined(NDEBUG) && !defined(NO_DUCHAIN_LOCK_TESTING)
#define ENSURE_CHAIN_READ_LOCKED Q_ASSERT(DUChain::lock()->currentThreadHasReadLock() || DUChain::lock()->currentThreadHasWriteLock());
#define ENSURE_CHAIN_WRITE_LOCKED Q_ASSERT(DUChain::lock()->currentThreadHasWriteLock());
#define ENSURE_CHAIN_NOT_LOCKED Q_ASSERT(!DUChain::lock()->currentThreadHasReadLock() && !DUChain::lock()->currentThreadHasWriteLock());
#else
#define ENSURE_CHAIN_READ_LOCKED
#define ENSURE_CHAIN_WRITE_LOCKED
#define ENSURE_CHAIN_NOT_LOCKED
#endif

/**
 * Customized read/write locker for the definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainLock
{
public:
  /// Constructor.
  DUChainLock();
  /// Destructor.
  ~DUChainLock();

  /**
   * Acquires a read lock. Will not return until the lock is acquired
   * or timeout
   *
   * Any number of read locks can be acquired at once, but not while
   * there is a write lock.  Read locks are recursive.
   * That means that a thread can acquire a read-lock when it already
   * has an arbitrary count of read- and write-locks acquired.
   * @param timeout A locking timeout in milliseconds. If it is reached, and the lock could not be acquired, false is returned. If null, the default timeout is used.
   */
  bool lockForRead(unsigned int timeout = 0);

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
   * @param timeout A timeout in milliseconds. If zero, the default-timeout is used(Currently 10 seconds).
   *
   * \warning Write-locks can NOT be acquired by threads that already have a read-lock.
   */
  bool lockForWrite(unsigned int timeout = 0);

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

/**
 * Customized read locker for the definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainReadLocker
{
public:
  /**
   * Constructor.  Attempts to acquire a read lock.
   *
   * \param duChainLock lock to read-acquire. If this is left zero, DUChain::lock() is used.
   * \param timeout Timeout in milliseconds. If this is not zero, you've got to check locked() to see whether the lock succeeded.
   */
  DUChainReadLocker(DUChainLock* duChainLock = 0, unsigned int timeout = 0);

  /// Destructor.
  ~DUChainReadLocker();

  /// Acquire the read lock (again). Uses the same timeout given to the constructor.
  bool lock();
  /// Unlock the read lock.
  void unlock();

  ///Returns true if a lock was requested and the lock succeeded, else false
  bool locked() const;

private:
  ///This class does not use a d-pointer for performance reasons (allocation+deletion in every high frequency is expensive)
  DUChainLock* m_lock;
  bool m_locked;
  unsigned int m_timeout;
};

/**
 * Customized write locker for the definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainWriteLocker
{
public:
  /**
   * Constructor.  Attempts to acquire a write lock.
   *
   * \param duChainLock lock to write-acquire. If this is left zero, DUChain::lock() is used.
   * \param timeout Timeout in milliseconds. If this is not zero, you've got to check locked() to see whether the lock succeeded.
   */
  DUChainWriteLocker(DUChainLock* duChainLock = 0, unsigned int timeout = 0);
  /// Destructor.
  ~DUChainWriteLocker();

  /// Acquire the write lock (again). Uses the same timeout given to the constructor.
  bool lock();
  /// Unlock the write lock.
  void unlock();

  ///Returns true if a lock was requested and the lock succeeded, else false
  bool locked() const;

private:
  ///This class does not use a d-pointer for performance reasons (allocation+deletion in every high frequency is expensive)
  DUChainLock* m_lock;
  bool m_locked;
  unsigned int m_timeout;
};

/**
 * Like the ENSURE_CHAIN_WRITE_LOCKED and .._READ_LOCKED, except that this should be used in items that can be detached from the du-chain, like DOContext's and Declarations.
 * Those items must implement an inDUChain() function that returns whether the item is in the du-chain.
 * Examples for such detachable items are DUContext's and Declarations, they can be written as long as they are not in the DUChain.
 * */
#if !defined(NDEBUG) && !defined(NO_DUCHAIN_LOCK_TESTING)
#define ENSURE_CAN_WRITE {if( inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ {if( inDUChain()) { ENSURE_CHAIN_READ_LOCKED }}
#else
#define ENSURE_CAN_WRITE
#define ENSURE_CAN_READ
#endif

}

#endif // KDEVPLATFORM_DUCHAINLOCK_H
