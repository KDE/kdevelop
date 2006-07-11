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

#ifndef DOCUMENTRANGE_H
#define DOCUMENTRANGE_H

#include <kurl.h>

#include <ktexteditor/range.h>

/**
 * Extends KTextEditor::Range with information about the URL to which the range
 * refers.
 *
 * \todo parent/child relationships here too?
 */
class DocumentRange : public KTextEditor::Range
{
public:
  DocumentRange(const KUrl& document, const KTextEditor::Cursor& start, const KTextEditor::Cursor& end);
  DocumentRange(const KUrl& document, const KTextEditor::Range& range = KTextEditor::Range::invalid());
  DocumentRange(const DocumentRange& copy);

  /// Returns the associated document.
  const KUrl& document() const;

  /// Sets the associated document.
  void setDocument(const KUrl& document);

private:
  KUrl m_document;
};

#endif // DOCUMENTRANGE_H

// kate: indent-width 2;
