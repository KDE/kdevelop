/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_MODIFICATIONREVISIONSET_H
#define KDEVPLATFORM_MODIFICATIONREVISIONSET_H

#include "modificationrevision.h"

namespace KDevelop {

/**
 * This class represents a set of modification-revisions assigned to file-names.
 * It is safe to store this class in the disk-mapped internal duchain data structures.
* */

class KDEVPLATFORMLANGUAGE_EXPORT ModificationRevisionSet
{
  public:
    ModificationRevisionSet(uint index = 0);

    static void clearCache();

    void clear();

    uint index() const {
      return m_index;
    }

    ///Returns the count of file dependencies in this set
    uint size() const;

    void addModificationRevision(const IndexedString& url, const ModificationRevision& revision);

    ///Returns true if the modification-revision was contained before.
    bool removeModificationRevision(const IndexedString& url, const ModificationRevision& revision);

    bool needsUpdate() const;

    QString toString() const;

    bool operator!=(const ModificationRevisionSet& rhs) const {
      return m_index != rhs.m_index;
    }

    bool operator==(const ModificationRevisionSet& rhs) const {
      return m_index == rhs.m_index;
    }

    ModificationRevisionSet& operator+=(const ModificationRevisionSet& rhs);
    ModificationRevisionSet& operator-=(const ModificationRevisionSet& rhs);

  private:
    uint m_index;
};
}

#endif // KDEVPLATFORM_MODIFICATIONREVISIONSET_H
