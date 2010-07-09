/*
* This file is part of KDevelop
*
* Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>
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
#include <ktexteditor/movinginterface.h>

#include <interfaces/foregroundlock.h>
#include <editor/modificationrevisionset.h>
#include <duchain/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include "backgroundparser.h"
#include <QApplication>

using namespace KTextEditor;

/**
 * @todo Track the exact changes to the document, and then:
 * Dont reparse if:
 *  - Comment added/changed
 *  - Newlines added/changed (ready)
 * Complete the document for validation:
 *  - Incomplete for-loops
 *  - ...
 * Only reparse after a statement was completed (either with statement-completion or manually), or after the cursor was switched away
 * Incremental parsing:
 *  - All changes within a local function (or function parameter context): Update only the context (and all its importers)
 * 
 * @todo: Prevent recursive updates after insignificant changes 
 *             (whitespace changes, or changes that don't affect publically visible stuff, eg. local incremental changes)
 *             -> Maybe alter the file-modification caches directly
 * */

namespace {
    QRegExp whiteSpaceRegExp("\\s");
};

namespace KDevelop
{

DocumentChangeTracker::DocumentChangeTracker( KTextEditor::Document* document )
    : m_needUpdate(false), m_changedRange(0), m_document(document), m_moving(0)
{
    Q_ASSERT(document);
    connect(document, SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
    connect(document, SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range)), SLOT(textRemoved(KTextEditor::Document*,KTextEditor::Range)));
    connect(document, SIGNAL(textChanged(KTextEditor::Document*,KTextEditor::Range,KTextEditor::Range)), SLOT(textChanged(KTextEditor::Document*,KTextEditor::Range,KTextEditor::Range)));
    connect(document, SIGNAL(destroyed(QObject*)), SLOT(documentDestroyed(QObject*)));
    
    m_moving = dynamic_cast<KTextEditor::MovingInterface*>(document);
    Q_ASSERT(m_moving);
    m_changedRange = m_moving->newMovingRange(KTextEditor::Range(), KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
    
    connect(m_document, SIGNAL(aboutToInvalidateMovingInterfaceContent (KTextEditor::Document*)), this, SLOT(aboutToInvalidateMovingInterfaceContent (KTextEditor::Document*)));
    
    ModificationRevision::setEditorRevisionForFile(IndexedString(document->url()), m_moving->revision());
    
    reset();
}

QList< QPair< SimpleRange, QString > > DocumentChangeTracker::completions() const
{
    VERIFY_FOREGROUND_LOCKED
    
    QList< QPair< SimpleRange , QString > > ret;
    return ret;
}

Range DocumentChangeTracker::changedRange() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_changedRange->toRange();
}

void DocumentChangeTracker::reset()
{
    VERIFY_FOREGROUND_LOCKED
    
    // We don't reset the insertion here, as it may continue
    m_needUpdate = false;
    m_changedRange->setRange(KTextEditor::Range::invalid());
    
    m_revisionAtLastReset = acquireRevision(m_moving->revision());
    m_textAtLastReset = m_document->text();
}

qint64 DocumentChangeTracker::currentRevision() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_moving->revision();
}

qint64 DocumentChangeTracker::revisionAtLastReset() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_revisionAtLastReset->revision();
}

QString DocumentChangeTracker::textAtLastReset() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_textAtLastReset;
}

bool DocumentChangeTracker::needUpdate() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_needUpdate;
}

void DocumentChangeTracker::textChanged( Document* document, Range oldRange, Range newRange )
{
    m_currentCleanedInsertion.clear();

    QString oldText = document->text(oldRange);
    QString newText = document->text(newRange);
    
    if(oldText.remove(whiteSpaceRegExp).isEmpty() && newText.remove(whiteSpaceRegExp).isEmpty())
    {
        // Only whitespace was changed, no update is required
    }else{
        m_needUpdate = true;
    }
    
    m_currentCleanedInsertion.clear();
    m_lastInsertionPosition = KTextEditor::Cursor::invalid();
    
    updateChangedRange(newRange);
}

void DocumentChangeTracker::updateChangedRange( Range changed )
{
    if(m_changedRange->toRange() == KTextEditor::Range::invalid())
        m_changedRange->setRange(changed);
    else
        m_changedRange->setRange(changed.encompass(m_changedRange->toRange()));
    
    Q_ASSERT(m_moving->revision() != m_revisionAtLastReset->revision());

    ModificationRevision::setEditorRevisionForFile(KDevelop::IndexedString(m_document->url()), m_moving->revision());
    
    if(needUpdate())
        ICore::self()->languageController()->backgroundParser()->addDocument(m_document->url(), TopDUContext::AllDeclarationsContextsAndUses);
}

