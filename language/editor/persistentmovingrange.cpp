/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "persistentmovingrange.h"
#include "persistentmovingrangeprivate.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <util/foregroundlock.h>

KDevelop::PersistentMovingRange::PersistentMovingRange(const KTextEditor::Range& range, const IndexedString& document, bool shouldExpand)
: m_p(new PersistentMovingRangePrivate)
{
  VERIFY_FOREGROUND_LOCKED;
  m_p->m_range = range;
  m_p->m_document = document;
  m_p->m_shouldExpand = shouldExpand;

  m_p->connectTracker();
}

KDevelop::IndexedString KDevelop::PersistentMovingRange::document() const
{
  return m_p->m_document;
}

void KDevelop::PersistentMovingRange::setZDepth(float depth) const
{
  VERIFY_FOREGROUND_LOCKED;
  m_p->m_zDepth = depth;
  
  if(m_p->m_movingRange)
    m_p->m_movingRange->setZDepth(depth);
}

KDevelop::PersistentMovingRange::~PersistentMovingRange()
{
  VERIFY_FOREGROUND_LOCKED;
  if(m_p->m_movingRange)
    delete m_p->m_movingRange;
  delete m_p;
}

KTextEditor::Range KDevelop::PersistentMovingRange::range() const
{
  VERIFY_FOREGROUND_LOCKED;
  
  m_p->updateRangeFromMoving();
  return m_p->m_range;
}

QString KDevelop::PersistentMovingRange::text() const
{
  VERIFY_FOREGROUND_LOCKED;
  
  if(m_p->m_movingRange)
    return m_p->m_movingRange->document()->text(m_p->m_movingRange->toRange());
  
  return QString();
}

bool KDevelop::PersistentMovingRange::valid() const
{
  VERIFY_FOREGROUND_LOCKED;
  return m_p->m_valid;
}

void KDevelop::PersistentMovingRange::setAttribute(KTextEditor::Attribute::Ptr attribute)
{
  VERIFY_FOREGROUND_LOCKED;
  if(m_p->m_movingRange)
    m_p->m_movingRange->setAttribute(attribute);
}
