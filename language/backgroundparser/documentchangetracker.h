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

#ifndef DOCUMENTCHANGETRACKER_H
#define DOCUMENTCHANGETRACKER_H

#include "../languageexport.h"
#include <QPair>
#include <language/editor/simplerange.h>
#include <ktexteditor/movingrange.h>

namespace KTextEditor
{
class Document;
class MovingRange;
class MovingInterface;
}

namespace KDevelop
{

class DocumentChangeTracker;
/**
 * These objects belongs to the foreground, and thus can only be accessed from background threads if the foreground lock is held.
 * */

class RevisionLockerAndClearerPrivate;

/** 
 * Helper class that locks a revision, and clears it on its destruction within the foreground thread.
 * Just delete it using deleteLater().
 * */
class KDEVPLATFORMLANGUAGE_EXPORT RevisionLockerAndClearer : public KShared {
public:
    typedef KSharedPtr<RevisionLockerAndClearer> Ptr;

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
     * If a zero target revision is given, the transformation is done to the current document revision.
     * */
    SimpleRange transformToRevision(const SimpleRange& range, const Ptr& to = Ptr());
    /**
     * Transform a cursor from this document revision to the given @p to.
     * If a zero target revision is given, the transformation is done to the current document revision.
     * */
    SimpleCursor transformToRevision(const SimpleCursor& cursor, const Ptr& to = Ptr(), KTextEditor::MovingCursor::InsertBehavior behavior = KTextEditor::MovingCursor::StayOnInsert);
    
    /**
     * Transform ranges from the given document revision @p from to the this one.
     * If a zero @p from revision is given, the transformation is done from the current document revision.
     * */
    SimpleRange transformFromRevision(const SimpleRange& range, const Ptr& from = Ptr());
    /**
     * Transform ranges from the given document revision @p from to the this one.
     * If a zero @p from revision is given, the transformation is done from the current document revision.
     * */
    SimpleCursor transformFromRevision(const SimpleCursor& cursor, const Ptr& from = Ptr(), KTextEditor::MovingCursor::InsertBehavior behavior = KTextEditor::MovingCursor::StayOnInsert);
    
private:
    friend class DocumentChangeTracker;
    
    RevisionLockerAndClearerPrivate* m_p;
};

typedef RevisionLockerAndClearer::Ptr RevisionReference;

/**
 * Internal helper class for RevisionLockerAndClearer
 * */
class KDEVPLATFORMLANGUAGE_EXPORT RevisionLockerAndClearerPrivate : public QObject {
    Q_OBJECT
public:
    RevisionLockerAndClearerPrivate(DocumentChangeTracker* tracker, qint64 revision);
    ~RevisionLockerAndClearerPrivate();
    inline qint64 revision() const {
        return m_revision;
    }
private:
    friend class RevisionLockerAndClearer;
    QPointer<DocumentChangeTracker> m_tracker;
    qint64 m_revision;
};

class KDEVPLATFORMLANGUAGE_EXPORT DocumentChangeTracker : public QObject
{
    Q_OBJECT
public:
    DocumentChangeTracker( KTextEditor::Document* document );
    virtual ~DocumentChangeTracker();

    /**
     * Completions of the users current edits that are supposed to complete
     * not-yet-finished statements, like for example for-loops for parsing.
     * */
    virtual QList<QPair<KDevelop::SimpleRange, QString> > completions() const;
    
    /**
     * Resets the tracking to the current revision.
     * */
    virtual void reset();

    /**
     * Returns the document revision at which reset() was called last.
     * 
     * The revision is being locked by the tracker in MovingInterface,
     * it will be unlocked as soon as reset() is called, so if you want to use
     * the revision afterwards, you have to lock it before calling reset.
     * 
     * -1 is returned if the revisions were invalidated after the last call.
     * */
    qint64 revisionAtLastReset() const;

    /**
     * Returns the document text at the last reset
     * */
    QString textAtLastReset() const;
    
    /**
     * Returns the current revision (which is not locked by the tracker)
     * */
    qint64 currentRevision() const;
    
    /**
     * Returns the range that was changed since the last reset
     * */
    virtual KTextEditor::Range changedRange() const;
    
    /**
     * Whether the changes that happened since the last reset are significant enough to require an update
     * */
    virtual bool needUpdate() const;
    
    /**
     * Returns the tracked document
     **/
    KTextEditor::Document* document() const;
    
    KTextEditor::MovingInterface* documentMovingInterface() const;
    
    /**
     * Returns the revision object which locks the revision representing the on-disk state.
     * Returns a zero object if the file is not on disk.
     * */
    RevisionReference diskRevision() const;
    
    /**
     * Returns whether the given revision is being current held, so that it can be used
     * for transformations in MovingInterface
     * */
    bool holdingRevision(qint64 revision) const;
    
    /**
     * Use this fuction to acquire a revision. As long as the returned object is stored somewhere,
     * the revision can be used for transformations in MovingInterface, and especially for 
     * DocumentChangeTracker::transformBetweenRevisions
     * */
    RevisionReference acquireRevision(qint64 revision);
    
    /**
     * Safely maps the given range between the two given revisions.
     * The mapping is only done if both the from- and to- revision are held,
     * else the original range is returned.
     * 
     * @warning: Make sure that you actually hold the referenced revisions, else no transformation will be done.
     * 
     * @warning It is much less error-prone to use RevisionReference->transformToRevision() and RevisionReference->transformFromRevision() directly.
     * */
    SimpleRange transformBetweenRevisions(SimpleRange range, qint64 fromRevision, qint64 toRevision = -1) const;
    SimpleCursor transformBetweenRevisions(SimpleCursor cursor, qint64 fromRevision, qint64 toRevision = -1, KTextEditor::MovingCursor::InsertBehavior behavior = KTextEditor::MovingCursor::StayOnInsert) const;
    
protected:
    QString m_textAtLastReset;
    RevisionReference m_revisionAtLastReset;
    bool m_needUpdate;
    QString m_currentCleanedInsertion;
    KTextEditor::Cursor m_lastInsertionPosition;
    KTextEditor::MovingRange* m_changedRange;
    
    KTextEditor::Document* m_document;
    KTextEditor::MovingInterface* m_moving;
    
    void updateChangedRange(KTextEditor::Range changed);
public slots:
    virtual void textInserted( KTextEditor::Document*,KTextEditor::Range );
    virtual void textRemoved( KTextEditor::Document* document, KTextEditor::Range range );
    virtual void textChanged( KTextEditor::Document* document, KTextEditor::Range oldRange, KTextEditor::Range newRange );
    void documentDestroyed( QObject* );
    void aboutToInvalidateMovingInterfaceContent ( KTextEditor::Document* document );
private:
    
    friend class RevisionLockerAndClearerPrivate;
    void lockRevision(qint64 revision);
    void unlockRevision(qint64 revision);
    
    QMap<qint64, int> m_revisionLocks;
};

}
#endif
