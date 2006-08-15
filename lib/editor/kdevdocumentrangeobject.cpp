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

#include "kdevdocumentrangeobject.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "kdevdocumentrange.h"

using namespace KTextEditor;

KDevDocumentRangeObject::KDevDocumentRangeObject(Range* range)
  : m_range(0)
{
  Q_ASSERT(range);
  setTextRange(range);
}

KDevDocumentRangeObject::~ KDevDocumentRangeObject( )
{
  delete m_range;
}

void KDevDocumentRangeObject::setTextRange( Range * range )
{
  if (m_range == range)
    return;

  if (m_range) {
    // TODO.. overkill???
    if (m_range->isSmartRange())
      m_range->toSmartRange()->removeWatcher(this);

    delete m_range;
  }

  m_range = range;

  if (m_range) {
    if (m_range->isSmartRange())
      m_range->toSmartRange()->addWatcher(this);
  }
}

const Range& KDevDocumentRangeObject::textRange( ) const
{
  return *m_range;
}

Range& KDevDocumentRangeObject::textRange( )
{
  return *m_range;
}

KUrl KDevDocumentRangeObject::url() const
{
  return url(m_range);
}

KUrl KDevDocumentRangeObject::url( const Range * range )
{
  if (range->isSmartRange())
    return static_cast<const SmartRange*>(range)->document()->url();
  else
    return static_cast<const KDevDocumentRange*>(range)->document();
}

SmartRange* KDevDocumentRangeObject::smartRange() const
{
  if (m_range->isSmartRange())
    return static_cast<SmartRange*>(m_range);

  return 0L;
}

bool KDevDocumentRangeObject::contains(const KDevDocumentCursor& cursor) const
{
  return url() == cursor.document() && m_range->contains(cursor);
}

Range* KDevDocumentRangeObject::textRangePtr() const
{
  return m_range;
}

Range * KDevDocumentRangeObject::takeTextRange()
{
  Range* r = m_range;
  m_range = 0;
  return r;
}

// kate: indent-width 2;

void KDevDocumentRangeObject::rangeDeleted(KTextEditor::SmartRange * range)
{
  Q_ASSERT(range == m_range);
  m_range = new KDevDocumentRange(url(), *m_range);
}
