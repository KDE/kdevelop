/* This file is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_SIMPLERANGE_H
#define KDEVPLATFORM_SIMPLERANGE_H

#include <ktexteditor/range.h>

#include <language/languageexport.h>
#include "simplecursor.h"

/**
 * Represents a range (start- and end-cursor) within a text document. Generally this is
 * a more efficient version of KTextEditor::Range.
 * In KDevelop, this object is used when referencing the most current document revision
 * (the document in its current version)
 *
 * TODO KF5: Just use KTextEditor::Range here? Cursors are now simple members just like here
 */

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT SimpleRange {
public:

    SimpleCursor start, end;

    SimpleRange(const SimpleCursor& _start, const SimpleCursor& _end) : start(_start), end(_end) {
    }

    SimpleRange(const SimpleCursor& _start, int length) : start(_start), end(_start.line, _start.column + length) {
    }

    SimpleRange() {
    }

    SimpleRange(const KTextEditor::Range& range) : start(range.start()), end(range.end()) {
    }

    SimpleRange(int sLine, int sCol, int eLine, int eCol) : start(sLine, sCol), end(eLine, eCol) {
    }

    static SimpleRange invalid() {
      return SimpleRange(-1, -1, -1, -1);
    }

    bool isValid() const {
        return start.column != -1 || start.line != -1 || end.column != -1 || end.line != -1;
    }

    bool isEmpty() const {
        return start == end;
    }

    bool contains(const SimpleCursor& position) const {
        return position >= start && position < end;
    }

    bool contains(const SimpleRange& range) const {
        return range.start >= start && range.end <= end;
    }

    bool operator ==( const SimpleRange& rhs ) const {
        return start == rhs.start && end == rhs.end;
    }

    bool operator !=( const SimpleRange& rhs ) const {
        return !(*this == rhs);
    }

    bool operator <( const SimpleRange& rhs ) const {
        return start < rhs.start;
    }

    KTextEditor::Range textRange() const {
        return KTextEditor::Range( KTextEditor::Cursor(start.line, start.column), KTextEditor::Cursor(end.line, end.column) );
    }

    /**
     * kDebug() stream operator.  Writes this range to the debug output in a nicely formatted way.
     */
    inline friend QDebug operator<< (QDebug s, const SimpleRange& range) {
      s.nospace() << '[' << range.start << ", " << range.end << ']';
      return s.space();
    }
};

inline uint qHash(const KDevelop::SimpleRange& range) {
    return qHash(range.start) + qHash(range.end)*41;
}

} // namespace KDevelop

Q_DECLARE_TYPEINFO(KDevelop::SimpleRange, Q_MOVABLE_TYPE);

#endif
