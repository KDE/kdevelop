/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

using namespace KTextEditor;

namespace KDevelop
{

DocumentCursorObject::DocumentCursorObject(Cursor* cursor)
  : m_cursor(0)
{
  setTextCursor(cursor);
}

DocumentCursorObject::~ DocumentCursorObject( )
{
  if (m_cursor->isSmartCursor())
    m_cursor->toSmartCursor()->setWatcher(0);

  delete m_cursor;
}

void DocumentCursorObject::setTextCursor( Cursor * cursor )
{
  Q_ASSERT(cursor);

  if (m_cursor == cursor)
    return;

  if (m_cursor) {
    // TODO.. overkill???
    if (m_cursor->isSmartCursor())
      m_cursor->toSmartCursor()->setWatcher(0);

    delete m_cursor;
  }

  m_cursor = cursor;

  if (m_cursor->isSmartCursor()) {
    m_cursor->toSmartCursor()->setWatcher(this);
    m_url = url(m_cursor);
  }
}

const Cursor& DocumentCursorObject::textCursor( ) const
{
  return *m_cursor;
}

Cursor& DocumentCursorObject::textCursor( )
{
  return *m_cursor;
}

const DocumentCursor& DocumentCursorObject::textDocCursor() const
{
  return *static_cast<DocumentCursor*>(m_cursor);
}

KUrl DocumentCursorObject::url() const
{
  return url(m_cursor);
}

KUrl DocumentCursorObject::url( const Cursor * cursor )
{
  if (cursor->isSmartCursor())
    return static_cast<const SmartCursor*>(cursor)->document()->url();
  else
    return static_cast<const DocumentCursor*>(cursor)->document();
}

SmartCursor* DocumentCursorObject::smartCursor() const
{
  if (m_cursor->isSmartCursor())
    return static_cast<SmartCursor*>(m_cursor);

  return 0L;
}

Cursor* DocumentCursorObject::textCursorPtr() const
{
  return m_cursor;
}

// kate: indent-width 2;

void DocumentCursorObject::deleted(KTextEditor::SmartCursor * cursor)
{
  Q_ASSERT(cursor == m_cursor);
  //Q_ASSERT(false);
  m_cursor = new DocumentCursor(m_url, *m_cursor);
}

}
