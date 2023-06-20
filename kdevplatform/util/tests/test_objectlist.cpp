/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_objectlist.h"

#include "objectlist.h"

#include <QTest>
#include <QStandardPaths>

QTEST_MAIN(TestObjectList)

using namespace KDevelop;

void TestObjectList::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

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

#include "moc_test_objectlist.cpp"
