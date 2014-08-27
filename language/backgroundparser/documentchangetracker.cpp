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
#include <ktexteditor/movinginterface.h>

#include <util/foregroundlock.h>
#include <editor/modificationrevisionset.h>
#include <serialization/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/ilanguage.h>
#include "backgroundparser.h"
#include <QApplication>

// Can be used to disable the 'clever' updating logic that ignores whitespace-only changes and such.
// #define ALWAYS_UPDATE

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
    : m_needUpdate(false), m_changedRange(0), m_document(document), m_moving(0), m_whitespaceSensitivity(ILanguageSupport::Insensitive)
{
    m_url = IndexedString(document->url());
    Q_ASSERT(document);
    Q_ASSERT(document->url().isValid());

    // Check whether a language plugin is tracking the document which belongs to a
    // whitespace-sensitive language (e.g. python)
    foreach ( KDevelop::ILanguage* lang, ICore::self()->languageController()->languagesForUrl(document->url()) ) {
        if ( ! lang || ! lang->languageSupport() ) {
            continue;
        }
        if ( lang->languageSupport()->whitespaceSensititivy() >= m_whitespaceSensitivity ) {
            m_whitespaceSensitivity = lang->languageSupport()->whitespaceSensititivy();
        }
    }

    connect(document, SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
    connect(document, SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range,QString)), SLOT(textRemoved(KTextEditor::Document*,KTextEditor::Range,QString)));
    connect(document, SIGNAL(textChanged(KTextEditor::Document*,KTextEditor::Range,QString,KTextEditor::Range)), SLOT(textChanged(KTextEditor::Document*,KTextEditor::Range,QString,KTextEditor::Range)));
    connect(document, SIGNAL(destroyed(QObject*)), SLOT(documentDestroyed(QObject*)));
    connect(document, SIGNAL(documentSavedOrUploaded(KTextEditor::Document*,bool)), SLOT(documentSavedOrUploaded(KTextEditor::Document*,bool)));

    m_moving = dynamic_cast<KTextEditor::MovingInterface*>(document);
    Q_ASSERT(m_moving);
    m_changedRange = m_moving->newMovingRange(KTextEditor::Range(), KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);

    connect(m_document, SIGNAL(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)), this, SLOT(aboutToInvalidateMovingInterfaceContent(KTextEditor::Document*)));

    ModificationRevision::setEditorRevisionForFile(m_url, m_moving->revision());

    reset();
}

QList< QPair< KTextEditor::Range, QString > > DocumentChangeTracker::completions() const
{
    VERIFY_FOREGROUND_LOCKED

    QList< QPair< KTextEditor::Range , QString > > ret;
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
    Q_ASSERT(m_revisionAtLastReset);
    m_textAtLastReset = m_document->text();
}

RevisionReference DocumentChangeTracker::currentRevision()
{
    VERIFY_FOREGROUND_LOCKED

    return acquireRevision(m_moving->revision());
}

