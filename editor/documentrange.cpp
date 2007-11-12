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

#include "documentrange.h"

using namespace KTextEditor;

namespace KDevelop
{

class DocumentRangePrivate{
public:
    DocumentRangePrivate( const KUrl& document ) : m_document(document)
        , m_parentRange(0)
    {}
    KUrl m_document;
    KTextEditor::Range* m_parentRange;
    QList<DocumentRange*> m_childRanges;
};

DocumentRange::DocumentRange()
    : d( new DocumentRangePrivate( KUrl() ) )
{
}

DocumentRange::DocumentRange(const KUrl& document, const KTextEditor::Cursor& start,
        const KTextEditor::Cursor& end, KTextEditor::Range* parent)
    : Range(start, end), d( new DocumentRangePrivate( document ) )
{
    setParentRange(parent);
}

DocumentRange::DocumentRange(const KUrl& document, const KTextEditor::Range& range, KTextEditor::Range* parent)
    : Range(range), d( new DocumentRangePrivate( document ) )
{
    setParentRange(parent);
}

DocumentRange::DocumentRange(const DocumentRange& copy)
    : Range(copy), d( new DocumentRangePrivate( copy.d->m_document ) )
{
    setParentRange(copy.parentRange());
}

DocumentRange::~DocumentRange()
{
    setParentRange(0);

    foreach (DocumentRange* child, d->m_childRanges)
        child->setParentRange(0);
    delete d;
}

const KUrl& DocumentRange::document() const
{
    return d->m_document;
}

void DocumentRange::setDocument(const KUrl& document)
{
    d->m_document = document;
}

const QList< DocumentRange * > & DocumentRange::childRanges() const
{
    return d->m_childRanges;
}

void DocumentRange::setParentRange(KTextEditor::Range* parent)
{
    if (d->m_parentRange && !d->m_parentRange->isSmartRange())
        static_cast<DocumentRange*>(d->m_parentRange)->d->m_childRanges.removeAll(this);

    d->m_parentRange = parent;

    if (d->m_parentRange && !d->m_parentRange->isSmartRange())
    {
        QMutableListIterator<DocumentRange*> it =
            static_cast<DocumentRange*>(d->m_parentRange)->d->m_childRanges;
        it.toBack();
        while (it.hasPrevious())
        {
            it.previous();
            if (start() >= it.value()->end())
            {
                it.next();
                it.insert(this);
                return;
            }
        }

        it.insert(this);
    }
}

Range* DocumentRange::parentRange() const
{
    return d->m_parentRange;
}

DocumentRange& DocumentRange::operator=(const DocumentRange& rhs)
{
    *d = *rhs.d;
    return *this;
}

}

