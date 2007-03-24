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

#ifndef KDEVDOCUMENTRANGE_H
#define KDEVDOCUMENTRANGE_H

#include "kdevexport.h"

#include <kurl.h>

#include <ktexteditor/range.h>

namespace KDevelop
{

/**
 * Extends KTextEditor::Range with information about the URL to which the range
 * refers.
 *
 * \todo parent/child relationships here too?
 */
class KDEVPLATFORM_EXPORT DocumentRange : public KTextEditor::Range
{
public:
  DocumentRange(const KUrl& document, const KTextEditor::Cursor& start, const KTextEditor::Cursor& end, KTextEditor::Range* parent = 0);
  DocumentRange(const KUrl& document, const KTextEditor::Range& range = KTextEditor::Range::invalid(), KTextEditor::Range* parent = 0);
  DocumentRange(const DocumentRange& copy);
  virtual ~DocumentRange();

  /// Returns the associated document.
  const KUrl& document() const;

  /// Sets the associated document.
  void setDocument(const KUrl& document);

  KTextEditor::Range* parentRange() const;
  void setParentRange(KTextEditor::Range* parent);

  const QList<DocumentRange*>& childRanges() const;

private:
  class DocumentRangePrivate* const d;
};

}
#endif // DOCUMENTRANGE_H

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

