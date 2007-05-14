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

DUChainLock::DUChainLock()
  : m_writer(0)
{
}

DUChainLock::~DUChainLock()
{
}

bool DUChainLock::lockForRead()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain read lock requested by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&m_mutex);

  int currentTime = 0;
  bool locked = false;

  // 10 second timeout...
  while (m_writer != 0 && currentTime < 1000) {
    lock.unlock();
    usleep(10000);
    currentTime++;
    lock.relock();
  }

  if (m_writer == 0) {
    m_readers << QThread::currentThreadId();
    locked = true;
  }

  return locked;
}

void DUChainLock::releaseReadLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain read lock released by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&m_mutex);
  m_readers.remove(QThread::currentThreadId());
}

bool DUChainLock::currentThreadHasReadLock()
{
  QMutexLocker lock(&m_mutex);
  return m_readers.contains(QThread::currentThreadId());
}

bool DUChainLock::lockForWrite()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain write lock requested by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&m_mutex);

  // Write locks are non-recursive.
  Q_ASSERT(m_writer != QThread::currentThreadId());

  int currentTime = 0;
  bool locked = false;

  // 10 second timeout...
  while (m_writer != 0 && !m_readers.empty() && currentTime < 1000) {
    lock.unlock();
    usleep(10000);
    currentTime++;
    lock.relock();
  }

  if (m_writer == 0 && m_readers.empty()) {
    m_writer = QThread::currentThreadId();
    locked = true;
  }

  return locked;
}

void DUChainLock::releaseWriteLock()
{
#ifdef DUCHAIN_LOCK_VERBOSE_OUTPUT
  kDebug(9007) << "DUChain write lock released by thread: " << QThread::currentThreadId() << endl;
#endif

  QMutexLocker lock(&m_mutex);
  m_writer = 0;
}

bool DUChainLock::currentThreadHasWriteLock()
{
  QMutexLocker lock(&m_mutex);
  return m_writer == QThread::currentThreadId();
}

#endif // NDEBUG

// kate: indent-width 2;
