/* This file is part of KDevelop
    Copyright (C) 2002,2003,2004 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef SMALLOBJECT_H
#define SMALLOBJECT_H

#include "rxx_allocator.h"
#include <cstring>

class pool
{
  rxx_allocator<char> __alloc;

public:
  inline void *allocate(std::size_t __size);

  // ### deprecated! it doesn't make sens to reallocate memory with a pool
  inline void *reallocate(void *old, std::size_t oldSize, std::size_t __size);
};

inline void *pool::allocate(std::size_t __size)
{
    return __alloc.allocate(__size);
}

inline void *pool::reallocate(void *__old, std::size_t __old_size, std::size_t __size)
{
    void *alloc = allocate(__size);
    ::memcpy(alloc, __old, __old_size);
    return alloc;
}


#endif
