/* This file is part of KDevelop
 *
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "test_vcsevent.h"

#include <QTest>
#include <QStandardPaths>

#include <vcs/vcsrevision.h>

using namespace KDevelop;

void TestVcsEvent::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestVcsEvent::setEvent(VcsEvent& event,
                            const VcsRevision& revision,
                            const QString& author,
                            const QDateTime& date,
                            const QString& message,
                            const QList<VcsItemEvent>& items)
{
    event.setRevision(revision);
    event.setAuthor(author);
    event.setDate(date);
    event.setMessage(message);
    event.setItems(items);
}

void TestVcsEvent::compareEvent(const VcsEvent& event,
                                const VcsRevision& revision,
                                const QString& author,
                                const QDateTime& date,
                                const QString& message,
                                const QList<VcsItemEvent>& items)
{
    QCOMPARE(event.revision(), revision);
    QCOMPARE(event.author(), author);
    QCOMPARE(event.date(), date);
    QCOMPARE(event.message(), message);
    QCOMPARE(event.items().count(), items.count());
}

void TestVcsEvent::testCopyConstructor()
{
    // test plain copy
    VcsRevision revision;
    revision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const QString author("author");
    const QDateTime date = QDateTime::fromString("2001-01-01T00:00:00+00:00", Qt::ISODate);
    const QString message("message");
    const QList<VcsItemEvent> items({{}});

    {
        VcsEvent eventA;
        setEvent(eventA,
                 revision, author, date, message, items);

        VcsEvent eventB(eventA);

        compareEvent(eventA,
                     revision, author, date, message, items);
        compareEvent(eventB,
                     revision, author, date, message, items);
    }

    VcsRevision revisionNew;
    revisionNew.setRevisionValue(2, VcsRevision::FileNumber);

    // test detach after changing A
    {
        VcsEvent eventA;
        setEvent(eventA,
                 revision, author, date, message, items);

        VcsEvent eventB(eventA);
        // change a property of A
        eventA.setRevision(revisionNew);

        compareEvent(eventA,
                     revisionNew, author, date, message, items);
        compareEvent(eventB,
                     revision,    author, date, message, items);
    }
}

void TestVcsEvent::testAssignOperator()
{
    // test plain copy
    VcsRevision revision;
    revision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const QString author("author");
    const QDateTime date = QDateTime::fromString("2001-01-01T00:00:00+00:00", Qt::ISODate);
    const QString message("message");
    const QList<VcsItemEvent> items({{}});

    {
        VcsEvent eventA;
        setEvent(eventA,
                 revision, author, date, message, items);

        VcsEvent eventB;
        eventB = eventA;

        compareEvent(eventA,
                     revision, author, date, message, items);
        compareEvent(eventB,
                     revision, author, date, message, items);
    }

    VcsRevision revisionNew;
    revisionNew.setRevisionValue(2, VcsRevision::FileNumber);

    // test detach after changing A
    {
        VcsEvent eventA;
        setEvent(eventA,
                 revision, author, date, message, items);

        VcsEvent eventB;
        eventB = eventA;
        // change a property of A
        eventA.setRevision(revisionNew);

        compareEvent(eventA,
                     revisionNew, author, date, message, items);
        compareEvent(eventB,
                     revision,    author, date, message, items);
    }
}

QTEST_GUILESS_MAIN(TestVcsEvent)
