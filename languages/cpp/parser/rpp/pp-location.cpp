/*
  Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#include <pp-location.h>

#include <kdebug.h>

using namespace rpp;

LocationTable::LocationTable()
{
  anchor(0, KDevelop::SimpleCursor(0,0));
}

LocationTable::LocationTable(const QByteArray& contents)
{
  anchor(0, KDevelop::SimpleCursor(0,0));

  const QChar newline = '\n';
  int line = 0;

  for (std::size_t i = 0; i < contents.size(); ++i)
    if (contents.at(i) == newline)
      anchor(i + 1, KDevelop::SimpleCursor(++line, 0));
}

void LocationTable::anchor(std::size_t offset, KDevelop::SimpleCursor cursor)
{
  if (cursor.column) {
    // Check to see if it's different to what we already know
    if (positionForOffset(offset) == cursor)
      return;
  }
  
  m_currentOffset = m_offsetTable.insert(offset, cursor);
}

KDevelop::SimpleCursor LocationTable::positionForOffset(std::size_t offset) const
{
  // Look nearby for a match first
  QMap<std::size_t, KDevelop::SimpleCursor>::ConstIterator constEnd = m_offsetTable.constEnd();

  if (m_currentOffset != constEnd) {
    std::size_t current = m_currentOffset.key();
    bool checkForwards = (current < offset);
    // TODO check for optimal number of iterations
    for (int i = 0; i < 5; ++i) {
      if (checkForwards) {
        if (m_currentOffset + 1 == constEnd)
          // Special case
          goto done;

        ++m_currentOffset;
        if (m_currentOffset != constEnd) {
          if (m_currentOffset.key() > offset) {
            // Gone forwards too much, but one back is correct
            --m_currentOffset;
            goto done;
          }

        } else {
          break;
        }

      } else {
        if (m_currentOffset == m_offsetTable.constBegin())
          // Special case
          goto done;

        ++m_currentOffset;
        if (m_currentOffset != constEnd) {
          if (m_currentOffset.key() < offset) {
            // Correct position :)
            goto done;
          }
        } else {
          break;
        }
      }
    }
  }

  m_currentOffset = m_offsetTable.lowerBound(offset);
  //kDebug() << k_funcinfo << offset << "found" << m_currentOffset.key();
  if (m_currentOffset == constEnd)
    --m_currentOffset;
  
  if (m_currentOffset.key() > offset)
    --m_currentOffset;

  done:
  Q_ASSERT(m_currentOffset != constEnd);
  return m_currentOffset.value() + KDevelop::SimpleCursor(0, offset - m_currentOffset.key());
}

void LocationTable::dump() const
{
  QMapIterator<std::size_t, KDevelop::SimpleCursor> it = m_offsetTable;
  kDebug(9007) << "Location Table:";
  while (it.hasNext()) {
    it.next();
    kDebug(9007) << it.key() << " => " << it.value().textCursor();
  }
}
