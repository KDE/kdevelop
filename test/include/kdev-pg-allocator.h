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

#ifndef KDEV_PG_ALLOCATOR_H
#define KDEV_PG_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include <memory>

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
    _M_block_index = max_block_count;
    _M_current_index = 0;
    _M_storage = 0;
    _M_current_block = 0;
  }

  ~rxx_allocator() {
    for (size_type index = 0; index < _M_block_index + 1; ++index)
      delete[] _M_storage[index];

    ::free(_M_storage);
  }

  pointer address(reference __val) { return &__val; }
  const_pointer address(const_reference __val) const { return &__val; }

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

#if defined(RXX_ALLOCATOR_INIT_0) // ### make it a policy
        ::memset(_M_current_block, 0, _S_block_size);
#endif
        _M_current_index = 0;
      }

    pointer p = reinterpret_cast<pointer>
      (_M_current_block + _M_current_index);

    _M_current_index += bytes;

    return p;
  }

  void deallocate(pointer __p, size_type __n) {}

  size_type max_size() const { return size_type(-1) / sizeof(_Tp); }

  void contruct(pointer __p, const_reference __val) { new (__p) _Tp(__val); }
  void destruct(pointer __p) { __p->~_Tp(); }

private:
  template <class _Tp1> struct rebind {
    typedef rxx_allocator<_Tp1> other;
  };

  template <class _Tp1> rxx_allocator(const rxx_allocator<_Tp1> &__o) {}

private:
  size_type _M_block_index;
  size_type _M_current_index;
  char *_M_current_block;
  char **_M_storage;
};

#endif // KDEV_PG_ALLOCATOR_H
