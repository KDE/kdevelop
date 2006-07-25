/* This file is part of kdev-pg
   Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KDEV_PG_MEMORY_POOL
#define KDEV_PG_MEMORY_POOL

#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>

struct block_t
{
  std::size_t _M_block_size;
  block_t *chain;
  char *data;
  char *ptr;
  char *end;

  inline void init(int block_size = 256)
  {
    _M_block_size = block_size;
    chain = 0;
    data = (char*) malloc(block_size);
    ptr = data;
    end = data + block_size;
  }

  inline void init0(int block_size = 256)
  {
    init(block_size);
    memset(data, '\0', block_size);
  }

  inline void destroy()
  {
    if (chain) {
      chain->destroy();
      free(chain);
    }

    free(data);
  }

  inline void *allocate(size_t size, block_t **right_most)
  {
    if (end < ptr + size) {
      //            assert( size < block_size );

      if (!chain) {
        chain = (block_t*) malloc(sizeof(block_t));
        chain->init0(_M_block_size << 2);
      }

      return chain->allocate(size, right_most);
    }

    char *r = ptr;
    ptr += size;

    if (right_most)
      *right_most = this;

    return r;
  }

};

struct kdev_pg_memory_pool
{
  block_t blk;
  block_t *right_most;

  inline kdev_pg_memory_pool() { blk.init0(); right_most = &blk; }
  inline ~kdev_pg_memory_pool() { blk.destroy(); }

  inline void *allocate(size_t size)
  { return right_most->allocate(size, &right_most); }
};

#endif
