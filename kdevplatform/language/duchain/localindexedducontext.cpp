/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "localindexedducontext.h"

#include "ducontextdata.h"
#include "ducontext.h"
#include "topducontextdynamicdata.h"
#include "ducontextdynamicdata.h"
#include "topducontext.h"

using namespace KDevelop;

LocalIndexedDUContext::LocalIndexedDUContext(uint contextIndex)
    : m_contextIndex(contextIndex)
{
}

LocalIndexedDUContext::LocalIndexedDUContext(DUContext* ctx)
{
    if (ctx) {
        m_contextIndex = ctx->m_dynamicData->m_indexInTopContext;
    } else {
        m_contextIndex = 0;
    }
}

bool LocalIndexedDUContext::isLoaded(TopDUContext* top) const
{
    if (!m_contextIndex)
        return false;
    else
        return top->m_dynamicData->isContextForIndexLoaded(m_contextIndex);
}

DUContext* LocalIndexedDUContext::data(TopDUContext* top) const
{
    if (!m_contextIndex)
        return nullptr;
    else
        return top->m_dynamicData->contextForIndex(m_contextIndex);
}
