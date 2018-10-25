/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */

#ifndef KDEVPLATFORM_PERSISTENTMOVINGRANGEPRIVATE_H
#define KDEVPLATFORM_PERSISTENTMOVINGRANGEPRIVATE_H

#include <backgroundparser/documentchangetracker.h>
#include <KTextEditor/MovingRange>
#include <QObject>
#include <QApplication>

namespace KDevelop {
class PersistentMovingRangePrivate
    : public QObject
{
    Q_OBJECT

public:
    PersistentMovingRangePrivate()
    {
        moveToThread(QApplication::instance()->thread());
    }

    void connectTracker();

    void disconnectTracker();

    bool m_valid = false;
    bool m_shouldExpand = false;
    KTextEditor::Range m_range;
    IndexedString m_document;
    KTextEditor::Attribute::Ptr m_attribte;
    KTextEditor::MovingRange* m_movingRange = nullptr;
    QPointer<DocumentChangeTracker> m_tracker;
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
