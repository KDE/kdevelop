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
#ifndef KDEVPLATFORM_SIMPLECURSOR_H
#define KDEVPLATFORM_SIMPLECURSOR_H

#include <ktexteditor/cursor.h>
#include "languageexport.h"

namespace KDevelop {

/**
 * Represents a cursor (line-number and column-number) within a text document. Generally this is
 * a more efficient version of KTextEditor::Cursor.
 * In KDevelop, this object is used when referencing the most current document revision
 * (the document in its current version)
 */

class KDEVPLATFORMLANGUAGE_EXPORT SimpleCursor {
public:

 int line, column;

 SimpleCursor() : line(0), column(0) {
 }

 SimpleCursor(int _line, int _column) : line(_line), column(_column) {
 }

 static SimpleCursor invalid() {
     return SimpleCursor(-1, -1);
 }

 explicit SimpleCursor(const KTextEditor::Cursor& cursor) : line(cursor.line()), column(cursor.column()) {
 }
 
 bool isValid() const {
     return line != -1 || column != -1;
 }

 bool operator<(const SimpleCursor& rhs) const {
     return line < rhs.line || (line == rhs.line && column < rhs.column);
 }
 
 bool operator<=(const SimpleCursor& rhs) const {
     return line < rhs.line || (line == rhs.line && column <= rhs.column);
 }

 bool operator>(const SimpleCursor& rhs) const {
     return line > rhs.line || (line == rhs.line && column > rhs.column);
 }
 
 bool operator>=(const SimpleCursor& rhs) const {
     return line > rhs.line || (line == rhs.line && column >= rhs.column);
 }

 bool operator ==(const SimpleCursor& rhs) const {
     return line == rhs.line && column == rhs.column;
 }
 
 bool operator !=(const SimpleCursor& rhs) const {
     return !(*this == rhs);
 }

 KTextEditor::Cursor textCursor() const {
     return KTextEditor::Cursor(line, column);
 }

 SimpleCursor operator +(const SimpleCursor& rhs) const {
     return SimpleCursor(line + rhs.line, column + rhs.column);
 }

 /**
  * kDebug() stream operator.  Writes this cursor to the debug output in a nicely formatted way.
  */
 inline friend QDebug operator<< (QDebug s, const SimpleCursor& cursor) {
     s.nospace() << "(" << cursor.line << ", " << cursor.column << ")";
     return s.space();
 }
};

inline uint qHash(const KDevelop::SimpleCursor& cursor) {
    return cursor.line * 53 + cursor.column * 47;
}

} // namespace KDevelop

Q_DECLARE_TYPEINFO(KDevelop::SimpleCursor, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KDevelop::SimpleCursor)

#endif
