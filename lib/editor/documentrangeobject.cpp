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

#include "documentrangeobject.h"

#include <QMutexLocker>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "editorintegrator.h"

using namespace KTextEditor;

namespace KDevelop
{

class DocumentRangeObjectPrivate
{
    public:
    DocumentRangeObjectPrivate() : m_range(0)
        , m_ownsRange(DocumentRangeObject::Own)
        , m_url(0)
    {}

    mutable QMutex m_rangeMutex;
    KTextEditor::Range* m_range;
    DocumentRangeObject::RangeOwning m_ownsRange;
    KUrl* m_url;
};

DocumentRangeObject::DocumentRangeObject(KTextEditor::Range* range)
  : d( new DocumentRangeObjectPrivate )
{
  setTextRange(range);
}

DocumentRangeObject::~ DocumentRangeObject( )
{
  if (d->m_range && d->m_range->isSmartRange())
    d->m_range->toSmartRange()->removeWatcher(this);

  if (d->m_ownsRange == DocumentRangeObject::Own)
    EditorIntegrator::releaseRange(d->m_range);

  delete d->m_url;
}

void DocumentRangeObject::setTextRange( KTextEditor::Range * range, RangeOwning ownsRange )
{
  Q_ASSERT(range);

  QMutexLocker lock(&d->m_rangeMutex);

  if (d->m_range == range)
    return;

  if (d->m_range) {
    // TODO.. overkill???
    if (d->m_range->isSmartRange()) {
      d->m_range->toSmartRange()->removeWatcher(this);
      Q_ASSERT(d->m_url);
      delete d->m_url;
      d->m_url = 0;
    }

    if (d->m_ownsRange == DocumentRangeObject::Own)
      EditorIntegrator::releaseRange(d->m_range);
  }

  d->m_range = range;
  d->m_ownsRange = ownsRange;

  if (d->m_range->isSmartRange()) {
    d->m_range->toSmartRange()->addWatcher(this);
    d->m_url = new KUrl(url(d->m_range));
  }
}

const Range DocumentRangeObject::textRange( ) const
{
  QMutexLocker lock(&d->m_rangeMutex);
  return *d->m_range;
}

void DocumentRangeObject::setRange(const KTextEditor::Range& range)
{
  QMutexLocker lock(&d->m_rangeMutex);
  *d->m_range = range;
}

const DocumentRange DocumentRangeObject::textDocRange() const
{
  QMutexLocker lock(&d->m_rangeMutex);
  return *static_cast<DocumentRange*>(d->m_range);
}

KUrl DocumentRangeObject::url() const
{
  QMutexLocker lock(&d->m_rangeMutex);
  return url(d->m_range);
}

KUrl DocumentRangeObject::url( const KTextEditor::Range * range )
{
  if (range->isSmartRange())
    return static_cast<const SmartRange*>(range)->document()->url();
  else
    return static_cast<const DocumentRange*>(range)->document();
}

SmartRange* DocumentRangeObject::smartRange() const
{
  QMutexLocker lock(&d->m_rangeMutex);

  if (d->m_range->isSmartRange())
    return static_cast<SmartRange*>(d->m_range);

  return 0L;
}

bool DocumentRangeObject::contains(const DocumentCursor& cursor) const
{
  QMutexLocker lock(&d->m_rangeMutex);
  return url(d->m_range) == cursor.document() && d->m_range->contains(cursor);
}

Range* DocumentRangeObject::textRangePtr() const
{
  QMutexLocker lock(&d->m_rangeMutex);
  return d->m_range;
}

void DocumentRangeObject::rangeDeleted(KTextEditor::SmartRange * range)
{
  QMutexLocker lock(&d->m_rangeMutex);
  Q_ASSERT(range == d->m_range);
  Q_ASSERT(d->m_url);
  //Q_ASSERT(false);
  d->m_range = new DocumentRange(*d->m_url, *d->m_range);
}

KTextEditor::Range* DocumentRangeObject::takeRange()
{
  QMutexLocker lock(&d->m_rangeMutex);

  KTextEditor::Range* ret = d->m_range;

  if (d->m_range) {
    // TODO.. overkill???
    if (d->m_range->isSmartRange())
      d->m_range->toSmartRange()->removeWatcher(this);

    d->m_range = 0;
  }

  return ret;
}

}
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on

