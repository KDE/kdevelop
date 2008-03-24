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

#ifndef KDEVDOCUMENTCURSOR_H
#define KDEVDOCUMENTCURSOR_H

#include "editorexport.h"

#include <kurl.h>

#include <ktexteditor/cursor.h>

namespace KTextEditor { class SmartCursor; }

namespace KDevelop
{
class HashedString;

/**
 * Extends KTextEditor::Range with information about the URL to which the range
 * refers.
 *
 * \todo override comparison operators and take them into account
 */
class KDEVPLATFORMEDITOR_EXPORT DocumentCursor : public KTextEditor::Cursor
{
public:
    explicit DocumentCursor(const HashedString& document, const KTextEditor::Cursor& cursor = KTextEditor::Cursor::invalid());
    ~DocumentCursor();

    enum Position {
        Start /**< cursor is at the start of the range */ ,
        End /**< cursor is at the end of the range */
    };

    /// Constructor for information extraction only, does not take ownership of the cursor.
    /// \a range must be either a DocumentRange or a KTextEditor::SmartRange.
    DocumentCursor(const HashedString& document, KTextEditor::Range* range, Position position);

    /// Constructor for information extraction only, does not take ownership of the cursor.
    /// \a cursor must be either a DocumentCursor or a KTextEditor::SmartCursor.
    DocumentCursor(KTextEditor::Cursor* cursor);

    DocumentCursor(const DocumentCursor& copy);
    DocumentCursor();

    /// Returns the associated document.
    const HashedString& document() const;

    /// Sets the associated document. Should be formatted from an url using prettyUrl()
    void setDocument(const HashedString& document);

    DocumentCursor& operator=(const DocumentCursor& rhs);

private:
    class DocumentCursorPrivate* const d;
};

}
#endif // DOCUMENTRANGE_H

