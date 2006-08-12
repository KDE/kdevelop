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

#ifndef KDEVDOCUMENTCURSOR_H
#define KDEVDOCUMENTCURSOR_H

#include "kdevexport.h"

#include <kurl.h>

#include <ktexteditor/cursor.h>

namespace KTextEditor { class SmartCursor; }

/**
 * Extends KTextEditor::Range with information about the URL to which the range
 * refers.
 *
 * \todo override comparison operators and take them into account
 */
class KDEVINTERFACES_EXPORT KDevDocumentCursor : public KTextEditor::Cursor
{
public:
  KDevDocumentCursor(const KUrl& document, const KTextEditor::Cursor& cursor = KTextEditor::Cursor::invalid());

  enum Position {
    Start,
    End
  };

  /// Constructor for information extraction only, does not take ownership of the cursor.
  /// \a range must be either a KDevDocumentRange or a KTextEditor::SmartRange.
  KDevDocumentCursor(KTextEditor::Range* range, Position position);

  /// Constructor for information extraction only, does not take ownership of the cursor.
  /// \a cursor must be either a KDevDocumentCursor or a KTextEditor::SmartCursor.
  KDevDocumentCursor(KTextEditor::Cursor* cursor);

  KDevDocumentCursor(const KDevDocumentCursor& copy);

  /// Returns the associated document.
  const KUrl& document() const;

  /// Sets the associated document.
  void setDocument(const KUrl& document);

private:
  KUrl m_document;
};

#endif // DOCUMENTRANGE_H

// kate: indent-width 2;
