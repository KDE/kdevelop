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

namespace KTextEditor
{
class Document;
class MovingRange;
class MovingInterface;
}

namespace KDevelop
{

/**
 * This object belongs to the foreground, and thus can only be accessed from background threads if the foreground lock is held.
 * */

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
     * Returns the document revision at which reset() was called last
     * */
    qint64 startRevision() const;

    /**
     * Returns the document text at the start revision
     * */
    QString startRevisionText() const;
    
    /**
     * Returns the current revision
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
    
protected:
    QString m_textAtStartRevision;
    qint64 m_startRevision;
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
};

}
#endif
