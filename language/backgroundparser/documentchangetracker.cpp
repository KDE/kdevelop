/*
* This file is part of KDevelop
*
* Copyright 2008 Hamish Rodda <rodda@kde.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "documentchangetracker.h"

#include <QMutex>
#include <QMutexLocker>

#include <kdebug.h>

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>

using namespace KTextEditor;

namespace KDevelop
{

class DocumentChangeTrackerPrivate
{
public:
    DocumentChangeTrackerPrivate()
        : contentsRetrieved(false)
        , contentsRetrievedMutex(new QMutex)
    {
    }

    ~DocumentChangeTrackerPrivate()
    {
        delete contentsRetrievedMutex;
    }

    bool contentsRetrieved;

    QMutex* contentsRetrievedMutex;
    QList<SmartRange*> changedRanges;
};

DocumentChangeTracker::DocumentChangeTracker()
    : d(new DocumentChangeTrackerPrivate)
{
}

DocumentChangeTracker::~DocumentChangeTracker()
{
    foreach (SmartRange* range, d->changedRanges)
        range->removeWatcher(this);

    delete d;
}

QList<SmartRange*> DocumentChangeTracker::changedRanges() const
{
    return d->changedRanges;
}

bool DocumentChangeTracker::addChangedRange(SmartRange* changed)
{
    QMutexLocker l(d->contentsRetrievedMutex);
    if (d->contentsRetrieved)
        return false;

    addChangedRangeInternal(changed);

    return true;
}

void DocumentChangeTracker::addChangedRangeInternal(SmartRange* changed)
{
    QMutableListIterator<SmartRange*> it = d->changedRanges;
    bool foundOverlap = false;
    while (it.hasNext()) {
        if (it.next() == changed) {
            foundOverlap = true;
            break;
        }

        if (it.value()->overlaps(*changed)) {
            int rangeDepth = it.value()->depth();
            int changedDepth = changed->depth();

            if (changedDepth < rangeDepth) {
                // We have a new parent range
                it.value()->removeWatcher(this);
                
                if (!foundOverlap) {
                    // Replace current range
                    it.value() = changed;
                    changed->addWatcher(this);
                    foundOverlap = true;
                    
                } else {
                    // Remove now child range
                    it.remove();
                }

            } else {
                // We're contained by the range we found
                // Nothing more to do
                foundOverlap = true;
                break;
            }
        }
    }

    if (!foundOverlap) {
        d->changedRanges.append(changed);
        changed->addWatcher(this);
    }
}

void DocumentChangeTracker::finaliseChangedRanges()
{
    QMutexLocker l(d->contentsRetrievedMutex);
    d->contentsRetrieved = true;
}

QMutex* DocumentChangeTracker::changeMutex() const
{
    return d->contentsRetrievedMutex;
}

void DocumentChangeTracker::setChangedRanges(const QList<SmartRange*>& changedRanges)
{
    QMutexLocker l(d->contentsRetrievedMutex);
    Q_ASSERT(!d->contentsRetrieved);
    Q_ASSERT(d->changedRanges.isEmpty());
    
    d->changedRanges = changedRanges;

    foreach (SmartRange* range, d->changedRanges)
        range->addWatcher(this);
}

bool DocumentChangeTracker::rangeChangesFinalised() const
{
    return d->contentsRetrieved;
}

void DocumentChangeTracker::rangeDeleted(SmartRange *range)
{
    QMutexLocker l(d->contentsRetrievedMutex);

    int index = d->changedRanges.indexOf(range);
    Q_ASSERT(index != -1);
    d->changedRanges.removeAt(index);

    if (range->parentRange())
        addChangedRangeInternal(range->parentRange());
    else
        kWarning() << "Top range deleted?";
}

}
