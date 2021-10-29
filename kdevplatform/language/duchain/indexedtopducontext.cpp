/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "indexedtopducontext.h"

#include "duchain.h"

using namespace KDevelop;

IndexedTopDUContext::IndexedTopDUContext(TopDUContext* context)
{
    if (context)
        m_index = context->ownIndex();
    else
        m_index = DummyMask;
}

bool IndexedTopDUContext::isLoaded() const
{
    if (index())
        return DUChain::self()->isInMemory(index());
    else
        return false;
}

IndexedString IndexedTopDUContext::url() const
{
    if (index())
        return DUChain::self()->urlForIndex(index());
    else
        return IndexedString();
}

TopDUContext* IndexedTopDUContext::data() const
{
//   ENSURE_CHAIN_READ_LOCKED
    if (index())
        return DUChain::self()->chainForIndex(index());
    else
        return nullptr;
}
