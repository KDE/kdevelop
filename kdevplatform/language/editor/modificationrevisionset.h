/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
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
    explicit ModificationRevisionSet(uint index = 0);

    static void clearCache();

    void clear();

    uint index() const
    {
        return m_index;
    }

    ///Returns the count of file dependencies in this set
    uint size() const;

    void addModificationRevision(const IndexedString& url, const ModificationRevision& revision);

    ///Returns true if the modification-revision was contained before.
    bool removeModificationRevision(const IndexedString& url, const ModificationRevision& revision);

    bool needsUpdate() const;

    QString toString() const;

    bool operator!=(const ModificationRevisionSet& rhs) const
    {
        return m_index != rhs.m_index;
    }

    bool operator==(const ModificationRevisionSet& rhs) const
    {
        return m_index == rhs.m_index;
    }

    ModificationRevisionSet& operator+=(const ModificationRevisionSet& rhs);
    ModificationRevisionSet& operator-=(const ModificationRevisionSet& rhs);

private:
    uint m_index;
};
}

#endif // KDEVPLATFORM_MODIFICATIONREVISIONSET_H
