/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_DOCUMENTCURSOR_H
#define KDEVPLATFORM_DOCUMENTCURSOR_H

#include <language/languageexport.h>
#include <serialization/indexedstring.h>

#include <KTextEditor/Cursor>

namespace KDevelop {
/**
 * Lightweight object that extends a cursor with information about the document URL to which the range
 * refers.
 */
class DocumentCursor
    : public KTextEditor::Cursor
{
public:
    DocumentCursor()
    {
    }

    DocumentCursor(const IndexedString& document, const KTextEditor::Cursor& cursor) : KTextEditor::Cursor(cursor)
        , document(document)
    {
    }

    static DocumentCursor invalid()
    {
        return DocumentCursor({}, KTextEditor::Cursor::invalid());
    }

    inline bool operator==(const DocumentCursor& rhs) const
    {
        return document == rhs.document && *static_cast<const KTextEditor::Cursor*>(this) == rhs;
    }

    IndexedString document;
};
}
#endif // KDEVPLATFORM_DOCUMENTCURSOR_H