RevisionReference DocumentChangeTracker::revisionAtLastReset() const
{
    VERIFY_FOREGROUND_LOCKED

    return m_revisionAtLastReset;
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

bool DocumentChangeTracker::checkMergeTokens(const KTextEditor::Range& range, QString /*oldText*/, QString /*newText*/)
{
    ///@todo Improve this so that it notices when we wrapped in/out of a line-comment
    ///@todo Improve this so that it really checks whether some merge-able tokens have been moved together
    if(m_document->documentRange().contains(range))
    {
        if(range.start().column() == 0 || m_document->text(KTextEditor::Range(range.start().line(), range.start().column()-1, range.start().line(), range.start().column()))[0].isSpace())
            return true;
        if(range.end().column() >= m_document->lineLength(range.end().line()) || m_document->text(KTextEditor::Range(range.end().line(), range.end().column(), range.end().line(), range.end().column()+1))[0].isSpace())
            return true;
    }
    return false;
}

void DocumentChangeTracker::textChanged( Document* document, Range /*oldRange*/, QString oldText, Range newRange )
{
    m_currentCleanedInsertion.clear();

    QString newText = document->text(newRange);

    QString oldTextWithoutWhitespace = oldText;
    oldTextWithoutWhitespace.remove(whiteSpaceRegExp);

    QString newTextWithoutWhitespace = newText;
    newTextWithoutWhitespace.remove(whiteSpaceRegExp);

    if(oldTextWithoutWhitespace.isEmpty() && newTextWithoutWhitespace.isEmpty() && checkMergeTokens(newRange, oldText, newText))
    {
        // Only whitespace was changed, no update is required
    }else{
        m_needUpdate = true;
    }

    #ifdef ALWAYS_UPDATE
    m_needUpdate = true;
    #endif

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

//     Q_ASSERT(m_moving->revision() != m_revisionAtLastReset->revision()); // May happen after reload

    // When reloading, textRemoved is called with an invalid m_document->url(). For that reason, we use m_url instead.

    ModificationRevision::setEditorRevisionForFile(m_url, m_moving->revision());

    if(needUpdate())
        ICore::self()->languageController()->backgroundParser()->addDocument(m_url, TopDUContext::AllDeclarationsContextsAndUses);
}

void DocumentChangeTracker::textInserted( Document* document, Range range )
{
    QString text = document->text(range);

    if ( m_whitespaceSensitivity == ILanguageSupport::Sensitive ) {
        m_needUpdate = true;
    }

    if ( ! m_needUpdate ) {
        QString textWithoutWhitespace = text;
        textWithoutWhitespace.remove(whiteSpaceRegExp);
        bool changeIsWhitespaceOnly = textWithoutWhitespace.isEmpty() && checkMergeTokens(range, "", text);
        if ( m_whitespaceSensitivity == ILanguageSupport::IndentOnly && changeIsWhitespaceOnly ) {
            // The language requires a document to be re-parsed if the indentation changes (e.g. python),
            // so do some special checks here to see if that is the case.
            if ( changeIsWhitespaceOnly ) {
                QString fromLineBeginning = document->text(Range(Cursor(range.start().line(), 0), range.end()));
                bool inIndent = true;
                for ( int i = fromLineBeginning.length() - 1; i >= 0; --i ) {
                    if ( ! fromLineBeginning.at(i).isSpace() ) {
                        inIndent = false;
                        break;
                    }
                }
                if ( inIndent ) {
                    m_needUpdate = true;
                }
            }
        }
        if ( ! changeIsWhitespaceOnly ) {
            // If we've inserted something else than whitespace, an update is required for all languages.
            m_needUpdate = true;
        }
    }

    #ifdef ALWAYS_UPDATE
    m_needUpdate = true;
    #endif

    if(m_lastInsertionPosition == KTextEditor::Cursor::invalid() || m_lastInsertionPosition == range.start())
    {
        m_currentCleanedInsertion.append(text);
        m_lastInsertionPosition = range.end();
    }

    updateChangedRange(range);
}

void DocumentChangeTracker::textRemoved( Document* /*document*/, Range oldRange, QString oldText )
{
    QString text = oldText;

    QString textWithoutWhitespace = text;
    textWithoutWhitespace.remove(whiteSpaceRegExp);

    if(textWithoutWhitespace.isEmpty() && checkMergeTokens(Range(oldRange.start(), oldRange.start()), oldText, ""))
    {
        // Only whitespace was changed, no update is required
    }else{
        m_needUpdate = true; // If we've inserted something else than whitespace, an update is required
    }

    #ifdef ALWAYS_UPDATE
    m_needUpdate = true;
    #endif

    m_currentCleanedInsertion.clear();
    m_lastInsertionPosition = KTextEditor::Cursor::invalid();

    updateChangedRange(oldRange);
}

void DocumentChangeTracker::documentSavedOrUploaded(KTextEditor::Document* doc,bool)
{
    ModificationRevision::clearModificationCache(IndexedString(doc->url()));
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
    m_revisionAtLastReset = RevisionReference();
    ModificationRevision::setEditorRevisionForFile(m_url, 0);
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

RangeInRevision DocumentChangeTracker::transformToRevision(KTextEditor::Range range, qint64 toRevision) const
{
    return transformBetweenRevisions(RangeInRevision::castFromSimpleRange(range), -1, toRevision);
}

CursorInRevision DocumentChangeTracker::transformToRevision(KTextEditor::Cursor cursor, qint64 toRevision, MovingCursor::InsertBehavior behavior) const
{
    return transformBetweenRevisions(CursorInRevision::castFromSimpleCursor(cursor), -1, toRevision, behavior);
}

KTextEditor::Range DocumentChangeTracker::transformToCurrentRevision(RangeInRevision range, qint64 fromRevision) const
{
    return transformBetweenRevisions(range, fromRevision, -1).castToSimpleRange();
}

KTextEditor::Cursor DocumentChangeTracker::transformToCurrentRevision(CursorInRevision cursor, qint64 fromRevision, MovingCursor::InsertBehavior behavior) const
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

    if(!holdingRevision(revision) && revision != m_moving->revision())
        return RevisionReference();

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

qint64 RevisionLockerAndClearer::revision() const
{
    return m_p->revision();
}

RangeInRevision RevisionLockerAndClearer::transformToRevision(const KDevelop::RangeInRevision& range,
                                                              const KDevelop::RevisionLockerAndClearer::Ptr& to) const
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

CursorInRevision RevisionLockerAndClearer::transformToRevision(const KDevelop::CursorInRevision& cursor,
                                                               const KDevelop::RevisionLockerAndClearer::Ptr& to,
                                                               MovingCursor::InsertBehavior behavior) const
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

RangeInRevision RevisionLockerAndClearer::transformFromRevision(const KDevelop::RangeInRevision& range,
                                                                const KDevelop::RevisionLockerAndClearer::Ptr& from) const
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

CursorInRevision RevisionLockerAndClearer::transformFromRevision(const KDevelop::CursorInRevision& cursor,
                                                                 const KDevelop::RevisionLockerAndClearer::Ptr& from,
                                                                 MovingCursor::InsertBehavior behavior) const
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

KTextEditor::Range RevisionLockerAndClearer::transformToCurrentRevision(const KDevelop::RangeInRevision& range) const
{
    return transformToRevision(range, KDevelop::RevisionLockerAndClearer::Ptr()).castToSimpleRange();
}

KTextEditor::Cursor RevisionLockerAndClearer::transformToCurrentRevision(const KDevelop::CursorInRevision& cursor,
                                                                  MovingCursor::InsertBehavior behavior) const
{
    return transformToRevision(cursor, KDevelop::RevisionLockerAndClearer::Ptr(), behavior).castToSimpleCursor();
}

RangeInRevision RevisionLockerAndClearer::transformFromCurrentRevision(const KTextEditor::Range& range) const
{
    return transformFromRevision(RangeInRevision::castFromSimpleRange(range), RevisionReference());
}

CursorInRevision RevisionLockerAndClearer::transformFromCurrentRevision(const KTextEditor::Cursor& cursor,
                                                                        MovingCursor::InsertBehavior behavior) const
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
