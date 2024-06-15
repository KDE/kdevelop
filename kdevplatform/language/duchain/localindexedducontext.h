/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_LOCALINDEXEDDUCONTEXT_H
#define KDEVPLATFORM_LOCALINDEXEDDUCONTEXT_H

#include <language/languageexport.h>
#include <QtGlobal>

namespace KDevelop {
class DUContext;
class TopDUContext;

/**
 * Represents a DUContext within a TopDUContext, without storing the TopDUContext(It must be given to data())
 */
class KDEVPLATFORMLANGUAGE_EXPORT LocalIndexedDUContext
{
public:
    explicit LocalIndexedDUContext(DUContext* decl);
    explicit LocalIndexedDUContext(uint contextIndex = 0);

    /**
     * @warning Duchain must be read locked
     */
    DUContext* data(TopDUContext* top) const;

    bool operator==(const LocalIndexedDUContext& rhs) const
    {
        return m_contextIndex == rhs.m_contextIndex;
    }

    bool isValid() const
    {
        return m_contextIndex != 0;
    }

    size_t hash() const
    {
        return m_contextIndex * 29;
    }

    bool operator<(const LocalIndexedDUContext& rhs) const
    {
        return m_contextIndex < rhs.m_contextIndex;
    }

    /**
     * Index within the top-context
     */
    uint localIndex() const
    {
        return m_contextIndex;
    }

    bool isLoaded(TopDUContext* top) const;

private:
    uint m_contextIndex;
};
}

Q_DECLARE_TYPEINFO(KDevelop::LocalIndexedDUContext, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_LOCALINDEXEDDUCONTEXT_H
