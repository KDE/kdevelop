/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef RXX_ALLOCATOR_H
#define RXX_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include <string.h>
#include <memory>

#include <QVector>
#include <QThreadStorage>

/**
 * A continous block of memory.
 */
struct rxx_allocator_block
{
  enum {
    BLOCK_SIZE = 1 << 16 // 64K
  };
  char data[BLOCK_SIZE];
};
Q_DECLARE_TYPEINFO(rxx_allocator_block, Q_MOVABLE_TYPE);

/**
 * This class handles the thread local cache of rxx_allocator_block's.
 *
 * The cache optimizes the repeated allocations required when we construct
 * many pools for small operations. This is done e.g. in the ExpressionParser.
 */
struct ThreadCache
{
  ThreadCache()
  {
    freeBlocks.reserve(MAX_CACHE_SIZE);
  }
  ~ThreadCache()
  {
    qDeleteAll(freeBlocks);
  }
  enum {
    // roughly 2MB
    MAX_CACHE_SIZE = 32
  };
  QVector<rxx_allocator_block*> freeBlocks;
};
static QThreadStorage< ThreadCache* > LocalThreadCache;

/**
 * The allocator which uses fixed size blocks for allocation of its elements.
 * Block size is currently 64k, allocated space is not reclaimed,
 * if the size of the element being allocated extends the amount of free
 * memory in the block then a new block is allocated.
 *
 * Free blocks are kept around until the current thread exits.
 *
 * The allocator supports standard c++ library interface but does not
 * make use of allocation hints.
*/
template <class _Tp>
class rxx_allocator
{
public:
  typedef _Tp value_type;
  typedef _Tp* pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp& reference;
  typedef const _Tp& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  rxx_allocator()
  {
    init();
  }

  rxx_allocator(const rxx_allocator &/*__o*/)
  {
    init();
  }

  ~rxx_allocator()
  {
    for(int i = 0; i <= m_currentBlock; ++i) {
      rxx_allocator_block* block = m_blocks.at(i);
      if (m_freeBlocks->size() < ThreadCache::MAX_CACHE_SIZE) {
        // cache block for reuse by another thread local allocator
        // this requires a 'prestine' state, i.e. memset to zero
        memset(block->data, 0, i == m_currentBlock ? m_currentIndex : static_cast<size_type>(rxx_allocator_block::BLOCK_SIZE));
        m_freeBlocks->append(block);
      } else {
        // otherwise we can discard this block
        delete block;
      }
    }
  }

  pointer address(reference __val) { return &__val; }
  const_pointer address(const_reference __val) const { return &__val; }

  /**Allocates @p __n elements continuosly in the pool. Warning! no
  check is done to check if the size of those @p __n elements
  fit into the block. You should assure you do not allocate more
  than the size of a block.*/
  pointer allocate(size_type __n, const void* = 0)
  {
    const size_type bytes = __n * sizeof(_Tp);

    if (rxx_allocator_block::BLOCK_SIZE < m_currentIndex + bytes) {
      // current block is full, use next one
      ++m_currentBlock;
      m_currentIndex = 0;
      Q_ASSERT(m_currentBlock == m_blocks.size());
      if (m_currentBlock == m_blocks.size()) {
        allocateBlock();
      } // else reuse existing storage
    }

    pointer p = reinterpret_cast<pointer>
      (m_blocks.at(m_currentBlock)->data + m_currentIndex);

    m_currentIndex += bytes;

    return p;
  }

  /**Deallocate does nothing in this implementation.*/
  void deallocate(pointer /*__p*/, size_type /*__n*/) {}

  size_type max_size() const { return size_type(-1) / sizeof(_Tp); }

  void construct(pointer __p, const_reference __val) { new (__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }

  template <class _Tp1> struct rebind {
    typedef rxx_allocator<_Tp1> other;
  };

  size_type size() const
  {
    return m_currentBlock * rxx_allocator_block::BLOCK_SIZE + m_currentIndex;
  }

private:
  void allocateBlock()
  {
    if (!m_freeBlocks->isEmpty()) {
      m_blocks.append(m_freeBlocks->last());
      m_freeBlocks->pop_back();
    } else {
      // allocate new memory block
      rxx_allocator_block* block = new rxx_allocator_block;
      memset(block->data, 0, rxx_allocator_block::BLOCK_SIZE);
      m_blocks.append(block);
    }
  }

  void init()
  {
    m_currentBlock = 0;
    m_currentIndex = 0;
    m_blocks.reserve(ThreadCache::MAX_CACHE_SIZE);
    if (!LocalThreadCache.hasLocalData()) {
      LocalThreadCache.setLocalData(new ThreadCache);
    }
    m_freeBlocks = &LocalThreadCache.localData()->freeBlocks;
    allocateBlock();
  }

  template <class _Tp1> rxx_allocator(const rxx_allocator<_Tp1> &__o) {}

private:
  QVector<rxx_allocator_block*> m_blocks;
  QVector<rxx_allocator_block*>* m_freeBlocks;
  int m_currentBlock;
  size_type m_currentIndex;
};

#endif // RXX_ALLOCATOR_H

