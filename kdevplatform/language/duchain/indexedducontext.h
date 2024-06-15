/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_INDEXEDDUCONTEXT_H
#define KDEVPLATFORM_INDEXEDDUCONTEXT_H

#include <language/languageexport.h>
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
    DUContext* data() const
    {
        return context();
    }

    bool operator==(const IndexedDUContext& rhs) const
    {
        return m_topContext == rhs.m_topContext && m_contextIndex == rhs.m_contextIndex;
    }
    size_t hash() const
    {
        return KDevHash() << m_topContext << m_contextIndex;
    }

    bool isValid() const
    {
        return !isDummy() && context() != nullptr;
    }

    bool operator<(const IndexedDUContext& rhs) const
    {
        Q_ASSERT(!isDummy());
        return m_topContext < rhs.m_topContext ||
               (m_topContext == rhs.m_topContext && m_contextIndex < rhs.m_contextIndex);
    }

    //Index within the top-context
    uint localIndex() const
    {
        if (isDummy())
            return 0;

        return m_contextIndex;
    }

    uint topContextIndex() const
    {
        return m_topContext;
    }

    IndexedTopDUContext indexedTopContext() const;

    /**
     * The following functions allow storing 2 integers in this object and marking it as a dummy,
     * which makes the isValid() function always return false for this object, and use the integers
     * for other purposes
     * Clears the contained data
     */
    void setIsDummy(bool dummy)
    {
        if (isDummy() == dummy)
            return;
        if (dummy)
            m_topContext = 1 << 31;
        else
            m_topContext = 0;
        m_contextIndex = 0;
    }

    bool isDummy() const
    {
        //We use the second highest bit to mark dummies, because the highest is used for the sign bit of stored
        //integers
        return ( bool )(m_topContext & (1 << 31));
    }

    QPair<uint, uint> dummyData() const
    {
        Q_ASSERT(isDummy());
        return qMakePair(m_topContext & (~(1 << 31)), m_contextIndex);
    }

    ///Do not call this when this object is valid. The first integer loses one bit of precision.
    void setDummyData(QPair<uint, uint> data)
    {
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

inline size_t qHash(const IndexedDUContext& ctx)
{
    return ctx.hash();
}
}

Q_DECLARE_TYPEINFO(KDevelop::IndexedDUContext, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_INDEXEDDUCONTEXT_H
