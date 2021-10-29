/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "indexeddeclaration.h"

#include "declaration.h"
#include "duchain.h"
#include "topducontextdynamicdata.h"

using namespace KDevelop;

IndexedDeclaration::IndexedDeclaration(uint topContext, uint declarationIndex)
    : m_topContext(topContext)
    , m_declarationIndex(declarationIndex)
{
}

IndexedDeclaration::IndexedDeclaration(const Declaration* decl)
{
    if (decl) {
        m_topContext = decl->topContext()->ownIndex();
        m_declarationIndex = decl->m_indexInTopContext;
    } else {
        m_topContext = 0;
        m_declarationIndex = 0;
    }
}

Declaration* IndexedDeclaration::declaration() const
{
    if (isDummy())
        return nullptr;
//   ENSURE_CHAIN_READ_LOCKED
    if (!m_topContext || !m_declarationIndex)
        return nullptr;

    TopDUContext* ctx = DUChain::self()->chainForIndex(m_topContext);
    if (!ctx)
        return nullptr;

    return ctx->m_dynamicData->declarationForIndex(m_declarationIndex);
}
