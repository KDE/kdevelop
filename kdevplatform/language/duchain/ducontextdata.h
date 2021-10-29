/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DUCONTEXTDATA_H
#define DUCONTEXTDATA_H

#include "duchainbase.h"
#include "ducontext.h"
#include "duchainpointer.h"
#include "declaration.h"
#include "use.h"
#include <language/languageexport.h>

#include "localindexeddeclaration.h"
#include "localindexedducontext.h"

namespace KDevelop {
class DUContext;

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_childContexts, LocalIndexedDUContext)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_importers, IndexedDUContext)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_importedContexts, DUContext::Import)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_localDeclarations, LocalIndexedDeclaration)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_uses, Use)

///This class contains data that needs to be stored to disk
class KDEVPLATFORMLANGUAGE_EXPORT DUContextData
    : public DUChainBaseData
{
public:
    DUContextData();
    ~DUContextData();
    DUContextData(const DUContextData& rhs);
    DUContextData& operator=(const DUContextData&) = delete;

    IndexedQualifiedIdentifier m_scopeIdentifier;
    IndexedDeclaration m_owner;
    using Import = DUContext::Import;
    START_APPENDED_LISTS_BASE(DUContextData, DUChainBaseData);
    APPENDED_LIST_FIRST(DUContextData, Import, m_importedContexts);
    APPENDED_LIST(DUContextData, LocalIndexedDUContext, m_childContexts, m_importedContexts);

    ///@todo Create an additional structure for importing to/from "temporary" contexts and classes in a way that it persists while saving/loading,
    ///      and doesn't require changing a top-contexts data only because a class was derived from.
    APPENDED_LIST(DUContextData, IndexedDUContext, m_importers, m_childContexts);

    ///@warning: Whenever m_localDeclarations is read or written, the duchain must be locked
    APPENDED_LIST(DUContextData, LocalIndexedDeclaration, m_localDeclarations, m_importers);
    /**
     * Vector of all uses in this context
     * Mutable for range synchronization
     * */
    APPENDED_LIST(DUContextData, Use, m_uses, m_localDeclarations);
    END_APPENDED_LISTS(DUContextData, m_uses);

    DUContext::ContextType m_contextType;
    bool m_inSymbolTable : 1;
    bool m_anonymousInParent : 1; //Whether this context was added anonymously into the parent. This means that it cannot be found as child-context in the parent.
    bool m_propagateDeclarations : 1;
};
}

#endif
