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

#include "kdevdocumentrange.h"

using namespace KTextEditor;

KDevDocumentRange::KDevDocumentRange(const KUrl& document, const Cursor& start, const Cursor& end, KDevDocumentRange* parent)
  : Range(start, end)
  , m_document(document)
  , m_parentRange(0)
{
  setParentRange(parent);
}

KDevDocumentRange::KDevDocumentRange(const KUrl& document, const Range& range, KDevDocumentRange* parent)
  : Range(range)
  , m_document(document)
  , m_parentRange(0)
{
  setParentRange(parent);
}

KDevDocumentRange::KDevDocumentRange(const KDevDocumentRange& copy)
  : Range(copy)
  , m_document(copy.document())
  , m_parentRange(0)
{
  setParentRange(copy.parentRange());
}

const KUrl& KDevDocumentRange::document() const
{
  return m_document;
}

void KDevDocumentRange::setDocument(const KUrl& document)
{
  m_document = document;
}

// kate: indent-width 2;

const QList< KDevDocumentRange * > & KDevDocumentRange::childRanges() const
{
  return m_childRanges;
}

void KDevDocumentRange::setParentRange(KDevDocumentRange * parent)
{
  if (m_parentRange)
    m_parentRange->m_childRanges.removeAll(this);

  m_parentRange = parent;

  if (m_parentRange) {
    QMutableListIterator<KDevDocumentRange*> it = m_parentRange->m_childRanges;
    while (it.hasNext()) {
      if (start() < it.next()->start()) {
        it.previous();
        it.insert(this);
        return;
      }
    }

    it.insert(this);
  }
}

KDevDocumentRange * KDevDocumentRange::parentRange() const
{
  return m_parentRange;
}
