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

#include "kdevdocumentcursorobject.h"

#include <ktexteditor/smartcursor.h>
#include <ktexteditor/document.h>

using namespace KTextEditor;

KDevDocumentCursorObject::KDevDocumentCursorObject(Cursor* cursor)
  : m_cursor(0)
{
  setTextCursor(cursor);
}

KDevDocumentCursorObject::~ KDevDocumentCursorObject( )
{
  if (m_cursor->isSmartCursor())
    m_cursor->toSmartCursor()->setWatcher(0);

  delete m_cursor;
}

void KDevDocumentCursorObject::setTextCursor( Cursor * cursor )
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

const Cursor& KDevDocumentCursorObject::textCursor( ) const
{
  return *m_cursor;
}

Cursor& KDevDocumentCursorObject::textCursor( )
{
  return *m_cursor;
}

const KDevDocumentCursor& KDevDocumentCursorObject::textDocCursor() const
{
  return *static_cast<KDevDocumentCursor*>(m_cursor);
}

KUrl KDevDocumentCursorObject::url() const
{
  return url(m_cursor);
}

KUrl KDevDocumentCursorObject::url( const Cursor * cursor )
{
  if (cursor->isSmartCursor())
    return static_cast<const SmartCursor*>(cursor)->document()->url();
  else
    return static_cast<const KDevDocumentCursor*>(cursor)->document();
}

SmartCursor* KDevDocumentCursorObject::smartCursor() const
{
  if (m_cursor->isSmartCursor())
    return static_cast<SmartCursor*>(m_cursor);

  return 0L;
}

Cursor* KDevDocumentCursorObject::textCursorPtr() const
{
  return m_cursor;
}

// kate: indent-width 2;

void KDevDocumentCursorObject::deleted(KTextEditor::SmartCursor * cursor)
{
  Q_ASSERT(cursor == m_cursor);
  //Q_ASSERT(false);
  m_cursor = new KDevDocumentCursor(m_url, *m_cursor);
}
