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

#ifndef KDEVPLATFORM_DOCUMENTRANGE_H
#define KDEVPLATFORM_DOCUMENTRANGE_H

#include "../languageexport.h"
#include "simplerange.h"
#include <language/duchain/indexedstring.h>
#include "rangeinrevision.h"

namespace KDevelop
{
class IndexedString;

/**
 * Lightweight object that extends a range with information about the URL to which the range refers.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DocumentRange : public SimpleRange
{
public:
    DocumentRange() {
    }
  
    inline DocumentRange(const IndexedString& document, const SimpleRange& range)  : SimpleRange(range), document(document) {
    }

    inline bool operator==(const DocumentRange& rhs) const {
      return document == rhs.document && SimpleRange::operator==(rhs);
    }

    static DocumentRange invalid() {
      return DocumentRange(IndexedString(), SimpleRange::invalid());
    }

    IndexedString document;
};

}
#endif // KDEVPLATFORM_DOCUMENTRANGE_H