void DocumentChangeTracker::textInserted( Document* document, Range range )
{
    QString text = document->text(range);
    
    if(text.remove(whiteSpaceRegExp).isEmpty())
    {
        // Only whitespace was changed, no update is required
    }else{
        m_needUpdate = true; // If we've inserted something else than whitespace, an update is required
    }
    
    if(m_lastInsertionPosition == KTextEditor::Cursor::invalid() || m_lastInsertionPosition == range.start())
    {
        m_currentCleanedInsertion.append(text);
        m_lastInsertionPosition = range.end();
    }

    updateChangedRange(range);
}

void DocumentChangeTracker::textRemoved( Document* document, Range range )
{
    QString text = document->text(range);
    
    if(text.remove(whiteSpaceRegExp).isEmpty())
    {
        // Only whitespace was changed, no update is required
    }else{
        m_needUpdate = true; // If we've inserted something else than whitespace, an update is required
    }
    
    m_currentCleanedInsertion.clear();
    m_lastInsertionPosition = KTextEditor::Cursor::invalid();
    
    updateChangedRange(range);
}

void DocumentChangeTracker::documentDestroyed( QObject* )
{
    m_document = 0;
    m_moving = 0;
    m_changedRange = 0;
}

DocumentChangeTracker::~DocumentChangeTracker()
{
    Q_ASSERT(m_document);
    ModificationRevision::clearEditorRevisionForFile(KDevelop::IndexedString(m_document->url()));
}

Document* DocumentChangeTracker::document() const
{
    return m_document;
}

MovingInterface* DocumentChangeTracker::documentMovingInterface() const
{
    return m_moving;
}

void DocumentChangeTracker::aboutToInvalidateMovingInterfaceContent ( Document* )
{
    // Release all revisions! They must not be used any more.
    kDebug() << "clearing all revisions";
    m_revisionLocks.clear();
}

KDevelop::RangeInRevision DocumentChangeTracker::transformBetweenRevisions(KDevelop::RangeInRevision range, qint64 fromRevision, qint64 toRevision) const
{
    VERIFY_FOREGROUND_LOCKED
    
    if((fromRevision == -1 || holdingRevision(fromRevision)) && (toRevision == -1 || holdingRevision(toRevision)))
    {
        m_moving->transformCursor(range.start.line, range.start.column, KTextEditor::MovingCursor::MoveOnInsert, fromRevision, toRevision);
        m_moving->transformCursor(range.end.line, range.end.column, KTextEditor::MovingCursor::StayOnInsert, fromRevision, toRevision);
    }
    
    return range;
}

KDevelop::CursorInRevision DocumentChangeTracker::transformBetweenRevisions(KDevelop::CursorInRevision cursor, qint64 fromRevision, qint64 toRevision, KTextEditor::MovingCursor::InsertBehavior behavior) const
{
    VERIFY_FOREGROUND_LOCKED
    
    if((fromRevision == -1 || holdingRevision(fromRevision)) && (toRevision == -1 || holdingRevision(toRevision)))
    {
        m_moving->transformCursor(cursor.line, cursor.column, behavior, fromRevision, toRevision);
    }
    
    return cursor;
}

RangeInRevision DocumentChangeTracker::transformToRevision(SimpleRange range, qint64 toRevision) const
{
    return transformBetweenRevisions(RangeInRevision::castFromSimpleRange(range), -1, toRevision);
}

CursorInRevision DocumentChangeTracker::transformToRevision(SimpleCursor cursor, qint64 toRevision, MovingCursor::InsertBehavior behavior) const
{
    return transformBetweenRevisions(CursorInRevision::castFromSimpleCursor(cursor), -1, toRevision, behavior);
}

SimpleRange DocumentChangeTracker::transformToCurrentRevision(RangeInRevision range, qint64 fromRevision) const
{
    return transformBetweenRevisions(range, fromRevision, -1).castToSimpleRange();
}

SimpleCursor DocumentChangeTracker::transformToCurrentRevision(CursorInRevision cursor, qint64 fromRevision, MovingCursor::InsertBehavior behavior) const
{
    return transformBetweenRevisions(cursor, fromRevision, -1, behavior).castToSimpleCursor();
}

RevisionLockerAndClearerPrivate::RevisionLockerAndClearerPrivate(DocumentChangeTracker* tracker, qint64 revision) : m_tracker(tracker), m_revision(revision)
{
    VERIFY_FOREGROUND_LOCKED
    
    moveToThread(QApplication::instance()->thread());
    
    // Lock the revision
    m_tracker->lockRevision(revision);
}

RevisionLockerAndClearerPrivate::~RevisionLockerAndClearerPrivate() {
    if (m_tracker)
        m_tracker->unlockRevision(m_revision);
}

RevisionLockerAndClearer::~RevisionLockerAndClearer()
{
    m_p->deleteLater(); // Will be deleted in the foreground thread, as the object was re-owned to the foreground
}

RevisionReference DocumentChangeTracker::acquireRevision(qint64 revision)
{
    VERIFY_FOREGROUND_LOCKED
    
    RevisionReference ret(new RevisionLockerAndClearer);
    ret->m_p = new RevisionLockerAndClearerPrivate(this, revision);
    return ret;
}

