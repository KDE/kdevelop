/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_INDEXEDTOPDUCONTEXT_H
#define KDEVPLATFORM_INDEXEDTOPDUCONTEXT_H

#include <QMetaType>
#include <QPair>

#include <language/languageexport.h>

namespace KDevelop {
class IndexedString;
class IndexedTopDUContextEmbeddedTreeHandler;
class TopDUContext;

/**
 * Allows simple indirect access to top-contexts with on-demand loading
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedTopDUContext
{
public:
    inline IndexedTopDUContext(uint index) : m_index(index)
    {
        if (!index)
            setIsDummy(true);
    }
    IndexedTopDUContext(TopDUContext* context = nullptr);

    enum {
        DummyMask = 1u << 31u
    };

    /**
     * Returns the top-context represented by this indexed top-context. If it wasn't loaded yet, it is loaded.
     *
     * The duchain must be read-locked when this is called!
     * To prevent it from being unloaded, store it into a ReferencedTopDUContext before
     * releasing the duchain lock.
     */
    TopDUContext* data() const;

    /**
     * Returns whether the top-context is currently loaded.
     *
     * If not, it will be loaded when you call data().
     */
    bool isLoaded() const;

    inline bool operator==(const IndexedTopDUContext& rhs) const
    {
        return m_index == rhs.m_index;
    }

    inline bool operator!=(const IndexedTopDUContext& rhs) const
    {
        return m_index != rhs.m_index;
    }

    inline bool operator<(const IndexedTopDUContext& rhs) const
    {
        return m_index < rhs.m_index;
    }

    inline bool isValid() const
    {
        return m_index && !isDummy();
    }

    inline uint index() const
    {
        if (isDummy())
            return 0;
        else
            return m_index;
    }

    inline bool isDummy() const
    {
        return m_index & DummyMask;
    }

    void setIsDummy(bool isDummy)
    {
        if (isDummy)
            m_index |= DummyMask;
        else
            m_index &= ~(( uint )DummyMask);
    }

    /**
     * Allows giving this IndexedTopDUContext some data while logically keeping it invalid.
     *
     * It will still return zero on index(), data(), etc.
     *
     * @param first The highest of this value bit will be removed.
     */
    void setDummyData(ushort first, ushort second)
    {
        Q_ASSERT(isDummy());
        m_index = (((( uint )first) << 16) + second) | DummyMask;
    }

    /**
     * The data previously set through setDummyData(). Initially 0.
     */
    QPair<ushort, ushort> dummyData() const
    {
        uint withoutMask = m_index & (~(( uint )DummyMask));
        return qMakePair(( ushort )(withoutMask >> 16), ( ushort )withoutMask);
    }

    IndexedString url() const;

private:
    uint m_index;
    friend class IndexedTopDUContextEmbeddedTreeHandler;
};

struct IndexedTopDUContextIndexConversion
{
    inline static uint toIndex(const IndexedTopDUContext& top)
    {
        return top.index();
    }

    inline static IndexedTopDUContext toItem(uint index)
    {
        return IndexedTopDUContext(index);
    }
};

inline size_t qHash(const IndexedTopDUContext& ctx)
{
    return ctx.index();
}
}

Q_DECLARE_METATYPE(KDevelop::IndexedTopDUContext)
Q_DECLARE_TYPEINFO(KDevelop::IndexedTopDUContext, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_INDEXEDTOPDUCONTEXT_H
