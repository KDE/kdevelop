/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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
#include <memory>

/**The allocator which uses fixed size blocks for allocation of its elements.
Block size is currently 64k, allocated space is not reclaimed,
if the size of the element being allocated extends the amount of free
memory in the block then a new block is allocated.

The allocator supports standard c++ library interface but does not
make use of allocation hints.
*/
template <class _Tp> class rxx_allocator {
public:
  typedef _Tp value_type;
  typedef _Tp* pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp& reference;
  typedef const _Tp& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  static const size_type max_block_count = size_type(-1);
  static const size_type _S_block_size = 1 << 16; // 64K

  rxx_allocator() {
    init();
  }

  rxx_allocator(const rxx_allocator &/*__o*/) {
    init();
  }

  ~rxx_allocator() {
    for (size_type index = 0; index < _M_block_index + 1; ++index)
      delete[] _M_storage[index];

    ::free(_M_storage);
  }

  pointer address(reference __val) { return &__val; }
  const_pointer address(const_reference __val) const { return &__val; }

  /**Allocates @p __n elements continuosly in the pool. Warning! no
  check is done to check if the size of those @p __n elements
  fit into the block. You should assure you do not allocate more
  than the size of a block.*/
  pointer allocate(size_type __n, const void* = 0) {
    const size_type bytes = __n * sizeof(_Tp);

    if (_M_current_block == 0
	|| _S_block_size < _M_current_index + bytes)
      {
	++_M_block_index;

	_M_storage = reinterpret_cast<char**>
	  (::realloc(_M_storage, sizeof(char*) * (1 + _M_block_index)));

	_M_current_block = _M_storage[_M_block_index] = reinterpret_cast<char*>
	  (new char[_S_block_size]);

	::memset(_M_current_block, 0, _S_block_size);
	_M_current_index = 0;
      }

    pointer p = reinterpret_cast<pointer>
      (_M_current_block + _M_current_index);

    _M_current_index += bytes;

    return p;
  }

  /**Deallocate does nothing in this implementation.*/
  void deallocate(pointer /*__p*/, size_type /*__n*/) {}

  size_type max_size() const { return size_type(-1) / sizeof(_Tp); }

  void construct(pointer __p, const_reference __val) { new (__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }

private:
  template <class _Tp1> struct rebind {
    typedef rxx_allocator<_Tp1> other;
  };

  void init()
  {
    _M_block_index = max_block_count;
    _M_current_index = 0;
    _M_storage = 0;
    _M_current_block = 0;
  }

  template <class _Tp1> rxx_allocator(const rxx_allocator<_Tp1> &__o) {}

private:
  size_type _M_block_index;
  size_type _M_current_index;
  char *_M_current_block;
  char **_M_storage;
};

#endif // RXX_ALLOCATOR_H

// kate: space-indent on; indent-width 2; replace-tabs on;
