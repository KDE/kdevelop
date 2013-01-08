/* This file is part of KDevelop

   Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
   Copyright 2013 Milian Wolff <mail@milianw.de>

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

#include "memorypool.h"

#include <QThreadStorage>

/**
 * This class handles the thread local caching of memory blocks.
 *
 * The cache optimizes the repeated allocations required when we construct
 * many pools for small operations. This is done e.g. in the ExpressionParser.
 */
struct MemoryPoolCache
{
  MemoryPoolCache()
  {
    freeBlocks.reserve(MemoryPool::MAX_CACHE_SIZE);
  }
  ~MemoryPoolCache()
  {
    qDeleteAll(freeBlocks);
  }
  QVector<MemoryPool::Block*> freeBlocks;
};

MemoryPool::MemoryPool()
: m_currentBlock(0)
, m_currentIndex(0)
{
  // preallocate some space for the potentially used blocks
  m_blocks.reserve(MAX_CACHE_SIZE);

  // setup thread local storage
  ///TODO: Once we can depend on Qt 4.8+ directly store a MemoryPoolCache
  ///      and not a pointer to it. This obsoletes the manual construction.
  static QThreadStorage< MemoryPoolCache* > threadLocalCache;
  if (!threadLocalCache.hasLocalData()) {
    threadLocalCache.setLocalData(new MemoryPoolCache);
  }
  m_freeBlocks = &threadLocalCache.localData()->freeBlocks;

  // ensure there is a block which we can use
  allocateBlock();
}

MemoryPool::~MemoryPool()
{
  for(int i = 0; i <= m_currentBlock; ++i) {
    Block* block = m_blocks.at(i);
    if (m_freeBlocks->size() < MAX_CACHE_SIZE) {
      // cache block for reuse by another thread local allocator
      // this requires a 'prestine' state, i.e. memset to zero
      memset(block->data, 0, i == m_currentBlock ? m_currentIndex : static_cast<size_t>(BLOCK_SIZE));
      m_freeBlocks->append(block);
    } else {
      // otherwise we can discard this block
      delete block;
    }
  }
}

void MemoryPool::allocateBlock()
{
  if (!m_freeBlocks->isEmpty()) {
    // reuse cached memory block
    m_blocks.append(m_freeBlocks->last());
    m_freeBlocks->pop_back();
  } else {
    // allocate new memory block
    Block* block = new Block;
    memset(block->data, 0, BLOCK_SIZE);
    m_blocks.append(block);
  }
}
