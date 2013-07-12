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

#include "localindexedducontext.h"

#include "ducontextdata.h"
#include "ducontext.h"
#include "topducontextdynamicdata.h"
#include "ducontextdynamicdata.h"

using namespace KDevelop;

LocalIndexedDUContext::LocalIndexedDUContext(uint contextIndex)
  : m_contextIndex(contextIndex)
{
}

LocalIndexedDUContext::LocalIndexedDUContext(DUContext* ctx)
{
  if(ctx) {
    m_contextIndex = ctx->m_dynamicData->m_indexInTopContext;
  }else{
    m_contextIndex = 0;
  }
}

bool LocalIndexedDUContext::isLoaded(TopDUContext* top) const
{
  if(!m_contextIndex)
    return false;
  else
    return top->m_dynamicData->isContextForIndexLoaded(m_contextIndex);
}

DUContext* LocalIndexedDUContext::data(TopDUContext* top) const
{
  if(!m_contextIndex)
    return 0;
  else
    return top->m_dynamicData->getContextForIndex(m_contextIndex);
}
