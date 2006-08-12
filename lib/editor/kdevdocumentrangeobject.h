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

#ifndef KDEVDOCUMENTRANGEOBJECT_H
#define KDEVDOCUMENTRANGEOBJECT_H

#include <ktexteditor/range.h>

#include "kdevdocumentcursor.h"

/**
 * Base class for any object which has an associated range of text.
 *
 * This allows text without a currently loaded text editor to be represented.
 */
class KDEVINTERFACES_EXPORT KDevDocumentRangeObject
{
public:
  KDevDocumentRangeObject(KTextEditor::Range* range);
  virtual ~KDevDocumentRangeObject();

  void setTextRange(KTextEditor::Range* range);

  KTextEditor::Range& textRange();
  const KTextEditor::Range& textRange() const;
  KTextEditor::Range* textRangePtr() const;
  KTextEditor::Range* takeTextRange();
  KTextEditor::SmartRange* smartRange() const;

  KUrl url() const;
  static KUrl url(const KTextEditor::Range* range);

  bool contains(const KDevDocumentCursor& cursor) const;

private:
  KTextEditor::Range* m_range;
};

#endif // RANGEOBJECT_H

// kate: indent-width 2;
