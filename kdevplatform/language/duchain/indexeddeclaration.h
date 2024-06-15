/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_INDEXEDDECLARATION_H
#define KDEVPLATFORM_INDEXEDDECLARATION_H

#include <language/languageexport.h>

#include "indexedtopducontext.h"
#include <language/util/kdevhash.h>

namespace KDevelop {
class Declaration;

/**
 * Represents a declaration only by its global indices
 */
class KDEVPLATFORMLANGUAGE_EXPORT IndexedDeclaration
{
public:
    IndexedDeclaration(const Declaration* decl = nullptr);
    IndexedDeclaration(uint topContext, uint declarationIndex);

    /**
     * \warning Duchain must be read locked
     */
    Declaration* declaration() const;

    /**
     * \warning Duchain must be read locked
     */
    Declaration* data() const
    {
        return declaration();
    }

    inline bool operator==(const IndexedDeclaration& rhs) const
    {
        return m_topContext == rhs.m_topContext && m_declarationIndex == rhs.m_declarationIndex;
    }

    inline bool operator!=(const IndexedDeclaration& rhs) const { return !operator==(rhs); }

    inline uint hash() const
    {
        if (isDummy())
            return 0;
        return KDevHash() << m_topContext << m_declarationIndex;
    }

    ///@warning The duchain needs to be locked when this is called
    inline bool isValid() const
    {
        return !isDummy() && declaration() != nullptr;
    }

    inline bool operator<(const IndexedDeclaration& rhs) const
    {
        Q_ASSERT(!isDummy());
        return m_topContext < rhs.m_topContext ||
               (m_topContext == rhs.m_topContext && m_declarationIndex < rhs.m_declarationIndex);
    }

    /**
     * \return Index of the Declaration within the top context
     */
    inline uint localIndex() const
    {
        if (isDummy())
            return 0;
        else
            return m_declarationIndex;
    }

    inline uint topContextIndex() const
    {
        if (isDummy())
            return 0;
        else
            return m_topContext;
    }

    inline IndexedTopDUContext indexedTopContext() const
    {
        if (isDummy())
            return IndexedTopDUContext();
        else
            return IndexedTopDUContext(m_topContext);
    }

    /**
     * The following functions allow storing 2 integers in this object and marking it as a dummy,
     * which makes the isValid() function always return false for this object, and use the integers
     * for other purposes.
     *
     * Clears the contained data
     */
    void setIsDummy(bool dummy)
    {
        if (isDummy() == dummy)
            return;
        if (dummy)
            m_topContext = 1u << 31u;
        else
            m_topContext = 0;
        m_declarationIndex = 0;
    }

    inline bool isDummy() const
    {
        //We use the second highest bit to mark dummies, because the highest is used for the sign bit of stored
        //integers
        return ( bool )(m_topContext & static_cast<uint>(1u << 31u));
    }

    inline QPair<uint, uint> dummyData() const
    {
        Q_ASSERT(isDummy());
        return qMakePair(m_topContext & (~(1u << 31u)), m_declarationIndex);
    }

    /**
     * \warning Do not call this when this object is valid.
     *
     * The first integer loses one bit of precision.
     */
    void setDummyData(QPair<uint, uint> data)
    {
        Q_ASSERT(isDummy());

        m_topContext = data.first;
        m_declarationIndex = data.second;
        Q_ASSERT(!isDummy());
        m_topContext |= (1u << 31u); //Mark as dummy
        Q_ASSERT(isDummy());
        Q_ASSERT(dummyData() == data);
    }

private:
    uint m_topContext;
    uint m_declarationIndex;
};

inline size_t qHash(const IndexedDeclaration& decl)
{
    return decl.hash();
}
}

Q_DECLARE_METATYPE(KDevelop::IndexedDeclaration)
Q_DECLARE_TYPEINFO(KDevelop::IndexedDeclaration, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_INDEXEDDECLARATION_H
