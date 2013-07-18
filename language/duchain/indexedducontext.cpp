/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
  if(ctx) {
    m_topContext = ctx->topContext()->ownIndex();
    m_contextIndex = ctx->m_dynamicData->m_indexInTopContext;
  }else{
    m_topContext = 0;
    m_contextIndex = 0;
  }
}

IndexedTopDUContext IndexedDUContext::indexedTopContext() const
{
  if(isDummy()) {
    return IndexedTopDUContext();
  }
  return IndexedTopDUContext(m_topContext);
}

DUContext* IndexedDUContext::context() const
{
  if(isDummy())
    return 0;
//   ENSURE_CHAIN_READ_LOCKED
  if(!m_topContext)
    return 0;

  TopDUContext* ctx = DUChain::self()->chainForIndex(m_topContext);
  if(!ctx)
    return 0;

  if(!m_contextIndex)
    return ctx;

  return ctx->m_dynamicData->getContextForIndex(m_contextIndex);
}