bool DocumentChangeTracker::holdingRevision(qint64 revision) const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_revisionLocks.contains(revision);
}

void DocumentChangeTracker::lockRevision(qint64 revision)
{
    VERIFY_FOREGROUND_LOCKED
    
    QMap< qint64, int >::iterator it = m_revisionLocks.find(revision);
    if(it != m_revisionLocks.end())
        ++(*it);
    else
    {
        m_revisionLocks.insert(revision, 1);
        m_moving->lockRevision(revision);
    }
}

void DocumentChangeTracker::unlockRevision(qint64 revision)
{
    VERIFY_FOREGROUND_LOCKED
    
    QMap< qint64, int >::iterator it = m_revisionLocks.find(revision);
    if(it == m_revisionLocks.end())
    {
        kDebug() << "cannot unlock revision" << revision << ", probably the revisions have been cleared";
        return;
    }
    --(*it);
    
    if(*it == 0)
    {
        m_moving->unlockRevision(revision);        
        m_revisionLocks.erase(it);
    }
}

qint64 RevisionLockerAndClearer::revision() const {
    return m_p->revision();
}
RangeInRevision RevisionLockerAndClearer::transformToRevision(const KDevelop::RangeInRevision& range, const KDevelop::RevisionLockerAndClearer::Ptr& to)
{
    VERIFY_FOREGROUND_LOCKED
    
    if(!m_p->m_tracker || !valid() || (to && !to->valid()))
        return range;
    
    qint64 fromRevision = revision();
    qint64 toRevision = -1;
    
    if(to)
        toRevision = to->revision();
    
    return m_p->m_tracker->transformBetweenRevisions(range, fromRevision, toRevision);
}

CursorInRevision RevisionLockerAndClearer::transformToRevision(const KDevelop::CursorInRevision& cursor, const KDevelop::RevisionLockerAndClearer::Ptr& to, MovingCursor::InsertBehavior behavior)
{
    VERIFY_FOREGROUND_LOCKED
    
    if(!m_p->m_tracker || !valid() || (to && !to->valid()))
        return cursor;
    
    qint64 fromRevision = revision();
    qint64 toRevision = -1;
    
    if(to)
        toRevision = to->revision();
    
    return m_p->m_tracker->transformBetweenRevisions(cursor, fromRevision, toRevision, behavior);
}

RangeInRevision RevisionLockerAndClearer::transformFromRevision(const KDevelop::RangeInRevision& range, const KDevelop::RevisionLockerAndClearer::Ptr& from)
{
    VERIFY_FOREGROUND_LOCKED
    
    if(!m_p->m_tracker || !valid())
        return range;
    
    qint64 toRevision = revision();
    qint64 fromRevision = -1;
    
    if(from)
        fromRevision = from->revision();
    
    return m_p->m_tracker->transformBetweenRevisions(range, fromRevision, toRevision);
}

CursorInRevision RevisionLockerAndClearer::transformFromRevision(const KDevelop::CursorInRevision& cursor, const KDevelop::RevisionLockerAndClearer::Ptr& from, MovingCursor::InsertBehavior behavior)
{
    VERIFY_FOREGROUND_LOCKED
    
    if(!m_p->m_tracker)
        return cursor;
    
    qint64 toRevision = revision();
    qint64 fromRevision = -1;
    
    if(from)
        fromRevision = from->revision();
    
    return m_p->m_tracker->transformBetweenRevisions(cursor, fromRevision, toRevision, behavior);
}


SimpleRange RevisionLockerAndClearer::transformToCurrentRevision(const KDevelop::RangeInRevision& range)
{
    return transformToRevision(range, KDevelop::RevisionLockerAndClearer::Ptr()).castToSimpleRange();
}

SimpleCursor RevisionLockerAndClearer::transformToCurrentRevision(const KDevelop::CursorInRevision& cursor, MovingCursor::InsertBehavior behavior)
{
    return transformToRevision(cursor, KDevelop::RevisionLockerAndClearer::Ptr(), behavior).castToSimpleCursor();
}

RangeInRevision RevisionLockerAndClearer::transformFromCurrentRevision(const KDevelop::SimpleRange& range)
{
    return transformFromRevision(RangeInRevision::castFromSimpleRange(range), RevisionReference());
}

CursorInRevision RevisionLockerAndClearer::transformFromCurrentRevision(const KDevelop::SimpleCursor& cursor, MovingCursor::InsertBehavior behavior)
{
    return transformFromRevision(CursorInRevision::castFromSimpleCursor(cursor), RevisionReference(), behavior);
}

bool RevisionLockerAndClearer::valid() const
{
    VERIFY_FOREGROUND_LOCKED
    
    if(!m_p->m_tracker)
        return false;
    
    if(revision() == -1)
        return true; // The 'current' revision is always valid
    
    return m_p->m_tracker->holdingRevision(revision());
}

RevisionReference DocumentChangeTracker::diskRevision() const
{
    ///@todo Track which revision was last saved to disk
    return RevisionReference();
}

}
