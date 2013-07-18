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

#ifndef KDEVPLATFORM_LOCALINDEXEDDUCONTEXT_H
#define KDEVPLATFORM_LOCALINDEXEDDUCONTEXT_H

#include "../languageexport.h"

namespace KDevelop {

class DUContext;
class TopDUContext;

/**
 * Represents a DUContext within a TopDUContext, without storing the TopDUContext(It must be given to data())
 */
class KDEVPLATFORMLANGUAGE_EXPORT LocalIndexedDUContext {
  public:
    LocalIndexedDUContext(DUContext* decl);
    LocalIndexedDUContext(uint contextIndex = 0);

    /**
     * @warning Duchain must be read locked
     */
    DUContext* data(TopDUContext* top) const;

    bool operator==(const LocalIndexedDUContext& rhs) const {
      return m_contextIndex == rhs.m_contextIndex;
    }

    bool isValid() const {
      return m_contextIndex != 0;
    }

    uint hash() const {
      return m_contextIndex * 29;
    }

    bool operator<(const LocalIndexedDUContext& rhs) const {
      return m_contextIndex < rhs.m_contextIndex;
    }

    /**
     * Index within the top-context
     */
    uint localIndex() const {
      return m_contextIndex;
    }

    bool isLoaded(TopDUContext* top) const;

  private:
    uint m_contextIndex;
};

}

Q_DECLARE_TYPEINFO(KDevelop::LocalIndexedDUContext, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_LOCALINDEXEDDUCONTEXT_H
