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

#include "rangeobject.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "documentrange.h"

using namespace KTextEditor;

RangeObject::RangeObject(Range* range)
  : m_range(range)
{
  Q_ASSERT(m_range);
}

RangeObject::~ RangeObject( )
{
  delete m_range;
}

void RangeObject::setTextRange( Range * range )
{
  if (m_range == range)
    return;

  delete m_range;
  m_range = range;
}

const Range& RangeObject::textRange( ) const
{
  return *m_range;
}

Range& RangeObject::textRange( )
{
  return *m_range;
}

KUrl RangeObject::url() const
{
  return url(m_range);
}

KUrl RangeObject::url( const Range * range )
{
  if (range->isSmartRange())
    return static_cast<const SmartRange*>(range)->document()->url();
  else
    return static_cast<const DocumentRange*>(range)->document();
}

SmartRange* RangeObject::smartRange() const
{
  if (m_range->isSmartRange())
    return static_cast<SmartRange*>(m_range);

  return 0L;
}

bool RangeObject::contains(const DocumentCursor& cursor) const
{
  return url() == cursor.document() && m_range->contains(cursor);
}

Range* RangeObject::textRangePtr() const
{
  return m_range;
}

Range * RangeObject::takeTextRange()
{
  Range* r = m_range;
  m_range = 0;
  return r;
}

// kate: indent-width 2;
