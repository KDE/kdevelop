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

#ifndef DOCUMENTRANGEOBJECT_P
#define DOCUMENTRANGEOBJECT_P

#include "documentrangeobject.h"
#include <ktexteditor/smartrange.h>
#include "simplerange.h"

namespace KDevelop
{

class DocumentRangeObjectPrivate
{
    public:
    DocumentRangeObjectPrivate() : m_smartRange(0)
        , m_ownsRange(DocumentRangeObject::Own)
    {}
    DocumentRangeObjectPrivate(const DocumentRangeObjectPrivate& rhs);

    mutable KTextEditor::SmartRange* m_smartRange; //Mutable for synchronization
    DocumentRangeObject::RangeOwning m_ownsRange;
    mutable SimpleRange m_range; //Mutable for synchronization
    HashedString m_document; ///@todo get rid of this, the information can be gotten from elsewhere

    void syncFromSmart() const;
    void syncToSmart() const;
};
}

#endif
