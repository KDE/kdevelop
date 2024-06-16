/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "persistentmovingrangeprivate.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <backgroundparser/backgroundparser.h>

#include <KTextEditor/Document>

void KDevelop::PersistentMovingRangePrivate::connectTracker()
{
    Q_ASSERT(m_tracker == nullptr);
    Q_ASSERT(m_movingRange == nullptr);

    m_tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(m_document);

    if (m_tracker) {
        // Create a moving range
        auto* const document = m_tracker->document();
        m_movingRange = document->newMovingRange(m_range);
        if (m_shouldExpand)
            m_movingRange->setInsertBehaviors(
                KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
        connect(document, &KTextEditor::Document::aboutToDeleteMovingInterfaceContent, this,
                &PersistentMovingRangePrivate::aboutToDeleteMovingInterfaceContent);
        connect(document, &KTextEditor::Document::aboutToInvalidateMovingInterfaceContent, this,
                &PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent);
        m_movingRange->setAttribute(m_attribte);
        m_movingRange->setZDepth(m_zDepth);
    }
}

void KDevelop::PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent()
{
    if (m_movingRange) {
        m_valid = false; /// @todo More precise tracking: Why is the document being invalidated? Try
        ///            keeping the range alive. DocumentChangeTracker to the rescue.
        delete m_movingRange;
        m_movingRange = nullptr;
        m_range = KTextEditor::Range::invalid();
    }
}

void KDevelop::PersistentMovingRangePrivate::aboutToDeleteMovingInterfaceContent()
{
    // The whole document is being closed. Map the range back to the last saved revision, and use that.
    updateRangeFromMoving();
    if (m_tracker && m_tracker->diskRevision()) {
        if (m_movingRange)
            m_range = m_tracker->diskRevision()->transformFromCurrentRevision(m_range).castToSimpleRange();
    } else {
        m_valid = false;
        m_range = KTextEditor::Range::invalid();
    }

    // No need to disconnect, as the document is being deleted. Simply set the references to zero.
    delete m_movingRange;
    m_movingRange = nullptr;
    m_tracker.clear();
}

#include "moc_persistentmovingrangeprivate.cpp"
