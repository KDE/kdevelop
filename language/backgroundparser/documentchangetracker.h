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

#ifndef KDEV_DOCUMENTCHANGETRACKER_H
#define KDEV_DOCUMENTCHANGETRACKER_H

#include <KTextEditor/SmartRangeWatcher>

#include "language/languageexport.h"

class QMutex;

namespace KDevelop
{

class KDEVPLATFORMLANGUAGE_EXPORT DocumentChangeTracker : protected KTextEditor::SmartRangeWatcher
{
public:
    DocumentChangeTracker();
    virtual ~DocumentChangeTracker();

    /**
     * Retrieve the list of changed smart ranges since the last successful parse.
     * Changes in child ranges will be replaced by changes in parent ranges if they occur.
     */
    QList<KTextEditor::SmartRange*> changedRanges() const;

    /**
     * Sets the list of changed smart ranges. For use when a new parse job is created,
     * and changed ranges already exist.
     */
    void setChangedRanges(const QList<KTextEditor::SmartRange*>& changedRanges);

    /**
     * Add a changed smart range.
     * This will select the highest encompassing range automatically.
     *
     * Child classes may reimplement this function, and decide to restart parsing if
     * they are not particularly far through parsing the previously retrieved text.
     *
     * \return true for success (if the job had not already started retrieving text from the editor),
     *         otherwise false to indicate failure (because it was already too late to add the range)
     */
    virtual bool addChangedRange(KTextEditor::SmartRange* range);

protected:
    /**
     * Informs the tracker not to accept any further changes to the changedRanges().
     */
    void finaliseChangedRanges();

    /// Mutex used to protect the changed ranges list
    QMutex* changeMutex() const;

    /// Check if the range changes are finalised. Call with the mutex held.
    bool rangeChangesFinalised() const;

    /// Performs the addition, call with the mutex held.
    void addChangedRangeInternal(KTextEditor::SmartRange* range);

    /// Detect and respond to a contained range being deleted
    virtual void rangeDeleted(KTextEditor::SmartRange *range);

private:
    class DocumentChangeTrackerPrivate* const d;
};

}
#endif

