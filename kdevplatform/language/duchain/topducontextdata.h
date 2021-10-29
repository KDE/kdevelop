/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TOPDUCONTEXTDATA_H
#define KDEVPLATFORM_TOPDUCONTEXTDATA_H

#include "ducontextdata.h"
#include "topducontext.h"
#include "declarationid.h"
#include "problem.h"

#include <serialization/indexedstring.h>

namespace KDevelop {
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(TopDUContextData, m_usedDeclarationIds, DeclarationId)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(TopDUContextData, m_problems, LocalIndexedProblem)

class KDEVPLATFORMLANGUAGE_EXPORT TopDUContextData
    : public DUContextData
{
public:
    explicit TopDUContextData(const IndexedString& url)
        : DUContextData()
        , m_url(url)
        , m_ownIndex(0)
        , m_currentUsedDeclarationIndex(0)
    {
        initializeAppendedLists();
    }

    TopDUContextData(const TopDUContextData& rhs)
        : DUContextData(rhs)
    {
        initializeAppendedLists();
        copyListsFrom(rhs);
        m_features = rhs.m_features;
        m_url = rhs.m_url;
        m_currentUsedDeclarationIndex = rhs.m_currentUsedDeclarationIndex;
        m_ownIndex = rhs.m_ownIndex;
        m_importsCache = rhs.m_importsCache;
    }

    ~TopDUContextData()
    {
        freeAppendedLists();
    }

    TopDUContextData& operator=(const TopDUContextData& rhs) = delete;

    TopDUContext::Features m_features;

    IndexedString m_url;
    uint m_ownIndex;

    ///If this is not empty, it means that the cache is used instead of the implicit structure.
    TopDUContext::IndexedRecursiveImports m_importsCache;

    ///Is used to count up the used declarations while building uses
    uint m_currentUsedDeclarationIndex;

    START_APPENDED_LISTS_BASE(TopDUContextData, DUContextData);
    ///Maps a declarationIndex to a DeclarationId, which is used when the entry in m_usedDeclaration is zero.
    APPENDED_LIST_FIRST(TopDUContextData, DeclarationId, m_usedDeclarationIds);
    APPENDED_LIST(TopDUContextData, LocalIndexedProblem, m_problems, m_usedDeclarationIds);
    END_APPENDED_LISTS(TopDUContextData, m_problems);

private:
    static void updateImportCacheRecursion(IndexedTopDUContext currentContext, std::set<uint>& visited);
    static void updateImportCacheRecursion(uint baseIndex, IndexedTopDUContext currentContext,
                                           TopDUContext::IndexedRecursiveImports& imports);
    friend class TopDUContext;
};
}
#endif
