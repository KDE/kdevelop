/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVWRITEUNLOCKER_H
#define KDEVWRITEUNLOCKER_H

#include "topducontext.h"

#include <QThread>

class KDevWriteUnlocker
{
public:
  inline KDevWriteUnlocker(const DUContext* context = 0)
    : m_lock(0)
    , m_other(0)
    , m_wasWriteLocked(false)
  {
    if (context)
      switchLock(context);
  }

  inline void switchLock(const DUContext* context, const DUContext* other = 0)
  {
    Q_ASSERT(!m_lock);
    m_lock = context->chainLock();
    m_wasWriteLocked = context->isWriteLocked();
    if (m_wasWriteLocked) {
      m_lock->unlock();
      m_lock->lockForRead();
    }

    m_other = other ? other->chainLock() : 0;
    if (m_other)
      m_other->lockForRead();
  }

  inline void readLock(const DUContext* context)
  {
    Q_ASSERT(context);
    m_other = context ? context->chainLock() : 0;
    m_other->lockForRead();
  }

  inline ~KDevWriteUnlocker()
  {
    if (m_other)
      m_other->unlock();

    if (m_wasWriteLocked) {
      m_lock->unlock();
      m_lock->lockForWrite();
    }
  }

private:
  Q_DISABLE_COPY(KDevWriteUnlocker)
  QReadWriteLock* m_lock;
  QReadWriteLock* m_other;
  bool m_wasWriteLocked;
};

#endif // KDEVWRITEUNLOCKER_H

// kate: indent-width 2;
