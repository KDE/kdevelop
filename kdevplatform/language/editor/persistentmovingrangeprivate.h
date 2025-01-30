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

    KTextEditor::Range m_range;
    IndexedString m_document;
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
