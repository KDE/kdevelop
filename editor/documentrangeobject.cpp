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
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <hashedstring.h>

#include "editorintegrator.h"
#include "documentrangeobject_p.h"

using namespace KTextEditor;

namespace KDevelop {

DocumentRangeObjectPrivate::DocumentRangeObjectPrivate(const DocumentRangeObjectPrivate& rhs) : m_smartRange(0), m_ownsRange(DocumentRangeObject::DontOwn), m_range(rhs.m_range), m_document(rhs.m_document) {
}


void DocumentRangeObjectPrivate::syncFromSmart() const {
    SimpleRange range;
    KTextEditor::Range* smartRange;
    {
        if(!m_smartRange)
            return;

        KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( m_smartRange->document() );
        Q_ASSERT(iface);
        //QMutexLocker l(iface->smartMutex()); //Nested to prevent deadlock in combination with m_mutex
        
        smartRange = m_smartRange;
        
        range = SimpleRange( *smartRange );
    }

    QMutexLocker lock(&DocumentRangeObject::m_mutex);
    m_range = range;
}

void DocumentRangeObjectPrivate::syncToSmart() const {
    SimpleRange range;
    KTextEditor::SmartRange* smartRange;
    KTextEditor::SmartInterface *iface = qobject_cast<KTextEditor::SmartInterface*>( m_smartRange->document() );
    {
        QMutexLocker lock(&DocumentRangeObject::m_mutex);
        if(!m_smartRange)
            return;
    
        Q_ASSERT(iface);
        range = m_range;
        smartRange = m_smartRange;
    }
    //Danger window
    QMutexLocker l(iface->smartMutex());
    
    smartRange->start().setLine(range.start.line);
    smartRange->start().setColumn(range.start.column);
    smartRange->end().setLine(range.end.line);
    smartRange->end().setColumn(range.end.column);
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
    SmartRange* deleteLater = 0;
    SmartRange* removeWatcher = 0;
    {
    QMutexLocker lock(&m_mutex);
    
    if (d->m_smartRange) {
        removeWatcher = d->m_smartRange;
        //If a smart-range is deleted, move it's child-ranges into it's parent.
        //That way we do not delete other DocumentRangeObject's smart-ranges, which corrupts the structure integrity.
        if (d->m_ownsRange == DocumentRangeObject::Own)
            deleteLater = d->m_smartRange;
    }

    delete d_ptr;
    }
    //Avoid deadlock by doing this with the mutex unlocked
    if(removeWatcher)
        removeWatcher->removeWatcher(this);
    if(deleteLater)
        EditorIntegrator::releaseRange(deleteLater);
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

    SmartRange* deleteLater = 0;
    SmartRange* addWatcher = 0;
    SmartRange* removeWatcher = 0;
    {
        QMutexLocker lock(&m_mutex);
    
        if (d->m_smartRange == range)
            return;
    
        if (d->m_smartRange)
        {
            removeWatcher = d->m_smartRange;
    
            if (d->m_ownsRange == DocumentRangeObject::Own)
                deleteLater = d->m_smartRange;
        }
    
        d->m_smartRange = range;
        d->m_ownsRange = ownsRange;
    
        if(d->m_smartRange)
            addWatcher = d->m_smartRange;
    }
    //Avoid deadlock by doing this with the mutex unlocked
    if(addWatcher)
        addWatcher->addWatcher(this);
    if(removeWatcher)
        removeWatcher->removeWatcher(this);
    if(deleteLater)
        EditorIntegrator::releaseRange(deleteLater);

}

SimpleRange DocumentRangeObject::range( ) const
{
    Q_D(const DocumentRangeObject);
    d->syncFromSmart();
    QMutexLocker lock(&m_mutex);
    return d->m_range;
}

void DocumentRangeObject::setRange(const SimpleRange& range)
{
    Q_D(DocumentRangeObject);
    {
        QMutexLocker lock(&m_mutex);
        d->m_range = range;
    }
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
    d->syncFromSmart();
    QMutexLocker lock(&m_mutex);
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
    QMutexLocker lock(&m_mutex);
    Q_ASSERT(range == d->m_smartRange);
    //Q_ASSERT(false);
    d->m_range = SimpleRange(*range);
    d->m_smartRange = 0;
    d->m_ownsRange = Own;
}

KTextEditor::SmartRange* DocumentRangeObject::takeRange()
{
    Q_D(DocumentRangeObject);
    KTextEditor::SmartRange* ret;
    {
        d->syncFromSmart();
        QMutexLocker lock(&m_mutex);
    
        ret = d->m_smartRange;
    
        if (d->m_smartRange)
        {
            d->m_smartRange = 0;
        }
    }

    ret->removeWatcher(this);
    
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

//This mutex protects the internal data of all document-range objects from simultaneous access
QMutex KDevelop::DocumentRangeObject::m_mutex(QMutex::Recursive);
