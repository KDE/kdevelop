;/*
   This file is part of KWrite
   Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include "kwbuffer.h"

// Includes for reading file
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <qfile.h>
#include <qtimer.h>
#include <qtextcodec.h>

#include <kvmallocator.h>

#include <assert.h>

#define LOADED_BLOCKS_MAX	10
#define DIRTY_BLOCKS_MAX        1
#define AVG_BLOCK_SIZE		8192

/**
 * Create an empty buffer.
 */
KWBuffer::KWBuffer()
{
   m_blocks.setAutoDelete(true);
   m_loader.setAutoDelete(true);
   connect( &m_loadTimer, SIGNAL(timeout()), this, SLOT(slotLoadFile()));
   m_vm = 0;
   clear();
}

/**
 * Reads @p size bytes from file-descriptor @p fd.
 */
static QByteArray readBlock(int fd, int size)
{
   QByteArray result(size);
   int bytesToRead = size;
   int bytesRead = 0;
   while(bytesToRead)
   {
      int n = ::read(fd, result.data()+bytesRead, bytesToRead);
      if (n == 0) break; // Done
      if ((n == -1) && (errno == EAGAIN))
         continue;
      if (n == -1)
      {
         // TODO: Do some error handling.
         break;
      }
      bytesRead += n;
      bytesToRead -= n;
   }
qWarning("Read = %d", bytesRead);
   result.truncate(bytesRead);
   return result;
}

/**
 * Writes the bytes buf[begin] to buf[end] to @p fd.
 */
static void writeBlock(int fd, const QByteArray &buf, int begin, int end)
{
   while(begin != end)
   {
      int n = ::write(fd, buf.data()+begin, end - begin);
      if ((n == -1) && (errno == EAGAIN))
         continue;
      if (n == -1)
         return; // TODO: Do some error handling.
      begin += n;
   }
}

void
KWBuffer::clear()
{
   delete m_vm;
   m_vm = new KVMAllocator;
   m_parsedBlocksClean.clear();
   m_parsedBlocksDirty.clear();
   m_loadedBlocks.clear();
   m_loader.clear();
   m_blocks.clear();
   KWBufState state;
   // Initial state.
   state.lineNr = 0;
   KWBufBlock *block = new KWBufBlock(state);
   m_blocks.insert(0, block);
   block->b_rawDataValid = true;
   block->b_appendEOL = true;   
   block->b_emptyBlock = true;
   block->m_endState.lineNr++;
   m_totalLines = block->m_endState.lineNr;
}

/**
 * Insert a file at line @p line in the buffer.
 */
void
KWBuffer::insertFile(int line, const QString &file, QTextCodec *codec)
{
  assert(line == 0); // Inserting at other places not yet handled.

  int fd = open(QFile::encodeName(file), O_RDONLY);
  if (fd < 0)
  {
     qWarning("Error loading file.");
     return; // Do some error propagation here.
  }

  KWBufFileLoader *loader = new KWBufFileLoader;
  loader->fd = fd;
  loader->dataStart = 0;
  loader->blockNr = 0;
  loader->codec = codec;
  m_loader.append(loader);

  loadFilePart();
}

