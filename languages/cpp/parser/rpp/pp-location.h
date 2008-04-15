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

#ifndef PP_LOCATION_H
#define PP_LOCATION_H

#include <QMap>

#include <editor/simplecursor.h>

#include <cppparserexport.h>
#include "anchor.h"

namespace rpp {

class KDEVCPPRPP_EXPORT LocationTable
{
  public:
    LocationTable();

    /// Generates the location table from the byte array's contents
    LocationTable(const QByteArray& contents);

    void anchor(std::size_t offset, Anchor anchor);

    /** @todo Correctly respect utf-8 characters. They may have the byte-length of 2, but actually are one character.
      * Returns the marked position for the character at @param offset. If the character is in a collapsed range, the collapsed member is true.
      * */
    Anchor positionForOffset(std::size_t offset) const;

    void dump() const;

    /**
     * Splits the given @param text using this location-table into sub-strings each assigned to a cursor where it starts.
     * @param textStartPosition must be given as the start-position, because the location-table might not contain an anchor
     * for the first character.
    * */
    void splitByAnchors(const QByteArray& text, const Anchor& textStartPosition, QList<QByteArray>& strings, QList<Anchor>& anchors) const;
  
  private:
    QMap<std::size_t, Anchor> m_offsetTable;
    mutable QMap<std::size_t, Anchor>::ConstIterator m_currentOffset;
};

}

#endif // PP_LOCATION_H
