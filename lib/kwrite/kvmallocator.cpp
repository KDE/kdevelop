/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

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
//----------------------------------------------------------------------------
//
// Virtual Memory Allocator

#include <unistd.h>
#include <sys/mman.h>

#include <qintdict.h>
#include <qlist.h>

#include <ktempfile.h>

#include "kvmallocator.h"


#define KVM_ALIGN 4095

struct KVMAllocator::Block
{
   off_t start;   
   size_t length;
   void *mmap;
};


class KVMAllocatorPrivate
{
public:
   KTempFile *tempfile;
   off_t max_length;
   QList<KVMAllocator::Block> used_blocks;
   QList<KVMAllocator::Block> free_blocks;
};

/**
 * Create a KVMAllocator 
 */
KVMAllocator::KVMAllocator()
{
   d = new KVMAllocatorPrivate;
   d->tempfile = 0;
   d->max_length = 0;
//   d->free_blocks = 0;
}

/**
 * Destruct the KVMAllocator and release all memory.
 */
KVMAllocator::~KVMAllocator()
{
   // TODO: Unmap all blocks.
   delete d->tempfile;
   delete d;
}

/**
 * Allocate a virtual memory block.
 * @param _size Size in bytes of the memory block.
 */
KVMAllocator::Block *
KVMAllocator::allocate(size_t _size)
{
   if (!d->tempfile)
   {
      d->tempfile = new KTempFile("/tmp/", "vmdata");
      d->tempfile->unlink();
   }
   // Search in free list

   // Create new block
   Block *block = new Block;
   block->start = d->max_length;
   block->length = _size;
   block->mmap = 0;
   d->used_blocks.prepend(block);
   d->max_length += (_size + KVM_ALIGN) & ~KVM_ALIGN;
   return block;
}
    
/**
 * Free a virtual memory block
 */
void 
KVMAllocator::free(Block *block)
{
   d->used_blocks.removeRef(block);
   d->free_blocks.append(block);
}
    
/**
 * Copy data from a virtual memory block to normal memory
 */
void 
KVMAllocator::copy(void *dest, Block *src, int _offset, size_t length)
{
qWarning("VM read: seek %d + %d", src->start,_offset);
   lseek(d->tempfile->handle(), src->start+_offset, SEEK_SET);
   if (length == 0)
      length = src->length - _offset;
   int to_go = length;
   int done = 0;
   char *buf = (char *) dest;
   while(done < to_go)
   {
      int n = read(d->tempfile->handle(), buf+done, to_go);
      if (n <= 0) return; // End of data or error
      done += n;
      to_go -= n;
   }
   // Done.
}
     
/**
 * Copy data from normal memory to a virtual memory block
 */
void 
KVMAllocator::copy(Block *dest, void *src, int _offset, size_t length)
{
qWarning("VM write: seek %d + %d", dest->start,_offset);
   lseek(d->tempfile->handle(), dest->start+_offset, SEEK_SET);
   if (length == 0)
      length = dest->length - _offset;
   int to_go = length;
   int done = 0;
   char *buf = (char *) src;
   while(done < to_go)
   {
      int n = write(d->tempfile->handle(), buf+done, to_go);
      if (n <= 0) return; // End of data or error
      done += n;
      to_go -= n;
   }
   // Done.
}

/**
 * Map a virtual memory block in memory
 */
void *
KVMAllocator::map(Block *block)
{
   if (block->mmap)
      return block->mmap;

   void *result = mmap(0, block->length, PROT_READ| PROT_WRITE, 
                       MAP_SHARED, d->tempfile->handle(), 0);
   block->mmap = result;
   return block->mmap;
}
    
/**
 * Unmap a virtual memory block
 */
void 
KVMAllocator::unmap(Block *block)
{
   munmap(block->mmap, block->length);
}
