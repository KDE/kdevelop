/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_DOCUMENTCHANGETRACKER_H
#define KDEVPLATFORM_DOCUMENTCHANGETRACKER_H

#include <language/languageexport.h>
#include <language/editor/rangeinrevision.h>
#include <serialization/indexedstring.h>

#include <KTextEditor/MovingCursor>

#include <QExplicitlySharedDataPointer>
#include <QObject>

namespace KTextEditor {
class Document;
}

namespace KDevelop {
class DocumentChangeTracker;
/**
 * These objects belongs to the foreground, and thus can only be accessed from background threads if the foreground lock is held.
 * */

class RevisionLockerAndClearerPrivate;

/**
 * Helper class that locks a revision, and clears it on its destruction within the foreground thread.
 * Just delete it using deleteLater().
 * */
class KDEVPLATFORMLANGUAGE_EXPORT RevisionLockerAndClearer
    : public QSharedData
{
public:
    using Ptr = QExplicitlySharedDataPointer<RevisionLockerAndClearer>;

    ~RevisionLockerAndClearer();

    /**
     * Returns the revision number
     * */
    qint64 revision() const;

    /**
     * Whether the revision is still being held. It may have been lost due to document-reloads,
     * in which case the revision must not be used.
     * */
    bool valid() const;
    /**
     * Transform a range from this document revision to the given @p to.
     * */
    RangeInRevision transformToRevision(const RangeInRevision& range, const Ptr& to) const;

    /**
     * Transform a cursor from this document revision to the given @p to.
     * If a zero target revision is given, the transformation is done to the current document revision.
     * */
    CursorInRevision transformToRevision(const CursorInRevision& cursor, const Ptr& to,
                                         KTextEditor::MovingCursor::InsertBehavior behavior =
                                             KTextEditor::MovingCursor::StayOnInsert) const;

    /**
     * Transforms the given range from this revision into the current revision.
     */
    KTextEditor::Range transformToCurrentRevision(const RangeInRevision& range) const;

    /**
     * Transforms the given cursor from this revision into the current revision.
     */
    KTextEditor::Cursor transformToCurrentRevision(const CursorInRevision& cursor,
                                                   KTextEditor::MovingCursor::InsertBehavior behavior =
                                                       KTextEditor::MovingCursor::StayOnInsert) const;

    /**
     * Transform ranges from the given document revision @p from to the this one.
     * If a zero @p from revision is given, the transformation is done from the current document revision.
     * */
    RangeInRevision transformFromRevision(const RangeInRevision& range, const Ptr& from = Ptr()) const;
    /**
     * Transform ranges from the given document revision @p from to the this one.
     * If a zero @p from revision is given, the transformation is done from the current document revision.
     * */
    CursorInRevision transformFromRevision(const CursorInRevision& cursor,
                                           const Ptr& from = Ptr(),
                                           KTextEditor::MovingCursor::InsertBehavior behavior =
                                               KTextEditor::MovingCursor::StayOnInsert) const;

    /**
     * Transforms the given range from the current revision into this revision.
     */
    RangeInRevision transformFromCurrentRevision(const KTextEditor::Range& range) const;

    /**
     * Transforms the given cursor from the current revision into this revision.
     */
    CursorInRevision transformFromCurrentRevision(const KTextEditor::Cursor& cursor,
                                                  KTextEditor::MovingCursor::InsertBehavior behavior =
                                                      KTextEditor::MovingCursor::StayOnInsert) const;

private:
    friend class DocumentChangeTracker;

    RevisionLockerAndClearerPrivate* m_p;
};

using RevisionReference = RevisionLockerAndClearer::Ptr;

class KDEVPLATFORMLANGUAGE_EXPORT DocumentChangeTracker
    : public QObject
{
    Q_OBJECT

public:
    /**
     * Construct a tracker.
     *
     * @param document a non-null document to be tracked that must remain valid throughout this tracker's lifetime.
     */
    explicit DocumentChangeTracker(KTextEditor::Document* document);
    ~DocumentChangeTracker() override;

    /**
     * Resets the tracking to the current revision.
     * */
    void reset();

    /**
     * Returns the document revision at which reset() was called last.
     *
     * The revision is being locked by the tracker in MovingInterface,
     * it will be unlocked as soon as reset() is called, so if you want to use
     * the revision afterwards, you have to lock it before calling reset.
     *
     * zero is returned if the revisions were invalidated after the last call.
     * */
    RevisionReference revisionAtLastReset() const;

    /**
     * Returns the tracked document
     **/
    KTextEditor::Document* document() const;

    /**
     * Returns whether the given revision is being current held, so that it can be used
     * for transformations in MovingInterface
     * */
    bool holdingRevision(qint64 revision) const;

    /**
     * Use this function to acquire a revision. As long as the returned object is stored somewhere,
     * the revision can be used for transformations in MovingInterface, and especially for
     * DocumentChangeTracker::transformBetweenRevisions.
     *
     * Returns a zero revision object if the revision could not be acquired (it wasn't held).
     * */
    RevisionReference acquireRevision(qint64 revision);

    /**
     * Safely maps the given range between the two given revisions.
     * The mapping is only done if both the from- and to- revision are held,
     * else the original range is returned.
     *
     * @warning: Make sure that you actually hold the referenced revisions, else no transformation will be done.
     * @note It is much less error-prone to use RevisionReference->transformToRevision() and RevisionReference->transformFromRevision() directly.
     * */
    RangeInRevision transformBetweenRevisions(RangeInRevision range, qint64 fromRevision, qint64 toRevision) const;
    CursorInRevision transformBetweenRevisions(CursorInRevision cursor, qint64 fromRevision, qint64 toRevision,
                                               KTextEditor::MovingCursor::InsertBehavior behavior =
                                                   KTextEditor::MovingCursor::StayOnInsert) const;

    KTextEditor::Range transformToCurrentRevision(RangeInRevision range, qint64 fromRevision) const;
    KTextEditor::Cursor transformToCurrentRevision(CursorInRevision cursor, qint64 fromRevision,
                                                   KTextEditor::MovingCursor::InsertBehavior behavior =
                                                       KTextEditor::MovingCursor::StayOnInsert) const;

    /// Transform the range from the current revision into the given one
    RangeInRevision transformToRevision(KTextEditor::Range range, qint64 toRevision) const;
    /// Transform the cursor from the current revision into the given one
    CursorInRevision transformToRevision(KTextEditor::Cursor cursor, qint64 toRevision,
                                         KTextEditor::MovingCursor::InsertBehavior behavior =
                                             KTextEditor::MovingCursor::StayOnInsert) const;

private:
    RevisionReference m_revisionAtLastReset;

    KTextEditor::Document* m_document;
    KDevelop::IndexedString m_url;

    /**
     * Store the current revision of the document in the cache of ModificationRevision.
     *
     * This function must be called whenever the current revision of the document
     * changes, before asking the background parser to reparse the URL.
     */
    void updateEditorRevision() const;

    void updateChangedRange(int delay);
    int recommendedDelay(KTextEditor::Document* doc, const KTextEditor::Range& range, const QString& text,
                         bool removal);

    void textInserted(KTextEditor::Document* document, const KTextEditor::Cursor& position, const QString& inserted);
    void textRemoved(KTextEditor::Document* document, const KTextEditor::Range& range, const QString& oldText);
    void lineWrapped(KTextEditor::Document* document, const KTextEditor::Cursor& position);
    void lineUnwrapped(KTextEditor::Document* document, int line);

    void aboutToInvalidateMovingInterfaceContent (KTextEditor::Document* document);
    void documentSavedOrUploaded(KTextEditor::Document*, bool);

    friend class RevisionLockerAndClearerPrivate;
    void lockRevision(qint64 revision);
    void unlockRevision(qint64 revision);

    QMap<qint64, int> m_revisionLocks;
};
}
#endif
