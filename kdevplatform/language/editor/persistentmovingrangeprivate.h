/*
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PERSISTENTMOVINGRANGEPRIVATE_H
#define KDEVPLATFORM_PERSISTENTMOVINGRANGEPRIVATE_H

#include <serialization/indexedstring.h>

#include <KTextEditor/MovingRange>
#include <QObject>
#include <QCoreApplication>

#include <memory>

namespace KDevelop {
class PersistentMovingRangePrivate
    : public QObject
{
    Q_OBJECT

public:
    PersistentMovingRangePrivate() { moveToThread(QCoreApplication::instance()->thread()); }

    void connectTracker(bool shouldExpand);

    /**
     * The last-retrieved document range.
     *
     * @note This data member is almost redundant. Only one scenario prevents replacing it with a
     *       Range parameter to connectTracker(): if during the construction of a PersistentMovingRange
     *       BackgroundParser::trackerForUrl(m_document) returns nullptr, the PersistentMovingRange object is
     *       invalid from the start, m_range remains equal to the range argument of PersistentMovingRange()
     *       forever and is returned from PersistentMovingRange::range() instead of an invalid range.
     * @todo Does any code rely on the original range being returned in this rare scenario? Should we support it?
     */
    KTextEditor::Range m_range;
    /**
     * The URL of this range's document.
     *
     * @note This data member is almost redundant. Only a single use of PersistentMovingRange::document()
     *       in RenameAssistant prevents replacing it with an IndexedString parameter to connectTracker().
     * @todo Consider removing this data member and keeping track of the URL in RenameAssistant instead,
     *       e.g. by adding an IndexedString m_newDeclarationUrl data member to RenameAssistantPrivate.
     */
    IndexedString m_document;
    /**
     * The MovingRange that handles most implementation details.
     *
     * Currently, PersistentMovingRange is a mere thin wrapper around the MovingRange.
     * If the MovingRange is null, the PersistentMovingRange is invalid.
     */
    std::unique_ptr<KTextEditor::MovingRange> m_movingRange;

    void updateRangeFromMoving()
    {
        if (m_movingRange) {
            m_range = m_movingRange->toRange();
        }
    }

private Q_SLOTS:
    void aboutToInvalidateMovingInterfaceContent();
};
}

#endif // KDEVPLATFORM_PERSISTENTMOVINGRANGEPRIVATE_H
