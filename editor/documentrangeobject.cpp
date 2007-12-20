/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <hashedstring.h>

#include "editorintegrator.h"
#include "documentrangeobject_p.h"

using namespace KTextEditor;

namespace KDevelop {

DocumentRangeObjectPrivate::DocumentRangeObjectPrivate(const DocumentRangeObjectPrivate& rhs) : m_smartRange(0), m_ownsRange(DocumentRangeObject::DontOwn), m_range(rhs.m_range), m_document(rhs.m_document) {
}


void DocumentRangeObjectPrivate::syncFromSmart() const {
    if(!m_smartRange)
        return;
    
    m_range = SimpleRange( *m_smartRange );
}

void DocumentRangeObjectPrivate::syncToSmart() const {
    if(!m_smartRange)
        return;
    
    m_smartRange->start().setLine(m_range.start.line);
    m_smartRange->start().setColumn(m_range.start.column);
    m_smartRange->end().setLine(m_range.end.line);
    m_smartRange->end().setColumn(m_range.end.column);
}

DocumentRangeObject::DocumentRangeObject(const HashedString& document, const SimpleRange& range)
    : d_ptr( new DocumentRangeObjectPrivate )
{
    QMutexLocker lock(&m_mutex);
    Q_D(DocumentRangeObject);
    if(!document.str().isEmpty())
        d->m_document = document;
    if(range.isValid())
        d->m_range = range;
}
    
DocumentRangeObject::DocumentRangeObject(DocumentRangeObjectPrivate& dd, const HashedString& document, const SimpleRange& range)
    : d_ptr( &dd )
{
    QMutexLocker lock(&m_mutex);
    Q_D(DocumentRangeObject);
    if(!document.str().isEmpty())
        d->m_document = document;
    if(range.isValid())
        d->m_range = range;
}

DocumentRangeObject::DocumentRangeObject(DocumentRangeObjectPrivate& dd)
    : d_ptr( &dd )
{
}


DocumentRangeObject::~ DocumentRangeObject( )
{
    Q_D(DocumentRangeObject);

    QMutexLocker lock(&m_mutex);
    
    if (d->m_smartRange) {
        //If a smart-range is deleted, move it's child-ranges into it's parent.
        //That way we do not delete other DocumentRangeObject's smart-ranges, which corrupts the structure integrity.
        if( d->m_smartRange->parentRange() )
        {
            SmartRange* oldParent = d->m_smartRange->parentRange();
            d->m_smartRange->setParentRange(0);
            QList<SmartRange*> childRanges = d->m_smartRange->childRanges();
            foreach( SmartRange* range, childRanges )
                range->setParentRange(oldParent);
        }
        d->m_smartRange->removeWatcher(this);
        
        if (d->m_ownsRange == DocumentRangeObject::Own)
            EditorIntegrator::releaseRange(d->m_smartRange);
    }


    delete d_ptr;
}

void DocumentRangeObject::setRangeOwning(RangeOwning ownsRange) {
    Q_D(DocumentRangeObject);
    
    QMutexLocker lock(&m_mutex);
    d->m_ownsRange = ownsRange;
}

DocumentRangeObject::RangeOwning DocumentRangeObject::ownsRange() const {
    Q_D(const DocumentRangeObject);
    
    QMutexLocker lock(&m_mutex);
    return d->m_ownsRange;
}

void DocumentRangeObject::setSmartRange(KTextEditor::SmartRange * range, RangeOwning ownsRange)
{
    Q_D(DocumentRangeObject);

    QMutexLocker lock(&m_mutex);

    if (d->m_smartRange == range)
        return;

    if (d->m_smartRange)
    {
        d->m_smartRange->removeWatcher(this);

        if (d->m_ownsRange == DocumentRangeObject::Own)
            EditorIntegrator::releaseRange(d->m_smartRange);
    }

    d->m_smartRange = range;
    d->m_ownsRange = ownsRange;

    if(d->m_smartRange)
    {
        d->m_smartRange->addWatcher(this);
        d->m_document = d->m_smartRange->document()->url().prettyUrl();
    }
}

SimpleRange DocumentRangeObject::range( ) const
{
    Q_D(const DocumentRangeObject);
    QMutexLocker lock(&m_mutex);
    return d->m_range;
}

void DocumentRangeObject::setRange(const SimpleRange& range)
{
    Q_D(DocumentRangeObject);
    QMutexLocker lock(&m_mutex);
    d->m_range = range;
    d->syncToSmart();
}

void DocumentRangeObject::setUrl(const HashedString& document)
{
    Q_D(DocumentRangeObject);
    QMutexLocker lock(&m_mutex);

    d->m_document = document;
}

HashedString DocumentRangeObject::url() const
{
    Q_D(const DocumentRangeObject);
    QMutexLocker lock(&m_mutex);

    return d->m_document;
}


SmartRange* DocumentRangeObject::smartRange() const
{
    Q_D(const DocumentRangeObject);
    QMutexLocker lock(&m_mutex);

    return d->m_smartRange;
}

bool DocumentRangeObject::contains(const SimpleCursor& cursor) const
{
    Q_D(const DocumentRangeObject);
    QMutexLocker lock(&m_mutex);
    d->syncFromSmart();
    return d->m_range.contains(cursor);
}

// bool DocumentRangeObject::contains(const KTextEditor::Cursor& cursor) const
// {
//     Q_D(const DocumentRangeObject);
//     QMutexLocker lock(&m_mutex);
//     d->syncFromSmart();
//     return d->m_document == cursor.document() && d->m_range->contains(SimpleCursor(cursor.line(), cursor.column()));
// }

void DocumentRangeObject::rangeDeleted(KTextEditor::SmartRange * range)
{
    Q_D(DocumentRangeObject);
  ///@todo syncFromRange if it still exists at this moment
    QMutexLocker lock(&m_mutex);
    Q_ASSERT(range == d->m_smartRange);
    //Q_ASSERT(false);
    d->m_smartRange = 0;
    d->m_ownsRange = Own;
}

KTextEditor::SmartRange* DocumentRangeObject::takeRange()
{
    Q_D(DocumentRangeObject);
    QMutexLocker lock(&m_mutex);
    d->syncFromSmart();

    KTextEditor::SmartRange* ret = d->m_smartRange;

    if (d->m_smartRange)
    {
        d->m_smartRange->removeWatcher(this);

        d->m_smartRange = 0;
    }

    return ret;
}

// HashedString DocumentRangeObject::url( const KTextEditor::Range * range )
// {
//     if (range->isSmartRange()) ///@todo this conversion is bad
//         return static_cast<const SmartRange*>(range)->document()->url().prettyUrl();
//     else
//         return static_cast<const DocumentRange*>(range)->document();
// }

}

QMutex KDevelop::DocumentRangeObject::m_mutex;
