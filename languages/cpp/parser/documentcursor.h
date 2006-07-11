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

#ifndef DOCUMENTCURSOR_H
#define DOCUMENTCURSOR_H

#include <kurl.h>

#include <ktexteditor/cursor.h>

namespace KTextEditor { class SmartCursor; }

/**
 * Extends KTextEditor::Range with information about the URL to which the range
 * refers.
 */
class DocumentCursor : public KTextEditor::Cursor
{
public:
  DocumentCursor(const KUrl& document, const KTextEditor::Cursor& cursor = KTextEditor::Cursor::invalid());

  enum Position {
    Start,
    End
  };

  /// Constructor for information extraction only, does not take ownership of the cursor.
  /// \a range must be either a DocumentRange or a KTextEditor::SmartRange.
  DocumentCursor(KTextEditor::Range* range, Position position);

  /// Constructor for information extraction only, does not take ownership of the cursor.
  DocumentCursor(KTextEditor::SmartCursor* cursor);

  DocumentCursor(const DocumentCursor& copy);

  /// Returns the associated document.
  const KUrl& document() const;

  /// Sets the associated document.
  void setDocument(const KUrl& document);

private:
  KUrl m_document;
};

#endif // DOCUMENTRANGE_H

// kate: indent-width 2;
