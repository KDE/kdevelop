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
#include <KDebug>
#include <QMutex>

/**The allocator which uses fixed size blocks for allocation of its elements.
Block size is currently 64k, allocated space is not reclaimed,
if the size of the element being allocated extends the amount of free
memory in the block then a new block is allocated.

The allocator supports standard c++ library interface but does not
make use of allocation hints.
*/

/*
if you want to check rxx_allocator isn't keeping too much memory for itself,
define this.
*/
//#define DEBUG_CHAIN_LENGTH

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
  static const size_type _S_block_size = 1 << 15; // 32K
  static const size_type _S_null_size  = 1<<10; // 1K


  struct block{
    block *next;
    char data[_S_block_size-sizeof(block*)];
  };

  static block* allocated_blocks;
  static QMutex chain_lock;

#ifdef DEBUG_CHAIN_LENGTH
  static size_type length(block*b){
  	if(!b) return 0;
  	return 1+length(b->next);
  }
#endif
  rxx_allocator() {
    init();
  }

  rxx_allocator(const rxx_allocator &/*__o*/) {
    init();
  }

  ~rxx_allocator() {
    if(_M_current_block){
      chain_lock.lock();
      _M_current_block->next=allocated_blocks;
      allocated_blocks=_M_first_block;
#ifdef DEBUG_CHAIN_LENGTH
      Q_ASSERT(length(allocated_blocks)*_S_block_size<(1<<25)) //32M
#endif
      chain_lock.unlock();
    }
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
        || (sizeof(_M_current_block->data)) < ( _M_current_index + bytes))
      {
        block *new_block=0;
        if(chain_lock.tryLock()){
          if(allocated_blocks){
            new_block=allocated_blocks;
            allocated_blocks=allocated_blocks->next;
          }
          chain_lock.unlock();
        }
        
        if(!new_block){
          new_block = new block;
        }

        ::memset(new_block->data, 0, _S_null_size - sizeof(block*) );
        if(_M_first_block){
          _M_current_block->next=new_block;
        } else {
          _M_first_block = new_block;
        }

        _M_current_block=new_block;
        _M_current_index = 0;
        _M_nulled_index=_S_null_size;
      }

    if( ( _M_nulled_index - sizeof(block*) ) < (_M_current_index + bytes) ){
      ::memset( (char*)_M_current_block + _M_nulled_index, 0, _S_null_size);
      _M_nulled_index+=_S_null_size;
      Q_ASSERT(_M_nulled_index <= _S_block_size );
    }


    pointer p = reinterpret_cast<pointer>
      (_M_current_block->data + _M_current_index);

    _M_current_index += bytes;

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

private:

  void init()
  {
    _M_current_index = 0;
    _M_current_block = 0;
    _M_first_block = 0;
  }

  template <class _Tp1> rxx_allocator(const rxx_allocator<_Tp1> &__o) {}

private:
  size_type _M_current_index;
  size_type _M_nulled_index;
  block *_M_current_block;
  block *_M_first_block;
};
template<class T>
typename rxx_allocator<T>::block *rxx_allocator<T>::allocated_blocks = 0;

template<class T> QMutex rxx_allocator<T>::chain_lock;

#endif // RXX_ALLOCATOR_H

