/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "persistentmovingrangeprivate.h"
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <backgroundparser/backgroundparser.h>
#include <backgroundparser/documentchangetracker.h>

#include <KTextEditor/Document>

void KDevelop::PersistentMovingRangePrivate::connectTracker(bool shouldExpand)
{
    Q_ASSERT(m_movingRange == nullptr);

    auto* const tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(m_document);
    if (!tracker) {
        return;
    }

    // Create a moving range
    auto* const document = tracker->document();
    m_movingRange.reset(document->newMovingRange(m_range));
    if (shouldExpand) {
        m_movingRange->setInsertBehaviors(KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
    }

    // TODO: map the range back to the last saved revision, and use that when the document is closed, e.g.:
    //       updateRangeFromMoving();
    //       m_range = m_tracker->diskRevision()->transformFromCurrentRevision(m_range).castToSimpleRange();
    connect(document, &KTextEditor::Document::aboutToInvalidateMovingInterfaceContent, this,
            &PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent);
}

void KDevelop::PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent()
{
    // TODO: more precise tracking. Why is the document being invalidated?
    //       Try keeping the range alive. DocumentChangeTracker to the rescue.
    disconnect(m_movingRange->document(), nullptr, this, nullptr);
    m_movingRange.reset();
    m_range = KTextEditor::Range::invalid();
}

#include "moc_persistentmovingrangeprivate.cpp"
