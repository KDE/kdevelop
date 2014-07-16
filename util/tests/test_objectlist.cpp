/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "test_objectlist.h"

#include "objectlist.h"

#include <QtTest>

QTEST_MAIN(TestObjectList);

using namespace KDevelop;

void TestObjectList::testBasicInterface()
{
    ObjectList<QObject> list;

    QObject o;
    list.append(&o);
    QCOMPARE(list.data().size(), 1);
    QCOMPARE(list.data().at(0), &o);
    QCOMPARE(list.data().value(0), &o);

    QCOMPARE(list.data().size(), 1);
    QCOMPARE(list.data().at(0), &o);

    QVERIFY(list.remove(&o));
    QCOMPARE(list.data().size(), 0);
    //QCOMPARE(list.at(0)), &o); // would crash
    QVERIFY(!list.data().value(0));

    QVERIFY(list.data().isEmpty());

    // try removing it again
    QVERIFY(!list.remove(&o));
}

void TestObjectList::testDeleteAll()
{
    ObjectList<QObject> list;

    QPointer<QObject> p(new QObject);
    list.append(p.data());
    list.deleteAll();
    QVERIFY(!p);
}

void TestObjectList::testBehaviorOnDestruction()
{
    QPointer<QObject> p(new QObject);

    {
        ObjectList<QObject> list;
        list.append(p.data());
        // nothing is supposed to happen on destruction
    }
    QVERIFY(p);

    {
        ObjectList<QObject> list(ObjectListTracker::CleanupWhenDone);
        list.append(p.data());
    }
    QVERIFY(!p);
}

#include "test_objectlist.moc"
