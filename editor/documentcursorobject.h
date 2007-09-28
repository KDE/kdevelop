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

#ifndef KDEVDOCUMENTCURSOROBJECT_H
#define KDEVDOCUMENTCURSOROBJECT_H

#include <ktexteditor/cursor.h>
#include <ktexteditor/cursorfeedback.h>

#include <documentcursor.h>

namespace KDevelop
{

/**
 * Base class for any object which has an associated range of text.
 *
 * This allows text without a currently loaded text editor to be represented.
 */
class KDEVPLATFORMEDITOR_EXPORT DocumentCursorObject : public KTextEditor::SmartCursorWatcher
{
public:
    DocumentCursorObject(KTextEditor::Cursor* cursor);
    virtual ~DocumentCursorObject();

    void setTextCursor(KTextEditor::Cursor* cursor);

    KTextEditor::Cursor& textCursor();
    const KTextEditor::Cursor& textCursor() const;
    const DocumentCursor& textDocCursor() const;
    KTextEditor::Cursor* textCursorPtr() const;
    KTextEditor::SmartCursor* smartCursor() const;

    KUrl url() const;
    static KUrl url(const KTextEditor::Cursor* range);

    virtual void deleted(KTextEditor::SmartCursor* cursor);

private:
    class DocumentCursorObjectPrivate* const d;
};

}
#endif // KDEVDOCUMENTCURSOROBJECT_H

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on