void
KWBuffer::loadFilePart()
{
  const int blockSize = AVG_BLOCK_SIZE;
  const int blockRead = 3; // Read 5 blocks in a row

  KWBufFileLoader *loader = m_loader.first();

  KWBufState state;
  if (loader->blockNr > 0)
  {
     KWBufBlock *block = m_blocks.at(loader->blockNr-1);
     state = block->m_endState;
  }
  else
  {
     // Initial state.
     state.lineNr = 0;
  }
  int startLine = state.lineNr;
  bool eof = false;
 
 
  for(int i = 0; i < blockRead; i++)
  {
     QByteArray currentBlock = readBlock(loader->fd, blockSize);
     eof = ((int) currentBlock.size()) != blockSize;
     KWBufBlock *block = new KWBufBlock(state);
     m_blocks.insert(loader->blockNr++, block);
     m_loadedBlocks.append(block);
     if (m_loadedBlocks.count() > LOADED_BLOCKS_MAX)
     {
        KWBufBlock *buf2 = m_loadedBlocks.take(2);
qWarning("swapOut(%p)", buf2);
        buf2->swapOut(m_vm);
     }
     block->m_codec = loader->codec;
     loader->dataStart = block->blockFill(loader->dataStart, 
                                  loader->lastBlock, currentBlock, eof);
     state = block->m_endState;
qWarning("current->ref = %d last->ref = %d", currentBlock.nrefs(), loader->lastBlock.nrefs());
     loader->lastBlock = currentBlock;
     if (eof) break;
  }
  if (eof)
  {
qWarning("Loading finished.");
     m_loader.removeRef(loader);
  }
  if (m_loader.count())
  {
qWarning("Starting timer...");
     m_loadTimer.start(0, true);
  }

  m_totalLines += state.lineNr - startLine;
}

void
KWBuffer::insertData(int line, const QByteArray &data, QTextCodec *codec)
{
   assert(line == m_totalLines);
   KWBufBlock *prev_block;

   // Remove all preceding empty blocks.
   while(true) 
   {
      prev_block = m_blocks.last();
      if (!prev_block || !prev_block->b_emptyBlock)
         break;

      m_totalLines -= prev_block->m_endState.lineNr - prev_block->m_beginState.lineNr;
      m_blocks.removeRef(prev_block);
      m_parsedBlocksClean.removeRef(prev_block);
      m_parsedBlocksDirty.removeRef(prev_block);
      m_loadedBlocks.removeRef(prev_block);
   }

   KWBufState state;
   if (prev_block)
   {
       state = prev_block->m_endState;
   }
   else
   {
        // Initial state.
       state.lineNr = 0;
   }

  int startLine = state.lineNr;
  KWBufBlock *block = new KWBufBlock(state);
  m_blocks.append(block);
  m_loadedBlocks.append(block);
  block->m_codec = codec;

  // TODO: We always create a new block.
  // It would be more correct to collect the data in larger blocks.
  // We should do that without unnecasserily copying the data though.
  QByteArray lastData;
  int lastLine = 0;
  if (prev_block && prev_block->b_appendEOL && (prev_block->m_codec == codec))
  {
     // Take the last line of the previous block and add it to the
     // the new block.
     prev_block->truncateEOL(lastLine, lastData);
     m_totalLines--;
  }
  block->blockFill(lastLine, lastData, data, true);
  state = block->m_endState;
  m_totalLines += state.lineNr - startLine;
}

void
KWBuffer::slotLoadFile()
{
  loadFilePart();
  emit linesChanged(m_totalLines);
}
   
/**
 * Return the total number of lines in the buffer.
 */
int 
KWBuffer::count()
{
   return m_totalLines;
}

KWBufBlock *
KWBuffer::findBlock(int i)
{
   if ((i < 0) || (i >= m_totalLines))
      return 0;

   int lastLine = 0;
   // This needs a bit of optimisation/caching so that we don't walk 
   // through the list every time.
   KWBufBlock *buf;
   for(buf = m_blocks.current(); buf; )
   {
      lastLine = buf->m_endState.lineNr;
      if (i < buf->m_beginState.lineNr)
      {
         // Search backwards
         buf = m_blocks.prev();
      }
      else if ((i >= buf->m_beginState.lineNr) && (i < lastLine))
      {
         // We found the block.
         break;
      }
      else
      {
         // Search forwards
         buf = m_blocks.next();
         // Adjust line numbering....
         if (buf->m_beginState.lineNr != lastLine)
         {
            int offset = lastLine - buf->m_beginState.lineNr;
            buf->m_beginState.lineNr += offset;
            buf->m_endState.lineNr += offset;
         }
      }
   }   

   if (!buf)
   {
      // Huh? Strange, m_totalLines must have been out of sync?
      assert(lastLine == m_totalLines);
      assert(false);
      return 0;
   }
   return buf;
}
   
