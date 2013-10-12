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

#ifndef RXX_ALLOCATOR_H
#define RXX_ALLOCATOR_H

#include <QVector>

#include "cppparserexport.h"

/**
 * A memory pool allocator which uses fixed size blocks to allocate its elements.
 *
 * Block size is currently 64k and allocated space is not reclaimed until the
 * memory pool is destroyed.
 *
 * Even then, free blocks are cached on a thread-local basis and kept around
 * until the thread exits. Up to MAX_CACHE_SIZE blocks are cached at any time.
 * This way it is very performant to repeatedly create this allocator
 * and use it for small numbers of allocations.
 *
 * If the size of an element being allocated extends the amount of free
 * memory left in the block then a new block is allocated.
 *
 * NOTE: Neither the elements constructor or destructor is being called. The
 *       allocated memory blocks are memset to 0 though. You need to call
 *       construct() or destroy() manually if you need to run the constructor
 *       or destructor.
 */
class KDEVCPPPARSER_EXPORT MemoryPool
{
public:
  MemoryPool();

  ~MemoryPool();

  /**
   * Allocates @p n elements of type @p T continuosly in the pool.
   *
   * @return pointer to first of @p n allocated objects of type @p T.
   *
   * @Warning Ensure you do not allocate more than the size of a block.
   * @sa BLOCK_SIZE
   */
  template<typename T>
  T* allocate(size_t n = 1)
  {
    const size_t bytes = n * sizeof(T);
    Q_ASSERT(bytes <= BLOCK_SIZE);

    if (BLOCK_SIZE < m_currentIndex + bytes) {
      // current block is full, use next one
      ++m_currentBlock;
      m_currentIndex = 0;
      Q_ASSERT(m_currentBlock == m_blocks.size());
      if (m_currentBlock == m_blocks.size()) {
        allocateBlock();
      } // else reuse existing storage
    }

    T* p = reinterpret_cast<T*>(m_blocks.at(m_currentBlock)->data + m_currentIndex);

    m_currentIndex += bytes;

    return p;
  }

  /**
   * @return the number of bytes that have been allocated.
   */
  size_t size() const
  {
    return m_currentBlock * BLOCK_SIZE + m_currentIndex;
  }

  /**
   * Construct an object of type @p T with the values of @p value
   * at the position of @p p.
   */
  template<typename T>
  void construct(T* p, const T& value) const
  {
    new (p) T(value);
  }

  /**
   * Run the destructor for the object of type @p T at @p p.
   */
  template<typename T>
  void destroy(T*p) const
  {
    p->~T();
  }

  enum {
    /**
     * Size of the continous memory blocks.
     *
     * Thus this also dictates the maximum size for a single allocation.
     */
    BLOCK_SIZE = 1 << 16, // 64K
    /**
     * Maximum number of free memory blocks that are cached
     * until the thread exists.
     */
    MAX_CACHE_SIZE = 32 // * BLOCK_SIZE = approx. 2MB
  };
private:
  Q_DISABLE_COPY(MemoryPool)

  /**
   * Look for a cached free memory block and consume it or
   * alternatively allocate a new memory block.
   */
  void allocateBlock();

  /**
   * A continous block of memory.
   */
  struct Block
  {
    char data[BLOCK_SIZE];
  };

private:
  QVector<Block*> m_blocks;
  int m_currentBlock;
  size_t m_currentIndex;

  friend struct MemoryPoolCache;
};

#endif // RXX_ALLOCATOR_H

