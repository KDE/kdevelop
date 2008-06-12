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

#include "documentcursorobject.h"

#include <ktexteditor/smartcursor.h>
#include <ktexteditor/document.h>
#include "hashedstring.h"

using namespace KTextEditor;

namespace KDevelop
{

class DocumentCursorObjectPrivate
{
public:
    DocumentCursorObjectPrivate() : m_cursor(0) {}
    KTextEditor::Cursor* m_cursor;
    HashedString m_url;
};

DocumentCursorObject::DocumentCursorObject(KTextEditor::Cursor* cursor)
    : d( new DocumentCursorObjectPrivate )
{
    setTextCursor(cursor);
}

DocumentCursorObject::~DocumentCursorObject( )
{
    if (d->m_cursor->isSmartCursor())
        d->m_cursor->toSmartCursor()->setWatcher(0);

    delete d->m_cursor;
    delete d;
}

void DocumentCursorObject::setTextCursor( KTextEditor::Cursor * cursor )
{
    Q_ASSERT(cursor);

    if (d->m_cursor == cursor)
        return;

    if (d->m_cursor)
    {
        // TODO.. overkill???
        if (d->m_cursor->isSmartCursor())
            d->m_cursor->toSmartCursor()->setWatcher(0);

        delete d->m_cursor;
    }

    d->m_cursor = cursor;

    if (d->m_cursor->isSmartCursor())
    {
        d->m_cursor->toSmartCursor()->setWatcher(this);
        d->m_url = url(d->m_cursor);
    }
}

const Cursor& DocumentCursorObject::textCursor( ) const
{
    return *d->m_cursor;
}

Cursor& DocumentCursorObject::textCursor( )
{
    return *d->m_cursor;
}

const DocumentCursor& DocumentCursorObject::textDocCursor() const
{
    return *static_cast<DocumentCursor*>(d->m_cursor);
}

HashedString DocumentCursorObject::url() const
{
    return url(d->m_cursor);
}

HashedString DocumentCursorObject::url( const KTextEditor::Cursor * cursor )
{
    if (cursor->isSmartCursor()) ///@todo this conversion is bad
        return static_cast<const SmartCursor*>(cursor)->document()->url().pathOrUrl();
    else
        return static_cast<const DocumentCursor*>(cursor)->document();
}

SmartCursor* DocumentCursorObject::smartCursor() const
{
    if (d->m_cursor->isSmartCursor())
        return static_cast<SmartCursor*>(d->m_cursor);

    return 0L;
}

Cursor* DocumentCursorObject::textCursorPtr() const
{
    return d->m_cursor;
}

void DocumentCursorObject::deleted(KTextEditor::SmartCursor * cursor)
{
    Q_ASSERT(cursor == d->m_cursor);
    //Q_ASSERT(false);
    d->m_cursor = new DocumentCursor(d->m_url, *d->m_cursor);
}

}

