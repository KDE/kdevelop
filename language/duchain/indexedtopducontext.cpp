/* This file is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "indexedtopducontext.h"

#include "duchain.h"

using namespace KDevelop;

IndexedTopDUContext::IndexedTopDUContext(TopDUContext* context) {
  if(context)
    m_index = context->ownIndex();
  else
    m_index = DummyMask;
}

bool IndexedTopDUContext::isLoaded() const {
  if(index())
    return DUChain::self()->isInMemory(index());
  else
    return false;
}

IndexedString IndexedTopDUContext::url() const {
  if(index())
    return DUChain::self()->urlForIndex(index());
  else
    return IndexedString();
}

TopDUContext* IndexedTopDUContext::data() const {
//   ENSURE_CHAIN_READ_LOCKED
  if(index())
    return DUChain::self()->chainForIndex(index());
  else
    return 0;
}
