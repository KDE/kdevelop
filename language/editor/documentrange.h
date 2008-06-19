/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVDOCUMENTRANGE_H
#define KDEVDOCUMENTRANGE_H

#include "../languageexport.h"

#include <kurl.h>

#include <ktexteditor/range.h>

namespace KDevelop
{
class HashedString;

/**
 * Extends KTextEditor::Range with information about the URL to which the range
 * refers.
 *
 * \todo parent/child relationships here too?
 */
class KDEVPLATFORMLANGUAGE_EXPORT DocumentRange : public KTextEditor::Range
{
public:
    DocumentRange();
    DocumentRange(const HashedString& document, const KTextEditor::Cursor& start, const KTextEditor::Cursor& end, KTextEditor::Range* parent = 0);
    explicit DocumentRange(const HashedString& document, const KTextEditor::Range& range = KTextEditor::Range::invalid(), KTextEditor::Range* parent = 0);
    DocumentRange(const DocumentRange& copy);
    virtual ~DocumentRange();

    /// Returns the associated document.
    const HashedString& document() const;

    /// Sets the associated document.
    void setDocument(const HashedString& document);

    KTextEditor::Range* parentRange() const;
    void setParentRange(KTextEditor::Range* parent);

    const QList<DocumentRange*>& childRanges() const;

    DocumentRange& operator=(const DocumentRange& rhs);

private:
    class DocumentRangePrivate* const d;
};

}
#endif // DOCUMENTRANGE_H


