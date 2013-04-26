/* This file is part of KDevelop
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_DOCUMENTCURSOR_H
#define KDEVPLATFORM_DOCUMENTCURSOR_H

#include "../languageexport.h"
#include "simplecursor.h"
#include <language/duchain/indexedstring.h>
#include "cursorinrevision.h"

namespace KDevelop
{
/**
 * Lightweight object that extends a cursor with information about the document URL to which the range
 * refers.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DocumentCursor : public SimpleCursor
{
public:
    DocumentCursor() {
    }
    
    DocumentCursor(const IndexedString& document, const SimpleCursor& cursor) : SimpleCursor(cursor), document(document) {
    }

    inline bool operator==(const DocumentCursor& rhs) const {
      return document == rhs.document && SimpleCursor::operator==(rhs);
    }

    IndexedString document;
};

}
#endif // KDEVPLATFORM_DOCUMENTCURSOR_H

