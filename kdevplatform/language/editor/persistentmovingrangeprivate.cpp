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

void KDevelop::PersistentMovingRangePrivate::connectTracker()
{
    Q_ASSERT(!m_valid);
    Q_ASSERT(m_movingRange == nullptr);

    auto* const tracker = ICore::self()->languageController()->backgroundParser()->trackerForUrl(m_document);
    if (!tracker) {
        return;
    }

    m_valid = true;

    // Create a moving range
    auto* const document = tracker->document();
    m_movingRange = document->newMovingRange(m_range);
    if (m_shouldExpand)
        m_movingRange->setInsertBehaviors(KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);

    // TODO: map the range back to the last saved revision, and use that when the document is closed, e.g.:
    //       updateRangeFromMoving();
    //       m_range = m_tracker->diskRevision()->transformFromCurrentRevision(m_range).castToSimpleRange();
#if KTEXTEDITOR_VERSION < QT_VERSION_CHECK(6, 9, 0)
    // Since https://commits.kde.org/ktexteditor/6ca19934786fb808ab2b307d558967a74f87e4f4
    // first included in KTextEditor version 6.9, KTextEditor::Document emits the signal
    // aboutToInvalidateMovingInterfaceContent() instead of aboutToDeleteMovingInterfaceContent()
    // from the destructor.
    connect(document, &KTextEditor::Document::aboutToDeleteMovingInterfaceContent, this,
            &PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent);
#endif
    connect(document, &KTextEditor::Document::aboutToInvalidateMovingInterfaceContent, this,
            &PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent);
    m_movingRange->setAttribute(m_attribte);
    m_movingRange->setZDepth(m_zDepth);
}

void KDevelop::PersistentMovingRangePrivate::aboutToInvalidateMovingInterfaceContent()
{
    m_valid = false; /// @todo More precise tracking: Why is the document being invalidated? Try
    ///            keeping the range alive. DocumentChangeTracker to the rescue.
    disconnect(m_movingRange->document(), nullptr, this, nullptr);
    delete m_movingRange;
    m_movingRange = nullptr;
    m_range = KTextEditor::Range::invalid();
}

#include "moc_persistentmovingrangeprivate.cpp"
