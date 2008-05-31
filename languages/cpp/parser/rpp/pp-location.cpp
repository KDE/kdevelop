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
#include <QStringList>
#include <kdebug.h>

using namespace rpp;

LocationTable::LocationTable()
{
  anchor(0, Anchor(0,0));
}

LocationTable::LocationTable(const QByteArray& contents)
{
  anchor(0, Anchor(0,0));

  const char newline = '\n';
  int line = 0;

  for (std::size_t i = 0; i < (std::size_t)contents.size(); ++i)
    if (contents.at(i) == newline)
      anchor(i + 1, Anchor(++line, 0));
}

void LocationTable::anchor(std::size_t offset, Anchor anchor)
{
  if (anchor.column) {
    // Check to see if it's different to what we already know
    rpp::Anchor known = positionForOffset(offset);
    if (known == anchor && !anchor.collapsed && known.macroExpansion == anchor.macroExpansion)
      return;
  }
  
  m_currentOffset = m_offsetTable.insert(offset, anchor);
}

Anchor LocationTable::positionForOffset(std::size_t offset, bool collapseIfMacroExpansion) const
{
  // Look nearby for a match first
  QMap<std::size_t, Anchor>::ConstIterator constEnd = m_offsetTable.constEnd();

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
  Anchor ret = m_currentOffset.value();
  if(ret.macroExpansion.isValid() && collapseIfMacroExpansion) {
    ret.collapsed = true;
    return ret;
  }
  if(ret.collapsed)
    return ret;
  else {
    return Anchor(ret+ KDevelop::SimpleCursor(0, offset - m_currentOffset.key()), ret.collapsed, ret.macroExpansion);
  }
}

void LocationTable::dump() const
{
  QMapIterator<std::size_t, Anchor> it = m_offsetTable;
  kDebug(9007) << "Location Table:";
  while (it.hasNext()) {
    it.next();
    kDebug(9007) << it.key() << " => " << it.value().textCursor();
  }
}

void LocationTable::splitByAnchors(const QByteArray& text, const Anchor& textStartPosition, QList<QByteArray>& strings, QList<Anchor>& anchors) const {

  Anchor currentAnchor = Anchor(textStartPosition);
  size_t currentOffset = 0;
  
  QMapIterator<std::size_t, Anchor> it = m_offsetTable;

  while (currentOffset < (size_t)text.length())
  {
    Anchor nextAnchor(KDevelop::SimpleCursor::invalid());
    size_t nextOffset;
    
    if(it.hasNext()) {
      it.next();
      nextOffset = it.key();
      nextAnchor = it.value();
    }else{
      nextOffset = text.length();
      nextAnchor = Anchor(KDevelop::SimpleCursor::invalid());
    }

    if( nextOffset-currentOffset > 0 ) {
      strings.append(text.mid(currentOffset, nextOffset-currentOffset));
      anchors.append(currentAnchor);
    }
    
    currentOffset = nextOffset;
    currentAnchor = nextAnchor;
  }
}
