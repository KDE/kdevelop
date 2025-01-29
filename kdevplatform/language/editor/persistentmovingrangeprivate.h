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

namespace KDevelop {
class PersistentMovingRangePrivate
    : public QObject
{
    Q_OBJECT

public:
    PersistentMovingRangePrivate() { moveToThread(QCoreApplication::instance()->thread()); }

    void connectTracker();

    bool m_valid = false;
    bool m_shouldExpand = false;
    KTextEditor::Range m_range;
    IndexedString m_document;
    KTextEditor::Attribute::Ptr m_attribte;
    KTextEditor::MovingRange* m_movingRange = nullptr;
    float m_zDepth = 0;

    void updateRangeFromMoving()
    {
        if (m_movingRange) {
            m_range = m_movingRange->toRange();
        }
    }

private Q_SLOTS:
    void aboutToDeleteMovingInterfaceContent();
    void aboutToInvalidateMovingInterfaceContent();
};
}

#endif // KDEVPLATFORM_PERSISTENTMOVINGRANGEPRIVATE_H
