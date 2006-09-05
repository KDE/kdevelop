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

#ifndef DUCHAINBASE_H
#define DUCHAINBASE_H

class TopDUContext;
class QReadWriteLock;

/**
 * Base class to enable the DUChain to be viewed as a model.
 */
class DUChainBase
{
  friend class ProxyObject;

public:
  DUChainBase(TopDUContext* top);
  virtual ~DUChainBase();

  /// \threadsafe
  TopDUContext* topContext() const;

  /// \threadsafe
  QReadWriteLock* chainLock() const;

  int modelRow;

private:
  // For proxy object
  DUChainBase();

private:
  TopDUContext* m_topContext;
};

#endif // DUCHAINBASE_H

// kate: indent-width 2;