/**
 * Return line @p i
 */
TextLine::Ptr
KWBuffer::line(int i)
{
   KWBufBlock *buf = findBlock(i);
   if (!buf)
      return 0;

   if (!buf->b_stringListValid) 
   {
      parseBlock(buf);
   }
   return buf->line(i - buf->m_beginState.lineNr);
}

void
KWBuffer::insertLine(int i, TextLine::Ptr line)
{
   KWBufBlock *buf;
   if (i == m_totalLines)
      buf = findBlock(i-1);
   else
      buf = findBlock(i);

   if (!buf)
   {
      KWBufState state;
      // Initial state.
      state.lineNr = 0;
      buf = new KWBufBlock(state);
      m_blocks.insert(0, buf);
      buf->b_rawDataValid = true;
   }

   if (!buf->b_stringListValid) 
   {
      parseBlock(buf);
   }
   if (buf->b_rawDataValid)
   {
      dirtyBlock(buf);
   }
   buf->insertLine(i -  buf->m_beginState.lineNr, line);
   m_totalLines++;
}

void
KWBuffer::removeLine(int i)
{
   KWBufBlock *buf = findBlock(i);
   assert(buf);
   if (!buf->b_stringListValid) 
   {
      parseBlock(buf);
   }
   if (buf->b_rawDataValid)
   {
      dirtyBlock(buf);
   }
   buf->removeLine(i -  buf->m_beginState.lineNr);
   m_totalLines--;
}

void
KWBuffer::changeLine(int i)
{
qWarning("changeLine(%d)", i);
   KWBufBlock *buf = findBlock(i);
   assert(buf);
   assert(buf->b_stringListValid); 
   if (buf->b_rawDataValid)
   {
      dirtyBlock(buf);
   }
}

void
KWBuffer::parseBlock(KWBufBlock *buf)
{
qWarning("parseBlock(%p)", buf);
   if (!buf->b_rawDataValid)
      loadBlock(buf);
   if (m_parsedBlocksClean.count() > 5)
   {
      KWBufBlock *buf2 = m_parsedBlocksClean.take(2);
      buf2->disposeStringList();
   }
   buf->buildStringList();
   m_parsedBlocksClean.append(buf);
}

void
KWBuffer::loadBlock(KWBufBlock *buf)
{
qWarning("loadBlock(%p)", buf);
   if (m_loadedBlocks.count() > LOADED_BLOCKS_MAX)
   {
      KWBufBlock *buf2 = m_loadedBlocks.take(2);
qWarning("swapOut(%p)", buf2);
      buf2->swapOut(m_vm);
   }

qWarning("swapIn(%p)", buf);
   buf->swapIn(m_vm);
   m_parsedBlocksClean.append(buf);
   m_loadedBlocks.append(buf);
}

void
KWBuffer::dirtyBlock(KWBufBlock *buf)
{
qWarning("dirtyBlock(%p)", buf);
   buf->b_emptyBlock = false;
   if (m_parsedBlocksDirty.count() > DIRTY_BLOCKS_MAX)
   {
      KWBufBlock *buf2 = m_parsedBlocksDirty.take(0);
      buf2->flushStringList(); // Copy stringlist to raw
      buf2->disposeStringList(); // dispose stringlist.
      m_loadedBlocks.append(buf2);
   }
   m_loadedBlocks.removeRef(buf);
   m_parsedBlocksClean.removeRef(buf);
   m_parsedBlocksDirty.append(buf);
   buf->disposeRawData();
   if (buf->b_vmDataValid)
      buf->disposeSwap(m_vm);
}

//-----------------------------------------------------------------

/**
 * The KWBufBlock class contains an amount of data representing 
 * a certain number of lines.
 */

/*
 * Create an empty block.
 */
