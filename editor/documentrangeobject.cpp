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

using namespace KTextEditor;

namespace KDevelop
{

class DocumentRangeObjectPrivate
{
    public:
    DocumentRangeObjectPrivate() : m_range(0)
        , m_ownsRange(DocumentRangeObject::Own)
    {}

    KTextEditor::Range* m_range;
    DocumentRangeObject::RangeOwning m_ownsRange;
    HashedString m_document;
};

DocumentRangeObject::DocumentRangeObject(const HashedString& document, KTextEditor::Range* range)
    : d( new DocumentRangeObjectPrivate )
{
    if( range )
        setTextRange(document, range);
}

DocumentRangeObject::~ DocumentRangeObject( )
{
    if (d->m_range && d->m_range->isSmartRange()) {
        //If a smart-range is deleted, move it's child-ranges into it's parent.
        //That way we do not delete other DocumentRangeObject's smart-ranges, which corrupts the structure integrity.
        SmartRange* smart = d->m_range->toSmartRange();
        if( smart->parentRange() )
        {
            SmartRange* oldParent = smart->parentRange();
            smart->setParentRange(0);
            QList<SmartRange*> childRanges = smart->childRanges();
            foreach( SmartRange* range, childRanges )
                range->setParentRange(oldParent);
        }
        smart->removeWatcher(this);
    }

    if (d->m_ownsRange == DocumentRangeObject::Own)
        EditorIntegrator::releaseRange(d->m_range);

    delete d;
}

void DocumentRangeObject::setRangeOwning(RangeOwning ownsRange) {
    d->m_ownsRange = ownsRange;
}

DocumentRangeObject::RangeOwning DocumentRangeObject::ownsRange() const {
    return d->m_ownsRange;
}

void DocumentRangeObject::setTextRange(const HashedString& document, KTextEditor::Range * range, RangeOwning ownsRange)
{
    Q_ASSERT(range);

    QMutexLocker lock(&m_mutex);

    d->m_document = document;
  
    if (d->m_range == range)
        return;

    if (d->m_range)
    {
        // TODO.. overkill???
        if (d->m_range->isSmartRange())
        {
            d->m_range->toSmartRange()->removeWatcher(this);
        }

        if (d->m_ownsRange == DocumentRangeObject::Own)
            EditorIntegrator::releaseRange(d->m_range);
    }

    d->m_range = range;
    d->m_ownsRange = ownsRange;

    if (d->m_range->isSmartRange())
    {
        d->m_range->toSmartRange()->addWatcher(this);
    }
}

const Range& DocumentRangeObject::textRange( ) const
{
    QMutexLocker lock(&m_mutex);
    return *d->m_range;
}

void DocumentRangeObject::setRange(const KTextEditor::Range& range)
{
    QMutexLocker lock(&m_mutex);
    *d->m_range = range;
}

const DocumentRange DocumentRangeObject::textDocRange() const
{
    QMutexLocker lock(&m_mutex);
    return *static_cast<DocumentRange*>(d->m_range);
}

HashedString DocumentRangeObject::url() const
{
    QMutexLocker lock(&m_mutex);

    return d->m_document;
}


SmartRange* DocumentRangeObject::smartRange() const
{
    QMutexLocker lock(&m_mutex);

    if (d->m_range->isSmartRange())
        return static_cast<SmartRange*>(d->m_range);

    return 0L;
}

bool DocumentRangeObject::contains(const DocumentCursor& cursor) const
{
    QMutexLocker lock(&m_mutex);
        return d->m_document == cursor.document() && d->m_range->contains(cursor);
}

Range* DocumentRangeObject::textRangePtr() const
{
    QMutexLocker lock(&m_mutex);
        return d->m_range;
}

void DocumentRangeObject::rangeDeleted(KTextEditor::SmartRange * range)
{
    QMutexLocker lock(&m_mutex);
    Q_ASSERT(range == d->m_range);
    //Q_ASSERT(false);
    d->m_range = new DocumentRange(d->m_document, *d->m_range);
    d->m_ownsRange = Own;
}

KTextEditor::Range* DocumentRangeObject::takeRange()
{
    QMutexLocker lock(&m_mutex);

    KTextEditor::Range* ret = d->m_range;

    if (d->m_range)
    {
        // TODO.. overkill???
        if (d->m_range->isSmartRange())
            d->m_range->toSmartRange()->removeWatcher(this);

        d->m_range = 0;
    }

    return ret;
}

HashedString DocumentRangeObject::url( const KTextEditor::Range * range )
{
    if (range->isSmartRange()) ///@todo this conversion is bad
        return static_cast<const SmartRange*>(range)->document()->url().prettyUrl();
    else
        return static_cast<const DocumentRange*>(range)->document();
}

}

