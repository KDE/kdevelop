/* This file is part of KDevelop
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

K_GLOBAL_STATIC(QMutex, s_mutex)

using namespace KTextEditor;

namespace KDevelop {

DocumentRangeObjectPrivate::DocumentRangeObjectPrivate(const DocumentRangeObjectPrivate& rhs) : m_smartRange(0), m_smartMutex(0), m_ownsRange(DocumentRangeObject::DontOwn), m_range(rhs.m_range), m_document(rhs.m_document) {
}


void DocumentRangeObjectPrivate::syncFromSmart() const {
    QMutexLocker l(m_smartMutex);

    if(!m_smartRange)
        return;

    m_range = *m_smartRange;
}

void DocumentRangeObjectPrivate::syncToSmart() const {
    QMutexLocker l(m_smartMutex);

    if(!m_smartRange)
        return;

    m_smartRange->setRange(m_range.textRange());
}

DocumentRangeObject::DocumentRangeObject(const HashedString& document, const SimpleRange& range)
    : d_ptr( new DocumentRangeObjectPrivate ), m_ownsData(true)
{
    Q_D(DocumentRangeObject);
    if(!document.str().isEmpty())
        d->m_document = document;
    if(range.isValid())
        d->m_range = range;
}

DocumentRangeObject::DocumentRangeObject(DocumentRangeObjectPrivate& dd, const HashedString& document, const SimpleRange& range)
    : d_ptr( &dd ), m_ownsData(true)
{
    Q_D(DocumentRangeObject);
    if(!document.str().isEmpty())
        d->m_document = document;
    if(range.isValid())
        d->m_range = range;
}

DocumentRangeObject::DocumentRangeObject(DocumentRangeObjectPrivate& dd, bool ownsData)
    : d_ptr( &dd ), m_ownsData(ownsData)
{
}


DocumentRangeObject::~ DocumentRangeObject( )
{
    Q_D(DocumentRangeObject);

    if(m_ownsData)
    {
        QMutexLocker l(d->m_smartMutex);
        if (d->m_smartRange) {
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

    // If we're being called from anything but a copy constructor, the smart lock should already be held for
    // the new range.

    if (d->m_smartRange == range)
        return;

    {
        QMutexLocker l(d->m_smartMutex);
        if (d->m_smartRange)
        {
            d->m_smartRange->removeWatcher(this);

            if (d->m_ownsRange == DocumentRangeObject::Own)
                EditorIntegrator::releaseRange(d->m_smartRange);
        }
    }

    if (range) {
        // Smart lock must already be held.
        d->m_smartRange = range;
        KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( range->document() );
        Q_ASSERT(iface);
        d->m_smartMutex = iface->smartMutex();
        d->m_ownsRange = ownsRange;

        if(d->m_smartRange)
            d->m_smartRange->addWatcher(this);

    } else {
        d->m_smartRange = 0;
        d->m_smartMutex = 0;
    }
}

void DocumentRangeObject::clearSmartRange()
{
    setSmartRange(0);
}

SimpleRange DocumentRangeObject::range( ) const
{
    Q_D(const DocumentRangeObject);

    d->syncFromSmart();
    return d->m_range;
}

void DocumentRangeObject::setRange(const SimpleRange& range)
{
    Q_D(DocumentRangeObject);

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

    /// \todo This is not very threadsafe, does it need to be?
    return d->m_smartRange;
}

bool DocumentRangeObject::contains(const SimpleCursor& cursor) const
{
    Q_D(const DocumentRangeObject);

    d->syncFromSmart();
    return d->m_range.contains(cursor);
}

void DocumentRangeObject::rangeDeleted(KTextEditor::SmartRange * range)
{
    Q_D(DocumentRangeObject);

    // Already smart locked
    Q_ASSERT(range == d->m_smartRange);
    //Q_ASSERT(false);
    d->m_range = SimpleRange(*range);
    d->m_smartMutex = 0;
    d->m_smartRange = 0;
    d->m_ownsRange = Own;
}

KTextEditor::SmartRange* DocumentRangeObject::takeRange()
{
    Q_D(DocumentRangeObject);

    QMutexLocker l(d->m_smartMutex);
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

QMutex* DocumentRangeObject::mutex()
{
    return s_mutex;
}

}
