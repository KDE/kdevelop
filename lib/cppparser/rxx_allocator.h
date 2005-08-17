/*
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

#ifndef RXX_ALLOCATOR_H
#define RXX_ALLOCATOR_H

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

  rxx_allocator() { _S_ref++; }

  ~rxx_allocator() {
    if (--_S_ref == 0) {
      ++_S_block_index;

      for (size_type index = 0; index < _S_block_index; ++index)
	delete[] _S_storage[index];

      --_S_block_index;

      ::free(_S_storage);
    }
  }

  pointer address(reference __val) { return &__val; }
  const_pointer address(const_reference __val) const { return &__val; }

  pointer allocate(size_type __n, const void* = 0) {
    const size_type bytes = __n * sizeof(_Tp);

    if (_S_current_block == 0
	|| _S_block_size < _S_current_index + bytes)
      {
	++_S_block_index;

	_S_storage = reinterpret_cast<char**>
	  (::realloc(_S_storage, sizeof(char*) * (1 + _S_block_index)));

	_S_current_block = _S_storage[_S_block_index] = reinterpret_cast<char*>
	  (new char[_S_block_size]);

#if defined(RXX_ALLOCATOR_INIT_0) // ### make it a policy
	::memset(_S_current_block, 0, _S_block_size);
#endif
	_S_current_index = 0;
      }

    pointer p = reinterpret_cast<pointer>
      (_S_current_block + _S_current_index);

    _S_current_index += bytes;

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
  static size_type _S_ref;
  static const size_type _S_block_size;
  static size_type _S_block_index;
  static size_type _S_current_index;
  static char *_S_current_block;
  static char **_S_storage;
};

template <class _Tp> typename rxx_allocator<_Tp>::size_type
rxx_allocator<_Tp>::_S_ref = 0;

template <class _Tp> const typename rxx_allocator<_Tp>::size_type
rxx_allocator<_Tp>::_S_block_size = 1 << 16; // 64K

template <class _Tp> typename rxx_allocator<_Tp>::size_type
rxx_allocator<_Tp>::_S_block_index = max_block_count;

template <class _Tp> typename rxx_allocator<_Tp>::size_type
rxx_allocator<_Tp>::_S_current_index = 0;

template <class _Tp> char**
rxx_allocator<_Tp>::_S_storage = 0;

template <class _Tp> char*
rxx_allocator<_Tp>::_S_current_block = 0;

#endif // RXX_ALLOCATOR_H
