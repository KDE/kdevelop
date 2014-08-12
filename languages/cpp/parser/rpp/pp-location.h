/*
  Copyright 2007 Hamish Rodda <rodda@kde.org>
  Copyright 2008-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "cpprppexport.h"
#include "anchor.h"

typedef QVector<unsigned int> PreprocessedContents;

namespace rpp {

class KDEVCPPRPP_EXPORT LocationTable
{
  public:
    LocationTable();

    /// Generates the location table from the contents
    LocationTable(const PreprocessedContents& contents);

    ///@param contents is allowed to be zero only if offset is zero, or if anchor.column is zero.
    void anchor(std::size_t offset, Anchor anchor, const PreprocessedContents* contents);

    /**
    * Return the position of the preprocessed source \a offset in the original source
    * If the "collapsed" member of the returned anchor is true, the position is within a collapsed range.
    @param collapseIfMacroExpansion @see LocationTable::positionForOffset
    * \note the return line starts from 0, not 1.
    * Returns the found position stored in the anchor, and the possible maximum length until the next anchored position, or zero.
    */
    QPair<rpp::Anchor, uint> positionAt(std::size_t offset, const PreprocessedContents& contents, bool collapseIfMacroExpansion = false) const;

    struct AnchorInTable {
      uint position; //Position of this anchor
      Anchor anchor; //This anchor
      uint nextPosition;//Position of the next following anchor, or zero
      Anchor nextAnchor;//The next following anchor
      bool operator==(const AnchorInTable& other) const;
    };

    /**
      * Returns the nearest anchor before @param position, and the position of the anchor.
     * If the character is in a collapsed range, the collapsed member is true.
      @param collapseIfMacroExpansion When this is true, all ranges that come from macro-expansion will be
                                      considered collapsed.(The returned anchor will also have the collapsed member set)
      * */
    AnchorInTable anchorForOffset(std::size_t position, bool collapseIfMacroExpansion = false) const;

    void dump() const;

    /**
     * Splits the given @param text using this location-table into sub-strings each assigned to a cursor where it starts.
     * @param textStartPosition must be given as the start-position, because the location-table might not contain an anchor
     * for the first character.
    * */
    void splitByAnchors(const PreprocessedContents& text, const Anchor& textStartPosition, QList<PreprocessedContents>& strings, QList<Anchor>& anchors) const;

  private:
    typedef QMap<std::size_t, Anchor> OffsetTable;
    OffsetTable m_offsetTable;
    mutable OffsetTable::ConstIterator m_currentOffset;
    //cache for positionAt
    mutable AnchorInTable m_lastAnchorInTable;
    mutable int m_positionAtColumnCache;
    mutable std::size_t m_positionAtLastOffset;
};

}

#endif // PP_LOCATION_H
