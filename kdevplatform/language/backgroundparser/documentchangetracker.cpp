/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentchangetracker.h"

#include <util/foregroundlock.h>
#include <editor/modificationrevision.h>
#include <serialization/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include "backgroundparser.h"
#include <debug.h>

#include <KTextEditor/Document>

#include <QCoreApplication>

// Can be used to disable the 'clever' updating logic that ignores whitespace-only changes and such.
// #define ALWAYS_UPDATE

using namespace KTextEditor;

/**
 * @todo Track the exact changes to the document, and then:
 * Do not reparse if:
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
 *             (whitespace changes, or changes that don't affect publicly visible stuff, eg. local incremental changes)
 *             -> Maybe alter the file-modification caches directly
 * */

namespace KDevelop {
/**
 * Internal helper class for RevisionLockerAndClearer
 * */
class RevisionLockerAndClearerPrivate
    : public QObject
{
    Q_OBJECT

public:
    RevisionLockerAndClearerPrivate(DocumentChangeTracker* tracker, qint64 revision);
    ~RevisionLockerAndClearerPrivate() override;
    inline qint64 revision() const
    {
        return m_revision;
    }

private:
    friend class RevisionLockerAndClearer;
    QPointer<DocumentChangeTracker> m_tracker;
    qint64 m_revision;
};

DocumentChangeTracker::DocumentChangeTracker(KTextEditor::Document* document)
    : m_needUpdate(false)
    , m_document(document)
    , m_url(IndexedString(document->url()))
{
    Q_ASSERT(document);
    Q_ASSERT(document->url().isValid());

    connect(document, &Document::textInserted, this, &DocumentChangeTracker::textInserted);
    connect(document, &Document::lineWrapped, this, &DocumentChangeTracker::lineWrapped);
    connect(document, &Document::lineUnwrapped, this, &DocumentChangeTracker::lineUnwrapped);
    connect(document, &Document::textRemoved, this, &DocumentChangeTracker::textRemoved);
    connect(document, &Document::documentSavedOrUploaded, this, &DocumentChangeTracker::documentSavedOrUploaded);
    connect(document, &Document::aboutToInvalidateMovingInterfaceContent, this,
            &DocumentChangeTracker::aboutToInvalidateMovingInterfaceContent);

    ModificationRevision::setEditorRevisionForFile(m_url, document->revision());

    reset();
}

QList<QPair<KTextEditor::Range, QString>> DocumentChangeTracker::completions() const
{
    VERIFY_FOREGROUND_LOCKED

    QList<QPair<KTextEditor::Range, QString>> ret;
    return ret;
}

void DocumentChangeTracker::reset()
{
    VERIFY_FOREGROUND_LOCKED

    // We don't reset the insertion here, as it may continue
        m_needUpdate = false;

    m_revisionAtLastReset = acquireRevision(m_document->revision());
    Q_ASSERT(m_revisionAtLastReset);
}

RevisionReference DocumentChangeTracker::currentRevision()
{
    VERIFY_FOREGROUND_LOCKED

    return acquireRevision(m_document->revision());
}

RevisionReference DocumentChangeTracker::revisionAtLastReset() const
{
    VERIFY_FOREGROUND_LOCKED

    return m_revisionAtLastReset;
}

bool DocumentChangeTracker::needUpdate() const
{
    VERIFY_FOREGROUND_LOCKED

    return m_needUpdate;
}

void DocumentChangeTracker::updateChangedRange(int delay)
{
    // Q_ASSERT(m_document->revision() != m_revisionAtLastReset->revision()); // May happen after reload

    // When reloading, textRemoved is called with an invalid m_document->url(). For that reason, we use m_url instead.

    ModificationRevision::setEditorRevisionForFile(m_url, m_document->revision());

    if (needUpdate()) {
        ICore::self()->languageController()->backgroundParser()->addDocument(m_url,
                                                                             TopDUContext::AllDeclarationsContextsAndUses,
                                                                             0, nullptr,
                                                                             ParseJob::IgnoresSequentialProcessing,
                                                                             delay);
    }
}

static Cursor cursorAdd(Cursor c, const QString& text)
{
    c.setLine(c.line() + text.count(QLatin1Char('\n')));
    c.setColumn(c.column() + (text.length() - qMin(0, text.lastIndexOf(QLatin1Char('\n')))));
    return c;
}

int DocumentChangeTracker::recommendedDelay(KTextEditor::Document* doc, const KTextEditor::Range& range,
                                            const QString& text, bool removal)
{
    const auto languages = ICore::self()->languageController()->languagesForUrl(doc->url());
    int delay = ILanguageSupport::NoUpdateRequired;
    for (const auto& lang : languages) {
        // take the largest value, because NoUpdateRequired is -2 and we want to make sure
        // that if one language requires an update it actually happens
        delay = qMax<int>(lang->suggestedReparseDelayForChange(doc, range, text, removal), delay);
    }

    return delay;
}

void DocumentChangeTracker::lineWrapped(KTextEditor::Document* document, const KTextEditor::Cursor& position)
{
    textInserted(document, position, QStringLiteral("\n"));
}

void DocumentChangeTracker::lineUnwrapped(KTextEditor::Document* document, int line)
{
    textRemoved(document, {{document->lineLength(line), line}, {0, line + 1}}, QStringLiteral("\n"));
}

void DocumentChangeTracker::textInserted(Document* document, const Cursor& cursor, const QString& text)
{
    /// TODO: get this data from KTextEditor directly, make its signal public
    KTextEditor::Range range(cursor, cursorAdd(cursor, text));

    if (!m_lastInsertionPosition.isValid() || m_lastInsertionPosition == cursor) {
        m_currentCleanedInsertion.append(text);
        m_lastInsertionPosition = range.end();
    }

    auto delay = recommendedDelay(document, range, text, false);
    m_needUpdate = delay != ILanguageSupport::NoUpdateRequired;
    updateChangedRange(delay);
}

void DocumentChangeTracker::textRemoved(Document* document, const KTextEditor::Range& oldRange, const QString& oldText)
{
    m_currentCleanedInsertion.clear();
    m_lastInsertionPosition = KTextEditor::Cursor::invalid();

    auto delay = recommendedDelay(document, oldRange, oldText, true);
    m_needUpdate = delay != ILanguageSupport::NoUpdateRequired;
    updateChangedRange(delay);
}

void DocumentChangeTracker::documentSavedOrUploaded(KTextEditor::Document* doc, bool)
{
    ModificationRevision::clearModificationCache(IndexedString(doc->url()));
}

DocumentChangeTracker::~DocumentChangeTracker()
{
    ModificationRevision::clearEditorRevisionForFile(KDevelop::IndexedString(m_document->url()));
}

Document* DocumentChangeTracker::document() const
{
    return m_document;
}

void DocumentChangeTracker::aboutToInvalidateMovingInterfaceContent(Document*)
{
    // Release all revisions! They must not be used any more.
    qCDebug(LANGUAGE) << "clearing all revisions";
    m_revisionLocks.clear();
    m_revisionAtLastReset = RevisionReference();
    ModificationRevision::setEditorRevisionForFile(m_url, 0);
}

KDevelop::RangeInRevision DocumentChangeTracker::transformBetweenRevisions(KDevelop::RangeInRevision range,
                                                                           qint64 fromRevision, qint64 toRevision) const
{
    VERIFY_FOREGROUND_LOCKED

    if ((fromRevision == -1 || holdingRevision(fromRevision)) && (toRevision == -1 || holdingRevision(toRevision))) {
        m_document->transformCursor(range.start.line, range.start.column, KTextEditor::MovingCursor::MoveOnInsert,
                                    fromRevision, toRevision);
        m_document->transformCursor(range.end.line, range.end.column, KTextEditor::MovingCursor::StayOnInsert,
                                    fromRevision, toRevision);
    }

    return range;
}

KDevelop::CursorInRevision DocumentChangeTracker::transformBetweenRevisions(KDevelop::CursorInRevision cursor,
                                                                            qint64 fromRevision, qint64 toRevision,
                                                                            KTextEditor::MovingCursor::InsertBehavior behavior)
const
{
    VERIFY_FOREGROUND_LOCKED

    if ((fromRevision == -1 || holdingRevision(fromRevision)) && (toRevision == -1 || holdingRevision(toRevision))) {
        m_document->transformCursor(cursor.line, cursor.column, behavior, fromRevision, toRevision);
    }

    return cursor;
}

RangeInRevision DocumentChangeTracker::transformToRevision(KTextEditor::Range range, qint64 toRevision) const
{
    return transformBetweenRevisions(RangeInRevision::castFromSimpleRange(range), -1, toRevision);
}

CursorInRevision DocumentChangeTracker::transformToRevision(KTextEditor::Cursor cursor, qint64 toRevision,
                                                            MovingCursor::InsertBehavior behavior) const
{
    return transformBetweenRevisions(CursorInRevision::castFromSimpleCursor(cursor), -1, toRevision, behavior);
}

KTextEditor::Range DocumentChangeTracker::transformToCurrentRevision(RangeInRevision range, qint64 fromRevision) const
{
    return transformBetweenRevisions(range, fromRevision, -1).castToSimpleRange();
}

KTextEditor::Cursor DocumentChangeTracker::transformToCurrentRevision(CursorInRevision cursor, qint64 fromRevision,
                                                                      MovingCursor::InsertBehavior behavior) const
{
    return transformBetweenRevisions(cursor, fromRevision, -1, behavior).castToSimpleCursor();
}

RevisionLockerAndClearerPrivate::RevisionLockerAndClearerPrivate(DocumentChangeTracker* tracker,
                                                                 qint64 revision) : m_tracker(tracker)
    , m_revision(revision)
{
    VERIFY_FOREGROUND_LOCKED

    moveToThread(QCoreApplication::instance()->thread());

    // Lock the revision
    m_tracker->lockRevision(revision);
}

RevisionLockerAndClearerPrivate::~RevisionLockerAndClearerPrivate()
{
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

    if (!holdingRevision(revision) && revision != m_document->revision())
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

    QMap<qint64, int>::iterator it = m_revisionLocks.find(revision);
    if (it != m_revisionLocks.end())
        ++(*it);
    else
    {
        m_revisionLocks.insert(revision, 1);
        m_document->lockRevision(revision);
    }
}

void DocumentChangeTracker::unlockRevision(qint64 revision)
{
    VERIFY_FOREGROUND_LOCKED

    QMap<qint64, int>::iterator it = m_revisionLocks.find(revision);
    if (it == m_revisionLocks.end()) {
        qCDebug(LANGUAGE) << "cannot unlock revision" << revision << ", probably the revisions have been cleared";
        return;
    }
    --(*it);

    if (*it == 0) {
        m_document->unlockRevision(revision);
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

    if (!m_p->m_tracker || !valid() || (to && !to->valid()))
        return range;

    qint64 fromRevision = revision();
    qint64 toRevision = -1;

    if (to)
        toRevision = to->revision();

    return m_p->m_tracker->transformBetweenRevisions(range, fromRevision, toRevision);
}

CursorInRevision RevisionLockerAndClearer::transformToRevision(const KDevelop::CursorInRevision& cursor,
                                                               const KDevelop::RevisionLockerAndClearer::Ptr& to,
                                                               MovingCursor::InsertBehavior behavior) const
{
    VERIFY_FOREGROUND_LOCKED

    if (!m_p->m_tracker || !valid() || (to && !to->valid()))
        return cursor;

    qint64 fromRevision = revision();
    qint64 toRevision = -1;

    if (to)
        toRevision = to->revision();

    return m_p->m_tracker->transformBetweenRevisions(cursor, fromRevision, toRevision, behavior);
}

RangeInRevision RevisionLockerAndClearer::transformFromRevision(const KDevelop::RangeInRevision& range,
                                                                const KDevelop::RevisionLockerAndClearer::Ptr& from)
const
{
    VERIFY_FOREGROUND_LOCKED

    if (!m_p->m_tracker || !valid())
        return range;

    qint64 toRevision = revision();
    qint64 fromRevision = -1;

    if (from)
        fromRevision = from->revision();

    return m_p->m_tracker->transformBetweenRevisions(range, fromRevision, toRevision);
}

CursorInRevision RevisionLockerAndClearer::transformFromRevision(const KDevelop::CursorInRevision& cursor,
                                                                 const KDevelop::RevisionLockerAndClearer::Ptr& from,
                                                                 MovingCursor::InsertBehavior behavior) const
{
    VERIFY_FOREGROUND_LOCKED

    if (!m_p->m_tracker)
        return cursor;

    qint64 toRevision = revision();
    qint64 fromRevision = -1;

    if (from)
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

    if (!m_p->m_tracker)
        return false;

    if (revision() == -1)
        return true; // The 'current' revision is always valid

    return m_p->m_tracker->holdingRevision(revision());
}

#if 0
/**
 * Returns the revision object which locks the revision representing the on-disk state.
 * Returns a zero object if the file is not on disk.
 */
RevisionReference DocumentChangeTracker::diskRevision() const
    ///@todo Track which revision was last saved to disk
#endif
}

#include "documentchangetracker.moc"
#include "moc_documentchangetracker.cpp"
