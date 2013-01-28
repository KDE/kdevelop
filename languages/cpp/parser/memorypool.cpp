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

/**
 * The thread local cache is only initialized after the first memory pool of
 * a given thread is destroyed. The reason is that before we cannot have any
 * cached values anyways.
 *
 * Furthermore this makes it safe to construct a MemoryPool but only use it
 * later in a different thread which is actually the common case for the
 * usage as the ParseSession's memory pool inside the CPPParseJob.
 */
static QThreadStorage< MemoryPoolCache* > threadLocalCache;

MemoryPool::MemoryPool()
: m_currentBlock(-1)
, m_currentIndex(BLOCK_SIZE)
{
  // preallocate some space for the potentially used blocks
  m_blocks.reserve(MAX_CACHE_SIZE);
}

MemoryPool::~MemoryPool()
{
  ///TODO: Once we can depend on Qt 4.8+ directly store a MemoryPoolCache
  ///      and not a pointer to it. This obsoletes the manual construction.
  MemoryPoolCache* cache = threadLocalCache.localData();
  if (!cache) {
    // setup thread local storage
    cache = new MemoryPoolCache;
    threadLocalCache.setLocalData(cache);
  }
  for(int i = 0; i <= m_currentBlock; ++i) {
    Block* block = m_blocks.at(i);
    if (cache->freeBlocks.size() < MAX_CACHE_SIZE) {
      // cache block for reuse by another thread local allocator
      // this requires a 'prestine' state, i.e. memset to zero
      memset(block->data, 0, i == m_currentBlock ? m_currentIndex : static_cast<size_t>(BLOCK_SIZE));
      cache->freeBlocks.append(block);
    } else {
      // otherwise we can discard this block
      delete block;
    }
  }
}

void MemoryPool::allocateBlock()
{
  // NOTE: thread local cache data might not be set, esp. if this is the first mem pool of a thread.
  MemoryPoolCache* cache = threadLocalCache.localData();
  if (cache && !cache->freeBlocks.isEmpty()) {
    // reuse cached memory block
    m_blocks.append(cache->freeBlocks.last());
    cache->freeBlocks.pop_back();
  } else {
    // allocate new memory block
    Block* block = new Block;
    memset(block->data, 0, BLOCK_SIZE);
    m_blocks.append(block);
  }
}
