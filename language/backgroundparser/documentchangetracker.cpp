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

using namespace KTextEditor;

/**
 * @todo Track the exact changes to the document, and then:
 * Dont reparse if:
 *  - Comment added/changed
 *  - Newlines added/changed (ready)
 * Complete the document for validation:
 *  - Incomplete for-loops
 *  - ...
 * Incremental parsing:
 *  - All changes within a local function (or function parameter context): Update only the context (and all its importers)
 * */

namespace {
    QRegExp whiteSpaceRegExp("\\s");
};

namespace KDevelop
{

DocumentChangeTracker::DocumentChangeTracker( KTextEditor::Document* document )
    : m_needUpdate(false), m_changedRange(0)
{
    connect(document, SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)), SLOT(textInserted(KTextEditor::Document*,KTextEditor::Range)));
    connect(document, SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range)), SLOT(textRemoved(KTextEditor::Document*,KTextEditor::Range)));
    connect(document, SIGNAL(textChanged(KTextEditor::Document*,KTextEditor::Range,KTextEditor::Range)), SLOT(textChanged(KTextEditor::Document*,KTextEditor::Range,KTextEditor::Range)));
    connect(document, SIGNAL(destroyed(QObject*)), SLOT(documentDestroyed(QObject*)));
    
    m_moving = dynamic_cast<KTextEditor::MovingInterface*>(document);
    Q_ASSERT(m_moving);
    m_changedRange = m_moving->newMovingRange(KTextEditor::Range(), KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
    
    connect(m_document, SIGNAL(aboutToInvalidateMovingInterfaceContent (KTextEditor::Document*)), this, SLOT(aboutToInvalidateMovingInterfaceContent (KTextEditor::Document*)));
    
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
    
    m_startRevision = m_moving->revision();
    m_textAtStartRevision = m_document->text();
}

qint64 DocumentChangeTracker::currentRevision() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_moving->revision();
}

qint64 DocumentChangeTracker::startRevision() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_startRevision;
}

QString DocumentChangeTracker::startRevisionText() const
{
    VERIFY_FOREGROUND_LOCKED
    
    return m_textAtStartRevision;
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
    
    ModificationRevisionSet::clearCache();
    KDevelop::ModificationRevision::clearModificationCache(IndexedString(m_document->url()));
    
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
}

Document* DocumentChangeTracker::document() const
{
    return m_document;
}

void DocumentChangeTracker::aboutToInvalidateMovingInterfaceContent ( Document* )
{
}

}