KWBufBlock::KWBufBlock(const KWBufState &beginState)
 : m_beginState(beginState), m_endState(beginState)
{
   m_rawData1Start = 0;
   m_rawData2End = 0;  
   m_rawSize = 0;
   m_vmblock = 0;
   b_stringListValid = false;
   b_rawDataValid = false;
   b_vmDataValid = false;
   b_appendEOL = false;
   b_emptyBlock = false;
   m_lastLine = 0;
}

/**
 * Remove the last line of this block.
 */
void
KWBufBlock::truncateEOL( int &lastLine, QByteArray &data1 )
{
   assert(b_appendEOL);
   assert(b_rawDataValid);      

   data1 = m_rawData2;
   lastLine = m_lastLine;
   b_appendEOL = false;
   m_rawData2End = m_lastLine;
   m_rawSize = m_rawData1.count() - m_rawData1Start + m_rawData2End;

   m_endState.lineNr--;
   if (b_stringListValid)
      m_stringList.remove(m_stringList.last());
}

/**
 * Fill block with lines from @p data1 and @p data2.
 * The first line starts at @p data1[@p dataStart].
 * If @p last is true, all bytes from @p data2 are stored.
 * @return The number of bytes stored form @p data2
 */
int 
KWBufBlock::blockFill(int dataStart, QByteArray data1, QByteArray data2, bool last)
{
   m_rawData1 = data1;
   m_rawData1Start = dataStart;
   m_rawData2 = data2;
  
   int lineNr = m_beginState.lineNr;

   const char *p;
   const char *e;
   QString lastLine;
   if (!m_rawData1.isEmpty())
   {
      p = m_rawData1.data() + m_rawData1Start;
      e = m_rawData1.data() + m_rawData1.count();
      while(p < e)
      {
         if (*p == '\n')
         {
            lineNr++;
         }
         p++;
      }
   }

   p = m_rawData2.data();
   e = m_rawData2.data() + m_rawData2.count();
   const char *l = 0;
   while(p < e)
   {
      if (*p == '\n')
      {
         lineNr++;
         l = p+1;
      }
      p++;
   }

   // If this is the end of the data OR 
   // if the data did not contain any linebreaks up to now
   // create a line break at the end of the block.
   if ((last && (e != l)) || 
       (l == 0))
   {
      if (!m_rawData1.isEmpty() || !m_rawData2.isEmpty())
      {
         b_appendEOL = true;
         if (l)
            m_lastLine = l - m_rawData2.data();
         else
            m_lastLine = 0;
         lineNr++;
      }
      l = e;
   }

   m_rawData2End = l - m_rawData2.data();
   m_endState.lineNr = lineNr;
qWarning("blockFill(%x) beginState = %d endState = %d", this, 
   m_beginState.lineNr, m_endState.lineNr);
   m_rawSize = m_rawData1.count() - m_rawData1Start + m_rawData2End;
   b_rawDataValid = true;
   return m_rawData2End;
}

/**
 * Swaps raw data to secondary storage.
 * Uses the filedescriptor @p swap_fd and the file-offset @p swap_offset
 * to store m_rawSize bytes.
 */
void 
KWBufBlock::swapOut(KVMAllocator *vm)
{
   assert(b_rawDataValid);
   // TODO: Error checking and reporting (?)
   if (!b_vmDataValid)
   {
      m_vmblock = vm->allocate(m_rawSize);
      off_t ofs = 0;
      if (!m_rawData1.isEmpty())
      {
         ofs = m_rawData1.count() - m_rawData1Start;
         vm->copy(m_vmblock, m_rawData1.data()+m_rawData1Start, 0, ofs);
      }
      if (!m_rawData2.isEmpty())
      {
         vm->copy(m_vmblock, m_rawData2.data(), ofs, m_rawData2End);
      }
      b_vmDataValid = true;
   }
   disposeRawData();
}

/**
 * Swaps m_rawSize bytes in from offset m_vmDataOffset in the file
 * with file-descirptor swap_fd.
 */
