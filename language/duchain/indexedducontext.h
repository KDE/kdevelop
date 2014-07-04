/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_INDEXEDDUCONTEXT_H
#define KDEVPLATFORM_INDEXEDDUCONTEXT_H

#include "languageexport.h"
#include <language/util/kdevhash.h>

#include <QPair>

namespace KDevelop {

class DUContext;
class IndexedTopDUContext;

/**
 * Represents a context only by its global indices
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedDUContext
{
  public:
    IndexedDUContext(DUContext* decl);
    IndexedDUContext(uint topContext = 0, uint contextIndex = 0);

    ///Duchain must be read locked
    DUContext* context() const;

    ///Duchain must be read locked
    DUContext* data() const {
      return context();
    }

    bool operator==(const IndexedDUContext& rhs) const {
      return m_topContext == rhs.m_topContext && m_contextIndex == rhs.m_contextIndex;
    }
    uint hash() const {
      return KDevHash() << m_topContext << m_contextIndex;
    }

    bool isValid() const {
      return !isDummy() && context() != 0;
    }

    bool operator<(const IndexedDUContext& rhs) const {
      Q_ASSERT(!isDummy());
      return m_topContext < rhs.m_topContext || (m_topContext == rhs.m_topContext && m_contextIndex < rhs.m_contextIndex);
    }

    //Index within the top-context
    uint localIndex() const {
      if(isDummy())
        return 0;

      return m_contextIndex;
    }

    uint topContextIndex() const {
      return m_topContext;
    }

    IndexedTopDUContext indexedTopContext() const;

    /**
     * The following functions allow storing 2 integers in this object and marking it as a dummy,
     * which makes the isValid() function always return false for this object, and use the integers
     * for other purposes
     * Clears the contained data
     */
    void setIsDummy(bool dummy) {
      if(isDummy() == dummy)
        return;
      if(dummy)
        m_topContext = 1 << 31;
      else
        m_topContext = 0;
      m_contextIndex = 0;
    }

    bool isDummy() const {
      //We use the second highest bit to mark dummies, because the highest is used for the sign bit of stored
      //integers
      return (bool)(m_topContext & (1 << 31));
    }

    QPair<uint, uint> dummyData() const {
      Q_ASSERT(isDummy());
      return qMakePair(m_topContext & (~(1<<31)), m_contextIndex);
    }

    ///Do not call this when this object is valid. The first integer loses one bit of precision.
    void setDummyData(QPair<uint, uint> data) {
      Q_ASSERT(isDummy());

      m_topContext = data.first;
      m_contextIndex = data.second;
      Q_ASSERT(!isDummy());
      m_topContext |= (1 << 31); //Mark as dummy
      Q_ASSERT(isDummy());
      Q_ASSERT(dummyData() == data);
    }

  private:
    uint m_topContext;
    uint m_contextIndex;
};

inline uint qHash(const IndexedDUContext& ctx)
{
  return ctx.hash();
}

}

Q_DECLARE_TYPEINFO(KDevelop::IndexedDUContext, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_INDEXEDDUCONTEXT_H
