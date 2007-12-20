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
#ifndef SIMPLECURSOR_H
#define SIMPLECURSOR_H

#include <ktexteditor/cursor.h>
#include "editorexport.h"

namespace KDevelop {
struct KDEVPLATFORMEDITOR_EXPORT SimpleCursor {

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
};
}

#endif