void 
KWBufBlock::swapIn(KVMAllocator *vm)
{
   assert(b_vmDataValid);
   assert(!b_rawDataValid);
   assert(m_vmblock);   

   m_rawData2.resize(m_rawSize);
   vm->copy(m_rawData2.data(), m_vmblock, 0, m_rawSize);
   m_rawData2End = m_rawSize;
   b_rawDataValid = true;
}


/**
 * Create a valid stringList.
 */
void 
KWBufBlock::buildStringList()
{
qWarning("KWBufBlock: buildStringList this = %p", this);
   assert(m_stringList.count() == 0);
   if (!m_codec && !m_rawData2.isEmpty())
   {
      buildStringListFast();
      return;
   }

   const char *p;
   const char *e;
   const char *l = 0; // Pointer to start of last line.
   QString lastLine;
   if (!m_rawData1.isEmpty())
   {
      p = m_rawData1.data() + m_rawData1Start;
      e = m_rawData1.data() + m_rawData1.count();
      l = p;
      while(p < e)
      {
         if (*p == '\n')
         {
            // TODO: Use codec
            QString line = m_codec->toUnicode(l, (p-l-1)+1);
            TextLine::Ptr textLine = new TextLine();
            textLine->append(line.unicode(), line.length());
            m_stringList.append(textLine);
            l = p+1;
         }
         p++;
      }
      if (l < e)
         lastLine = m_codec->toUnicode(l, (e-l)+1);
   }

   if (!m_rawData2.isEmpty())
   {
      p = m_rawData2.data();
      e = m_rawData2.data() + m_rawData2End;
      l = p;
      while(p < e)
      {
         if (*p == '\n')
         {
            QString line = m_codec->toUnicode(l, (p-l-1)+1);
            if (!lastLine.isEmpty())
            {
               line = lastLine + line;
               lastLine.truncate(0);
            }
            TextLine::Ptr textLine = new TextLine();
            textLine->append(line.unicode(), line.length());
            m_stringList.append(textLine);
            l = p+1;
         }
         p++;
      }

      // If this is the end of the data OR 
      // if the data did not contain any linebreaks up to now
      // create a line break at the end of the block.
      if (b_appendEOL)
      {
qWarning("KWBufBlock: buildStringList this = %p l = %p e = %p (e-l)+1 = %d",
	this, l, e, (e-l));
         QString line = m_codec->toUnicode(l, (e-l));
qWarning("KWBufBlock: line = '%s'", line.latin1());
         if (!lastLine.isEmpty())
         {
            line = lastLine + line;
            lastLine.truncate(0);
         }
         TextLine::Ptr textLine = new TextLine();
         textLine->append(line.unicode(), line.length());
         m_stringList.append(textLine);
      }
   }
   else 
   {
      if (b_appendEOL)
      {
         TextLine::Ptr textLine = new TextLine();
         m_stringList.append(textLine);
      }
   }
   assert((int) m_stringList.count() == (m_endState.lineNr - m_beginState.lineNr));
   m_stringListIt = m_stringList.begin();
   m_stringListCurrent = 0;
   b_stringListValid = true;
}

/**
 * Flush string list
 * Copies a string list back to the raw buffer.
 */
void
KWBufBlock::flushStringList()
{
qWarning("KWBufBlock: flushStringList this = %p", this);
   assert(b_stringListValid);
   assert(!b_rawDataValid);

   // Stores the data as lines of <lenght><length characters>
   // both <length> as well as <character> have size of sizeof(QChar)

   // Calculate size.
   int size = 0;
   for(TextLine::List::ConstIterator it = m_stringList.begin();
       it != m_stringList.end(); ++it)
   {
      size += (*it)->length()+1;
   }
qWarning("Size = %d", size);
   size = size*sizeof(QChar);
   m_rawData2 = QByteArray(size);
   m_rawData2End = size;
   m_rawSize = size;
   char *buf = m_rawData2.data();
   // Copy data
   for(TextLine::List::ConstIterator it = m_stringList.begin();
       it != m_stringList.end(); ++it)
   {
      ushort l = (*it)->length();
      QChar cSize(l);
      memcpy(buf, &cSize, sizeof(QChar));
      buf += sizeof(QChar);
      memcpy(buf, (char *)(*it)->getText(), sizeof(QChar)*l);
      buf += sizeof(QChar)*l;
   }
   assert(buf-m_rawData2.data() == size);
   m_codec = 0; // No codec
   b_rawDataValid = true;
}

