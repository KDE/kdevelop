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

#ifndef KDEV_PG_ALLOCATOR_H
#define KDEV_PG_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include <memory>

template <class _Tp>
class kdev_pg_allocator
{
public:
  typedef _Tp value_type;
  typedef _Tp* pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp& reference;
  typedef const _Tp& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  static const size_type max_block_count = size_type( -1);

  kdev_pg_allocator()
  {
    _S_ref++;
  }

  ~kdev_pg_allocator()
  {
    if (--_S_ref == 0)
      {
        ++_S_block_index;

        for (size_type index = 0; index < _S_block_index; ++index)
          delete[] _S_storage[index];

        --_S_block_index;

        ::free(_S_storage);
      }
  }

  pointer address(reference __val)
  {
    return &__val;
  }
  const_pointer address(const_reference __val) const
  {
    return &__val;
  }

  pointer allocate(size_type __n, const void* = 0)
  {
    const size_type bytes = __n * sizeof(_Tp);

    if (_S_current_block == 0
        || _S_block_size < _S_current_index + bytes)
      {
        ++_S_block_index;

        _S_storage = reinterpret_cast<char**>
          (::realloc(_S_storage, sizeof(char*) * (1 + _S_block_index)));

        _S_current_block = _S_storage[_S_block_index] = reinterpret_cast<char*>
          (new char[_S_block_size]);

        ::memset(_S_current_block, 0, _S_block_size);
        _S_current_index = 0;
      }

    pointer p = reinterpret_cast<pointer>
      (_S_current_block + _S_current_index);

    _S_current_index += bytes;

    return p;
  }

  void deallocate(pointer __p, size_type __n)
  {}

  size_type max_size() const
  {
    return size_type( -1) / sizeof(_Tp);
  }

  void contruct(pointer __p, const_reference __val)
  {
    new (__p) _Tp(__val);
  }
  void destruct(pointer __p)
  {
    __p->~_Tp();
  }

private:
  template <class _Tp1>
  struct rebind
  {
    typedef kdev_pg_allocator<_Tp1> other;
  };

  template <class _Tp1>
  kdev_pg_allocator(const kdev_pg_allocator<_Tp1> &__o)
  {}

private:
  static size_type _S_ref;
  static const size_type _S_block_size;
  static size_type _S_block_index;
  static size_type _S_current_index;
  static char *_S_current_block;
  static char **_S_storage;
};

template <class _Tp>
typename kdev_pg_allocator<_Tp>::size_type
kdev_pg_allocator<_Tp>::_S_ref = 0;

template <class _Tp>
const typename kdev_pg_allocator<_Tp>::size_type
kdev_pg_allocator<_Tp>::_S_block_size = 1 << 16; // 64K

template <class _Tp>
typename kdev_pg_allocator<_Tp>::size_type
kdev_pg_allocator<_Tp>::_S_block_index = max_block_count;

template <class _Tp>
typename kdev_pg_allocator<_Tp>::size_type
kdev_pg_allocator<_Tp>::_S_current_index = 0;

template <class _Tp>
char**
kdev_pg_allocator<_Tp>::_S_storage = 0;

template <class _Tp>
char*
kdev_pg_allocator<_Tp>::_S_current_block = 0;

#endif // KDEV_PG_ALLOCATOR_H
