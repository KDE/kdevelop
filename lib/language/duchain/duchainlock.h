/* This file is part of KDevelop
    Copyright (C) 2007 Kris Wong <kris.p.wong@gmail.com>

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

#include "kdevlanguageexport.h"

/**
 * In a DEBUG build, keeps track of additional locking information.
 * In a non-DEBUG build, is actually a QReadWriteLock.
 */
#ifdef NDEBUG

#include <QtCore/QReadWriteLock>

#define ENSURE_CHAIN_READ_LOCKED
#define ENSURE_CHAIN_WRITE_LOCKED

class KDEVPLATFORMLANGUAGE_EXPORT DUChainLock : public QReadWriteLock
{
public:
  DUChainLock() : QReadWriteLock() {}
  ~DUChainLock() {}
};

class KDEVPLATFORMLANGUAGE_EXPORT DUChainReadLocker : public QReadLocker
{
public:
  DUChainReadLocker(DUChainLock* duChainLock) : QReadLocker(duChainLock) {}
  ~DUChainReadLocker() {}
};

class KDEVPLATFORMLANGUAGE_EXPORT DUChainWriteLocker : public QWriteLocker
{
public:
  DUChainWriteLocker(DUChainLock* duChainLock) : QWriteLocker(duChainLock) {}
  ~DUChainWriteLocker() {}
};

#else

/**
 * Macros for ensuring the DUChain is locked properly.
 *
 * These should be used in every method that accesses or modifies a
 * member on the DUChain or one of its contexts.
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
   * or timeout is reached (10 seconds).
   *
   * Any number of read locks can be acquired at once, but not while
   * there is a write lock.  Read locks can be recursive.
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
   * There can be no other read or write locks held at the same time.
   * Write locks cannot be recursive.
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

#endif // DUCHAINLOCK_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