/**
 * Create a valid stringList from raw data in our own format.
 */
void 
KWBufBlock::buildStringListFast()
{
qWarning("KWBufBlock: buildStringListFast this = %p", this);
   char *buf = m_rawData2.data();
   char *end = buf + m_rawSize;
   while(buf < end)
   {
      QChar cSize;
      memcpy((char *) &cSize, buf, sizeof(QChar));
      buf += sizeof(QChar);
      uint l = cSize.unicode();
      TextLine::Ptr textLine = new TextLine();
      textLine->append((QChar *) buf, l);
      buf += sizeof(QChar)*l;
      m_stringList.append(textLine);
   }
qWarning("stringList.count = %d should be %d", m_stringList.count(), m_endState.lineNr - m_beginState.lineNr);
   assert((int) m_stringList.count() == (m_endState.lineNr - m_beginState.lineNr));
   m_stringListIt = m_stringList.begin();
   m_stringListCurrent = 0;
   b_stringListValid = true;
}

/**
 * Dispose of a stringList.
 */
void
KWBufBlock::disposeStringList()
{
qWarning("KWBufBlock: disposeStringList this = %p", this);
   assert(b_rawDataValid || b_vmDataValid);
   m_stringList.clear();
   b_stringListValid = false;      
}

/**
 * Dispose of raw data.
 */
void
KWBufBlock::disposeRawData()
{
qWarning("KWBufBlock: disposeRawData this = %p", this);
   assert(b_stringListValid || b_vmDataValid);
   b_rawDataValid = false;
   m_rawData1 = QByteArray();
   m_rawData1Start = 0;
   m_rawData2 = QByteArray();
   m_rawData2End = 0;
}

/**
 * Dispose of data in vm
 */
void
KWBufBlock::disposeSwap(KVMAllocator *vm)
{
qWarning("KWBufBlock: disposeSwap this = %p", this);
   assert(b_stringListValid || b_rawDataValid);
   vm->free(m_vmblock);
   m_vmblock = 0;
   b_vmDataValid = false;
}

/**
 * Make line @p i the current line
 */
void KWBufBlock::seek(int i)
{
   if (m_stringListCurrent == i)
      return;
   while(m_stringListCurrent < i)
   {
      ++m_stringListCurrent;
      ++m_stringListIt;
   }
   while(m_stringListCurrent > i)
   {
      --m_stringListCurrent;
      --m_stringListIt;
   }
}

/**
 * Return line @p i
 * The first line of this block is line 0.
 */
TextLine::Ptr
KWBufBlock::line(int i)
{
   assert(b_stringListValid);
   assert(i < (int) m_stringList.count());
   seek(i);
   return *m_stringListIt;
}

void
KWBufBlock::insertLine(int i, TextLine::Ptr line)
{
   assert(b_stringListValid);
   assert(i <= (int) m_stringList.count());
   seek(i);
   m_stringListIt = m_stringList.insert(m_stringListIt, line);
   m_stringListCurrent = i;
   m_endState.lineNr++;
}

void
KWBufBlock::removeLine(int i)
{
   assert(b_stringListValid);
   assert(i < (int) m_stringList.count());
   seek(i);
   m_stringListIt = m_stringList.remove(m_stringListIt);  
   m_stringListCurrent = i;
   m_endState.lineNr--;
}

#include <kwbuffer.moc>
