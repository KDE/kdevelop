/*
  This file is part of kdev-pg
  Copyright 2005, 2006 Roberto Raggi <roberto@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
