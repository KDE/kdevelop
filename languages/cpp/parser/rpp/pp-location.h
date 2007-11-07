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

#include <KTextEditor/Cursor>

#include <cppparserexport.h>

namespace rpp {

class KDEVCPPRPP_EXPORT LocationTable
{
  public:
    LocationTable();

    /// Generates the location table from the byte array's contents
    LocationTable(const QByteArray& contents);

    void anchor(std::size_t offset, KTextEditor::Cursor cursor);

    KTextEditor::Cursor positionForOffset(std::size_t offset) const;

    void dump() const;
    
  private:
    QMap<std::size_t, KTextEditor::Cursor> m_offsetTable;
    mutable QMap<std::size_t, KTextEditor::Cursor>::ConstIterator m_currentOffset;
};

}

#endif // PP_LOCATION_H
