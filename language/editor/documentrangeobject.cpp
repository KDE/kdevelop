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

#include <kglobal.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include "hashedstring.h"

#include "editorintegrator.h"
#include "documentrangeobject_p.h"

#define LOCK_SMART KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( d->m_smartRange ? d->m_smartRange->document() : 0); QMutexLocker l(iface ? iface->smartMutex() : 0);

K_GLOBAL_STATIC(QMutex, s_mutex)

using namespace KTextEditor;

namespace KDevelop {

DocumentRangeObjectPrivate::DocumentRangeObjectPrivate(const DocumentRangeObjectPrivate& rhs) : m_smartRange(0), m_ownsRange(DocumentRangeObject::DontOwn), m_range(rhs.m_range), m_document(rhs.m_document) {
}


void DocumentRangeObjectPrivate::syncFromSmart() const {
    if(!m_smartRange)
        return;

    m_range = *m_smartRange;
}

void DocumentRangeObjectPrivate::syncToSmart() const {
    if(!m_smartRange)
        return;

    m_smartRange->setRange(m_range.textRange());
}

DocumentRangeObject::DocumentRangeObject(const HashedString& document, const SimpleRange& range)
    : d_ptr( new DocumentRangeObjectPrivate )
{
    Q_D(DocumentRangeObject);
    if(!document.str().isEmpty())
        d->m_document = document;
    if(range.isValid())
        d->m_range = range;
}

DocumentRangeObject::DocumentRangeObject(DocumentRangeObjectPrivate& dd, const HashedString& document, const SimpleRange& range)
    : d_ptr( &dd )
{
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


    if (d->m_smartRange) {
        KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( d->m_smartRange->document() );
        if(iface) { //Probably the object is currently being destroyed
            QMutexLocker l(iface->smartMutex());
            d->m_smartRange->removeWatcher(this);

            //If a smart-range is deleted, move it's child-ranges into it's parent.
            //That way we do not delete other DocumentRangeObject's smart-ranges, which corrupts the structure integrity.
            if (d->m_ownsRange == DocumentRangeObject::Own)
                EditorIntegrator::releaseRange(d->m_smartRange);
        }
    }

    delete d_ptr;
}

void DocumentRangeObject::setSmartRange(KTextEditor::SmartRange * range, RangeOwning ownsRange)
{
    Q_D(DocumentRangeObject);

    if (d->m_smartRange == range)
        return;

    if (d->m_smartRange)
    {
        KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( d->m_smartRange->document() );
        if(iface) { //Probably the object is currently being destroyed
            QMutexLocker l(iface->smartMutex());

            d->m_smartRange->removeWatcher(this);

            if (d->m_ownsRange == DocumentRangeObject::Own)
                EditorIntegrator::releaseRange(d->m_smartRange);
        }
    }

    if (range) {
        KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( range->document() );
        if(iface) { //Probably the object is currently being destroyed
            QMutexLocker l(iface->smartMutex());

            d->m_smartRange = range;
            d->m_ownsRange = ownsRange;

            if(d->m_smartRange)
                d->m_smartRange->addWatcher(this);

        } else {
            range = 0;
        }

    } else {
        range = 0;
    }
}

void DocumentRangeObject::clearSmartRange()
{
    setSmartRange(0);
}

SimpleRange DocumentRangeObject::range( ) const
{
    Q_D(const DocumentRangeObject);

    LOCK_SMART
    d->syncFromSmart();
    return d->m_range;
}

void DocumentRangeObject::setRange(const SimpleRange& range)
{
    Q_D(DocumentRangeObject);

    LOCK_SMART
    d->m_range = range;
    d->syncToSmart();
}

void DocumentRangeObject::setUrl(const HashedString& document)
{
    Q_D(DocumentRangeObject);

    QMutexLocker l(s_mutex);
    d->m_document = document;
}

HashedString DocumentRangeObject::url() const
{
    Q_D(const DocumentRangeObject);

    QMutexLocker l(s_mutex);
    return d->m_document;
}


SmartRange* DocumentRangeObject::smartRange() const
{
    Q_D(const DocumentRangeObject);

    QMutexLocker l(s_mutex);
    return d->m_smartRange;
}

bool DocumentRangeObject::contains(const SimpleCursor& cursor) const
{
    Q_D(const DocumentRangeObject);

    LOCK_SMART
    d->syncFromSmart();
    return d->m_range.contains(cursor);
}

// bool DocumentRangeObject::contains(const KTextEditor::Cursor& cursor) const
// {
//     Q_D(const DocumentRangeObject);
//     QMutexLocker lock(s_mutex);
//     d->syncFromSmart();
//     return d->m_document == cursor.document() && d->m_range->contains(SimpleCursor(cursor.line(), cursor.column()));
// }

void DocumentRangeObject::rangeDeleted(KTextEditor::SmartRange * range)
{
    Q_D(DocumentRangeObject);

    // Already smart locked
    Q_ASSERT(range == d->m_smartRange);
    //Q_ASSERT(false);
    d->m_range = SimpleRange(*range);
    d->m_smartRange = 0;
    d->m_ownsRange = Own;
}

KTextEditor::SmartRange* DocumentRangeObject::takeRange()
{
    Q_D(DocumentRangeObject);

    LOCK_SMART
    KTextEditor::SmartRange* ret;

    d->syncFromSmart();
    ret = d->m_smartRange;
    if (d->m_smartRange)
    {
        d->m_smartRange = 0;
    }

    ret->removeWatcher(this);

    return ret;
}

// HashedString DocumentRangeObject::url( const KTextEditor::Range * range )
// {
//     if (range->isSmartRange()) ///@todo this conversion is bad
//         return static_cast<const SmartRange*>(range)->document()->url().pathOrUrl();
//     else
//         return static_cast<const DocumentRange*>(range)->document();
// }

QMutex* DocumentRangeObject::mutex()
{
    return s_mutex;
}

}
