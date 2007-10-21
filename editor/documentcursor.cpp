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

#include "documentcursor.h"

#include <ktexteditor/smartcursor.h>
#include <ktexteditor/document.h>

#include "documentrangeobject.h"

using namespace KTextEditor;

namespace KDevelop
{

class DocumentCursorPrivate
{
public:
    DocumentCursorPrivate( const KUrl& document = KUrl() ) : m_document(document)
    {}
    KUrl m_document;
};

DocumentCursor::DocumentCursor(const KUrl& document, const KTextEditor::Cursor& cursor)
    : Cursor(cursor)
    ,d( new DocumentCursorPrivate( document ) )
{
}

DocumentCursor::DocumentCursor(KTextEditor::Range* range, Position position )
    : Cursor(position == Start ? range->start() : range->end())
    ,d( new DocumentCursorPrivate( DocumentRangeObject::url(range) ) )
{
}

DocumentCursor::DocumentCursor(KTextEditor::Cursor* cursor)
    : Cursor(*cursor)
    ,d( new DocumentCursorPrivate )
{
    if (cursor->isSmartCursor())
        d->m_document = cursor->toSmartCursor()->document()->url();
    else
        d->m_document = static_cast<DocumentCursor*>(cursor)->document();
}

DocumentCursor::DocumentCursor(const DocumentCursor& copy)
    : Cursor(copy)
    , d( new DocumentCursorPrivate( copy.document() ) )
{
}

DocumentCursor::~DocumentCursor()
{
    delete d;
}

const KUrl& DocumentCursor::document() const
{
    return d->m_document;
}

void DocumentCursor::setDocument(const KUrl& document)
{
    d->m_document = document;
}

}
