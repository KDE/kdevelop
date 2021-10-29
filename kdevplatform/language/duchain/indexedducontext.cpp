/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "indexedducontext.h"

#include "ducontext.h"
#include "ducontextdata.h"
#include "ducontextdynamicdata.h"
#include "topducontext.h"
#include "duchain.h"
#include "topducontextdynamicdata.h"

using namespace KDevelop;

IndexedDUContext::IndexedDUContext(uint topContext, uint contextIndex)
    : m_topContext(topContext)
    , m_contextIndex(contextIndex)
{
}

IndexedDUContext::IndexedDUContext(DUContext* ctx)
{
    if (ctx) {
        m_topContext = ctx->topContext()->ownIndex();
        m_contextIndex = ctx->m_dynamicData->m_indexInTopContext;
    } else {
        m_topContext = 0;
        m_contextIndex = 0;
    }
}

IndexedTopDUContext IndexedDUContext::indexedTopContext() const
{
    if (isDummy()) {
        return IndexedTopDUContext();
    }
    return IndexedTopDUContext(m_topContext);
}

DUContext* IndexedDUContext::context() const
{
    if (isDummy())
        return nullptr;
//   ENSURE_CHAIN_READ_LOCKED
    if (!m_topContext)
        return nullptr;

    TopDUContext* ctx = DUChain::self()->chainForIndex(m_topContext);
    if (!ctx)
        return nullptr;

    if (!m_contextIndex)
        return ctx;

    return ctx->m_dynamicData->contextForIndex(m_contextIndex);
}
