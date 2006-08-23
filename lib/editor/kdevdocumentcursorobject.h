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

#ifndef KDEVDOCUMENTCURSOROBJECT_H
#define KDEVDOCUMENTCURSOROBJECT_H

#include <ktexteditor/cursor.h>
#include <ktexteditor/cursorfeedback.h>

#include "kdevdocumentcursor.h"

/**
 * Base class for any object which has an associated range of text.
 *
 * This allows text without a currently loaded text editor to be represented.
 */
class KDEVINTERFACES_EXPORT KDevDocumentCursorObject : public KTextEditor::SmartCursorWatcher
{
public:
  KDevDocumentCursorObject(KTextEditor::Cursor* cursor);
  virtual ~KDevDocumentCursorObject();

  void setTextCursor(KTextEditor::Cursor* cursor);

  KTextEditor::Cursor& textCursor();
  const KTextEditor::Cursor& textCursor() const;
  const KDevDocumentCursor& textDocCursor() const;
  KTextEditor::Cursor* textCursorPtr() const;
  KTextEditor::SmartCursor* smartCursor() const;

  KUrl url() const;
  static KUrl url(const KTextEditor::Cursor* range);

  virtual void deleted(KTextEditor::SmartCursor* cursor);

private:
  KTextEditor::Cursor* m_cursor;
  KUrl m_url;
};

#endif // KDEVDOCUMENTCURSOROBJECT_H

// kate: indent-width 2;
